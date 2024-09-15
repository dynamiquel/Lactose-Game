#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "DebugApp.h"

#include "DebugOverlaySubsystem.generated.h"

class UDebugOverlay;

static TAutoConsoleVariable<bool> CVarDebugOverlay(
	TEXT("Debug.Overlay"),
	false,
	TEXT("Show or hide the Debug Overlay"),
	ECVF_Default
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDebugOverlaySubsystemEvent,
	const class UDebugOverlaySubsystem*, Sender);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDebugOverlaySubsystemAppEvent,
	const class UDebugOverlaySubsystem*, Sender,
	const class UDebugOverlay*, Overlay,
	class UDebugApp*, App);

/**
 * 
 */
UCLASS(Config=Debug, DefaultConfig)
class DEBUGOVERLAY_API UDebugOverlaySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Debug Overlay")
	void SetDebugOverlayEnabled(bool bEnable);

	UFUNCTION(BlueprintPure, Category="Debug Overlay")
	bool IsOverlayEnabled() const { return bOverlayEnabled; }

	FDebugOverlaySubsystemEvent& GetDebugOverlayOpened() { return DebugOverlayOpened; }
	FDebugOverlaySubsystemEvent& GetDebugOverlayClosed() { return DebugOverlayClosed; }
	FDebugOverlaySubsystemAppEvent& GetDebugAppOpened() { return DebugAppOpened; }
	FDebugOverlaySubsystemAppEvent& GetDebugAppClosed() { return DebugAppClosed; }
	
protected:
	// Begin override UWorldSubsystem
	void PostInitialize() override;
	// Begin override UWorldSubsystem

	UFUNCTION()
	void OnWorldDebugFrame();

	UFUNCTION()
	void OnDebugAppOpened(const UDebugOverlay* Sender, UDebugApp* DebugApp);

	UFUNCTION()
	void OnDebugAppClosed(const UDebugOverlay* Sender, UDebugApp* DebugApp);

	void OnDebugOverlayCommandChanged(IConsoleVariable* ConsoleVariable);
	
protected:
	UPROPERTY(BlueprintAssignable, Category="Debug Overlay")
	FDebugOverlaySubsystemEvent DebugOverlayOpened;

	UPROPERTY(BlueprintAssignable, Category="Debug Overlay")
	FDebugOverlaySubsystemEvent DebugOverlayClosed;

	UPROPERTY(BlueprintAssignable, Category="Debug Overlay")
	FDebugOverlaySubsystemAppEvent DebugAppOpened;

	UPROPERTY(BlueprintAssignable, Category="Debug Overlay")
	FDebugOverlaySubsystemAppEvent DebugAppClosed;

	UPROPERTY(Transient)
	TObjectPtr<UDebugOverlay> DebugOverlay;

	bool bOverlayEnabled = false;
};
