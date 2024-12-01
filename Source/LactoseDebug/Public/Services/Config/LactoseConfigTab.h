// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "DebugHelpers.h"
#include "DebugImGuiHelpers.h"
#include "LactoseConfigTab.generated.h"

class ULactoseConfigCloudServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseConfigTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseConfigTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	UPROPERTY(Transient)
	TObjectPtr<ULactoseConfigCloudServiceSubsystem> ConfigSubsystem = nullptr;

	Debug::ImGui::FSearchBox ConfigSearchBox;
};
