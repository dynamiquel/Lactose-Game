// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Simulation/LactoseSimulationStatusTab.h"
#include "DebugImGuiHelpers.h"
#include "Services/LactoseServiceDebugUtils.h"
#include "Services/Simulation/LactoseSimulationDebugApp.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

ULactoseSimulationStatusTab::ULactoseSimulationStatusTab()
{
	SetOwningAppClass<ULactoseSimulationDebugApp>();
	SetTabName(TEXT("Status"));
}

void ULactoseSimulationStatusTab::Init()
{
	SimulationSubsystem = UGameInstance::GetSubsystem<ULactoseSimulationServiceSubsystem>(GetWorld()->GetGameInstance());
	StatusSection = CreateSr<Lactose::Debug::Services::FStatusSection>(SimulationSubsystem);
}

void ULactoseSimulationStatusTab::Render()
{
	if (!IsValid(SimulationSubsystem))
		return Debug::ImGui::Error("Identity Subsystem could not be found");
	
	if (StatusSection)
		StatusSection->Render();
}
