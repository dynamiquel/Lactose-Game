// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "LactoseSimulationStatusTab.generated.h"

namespace Lactose::Debug::Services
{
	class FStatusSection;
}

class ULactoseSimulationServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseSimulationStatusTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseSimulationStatusTab();
	
	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	TSharedPtr<Lactose::Debug::Services::FStatusSection> StatusSection;
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseSimulationServiceSubsystem> SimulationSubsystem;
};
