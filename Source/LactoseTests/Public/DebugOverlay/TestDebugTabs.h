#pragma once

#include "CoreMinimal.h"

#include "DebugAppTab.h"

#include "TestDebugTabs.generated.h"

/**
 * 
 */
UCLASS()
class LACTOSETESTS_API UTestDebugTab : public UDebugAppTab
{
	GENERATED_BODY()

	UTestDebugTab();

	void Render() override;
};

UCLASS()
class LACTOSETESTS_API UTestDebugTab2 : public UDebugAppTab
{
	GENERATED_BODY()

	UTestDebugTab2();

	void Render() override;
};
