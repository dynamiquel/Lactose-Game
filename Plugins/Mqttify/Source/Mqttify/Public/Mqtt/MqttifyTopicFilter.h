#pragma once

#include "CoreMinimal.h"
#include "Mqtt/MqttifyQualityOfService.h"
#include "Mqtt/MqttifyRetainHandlingOptions.h"
#include "MqttifyTopicFilter.generated.h"

/**
 * @brief Represents an MQTT topic for publish/subscribe operations.
 * https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901241
 * http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718106
 */
USTRUCT(BlueprintType)
struct MQTTIFY_API FMqttifyTopicFilter final
{
	GENERATED_BODY()

	bool operator==(const FMqttifyTopicFilter& Other) const
	{
		return Filter == Other.Filter &&
			InQualityOfService == Other.InQualityOfService &&
			bNoLocal == Other.bNoLocal &&
			bRetainAsPublished == Other.bRetainAsPublished &&
			RetainHandlingOptions == Other.RetainHandlingOptions;
	}

	bool operator==(const FString& Other) const
	{
		return Filter == Other;
	}

	bool operator!=(const FMqttifyTopicFilter& Other) const
	{
		return !(*this == Other);
	}

	bool operator!=(const FString& Other) const
	{
		return !(*this == Other);
	}

	// TODO: Not sure if we will need this for our client but if we
	// create a server it will 100% need this.
	/**
	 * @brief Matches a given topic with the stored filter, considering MQTT-style wildcards (+ and #).
	 * @param InTopic - topic to check against the filter.
	 * @return True on successful match, false otherwise.
	*/
	bool MatchesWildcard(const FString& InTopic) const
	{
	    if (Filter == TEXT("#"))
	    {
	        return true;
	    }

	    const int32 FilterLen = Filter.Len();
	    const int32 TopicLen = InTopic.Len();
	    int32 FilterPos = 0;
	    int32 TopicPos = 0;

	    // Handle leading '/' for absolute paths
	    const bool bFilterStartsWithSlash = FilterLen > 0 && Filter[0] == TEXT('/');
	    const bool bTopicStartsWithSlash = TopicLen > 0 && InTopic[0] == TEXT('/');
	    if (bFilterStartsWithSlash != bTopicStartsWithSlash)
	        return false;

	    // If both start with '/', effectively skip them as they represent an initial empty level.
	    if (bFilterStartsWithSlash)
	    {
	        FilterPos = 1;
	        TopicPos = 1;

	        // Special case: Filter is exactly "/" and Topic is exactly "/"
	        if (FilterLen == 1 && TopicLen == 1)
        		return true;
    		
	        // Special case: Filter/Topic is "/" but Topic/Filter is longer (e.g., "/a") -> No match
	        if (FilterLen == 1 || TopicLen == 1)
        		return false;
	    }

	    while (FilterPos < FilterLen && TopicPos < TopicLen)
	    {
	        const TCHAR FilterChar = Filter[FilterPos];
	        const TCHAR TopicChar = InTopic[TopicPos];

	        if (FilterChar == TEXT('+'))
	        {
	            // '+' matches exactly one level.
	            // Consume characters in the topic until the next '/' or end of topic.
	            while (TopicPos < TopicLen && InTopic[TopicPos] != TEXT('/'))
	                ++TopicPos;
        		
	            // Advance filter position past the '+'.
	            ++FilterPos;
	        }
	        else if (FilterChar == TEXT('#'))
	        {
	            // '#' matches zero or more levels.
	            // It must be the last character in the filter.
	            if (FilterPos != FilterLen - 1)
	            {
	                UE_LOG(LogTemp, Warning, TEXT("MQTT Topic Filter Error: Multi-level wildcard '#' is not the last character in filter '%s'. Invalid filter syntax."), *Filter);
	                return false;
	            }
        		
	            // If '#' is the last character, it matches all remaining topic levels (including zero).
	            return true;
	        }
	        else
	        {
	            if (FilterChar != TopicChar)
	                return false;
        		
	            ++FilterPos;
	            ++TopicPos;
	        }
	    }
		
	    if (FilterPos == FilterLen && TopicPos == TopicLen)
	        return true;

	    // Only the filter has remaining characters.
	    // This is only a match if the remaining filter characters are all '/'s.
	    // This handles cases like filter "a/b/" matching topic "a/b" (where "a/b/" implies an empty last level).
	    if (FilterPos < FilterLen)
	    {
	        while (FilterPos < FilterLen)
	        {
	            if (Filter[FilterPos] != TEXT('/'))
	                return false;
        		
	            ++FilterPos;
	        }
    		
	        return true;
	    }

	    // Only the topic has remaining characters.
	    // This means the filter was not specific enough to cover all topic levels.
	    // (e.g., filter "a/b", topic "a/b/c").
	    if (TopicPos < TopicLen)
	        return false;

	    // Fallback: This point should ideally not be reached if all conditions are covered.
	    return false;
	}

private:
	/**
	 * @brief Topic path string, e.g., "home/livingroom/temperature".
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Topic", meta=(AllowPrivateAccess="true"))
	FString Filter;

	/**
	 * @brief Quality of service. Default is AtMostOnce. This gives the maximum QoS level at which the Server can send Application Messages to the Client.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Quality of Service", meta=(AllowPrivateAccess="true"))
	EMqttifyQualityOfService InQualityOfService;

	/**
	 * @brief No Local flag. Default is true. If true, the Server will not forward Application Messages published to its own
	 * This option is only used for MQTT v5.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="No Local", meta=(AllowPrivateAccess="true"))
	bool bNoLocal;

	/**
	 * @brief Retain as Published flag. Default is true. If true, Application Messages forwarded using this Subscription
	 * This option is only used for MQTT v5.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Retain as Published", meta=(AllowPrivateAccess="true"))
	bool bRetainAsPublished;

	/**
	 * @brief Retain Handling Options. Default is SendRetainedMessagesAtSubscribeTime. This option is only used for MQTT v5.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Retain Handling Options", meta=(AllowPrivateAccess="true"))
	EMqttifyRetainHandlingOptions RetainHandlingOptions;

public:
	/// @brief U+002F (/): Separator character for different levels of the topic
	static constexpr TCHAR TopicLevelSeparatorChar = '/';

	/// @brief U+002B (+): Single-level wildcard character, must occupy an entire level of the filter if used
	static constexpr TCHAR SingleLevelWildcardChar = '+';

	/// @brief U+0023 (#): Multi-level wildcard character, must be the last character in the topic filter and either on its own or following a topic level separator
	static constexpr TCHAR MultiLevelWildcardChar = '#';

	/// @brief Not part of the MQTT specification, but widely used by servers for system topics
	static constexpr TCHAR SystemChar = '$';

	/**
	 * @brief Default constructor.
	 */
	FMqttifyTopicFilter();

	/**
	 * @brief Constructor from topic path.
	 * @param InFilter Topic path.
	 * @param InQualityOfService This gives the maximum QoS level at which the Server can send Application Messages to the Client.
	 * @param bInNoLocal If true, the Server will not forward Application Messages published to its own This option is only used for MQTT v5.
	 * @param bInRetainAsPublished If true, Application Messages forwarded using this Subscription This option is only used for MQTT v5.
	 * @param InRetainHandlingOptions This option is only used for MQTT v5.
	 * This option specifies whether retained messages are sent when the subscription is established.
	 * This does not affect the sending of retained messages at any point after the subscribe.
	 * If there are no retained messages matching the Topic Filter, all of these values act the same. The values are:
	 */
	explicit FMqttifyTopicFilter(const FString& InFilter,
								const EMqttifyQualityOfService InQualityOfService =
									EMqttifyQualityOfService::AtMostOnce,
								const bool bInNoLocal                                       = true,
								const bool bInRetainAsPublished                             = true,
								const EMqttifyRetainHandlingOptions InRetainHandlingOptions =
									EMqttifyRetainHandlingOptions::SendRetainedMessagesAtSubscribeTime)
		: Filter(InFilter)
		, InQualityOfService(InQualityOfService)
		, bNoLocal(bInNoLocal)
		, bRetainAsPublished(bInRetainAsPublished)
		, RetainHandlingOptions(InRetainHandlingOptions) {}

	/**
	 * @brief Get topic filter
	 */
	const FString& GetFilter() const { return Filter; }

	/**
	 * @brief Get quality of service
	 */
	EMqttifyQualityOfService GetQualityOfService() const { return InQualityOfService; }

	/**
	 * @brief Get no local flag
	 */
	bool GetIsNoLocal() const { return bNoLocal; }

	/**
	 * @brief Get retain as published flag
	 */
	bool GetIsRetainAsPublished() const { return bRetainAsPublished; }

	/**
	 * @brief Get retain handling options
	 */
	EMqttifyRetainHandlingOptions GetRetainHandlingOptions() const { return RetainHandlingOptions; }

	/**
	 * @brief Destructor.
	 */
	~FMqttifyTopicFilter() = default;

	/**
	 * @brief Validates the topic.
	 * @return True if the topic is valid, false otherwise.
	 */
	bool IsValid() const;

	friend FORCEINLINE uint32 GetTypeHash(const FMqttifyTopicFilter& InTopicFilter)
	{
		uint32 Hash = 0;

		Hash = HashCombine(Hash, GetTypeHash(InTopicFilter.Filter));
		Hash = HashCombine(Hash, GetTypeHash(InTopicFilter.InQualityOfService));
		Hash = HashCombine(Hash, GetTypeHash(InTopicFilter.bNoLocal));
		Hash = HashCombine(Hash, GetTypeHash(InTopicFilter.bRetainAsPublished));
		Hash = HashCombine(Hash, GetTypeHash(InTopicFilter.RetainHandlingOptions));
		return Hash;
	}
};
