#include "Profiler/DebugProfilerSummaryTab.h"

#include "ImGuiModule.h"

#include "Profiler/DebugProfilerApp.h"

UDebugProfilerSummaryTab::UDebugProfilerSummaryTab()
{
	SetOwningAppClass<UDebugProfilerApp>();
	SetTabName(TEXT("Summary"));
}

void UDebugProfilerSummaryTab::Init()
{
}

void UDebugProfilerSummaryTab::Render()
{
}
