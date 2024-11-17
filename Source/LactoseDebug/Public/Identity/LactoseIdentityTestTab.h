#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "LactoseIdentityTestTab.generated.h"

class ULactoseIdentityServiceSubsystem;

/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseIdentityTestTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseIdentityTestTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	UPROPERTY(Transient)
	TObjectPtr<ULactoseIdentityServiceSubsystem> IdentitySubsystem;
};
