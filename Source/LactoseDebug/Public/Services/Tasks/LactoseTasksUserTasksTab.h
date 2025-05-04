#pragma once

#include "CoreMinimal.h"
#include "Simp.h"
#include "DebugAppTab.h"
#include "DebugImGuiHelpers.h"
#include "LactoseTasksUserTasksTab.generated.h"

class ULactoseEconomyServiceSubsystem;
class ULactoseTasksServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseTasksUserTasksTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseTasksUserTasksTab();

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
