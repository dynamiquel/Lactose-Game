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

USTRUCT()
struct FLactoseSimulationCrop
{
	GENERATED_BODY()

	UPROPERTY()
	FString Id;

	UPROPERTY()
	FString Type;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	double HarvestSeconds;

	UPROPERTY()
	TArray<FLactoseEconomyUserItem> CostItems;

	UPROPERTY()
	TArray<FLactoseEconomyUserItem> HarvestItems;

	UPROPERTY()
	TArray<FLactoseEconomyUserItem> DestroyItems;

	UPROPERTY()
	FString FertiliserItemId;
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