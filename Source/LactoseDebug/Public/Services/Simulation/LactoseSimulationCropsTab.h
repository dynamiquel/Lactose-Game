// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "DebugImGuiHelpers.h"
#include "LactoseSimulationCropsTab.generated.h"

class ULactoseEconomyServiceSubsystem;
class ULactoseSimulationServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseSimulationCropsTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseSimulationCropsTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	UPROPERTY(Transient)
	TObjectPtr<ULactoseSimulationServiceSubsystem> SimulationSubsystem;
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseEconomyServiceSubsystem> EconomySubsystem;

	Debug::ImGui::FSearchBox CropsSearchBox;
};
