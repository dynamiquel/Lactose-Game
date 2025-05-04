#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "Simp.h"
#include "Services/LactoseServiceDebugUtils.h"
#include "LactoseTasksStatusTab.generated.h"

class ULactoseTasksServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseTasksStatusTab : public UDebugAppTab
{
	GENERATED_BODY()
	
	ULactoseTasksStatusTab();
	
	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	Sp<Lactose::Debug::Services::FStatusSection> StatusSection;
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseTasksServiceSubsystem> TasksSubsystem;
};
