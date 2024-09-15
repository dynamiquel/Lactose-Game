#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include <imgui.h>

#include "IDebugOverlayDrawable.h"

#include "DebugOverlay.generated.h"

struct ImGuiStyle;
class UDebugApp;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDebugOverlayAppEvent,
	const UDebugOverlay*, Sender,
	UDebugApp*, App);

struct FDebugOverlayScopedStyling
{
	FDebugOverlayScopedStyling();
	~FDebugOverlayScopedStyling();

	ImGuiStyle OriginalStyle;
};

/**
 * The Debug Overlay is an interactable in-game menu that allows for displaying certain debug information in the form
 * of 'Apps'. An App is basically a Window that gets created or destroyed based on whether the App is desired.
 */
UCLASS()
class DEBUGOVERLAY_API UDebugOverlay : public UObject, public IDebugOverlayDrawable
{
	GENERATED_BODY()

public:
	// Begin override IDebugDraw
	void Init() override;
	void Render() override;
	// End override IDebugDraw

	FDebugOverlayAppEvent& GetDebugAppOpened() { return DebugAppOpened; }
	FDebugOverlayAppEvent& GetDebugAppClosed() { return DebugAppClosed; }

protected:
	void RegisterApps();

	void RenderMiscActions();
	void RenderAppBar();
	void RenderApps();

	void OpenApp(const TSubclassOf<UDebugApp>& AppClass);
	void CloseApp(const TSubclassOf<UDebugApp>& AppClass);
	void CloseAllApps();

protected:
	UPROPERTY(BlueprintAssignable, Category="Debug Overlay")
	FDebugOverlayAppEvent DebugAppOpened;

	UPROPERTY(BlueprintAssignable, Category="Debug Overlay")
	FDebugOverlayAppEvent DebugAppClosed;
	
	// CDOs of all the Debug Apps that can be opened.
	UPROPERTY(Transient)
	TArray<TObjectPtr<const UDebugApp>> RegisteredApps;

	// Instantiated Debug Apps that are currently active.
	UPROPERTY(Transient)
	TMap<TSubclassOf<UDebugApp>, TObjectPtr<UDebugApp>> ActiveApps;
};
