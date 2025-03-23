#include "LactoseMqttSubsystem.h"
#include "Simp.h"
#include "IMqttifyModule.h"
#include "LactoseMqttLog.h"
#include "Mqtt/Interface/IMqttifyClient.h"

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
	
	MqttClient->ConnectAsync(false);
}

void ULactoseMqttSubsystem::Deinitialize()
{
	if (MqttClient.IsValid() && MqttClient->IsConnected())
	{
		MqttClient->PublishAsync({
			.Topic = TEXT("players/unknown/disconnected")
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
	
	UE_LOG(LogLactoseMqtt, Log, TEXT("Connected"));
	
	MqttClient->PublishAsync({
		.Topic = TEXT("players/unknown/connected")
	});
}

void ULactoseMqttSubsystem::OnDisconnected(bool bDisconnected)
{
	UE_LOG(LogLactoseMqtt, Log, TEXT("Disconnected"));
}

void ULactoseMqttSubsystem::OnMessageReceived(const FMqttifyMessage& Message)
{
}

void ULactoseMqttSubsystem::OnPublished(bool bPublished)
{
}

void ULactoseMqttSubsystem::OnSubscribed(const TSharedPtr<TArray<FMqttifySubscribeResult>>& Subscriptions)
{
}

void ULactoseMqttSubsystem::OnUnsubscribed(const TSharedPtr<TArray<FMqttifyUnsubscribeResult>>& Unsubscriptions)
{
}
