// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Simp.h"
#include "Subsystems/WorldSubsystem.h"
#include "LactoseCropWorldSubsystem.generated.h"

class FLactoseConfigCloudConfig;
class ULactoseConfigCloudServiceSubsystem;
struct FLactoseSimulationCrop;
class ACropActor;
struct FLactoseSimulationUserCropInstance;
class FLactoseSimulationUserCrops;
class ULactoseSimulationServiceSubsystem;

/**
 * 
 */
UCLASS(Config=Services, DefaultConfig)
class LACTOSEGAME_API ULactoseCropWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	bool CanCreateCrops() const;
	void RegisterCropActor(ACropActor& CropActor);
	void DeregisterCropActor(const ACropActor& CropActor);
	ACropActor* FindCropActorForCropInstance(const FString& CropInstanceId) const;
	TSubclassOf<ACropActor> FindCropActorClassForCrop(const FString& CropId) const;
	ACropActor* ResetCropActor(ACropActor& CropActor);
	
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
	
	bool bWaitingForCrops = true;
	bool bWaitingForUserCrops = true;
	bool bWaitingForCropActorClassMap = true;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<ACropActor>> RegisteredCropActors;

	UPROPERTY(Transient)
	TMap<FString, TSubclassOf<ACropActor>> CropIdToCropActorMap;
};