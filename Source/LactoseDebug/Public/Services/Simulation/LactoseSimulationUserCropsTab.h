// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <optional>
#include <string>

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "DebugImGuiHelpers.h"
#include "LactoseSimulationUserCropsTab.generated.h"

class ULactoseEconomyServiceSubsystem;
class ULactoseSimulationServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseSimulationUserCropsTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseSimulationUserCropsTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	UPROPERTY(Transient)
	TObjectPtr<ULactoseSimulationServiceSubsystem> SimulationSubsystem;
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseEconomyServiceSubsystem> EconomySubsystem;

	Debug::ImGui::FSearchBox UserCropsSearchBox;

	TSet<FString> SelectedUserCrops;

	std::string OverrideSeedCropId;
};
