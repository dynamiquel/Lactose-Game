#pragma once

#include "Rest/LactoseRestRequest.h"

#include "LactoseEconomyItemsRequests.generated.h"

USTRUCT()
struct FLactoseEconomyQueryItemsRequest
{
	GENERATED_BODY()
	
};

USTRUCT()
struct FLactoseEconomyQueryItemsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> ItemIds;
};

USTRUCT()
struct FLactoseEconomyGetItemsRequest
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> ItemIds;
};

USTRUCT()
struct FLactoseEconomyItem
{
	GENERATED_BODY()

	UPROPERTY()
	FString Id;

	UPROPERTY()
	FString Type;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString Description;
};

USTRUCT()
struct FLactoseEconomyGetItemsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLactoseEconomyItem> Items;
};

using FQueryEconomyItemsRequest = Lactose::Rest::TRequest<FLactoseEconomyQueryItemsRequest, FLactoseEconomyQueryItemsResponse>;
using FGetEconomyItemsRequest = Lactose::Rest::TRequest<FLactoseEconomyGetItemsRequest, FLactoseEconomyGetItemsResponse>;