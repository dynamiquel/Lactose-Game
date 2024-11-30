// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LactoseSimulationCropsRequests.h"
#include "LactoseSimulationUserCropsRequests.h"
#include "Services/LactoseServiceSubsystem.h"
#include "LactoseSimulationServiceSubsystem.generated.h"

struct FLactoseIdentityGetUserResponse;
class ULactoseIdentityServiceSubsystem;

UENUM()
enum class ELactoseSimulationCropsStatus
{
	None,
	Querying,
	Retrieving,
	Loaded
};

UENUM()
enum class ELactoseSimulationUserCropsStatus
{
	None,
	Retrieving,
	Loaded
};

class LACTOSEGAME_API FLactoseSimulationUserCrops
{
	friend class ULactoseSimulationServiceSubsystem;

public:
	TConstArrayView<TSharedRef<FLactoseSimulationUserCropInstance>> GetAllCropInstances() const;
	TSharedPtr<const FLactoseSimulationUserCropInstance> FindCropInstance(const FString& CropInstanceId) const;
	TSharedRef<const FLactoseSimulationUserCropInstance> UpdateCropInstance(const FLactoseSimulationUserCropInstance& NewCropInstanceData);
	bool DeleteCropInstance(const FString& CropInstanceId);

private:
	TSharedPtr<FLactoseSimulationUserCropInstance> FindCropInstance(const FString& CropInstanceId);

private:
	TArray<TSharedRef<FLactoseSimulationUserCropInstance>> Database;
};

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseSimulationServiceSubsystem : public ULactoseServiceSubsystem
{
	GENERATED_BODY()

	ULactoseSimulationServiceSubsystem();
	
	// Begin override ULactoseServiceSubsystem
	void Initialize(FSubsystemCollectionBase& Collection) override;
	// End override ULactoseServiceSubsystem

public:
	ELactoseSimulationCropsStatus GetAllCropsStatus() const;
	const TMap<FString, TSharedRef<FLactoseSimulationCrop>>& GetAllCrops() const { return AllCrops; }
	TSharedPtr<const FLactoseSimulationCrop> FindCrop(const FString& CropId) const;

	void LoadAllCrops();

	ELactoseSimulationUserCropsStatus GetCurrentUserCropsStatus() const;
	TSharedPtr<const FLactoseSimulationUserCrops> GetCurrentUserCrops() const;
	FDateTime GetCurrentUserPreviousSimulationTime() const { return PreviousUserSimulationTime; }

	void LoadCurrentUserCrops();

protected:
	void OnAllCropsQueried(TSharedRef<FQuerySimulationCropsRequest::FResponseContext> Context);
	void OnAllCropsRetrieved(TSharedRef<FGetSimulationCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsRetrieved(TSharedRef<FGetSimulationUserCropsRequest::FResponseContext> Context);
	
	void OnUserLoggedIn(
		const ULactoseIdentityServiceSubsystem& Sender,
		const TSharedRef<FLactoseIdentityGetUserResponse>& User);

private:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bAutoRetrieveCrops = true;
	
	TFuture<TSharedPtr<FQuerySimulationCropsRequest::FResponseContext>> QueryAllCropsFuture;
	TFuture<TSharedPtr<FGetSimulationCropsRequest::FResponseContext>> GetAllCropsFuture;
	TMap<FString, TSharedRef<FLactoseSimulationCrop>> AllCrops;

	TFuture<TSharedPtr<FGetSimulationUserCropsRequest::FResponseContext>> GetCurrentUserCropsFuture;
	TSharedPtr<FLactoseSimulationUserCrops> CurrentUserCrops;

	FDateTime PreviousUserSimulationTime;
};

namespace Lactose::Simulation::Events
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FAllCropsLoaded,
		const ULactoseSimulationServiceSubsystem& /* Sender */);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FUserCropsLoaded,
		const ULactoseSimulationServiceSubsystem& /* Sender */,
		TSharedRef<FLactoseSimulationUserCrops> UserCrops);

	inline FAllCropsLoaded OnAllCropsLoaded;
	inline FUserCropsLoaded OnCurrentUserCropsLoaded;

}

namespace Lactose::Simulation::States
{
	constexpr auto* Empty = TEXT("Empty");
	constexpr auto* Growing = TEXT("Growing");
	constexpr auto* Harvestable = TEXT("Harvestable");
}