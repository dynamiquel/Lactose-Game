#pragma once

#include "LactoseEconomyUserItemsRequests.h"
#include "Rest/LactoseRestRequest.h"

#include "LactoseEconomyShopItemsRequests.generated.h"

USTRUCT()
struct FLactoseEconomyGetUserShopItemsRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId;

	UPROPERTY()
	bool RetrieveUserQuantity = false; // No b prefix since the JSON serialiser will undesirably include it.
};

USTRUCT(BlueprintType)
struct FLactoseEconomyShopItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Id;

	UPROPERTY(BlueprintReadOnly)
	FString UserId;
	
	UPROPERTY(BlueprintReadOnly)
	FString ItemId;

	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadOnly)
	FString TransactionType;

	UPROPERTY(BlueprintReadOnly)
	TArray<FLactoseEconomyUserItem> TransactionItems;
};

USTRUCT()
struct FLactoseEconomyGetUserShopItemsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLactoseEconomyShopItem> ShopItems;
};

USTRUCT()
struct FLactoseEconomyShopItemTradeRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId;

	UPROPERTY()
	FString ShopItemId;
};

USTRUCT()
struct FLactoseEconomyShopItemTradeResponse
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Reason = 0;
};


using FGetEconomyUserShopItemsRequest = Lactose::Rest::TRequest<FLactoseEconomyGetUserShopItemsRequest, FLactoseEconomyGetUserShopItemsResponse>;
using FEconomyShopItemTradeRequest = Lactose::Rest::TRequest<FLactoseEconomyShopItemTradeRequest, FLactoseEconomyShopItemTradeResponse>;