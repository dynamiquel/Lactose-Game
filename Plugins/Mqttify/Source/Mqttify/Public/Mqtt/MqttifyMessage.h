#pragma once

#include "CoreMinimal.h"
#include "Mqtt/MqttifyProtocolVersion.h"
#include "Mqtt/MqttifyQualityOfService.h"
#include "MqttifyMessage.generated.h"

namespace Mqttify
{
	template <EMqttifyProtocolVersion InProtocolVersion>
	struct TMqttifyPublishPacket;
}

/**
 * @brief Represents an MQTT message.
 *
 * This class encapsulates the data and related operations for an MQTT message.
 * It provides methods to manipulate and access the message content,
 * topic, and other related properties.
 */
USTRUCT(BlueprintType)
struct MQTTIFY_API FMqttifyMessage
{
	GENERATED_BODY()

public:
	/** Packet topic. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MQTT", meta = (AllowPrivateAccess = true))
	FString Topic;

	/** Packet content. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MQTT", meta = (AllowPrivateAccess = true))
	TArray<uint8> Payload;

	/**
	 * @brief Indicates whether the broker should retain the message for new subscribers.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MQTT", meta = (AllowPrivateAccess = true))
	bool bRetain = false;

	/** Quality of Service. */
	UPROPERTY(EditAnywhere,
		BlueprintReadOnly,
		Category = "MQTT",
		meta = (DisplayName = "Quality of Service", AllowPrivateAccess = true))
	EMqttifyQualityOfService QualityOfService = EMqttifyQualityOfService::AtMostOnce;

	/** TimeStamp as UTC. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MQTT", meta = (AllowPrivateAccess = true))
	FDateTime TimeStamp = FDateTime::UtcNow();

	friend struct Mqttify::TMqttifyPublishPacket<EMqttifyProtocolVersion::Mqtt_3_1_1>;
	friend struct Mqttify::TMqttifyPublishPacket<EMqttifyProtocolVersion::Mqtt_5>;
};
