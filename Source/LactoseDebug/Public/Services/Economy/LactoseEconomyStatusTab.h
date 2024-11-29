#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "LactoseEconomyStatusTab.generated.h"

class ULactoseEconomyServiceSubsystem;

namespace Lactose::Debug::Services
{
	class FStatusSection;
}

/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseEconomyStatusTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseEconomyStatusTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	TSharedPtr<Lactose::Debug::Services::FStatusSection> StatusSection;
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseEconomyServiceSubsystem> EconomySubsystem;
};
