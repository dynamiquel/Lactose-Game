#pragma once

#include <CoreMinimal.h>

#include "DebugAppTab.h"
#include "LactoseIdentityStatusTab.generated.h"

namespace Lactose::Debug::Services
{
	class FStatusSection;
}

struct FLactoseServiceInfo;
class ULactoseIdentityServiceSubsystem;

/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseIdentityStatusTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseIdentityStatusTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	TSharedPtr<Lactose::Debug::Services::FStatusSection> StatusSection;
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseIdentityServiceSubsystem> IdentitySubsystem;
};
