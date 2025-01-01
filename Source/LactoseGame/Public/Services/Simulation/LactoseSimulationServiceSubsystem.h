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
	TConstArrayView<Sr<FLactoseSimulationUserCropInstance>> GetAllCropInstances() const;
	Sp<const FLactoseSimulationUserCropInstance> FindCropInstance(const FString& CropInstanceId) const;
	Sr<const FLactoseSimulationUserCropInstance> UpdateCropInstance(const FLactoseSimulationUserCropInstance& NewCropInstanceData);
	bool DeleteCropInstance(const FString& CropInstanceId);

	TArray<Sr<const FLactoseSimulationUserCropInstance>> FindCropInstances(TConstArrayView<FString> CropInstanceIds) const;
	TArray<Sr<FLactoseSimulationUserCropInstance>> FindMutableCropInstances(TConstArrayView<FString> CropInstanceIds);

	
private:
	Sp<FLactoseSimulationUserCropInstance> FindMutableCropInstance(const FString& CropInstanceId);
	void EmplaceCropInstance(const Sr<FLactoseSimulationUserCropInstance>& ExistingCropInstance);

private:
	TArray<Sr<FLactoseSimulationUserCropInstance>> Database;
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
	const TMap<FString, Sr<FLactoseSimulationCrop>>& GetAllCrops() const { return AllCrops; }
	Sp<const FLactoseSimulationCrop> FindCrop(const FString& CropId) const;

	void LoadAllCrops();

	ELactoseSimulationUserCropsStatus GetCurrentUserCropsStatus() const;
	Sp<const FLactoseSimulationUserCrops> GetCurrentUserCrops() const;
	FDateTime GetCurrentUserPreviousSimulationTime() const { return PreviousUserSimulationTime; }

	void LoadCurrentUserCrops();

	void Simulate();
	void HarvestCropInstances(TConstArrayView<FString> CropInstanceIds);
	void SeedCropInstances(TConstArrayView<FString> CropInstanceIds, const FString& CropId);
	void FertiliseCropInstances(TConstArrayView<FString> CropInstanceIds);
	void DestroyCropInstances(TConstArrayView<FString> CropInstanceIds);
	void CreateEmptyPlot(const FVector& Location, const FRotator& Rotation);
	void CreateCrop(const FString& CropId, const FVector& Location, const FRotator& Rotation);
	void EnableSimulateTicker();
	void DisableSimulateTicker();
	bool IsAutoSimulateTicking() const { return SimulateTicker.IsValid(); }

	bool CanCurrentUserAffordCrop(const FString& CropId) const;

protected:
	Sp<FLactoseSimulationUserCrops> GetMutableCurrentUserCrops();
	
	void OnAllCropsQueried(Sr<FQuerySimulationCropsRequest::FResponseContext> Context);
	void OnAllCropsRetrieved(Sr<FGetSimulationCropsRequest::FResponseContext> Context);
	
	void OnCurrentUserCropsRetrieved(Sr<FGetSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsSimulated(Sr<FSimulateSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsHarvested(Sr<FHarvestSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsSeeded(Sr<FSeedSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsFertilised(Sr<FFertiliseSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsDestroyed(Sr<FDeleteSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsCreated(Sr<FCreateSimulationUserCropRequest::FResponseContext> Context);

	void OnUserLoggedIn(
		const ULactoseIdentityServiceSubsystem& Sender,
		const Sr<FLactoseIdentityGetUserResponse>& User);

	void OnUserLoggedOut(const ULactoseIdentityServiceSubsystem& Sender);

	void OnSimulateTick();

private:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bAutoRetrieveCrops = true;

	UPROPERTY(EditDefaultsOnly, Config)
	float SimulateTickInterval = 2.f;

	UPROPERTY(EditDefaultsOnly, Config)
	bool bRefreshUserCropsOnSimulated = true;

	UPROPERTY(EditAnywhere, Config)
	bool bClientSidePrediction = true;
	
	TFuture<Sp<FQuerySimulationCropsRequest::FResponseContext>> QueryAllCropsFuture;
	TFuture<Sp<FGetSimulationCropsRequest::FResponseContext>> GetAllCropsFuture;
	TMap<FString, Sr<FLactoseSimulationCrop>> AllCrops;

	TFuture<Sp<FGetSimulationUserCropsRequest::FResponseContext>> GetCurrentUserCropsFuture;
	Sp<FLactoseSimulationUserCrops> CurrentUserCrops;

	TFuture<Sp<FSimulateSimulationUserCropsRequest::FResponseContext>> SimulateCurrentUserCropsFuture;

	FDateTime PreviousUserSimulationTime;
	FTimerHandle SimulateTicker;
};

namespace Lactose::Simulation::Events
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FAllCropsLoaded,
		const ULactoseSimulationServiceSubsystem& /* Sender */);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FUserCropsLoaded,
		const ULactoseSimulationServiceSubsystem& /* Sender */,
		Sr<FLactoseSimulationUserCrops> /* UserCrops */);

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FUserCropsSimulated,
		const ULactoseSimulationServiceSubsystem& /* Sender */,
		FDateTime /* PreviousSimulationTime */,
		FDateTime /* NewSimulationTime */);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FUserCropsDelegate,
		const ULactoseSimulationServiceSubsystem& /* Sender */,
		const TArray<Sr<const FLactoseSimulationUserCropInstance>>& /* ModifiedUserCrops */);

	inline FAllCropsLoaded OnAllCropsLoaded;
	inline FUserCropsLoaded OnCurrentUserCropsLoaded;
	inline FUserCropsSimulated OnCurrentUserCropsSimulated;
	inline FUserCropsDelegate OnCurrentUserCropsHarvested;
	inline FUserCropsDelegate OnCurrentUserCropsSeeded;
	inline FUserCropsDelegate OnCurrentUserCropsFertilised;
	inline FUserCropsDelegate OnCurrentUserCropsDestroyed;
	inline FUserCropsDelegate OnCurrentUserCropsCreated;
}

namespace Lactose::Simulation::States
{
	constexpr auto* Empty = TEXT("Empty");
	constexpr auto* Growing = TEXT("Growing");
	constexpr auto* Harvestable = TEXT("Harvestable");
}

namespace Lactose::Simulation::Types
{
	constexpr auto* Plot = TEXT("Plot");
	constexpr auto* Tree = TEXT("Tree");
	constexpr auto* Animal = TEXT("Animal");
}