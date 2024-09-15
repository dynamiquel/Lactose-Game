// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugOverlaySettings.h"

#include "DebugOverlay.h"

UDebugOverlaySettings::UDebugOverlaySettings()
{
	DebugOverlayClass = UDebugOverlay::StaticClass();
	AppBarButtonDimensions = FVector2D(80, 80);
	AppOpenedButtonColour = FColor(153, 170, 187);
	AppBarPadding = FVector2D(16, 16);
}
