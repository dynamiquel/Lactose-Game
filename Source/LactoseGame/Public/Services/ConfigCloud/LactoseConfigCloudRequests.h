#pragma once

#include "Rest/LactoseRestRequest.h"

#include "LactoseConfigCloudRequests.generated.h"

USTRUCT()
struct FLactoseConfigCloudGetConfigRequest
{
	GENERATED_BODY()
};

USTRUCT()
struct FLactoseConfigCloudGetConfigResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FString, FString> Entries;
};

using FGetConfigRequest = Lactose::Rest::TRequest<FLactoseConfigCloudGetConfigRequest, FLactoseConfigCloudGetConfigResponse>;