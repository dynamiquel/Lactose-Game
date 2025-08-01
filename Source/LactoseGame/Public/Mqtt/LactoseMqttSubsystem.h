#pragma once

#include "Mqtt/Interface/IMqttifyCredentialsProvider.h"

#include "LactoseMqttSubsystem.generated.h"

struct FMqttifyUnsubscribeResult;
struct FMqttifySubscribeResult;
struct FMqttifyMessage;
class IMqttifyClient;

using FMqttDelegate = TDelegate<void(const FMqttifyMessage&)>;
using FRoutedSubscriptionHandle = TTuple<FString, int32>;

class FLactoseMqttifyCredentialsProvider : public IMqttifyCredentialsProvider
{
public:
	FLactoseMqttifyCredentialsProvider(class ULactoseMqttSubsystem& InMqttSubsystem);
	
	FMqttifyCredentials GetCredentials() override;

private:
	TWeakObjectPtr<ULactoseMqttSubsystem> MqttSubsystem;
};

UCLASS(Config=Services)
class ULactoseMqttSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	ULactoseMqttSubsystem();
	
	TSharedPtr<IMqttifyClient> GetMqttClient() const { return MqttClient; }
	
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	void OnConnected(bool bInConnected);
	void OnDisconnected(bool bInDisconnected);
	void OnMessageReceived(const FMqttifyMessage& Message);
	void OnPublished(bool bInPublished);
	void OnSubscribed(const TSharedPtr<TArray<FMqttifySubscribeResult>>& Subscriptions);
	void OnUnsubscribed(const TSharedPtr<TArray<FMqttifyUnsubscribeResult>>& Unsubscriptions);

	// I came aware that Mqttify already kinda has its own routing mechanism after
	// I already implemented this. I'm not switching over because I don't entirely
	// know how it works internally and don't wanna waste time looking.
	FRoutedSubscriptionHandle RouteSubscription(const FString& Topic, FMqttDelegate&& Delegate);
	void UnrouteSubscription(const FRoutedSubscriptionHandle& Handle);
	void UnrouteAllSubscriptions(const FString& Topic);

protected:
	UPROPERTY(EditDefaultsOnly, Config)
	FString MqttUrl;
	
	TMap<FString, TArray<FMqttDelegate>> RoutedSubscriptions;
	
	TSharedPtr<IMqttifyClient> MqttClient;
	// Using our own connected var since IMqttifyClient::IsConnected crashes for some reason.
	bool bConnected = false;
};
