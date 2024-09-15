// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugOverlaySubsystem.h"

#include "DebugOverlay.h"
#include "DebugOverlayModule.h"
#include "DebugOverlaySettings.h"
#include "ImGuiDelegates.h"
#include "ImGuiModule.h"

void UDebugOverlaySubsystem::SetDebugOverlayEnabled(const bool bEnable)
{
	if (bEnable == bOverlayEnabled)
		return;

	if (!IsValid(DebugOverlay))
	{
		UE_LOG(LogDebugOverlay, Warning, TEXT("Debug Overlay Subsystem cannot be enabled as no Debug Overlay was created"));
		return;
	}

	bOverlayEnabled = bEnable;
	
	if (bOverlayEnabled)
	{
		FImGuiDelegates::OnWorldDebug(GetWorld()).AddUObject(this, &ThisClass::OnWorldDebugFrame);
		FImGuiModule::Get().GetProperties().SetInputEnabled(true);
		DebugOverlay->Open();
		DebugOverlayOpened.Broadcast(this);
	}
	else
	{
		FImGuiDelegates::OnWorldDebug(GetWorld()).RemoveAll(this);
		FImGuiModule::Get().GetProperties().SetInputEnabled(false);
		DebugOverlay->Close();
		DebugOverlayClosed.Broadcast(this);
	}
}

void UDebugOverlaySubsystem::PostInitialize()
{
	auto DebugOverlayClass = GetDefault<UDebugOverlaySettings>()->DebugOverlayClass;
	
	if (!IsValid(DebugOverlayClass))
	{
		UE_LOG(LogDebugOverlay, Warning, TEXT("Debug Overlay Class specified in Debug Overlay Subsystem is invalid. Debug Overlay will not be enabled"));
		return;
	}
	
	DebugOverlay = NewObject<UDebugOverlay>(this, DebugOverlayClass);
	check(DebugOverlay);

	DebugOverlay->GetDebugAppOpened().AddUniqueDynamic(this, &ThisClass::OnDebugAppOpened);
	DebugOverlay->GetDebugAppClosed().AddUniqueDynamic(this, &ThisClass::OnDebugAppClosed);
	DebugOverlay->Init();
	
	CVarDebugOverlay->OnChangedDelegate().AddUObject(this, &ThisClass::OnDebugOverlayCommandChanged);
}

void UDebugOverlaySubsystem::OnWorldDebugFrame()
{
	check(DebugOverlay);
	DebugOverlay->Render();
}

void UDebugOverlaySubsystem::OnDebugAppOpened(const UDebugOverlay* Sender, UDebugApp* DebugApp)
{
	GetDebugAppOpened().Broadcast(this, Sender, DebugApp);
}

void UDebugOverlaySubsystem::OnDebugAppClosed(const UDebugOverlay* Sender, UDebugApp* DebugApp)
{
	GetDebugAppClosed().Broadcast(this, Sender, DebugApp);
}

void UDebugOverlaySubsystem::OnDebugOverlayCommandChanged(IConsoleVariable* ConsoleVariable)
{
	SetDebugOverlayEnabled(CVarDebugOverlay.GetValueOnGameThread());
}
