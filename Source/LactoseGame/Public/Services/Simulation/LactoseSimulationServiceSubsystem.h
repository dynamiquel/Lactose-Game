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

	TArray<TSharedRef<const FLactoseSimulationUserCropInstance>> FindCropInstances(TConstArrayView<FString> CropInstanceIds) const;
	TArray<TSharedRef<FLactoseSimulationUserCropInstance>> FindMutableCropInstances(TConstArrayView<FString> CropInstanceIds);

	
private:
	TSharedPtr<FLactoseSimulationUserCropInstance> FindMutableCropInstance(const FString& CropInstanceId);
	void EmplaceCropInstance(const TSharedRef<FLactoseSimulationUserCropInstance>& ExistingCropInstance);

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

protected:
	TSharedPtr<FLactoseSimulationUserCrops> GetMutableCurrentUserCrops();
	
	void OnAllCropsQueried(TSharedRef<FQuerySimulationCropsRequest::FResponseContext> Context);
	void OnAllCropsRetrieved(TSharedRef<FGetSimulationCropsRequest::FResponseContext> Context);
	
	void OnCurrentUserCropsRetrieved(TSharedRef<FGetSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsSimulated(TSharedRef<FSimulateSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsHarvested(TSharedRef<FHarvestSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsSeeded(TSharedRef<FSeedSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsFertilised(TSharedRef<FFertiliseSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsDestroyed(TSharedRef<FDeleteSimulationUserCropsRequest::FResponseContext> Context);
	void OnCurrentUserCropsCreated(TSharedRef<FCreateSimulationUserCropRequest::FResponseContext> Context);

	void OnUserLoggedIn(
		const ULactoseIdentityServiceSubsystem& Sender,
		const TSharedRef<FLactoseIdentityGetUserResponse>& User);

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
	
	TFuture<TSharedPtr<FQuerySimulationCropsRequest::FResponseContext>> QueryAllCropsFuture;
	TFuture<TSharedPtr<FGetSimulationCropsRequest::FResponseContext>> GetAllCropsFuture;
	TMap<FString, TSharedRef<FLactoseSimulationCrop>> AllCrops;

	TFuture<TSharedPtr<FGetSimulationUserCropsRequest::FResponseContext>> GetCurrentUserCropsFuture;
	TSharedPtr<FLactoseSimulationUserCrops> CurrentUserCrops;

	TFuture<TSharedPtr<FSimulateSimulationUserCropsRequest::FResponseContext>> SimulateCurrentUserCropsFuture;

	FDateTime PreviousUserSimulationTime;

	FTimerHandle SimulateTicker;
};

namespace Lactose::Simulation::Events
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FAllCropsLoaded,
		const ULactoseSimulationServiceSubsystem& /* Sender */);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FUserCropsLoaded,
		const ULactoseSimulationServiceSubsystem& /* Sender */,
		TSharedRef<FLactoseSimulationUserCrops> /* UserCrops */);

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FUserCropsSimulated,
		const ULactoseSimulationServiceSubsystem& /* Sender */,
		FDateTime /* PreviousSimulationTime */,
		FDateTime /* NewSimulationTime */);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FUserCropsDelegate,
		const ULactoseSimulationServiceSubsystem& /* Sender */,
		const TArray<TSharedRef<const FLactoseSimulationUserCropInstance>>& /* ModifiedUserCrops */);

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