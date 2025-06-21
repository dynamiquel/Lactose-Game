// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Simp.h"
#include "Subsystems/WorldSubsystem.h"
#include "LactoseCropWorldSubsystem.generated.h"

class APlotCropActor;
class FLactoseConfigCloudConfig;
class ULactoseConfigCloudServiceSubsystem;
struct FLactoseSimulationCrop;
class ACropActor;
struct FLactoseSimulationUserCropInstance;
class FLactoseSimulationUserCrops;
class ULactoseSimulationServiceSubsystem;

namespace Crops
{
	static constexpr auto CropCullChannel = ECC_Vehicle;
	static constexpr auto CropTraceChannel = ECC_GameTraceChannel1;
	static constexpr float PlotSizeCm = 150.f;
	static constexpr float PlotHalfExtentCm = PlotSizeCm * 0.5f;
	static constexpr float MagnetiseToleranceCm = 50.0f; 
	static constexpr float MagnetiseSearchRadiusCm = PlotSizeCm * 1.5f;
}

UENUM()
enum class ECropMagnetType : uint8
{
	None,
	NearbyCrop,
	Grid
};

/**
 * 
 */
UCLASS(Config=Services, DefaultConfig)
class LACTOSEGAME_API ULactoseCropWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ULactoseCropWorldSubsystem();
	
	bool CanCreateCrops() const;
	void RegisterCropActor(ACropActor& CropActor);
	void DeregisterCropActor(const ACropActor& CropActor);
	ACropActor* FindCropActorForCropInstance(const FString& CropInstanceId) const;
	TSubclassOf<ACropActor> FindCropActorClassForCrop(const FString& CropId) const;
	ACropActor* ResetCropActor(ACropActor& CropActor);

	bool IsLocationObstructed(const FVector& ProposedLocation, const AActor* ActorToIgnore = nullptr) const;
	FVector GetMagnetizedPlotLocation(const FVector& ProposedLocation) const;

protected:
	bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Begin override UWorldSubsystem
	void OnWorldBeginPlay(UWorld& InWorld) override;
	// End override UWorldSubsystem

	void OnAllCropsLoaded(const ULactoseSimulationServiceSubsystem& Sender);
	
	void OnUserCropsLoaded(
		const ULactoseSimulationServiceSubsystem& Sender,
		Sr<FLactoseSimulationUserCrops> UserCrops);
	
	void CreateRequiredUserCrops();
	
	ACropActor* CreateUserCrop(
		const Sr<const FLactoseSimulationCrop>& Crop,
		const Sr<const FLactoseSimulationUserCropInstance>& CropInstance);

	void LoadCropActorClasses();

private:
	UPROPERTY(EditAnywhere, Config)
	FString CropIdToCropActorMapEntryId = TEXT("CropActorClassesMap");

	UPROPERTY(EditAnywhere, Config)
	TSoftClassPtr<APlotCropActor> EmptyPlotCropActor;

	UPROPERTY(EditAnywhere, Config)
	ECropMagnetType MagnetType = ECropMagnetType::Grid;
	
	bool bWaitingForCrops = true;
	bool bWaitingForUserCrops = true;
	bool bWaitingForCropActorClassMap = true;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<ACropActor>> RegisteredCropActors;

	UPROPERTY(Transient)
	TMap<FString, TSubclassOf<ACropActor>> CropIdToCropActorMap;
};