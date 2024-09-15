// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "IDebugOverlayDrawable.h"

#include "DebugApp.generated.h"

class UDebugOverlay;
class UDebugAppTab;

/**
 * A Debug App is a type of Window that can be shown in the Debug Overlay. You can draw custom elements on it or
 * take advantage of Debug App Tabs to automatically categorise debug info.
 */
UCLASS(Abstract, Config=Debug, DefaultConfig)
class DEBUGOVERLAY_API UDebugApp : public UObject, public IDebugOverlayDrawable
{
	GENERATED_BODY()

public:
	// Begin override IDebugDraw
	void Init() override;
	void Render() override;
	// End override IDebugDraw
	
	virtual bool ShouldCreateApp(const UDebugOverlay& Overlay) const { return bEnabled; }

	const FString& GetAppName() const { return AppName; }

	void SetAppName(FString&& InAppName);

protected:
	virtual void RegisterTabs();
	void RegisterTab(UDebugAppTab& AppTab);

	void RenderTabs();
	
protected:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bEnabled = true;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDebugAppTab>> AppTabs;

private:
	UPROPERTY(EditDefaultsOnly, Config)
	FString AppName;
};
