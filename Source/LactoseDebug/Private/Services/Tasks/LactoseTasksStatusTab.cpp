// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Tasks/LactoseTasksStatusTab.h"

#include "DebugImGuiHelpers.h"
#include "Services/Tasks/LactoseTasksDebugApp.h"
#include "Services/Tasks/LactoseTasksServiceSubsystem.h"

ULactoseTasksStatusTab::ULactoseTasksStatusTab()
{
	SetOwningAppClass<ULactoseTasksDebugApp>();
	SetTabName(TEXT("Status"));
}

void ULactoseTasksStatusTab::Init()
{
	TasksSubsystem = Subsystems::Get<ULactoseTasksServiceSubsystem>(self);
	StatusSection = CreateSr<Lactose::Debug::Services::FStatusSection>(TasksSubsystem);
}

void ULactoseTasksStatusTab::Render()
{
	if (!IsValid(TasksSubsystem))
		return Debug::ImGui::Error("Identity Subsystem could not be found");
	
	if (StatusSection)
		StatusSection->Render();
}
