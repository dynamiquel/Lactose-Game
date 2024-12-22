// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Kismet/BlueprintFunctionLibrary.h>

#include "LactoseSimulationCropsRequests.h"
#include "LactoseSimulationBP.generated.h"

class ULactoseSimulationServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseSimulationBP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Lactose Simulation", meta=(WorldContext="WorldContextObject"))
	static TMap<FString, FLactoseSimulationCrop> GetCrops(const ULactoseSimulationServiceSubsystem* Simulation);

	UFUNCTION(BlueprintPure, Category = "Lactose Simulation", meta=(WorldContext="WorldContextObject"))
	static FLactoseSimulationCrop GetCrop(const ULactoseSimulationServiceSubsystem* Simulation, const FString& CropName);

	UFUNCTION(BlueprintCallable, Category = "Lactose Simulation", meta=(WorldContext="WorldContextObject"))
	static void SeedCurrentUserCrops(ULactoseSimulationServiceSubsystem* Simulation, const TArray<FString>& CropInstanceIds, const FString& CropId);

	UFUNCTION(BlueprintPure, Category = "Lactose Simulation", meta=(WorldContext="WorldContextObject"))
	static TArray<FLactoseEconomyUserItem> GetCropCostUserItems(const ULactoseSimulationServiceSubsystem* Simulation, const FString& CropId);

	UFUNCTION(BlueprintCallable, Category = "Lactose Simulation", meta=(WorldContext="WorldContextObject"))
	static bool CanCurrentUserAffordCrop(const ULactoseSimulationServiceSubsystem* Simulation, const FString& CropId);
};
