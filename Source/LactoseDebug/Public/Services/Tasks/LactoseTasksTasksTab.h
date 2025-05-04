#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "DebugImGuiHelpers.h"
#include "LactoseTasksTasksTab.generated.h"

class ULactoseEconomyServiceSubsystem;
class ULactoseTasksServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseTasksTasksTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseTasksTasksTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

	UPROPERTY(Transient)
	TObjectPtr<ULactoseTasksServiceSubsystem> TasksSubsystem;
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseEconomyServiceSubsystem> EconomySubsystem;

	Debug::ImGui::FSearchBox ItemsSearchBox;
};
