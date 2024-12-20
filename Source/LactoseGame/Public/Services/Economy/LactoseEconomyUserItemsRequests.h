#pragma once

#include "Rest/LactoseRestRequest.h"

#include "LactoseEconomyUserItemsRequests.generated.h"

USTRUCT()
struct FLactoseEconomyGetUserItemsRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId;
};

USTRUCT(BlueprintType)
struct FLactoseEconomyUserItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ItemId;

	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 0;
};


USTRUCT()
struct FLactoseEconomyGetUserItemsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLactoseEconomyUserItem> Items;
};

using FGetEconomyUserItemsRequest = Lactose::Rest::TRequest<FLactoseEconomyGetUserItemsRequest, FLactoseEconomyGetUserItemsResponse>;