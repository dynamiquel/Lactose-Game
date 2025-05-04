// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Tasks/LactoseTasksTasksTab.h"

#include "SimpSubsystems.h"
#include "Api/Tasks/LactoseTasks.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Tasks/LactoseTasksDebugApp.h"
#include "Services/Tasks/LactoseTasksServiceSubsystem.h"

ULactoseTasksTasksTab::ULactoseTasksTasksTab()
{
	SetOwningAppClass<ULactoseTasksDebugApp>();
	SetTabName(TEXT("Tasks"));
}

void ULactoseTasksTasksTab::Init()
{
	TasksSubsystem = Subsystems::Get<ULactoseTasksServiceSubsystem>(self);
	EconomySubsystem = Subsystems::Get<ULactoseEconomyServiceSubsystem>(self);
}

void ULactoseTasksTasksTab::Render()
{
	if (!TasksSubsystem)
		return Debug::ImGui::Error("Tasks Subsystem could not be found");

	const ELactoseTasksTasksStatus EconomyItemsStatus = TasksSubsystem->GetTasksStatus();

	ImGui::Text("Status: ");
	ImGui::SameLine();
	
	switch (EconomyItemsStatus)
	{
		case ELactoseTasksTasksStatus::None:
			ImGui::Text("None");
			if (ImGui::Button("Load Tasks"))
				TasksSubsystem->LoadTasks();
			break;
		case ELactoseTasksTasksStatus::Querying:
			ImGui::Text("Querying");
			break;
		case ELactoseTasksTasksStatus::Retrieving:
			ImGui::Text("Retrieving");
			break;
		case ELactoseTasksTasksStatus::Loaded:
			ImGui::Text("Loaded");
			if (ImGui::Button("Load Tasks"))
				TasksSubsystem->LoadTasks();
			break;
		default:
			ImGui::Text("Unknown");
	}

	ImGui::Spacing();

	ImGui::BeginChild("Tasks", ImVec2(0, 0), /* bBorder*/ true);
	ON_SCOPE_EXIT
	{
		ImGui::EndChild();
	};

	const TMap<FString, Sr<FLactoseTasksGetTaskResponse>>& Tasks = TasksSubsystem->GetAllTasks();
	if (Tasks.IsEmpty())
	{
		ImGui::Text("No Tasks Found");
		return;
	}

	ItemsSearchBox.Draw();

	for (const auto& Task : Tasks)
	{
		const FString TaskLabel = FString::Printf(TEXT("%s (%s)"), *Task.Value->Id, *Task.Value->Name);

		if (!ItemsSearchBox.PassesFilter(TaskLabel))
			continue;
		
		if (ImGui::CollapsingHeader(STR_TO_ANSI(TaskLabel)))
		{
			ImGui::Indent();
			
			ImGui::Text("Id: %s", STR_TO_ANSI(Task.Value->Id));
			ImGui::Text("Name: %s", STR_TO_ANSI(Task.Value->Name));

			if (Task.Value->Description.IsSet())
			{
				ImGui::Text("Description:");
				ImGui::Indent();
				ImGui::TextWrapped("%s", STR_TO_ANSI(Task.Value->Description.GetValue()));
				ImGui::Unindent();
			}
			
			ImGui::Text("Required Progress: %f", Task.Value->RequiredProgress);

			if (!Task.Value->Rewards.IsEmpty())
			{
				ImGui::Text("Rewards:");
				ImGui::Indent();
				for (const FLactoseTasksItemRewardDto& Reward : Task.Value->Rewards)
				{
					FString RewardItemLabel = Reward.ItemId;
					
					if (EconomySubsystem)
						if (Sp<const FLactoseEconomyItem> FoundItem = EconomySubsystem->GetItem(Reward.ItemId))
							RewardItemLabel += FString::Printf(TEXT(" (%s)"), *FoundItem->Name);
					
					ImGui::Text("%d x %s", Reward.Quantity, STR_TO_ANSI(RewardItemLabel));
				}
				ImGui::Unindent();
			}

			ImGui::Unindent();
		}
	}
}
