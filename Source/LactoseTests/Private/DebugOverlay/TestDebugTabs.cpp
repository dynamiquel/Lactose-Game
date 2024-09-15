#include "DebugOverlay/TestDebugTabs.h"

#include "ImGuiModule.h"

#include "DebugOverlay/TestDebugApp.h"

UTestDebugTab::UTestDebugTab()
{
	SetTabName(TEXT("Test Tab 1"));
	SetOwningAppClass<UTestDebugApp>();
}

void UTestDebugTab::Render()
{
	ImGui::Text("Tab 1 is open");
}

UTestDebugTab2::UTestDebugTab2()
{
	SetTabName(TEXT("Test Tab 2"));
	SetOwningAppClass<UTestDebugApp>();
}

void UTestDebugTab2::Render()
{
	ImGui::Text("Tab 2 is open");
}
