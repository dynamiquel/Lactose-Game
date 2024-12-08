// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LactoseCropWorldSubsystem.generated.h"

struct FLactoseSimulationCrop;
class ACropActor;
struct FLactoseSimulationUserCropInstance;
class FLactoseSimulationUserCrops;
class ULactoseSimulationServiceSubsystem;

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseCropWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	bool CanCreateCrops() const { return !bWaitingForCrops && !bWaitingForUserCrops; }
	void RegisterCropActor(ACropActor& CropActor);
	void DeregisterCropActor(const ACropActor& CropActor);
	ACropActor* FindCropActorForCropInstance(const FString& CropInstanceId) const;

protected:
	// Begin override UWorldSubsystem
	void OnWorldBeginPlay(UWorld& InWorld) override;
	// End override UWorldSubsystem

	void OnAllCropsLoaded(const ULactoseSimulationServiceSubsystem& Sender);
	void OnUserCropsLoaded(
		const ULactoseSimulationServiceSubsystem& Sender,
		TSharedRef<FLactoseSimulationUserCrops> UserCrops);

	void CreateRequiredUserCrops();
	bool CreateUserCrop(
		const TSharedRef<const FLactoseSimulationCrop>& Crop,
		const TSharedRef<const FLactoseSimulationUserCropInstance>& CropInstance);


private:
	bool bWaitingForCrops = true;
	bool bWaitingForUserCrops = true;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<ACropActor>> RegisteredCropActors;
};
