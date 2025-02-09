#pragma once

#include "Services/Economy/LactoseEconomyUserItemsRequests.h"

#include "LactoseSimulationCropsRequests.generated.h"

USTRUCT()
struct FLactoseSimulationQueryCropsRequest
{
	GENERATED_BODY()
};

USTRUCT()
struct FLactoseSimulationQueryCropsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> CropIds;
};

USTRUCT()
struct FLactoseSimulationGetCropsRequest
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FString> CropIds;
};

USTRUCT(BlueprintType)
struct FLactoseSimulationCrop
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Id;

	UPROPERTY(BlueprintReadOnly)
	FString Type;

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	double HarvestSeconds = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FLactoseEconomyUserItem> CostItems;

	UPROPERTY(BlueprintReadOnly)
	TArray<FLactoseEconomyUserItem> HarvestItems;

	UPROPERTY(BlueprintReadOnly)
	TArray<FLactoseEconomyUserItem> DestroyItems;

	UPROPERTY(BlueprintReadOnly)
	FString FertiliserItemId;

	UPROPERTY(BlueprintReadOnly)
	FString GameCrop;
};

USTRUCT()
struct FLactoseSimulationGetCropsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLactoseSimulationCrop> Crops;
};

using FQuerySimulationCropsRequest = Lactose::Rest::TRequest<FLactoseSimulationQueryCropsRequest, FLactoseSimulationQueryCropsResponse>;
using FGetSimulationCropsRequest = Lactose::Rest::TRequest<FLactoseSimulationGetCropsRequest, FLactoseSimulationGetCropsResponse>;