#include "Mqtt/LactoseMqttSubsystem.h"
#include "IMqttifyModule.h"
#include "Mqtt/LactoseMqttLog.h"
#include "Mqtt/Interface/IMqttifyClient.h"
#include "Simp.h"
#include "Rest/LactoseRestSubsystem.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

FLactoseMqttifyCredentialsProvider::FLactoseMqttifyCredentialsProvider(class ULactoseMqttSubsystem& InMqttSubsystem)
	: MqttSubsystem(&InMqttSubsystem)
{ }

FMqttifyCredentials FLactoseMqttifyCredentialsProvider::GetCredentials()
{
	ULactoseMqttSubsystem* Subsystem = MqttSubsystem.Get();
	if (!Subsystem)
		return FMqttifyCredentials();

	TOptional<FString> AccessToken = Subsystems::GetRef<ULactoseRestSubsystem>(Subsystem).GetAccessToken();
	if (!AccessToken.IsSet())
		return FMqttifyCredentials();

	return FMqttifyCredentials(AccessToken.GetValue(), TEXT("69"));
}

void ULactoseMqttSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IMqttifyModule& MqttifyModule = IMqttifyModule::Get();
	
	MqttClient = MqttifyModule.GetOrCreateClient(FString(TEXT("wss://lactose.mookrata.ovh/mqtt")));
	if (!MqttClient)
	{
		UE_LOG(LogLactoseMqtt, Error, TEXT("Could not create MQTT client"));
		return;
	}
	
	MqttClient->OnConnect().AddUObject(this, &ThisClass::OnConnected);
	MqttClient->OnDisconnect().AddUObject(this, &ThisClass::OnDisconnected);
	MqttClient->OnMessage().AddUObject(this, &ThisClass::OnMessageReceived);
	MqttClient->OnPublish().AddUObject(this, &ThisClass::OnPublished);
	MqttClient->OnSubscribe().AddUObject(this, &ThisClass::OnSubscribed);
	MqttClient->OnUnsubscribe().AddUObject(this, &ThisClass::OnUnsubscribed);

	MqttClient->GetConnectionSettings()->InitialRetryConnectionIntervalSeconds = 1;
	MqttClient->GetConnectionSettings()->KeepAliveIntervalSeconds = 10;
	MqttClient->GetConnectionSettings()->CredentialsProvider = MakeShared<FLactoseMqttifyCredentialsProvider>(self);
	
	UE_LOG(LogLactoseMqtt, Log, TEXT("Waiting for user log in before connecting to MQTT"));

	Lactose::Identity::Events::OnUserLoggedIn.AddLambda([WeakThis = MakeWeakObjectPtr(this)]
		(const ULactoseIdentityServiceSubsystem& Sender, const Sr<FLactoseIdentityGetUserResponse>& User)
	{
		auto* This = WeakThis.Get();
		if (!This)
			return;

		UE_LOG(LogLactoseMqtt, Log, TEXT("Connecting to MQTT"));
		This->MqttClient->ConnectAsync(false);
	});
}

void ULactoseMqttSubsystem::Deinitialize()
{
	if (MqttClient.IsValid() && MqttClient->IsConnected())
	{
		auto& IdentitySubsystem = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);
		const FString UserId = IdentitySubsystem.GetLoggedInUserInfo() ? IdentitySubsystem.GetLoggedInUserInfo()->Id : TEXT("unknown");

		MqttClient->PublishAsync({
			.Topic = FString::Printf(TEXT("players/%s/diconnected"), *UserId)
		});

		// Little delay to ensure everything gets sent before closing the game.
		MqttClient->DisconnectAsync().WaitFor(FTimespan::FromMilliseconds(300));
	}
	
	Super::Deinitialize();
}

void ULactoseMqttSubsystem::OnConnected(bool bConnected)
{
	if (!bConnected)
	{
		UE_LOG(LogLactoseMqtt, Log, TEXT("Failed to connect"));
		return;
	}
	
	self.bConnected = true;
	
	UE_LOG(LogLactoseMqtt, Log, TEXT("Connected"));

	auto& IdentitySubsystem = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);

	const FString UserId = IdentitySubsystem.GetLoggedInUserInfo() ? IdentitySubsystem.GetLoggedInUserInfo()->Id : TEXT("unknown");
	MqttClient->PublishAsync({
		.Topic = FString::Printf(TEXT("players/%s/connected"), *UserId)
	});

	TArray<FMqttifyTopicFilter> TopicFilters;
	TopicFilters.Reserve(RoutedSubscriptions.Num());

	// Subscriptions don't seem to persist with this client so resubscribe.
	for (TPair<FString, TArray<FMqttDelegate>>& RoutedSubscription : RoutedSubscriptions)
	{
		FMqttifyTopicFilter TopicFilter(RoutedSubscription.Key);
		TopicFilters.Emplace(FMqttifyTopicFilter(RoutedSubscription.Key));
		UE_LOG(LogLactoseMqtt, Log, TEXT("Subscribing to topic '%s'"), *TopicFilter.GetFilter());
	}
	
	MqttClient->SubscribeAsync(TopicFilters).Next([WeakThis = MakeWeakObjectPtr(this)]
		(const TMqttifyResult<TArray<FMqttifySubscribeResult>>& Result)
	{
		if (!Result.HasSucceeded())
		{
			UE_LOG(LogLactoseMqtt, Error, TEXT("Failed to subscribe to topics"));
		}
		else if (Result.GetResult().IsValid() && !Result.GetResult()->IsEmpty())
		{
			for (const FMqttifySubscribeResult& Topic : *Result.GetResult())
			{
				if (!Topic.WasSuccessful())
				{
					UE_LOG(LogLactoseMqtt, Error, TEXT("Failed to subscribe to topic '%s'"), *Topic.GetFilter().GetFilter());
				}
				else
				{
					UE_LOG(LogLactoseMqtt, Log, TEXT("Subscribed to topic '%s'"), *Topic.GetFilter().GetFilter());
				}
			}
		}
		else
		{
			UE_LOG(LogLactoseMqtt, Error, TEXT("Subscribe suceeded but no results were returned?"));
		}
	});
}

void ULactoseMqttSubsystem::OnDisconnected(bool bDisconnected)
{
	UE_LOG(LogLactoseMqtt, Log, TEXT("Disconnected"));
	bConnected = false;
}

void ULactoseMqttSubsystem::OnMessageReceived(const FMqttifyMessage& Message)
{
	for (const TPair<FString, TArray<FMqttDelegate>>& RoutedSubscription : RoutedSubscriptions)
	{
		UE_LOG(LogLactoseMqtt, Verbose, TEXT("Received Message with Topic: '%s'"), *Message.Topic);
		
		FMqttifyTopicFilter TopicFilter(RoutedSubscription.Key);
		if (!TopicFilter.MatchesWildcard(Message.Topic))
		{
			UE_LOG(LogLactoseMqtt, VeryVerbose, TEXT("Subscription '%s' does not match Topic '%s'"), *RoutedSubscription.Key, *Message.Topic);
			continue;
		}

		UE_LOG(LogLactoseMqtt, Verbose, TEXT("Found Subscription '%s' for Topic '%s'"), *RoutedSubscription.Key, *Message.Topic);
		
		for (const FMqttDelegate& Delegate : RoutedSubscription.Value)
			Delegate.ExecuteIfBound(Message);
	}
}

void ULactoseMqttSubsystem::OnPublished(bool bPublished)
{
}

void ULactoseMqttSubsystem::OnSubscribed(const TSharedPtr<TArray<FMqttifySubscribeResult>>& Subscriptions)
{
	if (!Subscriptions)
		return;
	
	for (const FMqttifySubscribeResult& Subscription : *Subscriptions)
		UE_LOG(LogLactoseMqtt, Log, TEXT("Subscribed to topic '%s'"), *Subscription.GetFilter().GetFilter());
}

void ULactoseMqttSubsystem::OnUnsubscribed(const TSharedPtr<TArray<FMqttifyUnsubscribeResult>>& Unsubscriptions)
{
	if (!Unsubscriptions)
		return;
	
	for (const FMqttifyUnsubscribeResult& Unsubscription : *Unsubscriptions)
		UE_LOG(LogLactoseMqtt, Log, TEXT("Ubsubscribed from topic '%s'"), *Unsubscription.GetFilter().GetFilter());
}

FRoutedSubscriptionHandle ULactoseMqttSubsystem::RouteSubscription(const FString& Topic, FMqttDelegate&& Delegate)
{
	TArray<FMqttDelegate>* ExistingTopicRoutes = RoutedSubscriptions.Find(Topic);
	if (!ExistingTopicRoutes)
	{
		// First time we are routing this topic; subscribe to the topic.
		ExistingTopicRoutes = &RoutedSubscriptions.Add(Topic);

		if (MqttClient.IsValid() && bConnected)
		{
			MqttClient->SubscribeAsync(FMqttifyTopicFilter(Topic)).Next([Topic, WeakThis = MakeWeakObjectPtr(this)]
				(const TMqttifyResult<FMqttifySubscribeResult>& Result)
			{
				if (!Result.HasSucceeded())
					UE_LOG(LogLactoseMqtt, Error, TEXT("Failed to subscribe to topic '%s'"), *Topic);
			});
		}
	}
	
	ExistingTopicRoutes->Add(MoveTemp(Delegate));
	return FRoutedSubscriptionHandle(Topic, ExistingTopicRoutes->Num() - 1);
}

void ULactoseMqttSubsystem::UnrouteSubscription(const FRoutedSubscriptionHandle& Handle)
{
	if (TArray<FMqttDelegate>* TopicRoutes = RoutedSubscriptions.Find(Handle.Key))
		if (TopicRoutes->IsValidIndex(Handle.Value))
			(*TopicRoutes)[Handle.Value].Unbind();
}

void ULactoseMqttSubsystem::UnrouteAllSubscriptions(const FString& Topic)
{
	if (TArray<FMqttDelegate>* TopicRoutes = RoutedSubscriptions.Find(Topic))
		for (FMqttDelegate& TopicRoute : *TopicRoutes)
			TopicRoute.Unbind();
}
