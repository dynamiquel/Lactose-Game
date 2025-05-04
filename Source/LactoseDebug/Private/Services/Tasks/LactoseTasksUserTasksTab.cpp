
#include "Services/Tasks/LactoseTasksUserTasksTab.h"

#include "Api/Tasks/LactoseTasks.h"
#include "Api/Tasks/LactoseTasksUserTasks.h"
#include "Services/Tasks/LactoseTasksDebugApp.h"
#include "Services/Tasks/LactoseTasksServiceSubsystem.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"


ULactoseTasksUserTasksTab::ULactoseTasksUserTasksTab()
{
	SetOwningAppClass<ULactoseTasksDebugApp>();
	SetTabName(TEXT("User Tasks"));
}

void ULactoseTasksUserTasksTab::Init()
{
	TasksSubsystem = Subsystems::Get<ULactoseTasksServiceSubsystem>(self);
	EconomySubsystem = Subsystems::Get<ULactoseEconomyServiceSubsystem>(self);
}

void ULactoseTasksUserTasksTab::Render()
{
	if (!TasksSubsystem)
		return Debug::ImGui::Error("Tasks Subsystem could not be found");

	const ELactoseTasksUserTasksStatus CurrentUserTasksStatus = TasksSubsystem->GetCurrentUserTasksStatus();

	ImGui::Text("Status: ");
	ImGui::SameLine();
	
	switch (CurrentUserTasksStatus)
	{
		case ELactoseTasksUserTasksStatus::None:
			ImGui::Text("None");
			if (ImGui::Button("Load User Tasks"))
				TasksSubsystem->LoadCurrentUserTasks();
			break;
		case ELactoseTasksUserTasksStatus::Querying:
			ImGui::Text("Querying");
			break;
		case ELactoseTasksUserTasksStatus::Retrieving:
			ImGui::Text("Retrieving");
			break;
		case ELactoseTasksUserTasksStatus::Loaded:
			ImGui::Text("Loaded");
			if (ImGui::Button("Load User Tasks"))
				TasksSubsystem->LoadCurrentUserTasks();
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

	const TMap<FString, Sr<FLactoseTasksUserTaskDto>>& UserTasks = TasksSubsystem->GetCurrentUserTasks();
	if (UserTasks.IsEmpty())
	{
		ImGui::Text("No User Tasks Found");
		return;
	}

	ItemsSearchBox.Draw();

	for (const auto& UserTask : UserTasks)
	{
		Sp<const FLactoseTasksGetTaskResponse> FoundTask = TasksSubsystem->FindTask(UserTask.Value->TaskId);
		
		FString TaskLabel = UserTask.Value->Id;
		if (FoundTask.IsValid())
			TaskLabel += FString::Printf(TEXT(" (%s)"), *FoundTask->Name);

		if (!ItemsSearchBox.PassesFilter(TaskLabel) && !ItemsSearchBox.PassesFilter(UserTask.Value->TaskId))
			continue;
		
		if (ImGui::CollapsingHeader(STR_TO_ANSI(TaskLabel)))
		{
			ImGui::Indent();
			
			ImGui::Text("Id: %s", STR_TO_ANSI(UserTask.Value->Id));
			ImGui::Text("User Id: %s", STR_TO_ANSI(UserTask.Value->UserId));
			ImGui::Text("Task Id: %s", STR_TO_ANSI(UserTask.Value->TaskId));
			if (FoundTask.IsValid())
				ImGui::Text("Task Name: %s", STR_TO_ANSI(FoundTask->Name));

			FString ProgressText = FString::Printf(TEXT("Progress: %f"), UserTask.Value->Progress);
			if (FoundTask.IsValid())
				ProgressText += FString::Printf(TEXT(" / %f"), FoundTask->RequiredProgress); 
			
			ImGui::Text("%s", STR_TO_ANSI(ProgressText));
			ImGui::Text("Completed: %d", UserTask.Value->Completed);

			ImGui::Unindent();
		}
	}
}
