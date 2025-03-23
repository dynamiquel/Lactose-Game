#pragma once

#include "LactoseMqttSubsystem.generated.h"

struct FMqttifyUnsubscribeResult;
struct FMqttifySubscribeResult;
struct FMqttifyMessage;
class IMqttifyClient;

UCLASS()
class ULactoseMqttSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	TSharedPtr<IMqttifyClient> GetMqttClient() const { return MqttClient; }
	
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	void OnConnected(bool bConnected);
	void OnDisconnected(bool bDisconnected);
	void OnMessageReceived(const FMqttifyMessage& Message);
	void OnPublished(bool bPublished);
	void OnSubscribed(const TSharedPtr<TArray<FMqttifySubscribeResult>>& Subscriptions);
	void OnUnsubscribed(const TSharedPtr<TArray<FMqttifyUnsubscribeResult>>& Unsubscriptions);

	TSharedPtr<IMqttifyClient> MqttClient;
};
