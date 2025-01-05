#pragma once

#include "Services/Economy/LactoseEconomyUserItemsRequests.h"

#include "LactoseSimulationUserCropsRequests.generated.h"

USTRUCT()
struct FLactoseSimulationGetUserCropsRequest
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString UserId;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FLactoseSimulationUserCropInstanceDelegate,
	const TSharedRef<const struct FLactoseSimulationUserCropInstance>& /* Sender */);

USTRUCT()
struct FLactoseSimulationUserCropInstance
{
	GENERATED_BODY()
	
	FLactoseSimulationUserCropInstance& operator=(const FLactoseSimulationUserCropInstance& Other)
	{
		// Copy properties only.
		Id = Other.Id;
		CropId = Other.CropId;
		State = Other.State;
		Location = Other.Location;
		Rotation = Other.Rotation;
		CreationTime = Other.CreationTime;
		RemainingHarvestSeconds = Other.RemainingHarvestSeconds;
		RemainingFertiliserSeconds = Other.RemainingFertiliserSeconds;

		return self;
	}

	UPROPERTY()
	FString Id;

	UPROPERTY()
	FString CropId;

	UPROPERTY()
	FString State;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FVector Rotation = FVector::ZeroVector;

	UPROPERTY()
	FDateTime CreationTime;

	UPROPERTY()
	double RemainingHarvestSeconds = 0;

	UPROPERTY()
	double RemainingFertiliserSeconds = 0;

	mutable FLactoseSimulationUserCropInstanceDelegate OnLoaded;
	mutable FLactoseSimulationUserCropInstanceDelegate OnHarvested;
	mutable FLactoseSimulationUserCropInstanceDelegate OnFertilised;
	mutable FLactoseSimulationUserCropInstanceDelegate OnSeeded;
	mutable FLactoseSimulationUserCropInstanceDelegate OnDestroyed;
};

USTRUCT()
struct FLactoseSimulationGetUserCropsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FDateTime PreviousSimulationTime;
	
	UPROPERTY()
	TArray<FLactoseSimulationUserCropInstance> CropInstances;
};

USTRUCT()
struct FLactoseSimulationSimulateUserCropsRequest
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString UserId;
};

USTRUCT()
struct FLactoseSimulationSimulateUserCropsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FDateTime PreviousSimulationTime;

	UPROPERTY()
	FDateTime NewSimulationTime;
};

USTRUCT()
struct FLactoseSimulationCreateUserCropRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId;

	UPROPERTY()
	FString CropId;

	UPROPERTY()
	FVector CropLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector CropRotation = FVector::ZeroVector;
};

USTRUCT()
struct FLactoseSimulationCreateUserCropResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserCropInstanceId;
};

USTRUCT()
struct FLactoseSimulationHarvestUserCropsRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId;

	UPROPERTY()
	TArray<FString> CropInstanceIds;
};

USTRUCT()
struct FLactoseSimulationHarvestUserCropsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> HarvestedCropInstanceIds;
};

USTRUCT()
struct FLactoseSimulationFertiliseUserCropsRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId;

	UPROPERTY()
	TArray<FString> CropInstanceIds;
};

USTRUCT()
struct FLactoseSimulationFertiliseUserCropsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> FertilisedCropInstanceIds;
};

USTRUCT()
struct FLactoseSimulationSeedUserCropsRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId;

	UPROPERTY()
	FString CropId;

	UPROPERTY()
	TArray<FString> CropInstanceIds;
};

USTRUCT()
struct FLactoseSimulationSeedUserCropsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> SeededCropInstanceIds;
};

USTRUCT()
struct FLactoseSimulationDeleteUserCropsRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId;

	UPROPERTY()
	TArray<FString> CropInstanceIds;
};

USTRUCT()
struct FLactoseSimulationDeleteUserCropsResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> DestroyedCropInstanceIds;
};

using FGetSimulationUserCropsRequest = Lactose::Rest::TRequest<FLactoseSimulationGetUserCropsRequest, FLactoseSimulationGetUserCropsResponse>;
using FSimulateSimulationUserCropsRequest = Lactose::Rest::TRequest<FLactoseSimulationSimulateUserCropsRequest, FLactoseSimulationSimulateUserCropsResponse>;
using FCreateSimulationUserCropRequest = Lactose::Rest::TRequest<FLactoseSimulationCreateUserCropRequest, FLactoseSimulationCreateUserCropResponse>;
using FHarvestSimulationUserCropsRequest = Lactose::Rest::TRequest<FLactoseSimulationHarvestUserCropsRequest, FLactoseSimulationHarvestUserCropsResponse>;
using FFertiliseSimulationUserCropsRequest = Lactose::Rest::TRequest<FLactoseSimulationFertiliseUserCropsRequest, FLactoseSimulationFertiliseUserCropsResponse>;
using FSeedSimulationUserCropsRequest = Lactose::Rest::TRequest<FLactoseSimulationSeedUserCropsRequest, FLactoseSimulationSeedUserCropsResponse>;
using FDeleteSimulationUserCropsRequest = Lactose::Rest::TRequest<FLactoseSimulationDeleteUserCropsRequest, FLactoseSimulationDeleteUserCropsResponse>;