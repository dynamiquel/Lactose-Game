#include "DebugOverlay.h"

#include "Kismet/GameplayStatics.h"

#include "imgui.h"
#include "ImGuiModule.h"

#include "DebugApp.h"
#include "DebugHelpers.h"
#include "DebugImGuiHelpers.h"
#include "DebugOverlaySettings.h"
#include "DebugOverlaySubsystem.h"

FDebugOverlayScopedStyling::FDebugOverlayScopedStyling()
{
	ImGuiStyle& CurrentStyle = ImGui::GetStyle();
	OriginalStyle = CurrentStyle;

	const auto& DebugOverlaySettings = UDebugOverlaySettings::Get();

	{
		const auto GlobalPadding = Debug::ImGui::ConvertVector2D<ImVec2>(DebugOverlaySettings.Sizes.GlobalPadding);
		CurrentStyle.WindowPadding = GlobalPadding;
		CurrentStyle.FramePadding = GlobalPadding;
		CurrentStyle.CellPadding = GlobalPadding;
		CurrentStyle.DisplayWindowPadding = GlobalPadding;
	}

	{
		const float Rounding = DebugOverlaySettings.Sizes.GlobalRounding;
		CurrentStyle.WindowRounding = Rounding;
		CurrentStyle.FrameRounding = Rounding;
		CurrentStyle.ChildRounding = Rounding;
		CurrentStyle.GrabRounding = Rounding;
		CurrentStyle.PopupRounding = Rounding;
		CurrentStyle.ScrollbarRounding = Rounding;
		CurrentStyle.TabRounding = DebugOverlaySettings.Sizes.TabRounding;
	}

	{
		CurrentStyle.GrabMinSize = DebugOverlaySettings.Sizes.GrabSize;
		CurrentStyle.ScrollbarSize = DebugOverlaySettings.Sizes.ScrollbarSize;
	}
}

FDebugOverlayScopedStyling::~FDebugOverlayScopedStyling()
{
	ImGui::GetStyle() = OriginalStyle;
}

void UDebugOverlay::Init()
{
	RegisterApps();
}

void UDebugOverlay::Render()
{
	auto ScopedStyling = FDebugOverlayScopedStyling();
	
	RenderMiscActions();
	RenderAppBar();
	RenderApps();
}

void UDebugOverlay::RegisterApps()
{
	TArray<TSubclassOf<UDebugApp>> FoundAppClasses = Debug::GetClassesOfType<UDebugApp>(false,
		TFunction<bool(const UDebugApp&)>([&](const UDebugApp& DefaultApp)
		{
			return DefaultApp.ShouldCreateApp(*this);
		}));

	for (auto FoundAppClass : FoundAppClasses)
	{
		RegisteredApps.Add(FoundAppClass.GetDefaultObject());
	}
}

void UDebugOverlay::RenderMiscActions()
{
	constexpr auto ActionBarFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(1000.f, 200.f), ImGuiCond_Always);
	
	if (!ImGui::Begin("Actions", nullptr, ActionBarFlags))
		return;

	if (ImGui::Button("Close"))
	{
		UDebugOverlaySubsystem& DebugOverlaySubsystem = *CastChecked<UDebugOverlaySubsystem>(GetOuter());
		DebugOverlaySubsystem.SetDebugOverlayEnabled(false);
	}

	ImGui::SameLine();

	auto& Properties = FImGuiModule::Get().GetProperties();
	
	if (Properties.IsInputEnabled() && ImGui::Button("Return to Game Input"))
		FImGuiModule::Get().GetProperties().SetInputEnabled(false);

	ImGui::SameLine();
	
	if (!ActiveApps.IsEmpty() && ImGui::Button("Close All Apps"))
		CloseAllApps();

	ImGui::SameLine();

	if (ImGui::Button("Exit Game"))
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			PC->ConsoleCommand("Quit");

	ImGui::SameLine();
	
	if (ImGui::Button("Restart Level"))
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			PC->ConsoleCommand("RestartLevel");

	ImGui::End();
}

void UDebugOverlay::RenderAppBar()
{
	const auto& DebugOverlaySettings = UDebugOverlaySettings::Get();

	constexpr auto ActionBarFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	
	ImGuiViewport* Viewport = ImGui::GetMainViewport();
	check(Viewport);

	const float DesiredHeight = Viewport->WorkSize.y * .5f;
	
	ImGui::SetNextWindowPos(ImVec2(0.f, 50.f), ImGuiCond_Always);

	ImVec2 WindowSize = ImVec2(
	    DebugOverlaySettings.AppBarButtonDimensions.X + DebugOverlaySettings.Sizes.GlobalPadding.X * 2,
	    DesiredHeight);
	
	ImGui::SetNextWindowSize(WindowSize, ImGuiCond_Always);
	
	if (!ImGui::Begin("Apps"))
		return;
	
	for (const TObjectPtr<const UDebugApp>& RegisteredApp : RegisteredApps)
	{
		check(RegisteredApp);
		TSubclassOf<UDebugApp> AppClass = RegisteredApp.GetClass();
		check(AppClass);
		
		const bool bAppOpen = ActiveApps.Contains(AppClass);
		
		if (bAppOpen)
			ImGui::PushStyleColor(ImGuiCol_Button, Debug::ImGui::Color(DebugOverlaySettings.AppOpenedButtonColour));

		const bool bButtonPressed = ImGui::Button(
			STR_TO_ANSI(RegisteredApp->GetAppName()),
			Debug::ImGui::ConvertVector2D<ImVec2>(DebugOverlaySettings.AppBarButtonDimensions));

		if (bAppOpen)
			ImGui::PopStyleColor();	
		
		if (bButtonPressed)
			bAppOpen ? CloseApp(AppClass) : OpenApp(AppClass);
	}
	
	ImGui::End();
}

void UDebugOverlay::RenderApps()
{
	// Should be near enough impossible to close multiple apps at once.
	TArray<TSubclassOf<UDebugApp>, TInlineAllocator<1>> AppsToClose;

	for (TTuple<TSubclassOf<UDebugApp>, TObjectPtr<UDebugApp>> ActiveApp : ActiveApps)
	{
		check(ActiveApp.Value);
		bool bStayOpen = true;
		
		ImGui::Begin(STR_TO_ANSI(ActiveApp.Value->GetAppName()), &bStayOpen);
		ActiveApp.Value->Render();
		ImGui::End();

		// User closed the Window.
		if (!bStayOpen)
			AppsToClose.Add(ActiveApp.Key);
	}

	for (auto AppToClose : AppsToClose)
		CloseApp(AppToClose);
}

void UDebugOverlay::OpenApp(const TSubclassOf<UDebugApp>& AppClass)
{
	TObjectPtr<UDebugApp> FoundApp = ActiveApps.FindRef(AppClass);
	check(!FoundApp);

	auto InstantiatedApp = NewObject<UDebugApp>(this, AppClass);
	check(InstantiatedApp);
	ActiveApps.Add(AppClass, InstantiatedApp);
	InstantiatedApp->Init();
	DebugAppOpened.Broadcast(this, InstantiatedApp);
}

void UDebugOverlay::CloseApp(const TSubclassOf<UDebugApp>& AppClass)
{
	TObjectPtr<UDebugApp> FoundApp = ActiveApps.FindRef(AppClass);
	check(FoundApp);

	FoundApp->Close();
	DebugAppClosed.Broadcast(this, FoundApp);
	ActiveApps.Remove(AppClass);
}

void UDebugOverlay::CloseAllApps()
{
	TArray<TSubclassOf<UDebugApp>, TInlineAllocator<8>> AppsToClose;
	ActiveApps.GetKeys(OUT AppsToClose);
	
	for (const TSubclassOf<UDebugApp>& AppToClose : AppsToClose)
		CloseApp(AppToClose);
}
