#include "DebugApp.h"

#include "DebugAppTab.h"
#include "DebugHelpers.h"
#include "DebugImGuiHelpers.h"

#include "ImGuiModule.h"

void UDebugApp::Init()
{
	RegisterTabs();
}

void UDebugApp::Render()
{
	RenderTabs();
}

void UDebugApp::SetAppName(FString&& InAppName)
{
	check(!InAppName.IsEmpty());
	AppName = InAppName;
}

void UDebugApp::RegisterTabs()
{
	TArray<TSubclassOf<UDebugAppTab>> FoundTabClasses = Debug::GetClassesOfType<UDebugAppTab>(false,
		TFunction<bool(const UDebugAppTab&)>([&](const UDebugAppTab& DefaultTab)
		{
			return DefaultTab.GetOwningAppClass() == GetClass() && DefaultTab.ShouldCreateTab(*this);
		}));

	for (auto FoundTabClass : FoundTabClasses)
	{
		auto InstantiatedTab = NewObject<UDebugAppTab>(this, FoundTabClass);
		check(InstantiatedTab);
		RegisterTab(*InstantiatedTab);
	}
}

void UDebugApp::RegisterTab(UDebugAppTab& AppTab)
{
	AppTabs.Add(&AppTab);
	AppTab.Init();
}

void UDebugApp::RenderTabs()
{
	if (AppTabs.IsEmpty())
		return;
	
	FString TabBarId = FString::Printf(TEXT("###%sTabBar"), *GetName());
	ImGui::BeginTabBar(STR_TO_ANSI(TabBarId));

	for (int32 AppTabIdx = 0; AppTabIdx < AppTabs.Num(); AppTabIdx++)
	{
		const TObjectPtr<UDebugAppTab> AppTab = AppTabs[AppTabIdx];
		check(AppTab);

		if (ImGui::BeginTabItem(STR_TO_ANSI(AppTab->GetTabName())))
		{
			AppTab->Render();
			ImGui::EndTabItem();
		}
	}
}