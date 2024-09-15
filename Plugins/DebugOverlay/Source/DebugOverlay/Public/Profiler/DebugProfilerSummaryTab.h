#pragma once

#include "CoreMinimal.h"

#include "DebugAppTab.h"

#include "DebugProfilerSummaryTab.generated.h"

/**
 * 
 */
UCLASS()
class DEBUGOVERLAY_API UDebugProfilerSummaryTab : public UDebugAppTab
{
	GENERATED_BODY()

protected:
	UDebugProfilerSummaryTab();

	void Init() override;
	void Render() override;
};
