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

USTRUCT()
struct FLactoseEconomyUserItem
{
	GENERATED_BODY()

	UPROPERTY()
	FString ItemId;

	UPROPERTY()
	int32 Quantity;
};


USTRUCT()
struct FLactoseEconomyGetUserItemsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLactoseEconomyUserItem> Items;
};

using FGetEconomyUserItemsRequest = Lactose::Rest::TRequest<FLactoseEconomyGetUserItemsRequest, FLactoseEconomyGetUserItemsResponse>;