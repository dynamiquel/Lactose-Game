#pragma once

#include "CoreMinimal.h"

#include "DebugAppTab.h"

#include "LactoseIdentityDiscordDebugTab.generated.h"

class UDiscordGameSubsystem;

/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseIdentityDiscordDebugTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseIdentityDiscordDebugTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	UPROPERTY(Transient)
	TObjectPtr<UDiscordGameSubsystem> DiscordSubsystem;
};
