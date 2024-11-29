#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "DebugImGuiHelpers.h"
#include "LactoseEconomyItemsTab.generated.h"

class ULactoseEconomyServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseEconomyItemsTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseEconomyItemsTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

	UPROPERTY(Transient)
	TObjectPtr<ULactoseEconomyServiceSubsystem> EconomySubsystem;

	Debug::ImGui::FSearchBox ItemsSearchBox;
};
