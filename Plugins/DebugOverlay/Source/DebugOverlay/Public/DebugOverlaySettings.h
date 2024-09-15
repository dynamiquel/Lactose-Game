#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "DebugOverlaySettings.generated.h"

class UDebugOverlay;

USTRUCT()
struct DEBUGOVERLAY_API FDebugOverlayStylingSizes
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FIntVector2 GlobalPadding = FIntVector2(8, 5);

	UPROPERTY(EditDefaultsOnly)
	float GlobalRounding = 8;

	UPROPERTY(EditDefaultsOnly)
	float TabRounding = 6;

	UPROPERTY(EditDefaultsOnly)
	int32 ScrollbarSize = 16;
	
	UPROPERTY(EditDefaultsOnly)
	int32 GrabSize = 12;
};

UCLASS(Config=Debug, DefaultConfig)
class DEBUGOVERLAY_API UDebugOverlaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

	UDebugOverlaySettings();

public:
	static const UDebugOverlaySettings& Get() { return *GetDefault<ThisClass>(); }

public:
	UPROPERTY(EditDefaultsOnly, Config, Category="Debug Overlay")
	TSubclassOf<UDebugOverlay> DebugOverlayClass;
	
	UPROPERTY(EditDefaultsOnly, Config, Category="Debug Overlay")
	FDebugOverlayStylingSizes Sizes;

	UPROPERTY(EditDefaultsOnly, Config, Category="Debug Overlay")
	FVector2D AppBarButtonDimensions;

	UPROPERTY(EditDefaultsOnly, Config, Category="Debug Overlay")
	FLinearColor AppOpenedButtonColour;

	UPROPERTY(EditDefaultsOnly, Config, Category="Debug Overlay")
	FVector2D AppBarPadding;

	UPROPERTY(EditDefaultsOnly, Config, Category="Debug Overlay")
	FFilePath CustomFont;
};
