
#include "Services/Tasks/LactoseTasksBP.h"

#include "Services/Tasks/LactoseTasksServiceSubsystem.h"

TMap<FString, FLactoseTasksGetTaskResponse> ULactoseTasksBP::GetTasks(const ULactoseTasksServiceSubsystem* Tasks)
{
	return Tasks ? Lactose::ExtractSpMap(Tasks->GetAllTasks()) : TMap<FString, FLactoseTasksGetTaskResponse>();
}

FLactoseTasksGetTaskResponse ULactoseTasksBP::GetTask(const ULactoseTasksServiceSubsystem* Tasks, const FString& TaskId)
{
	if (!Tasks)
		return {};

	Sp<const FLactoseTasksGetTaskResponse> FoundTask = Tasks->FindTask(TaskId);
	return FoundTask ? *FoundTask : FLactoseTasksGetTaskResponse();
}

bool ULactoseTasksBP::IsValidTask(const FLactoseTasksGetTaskResponse& Task)
{
	return !Task.Id.IsEmpty();
}

FString ULactoseTasksBP::GetTaskDescription(const FLactoseTasksGetTaskResponse& Task)
{
	return Task.Description.Get({});
}

TMap<FString, FLactoseTasksUserTaskDto> ULactoseTasksBP::GetCurrentUserTasks(const ULactoseTasksServiceSubsystem* Tasks)
{
	return Tasks ? Lactose::ExtractSpMap(Tasks->GetCurrentUserTasks()) : TMap<FString, FLactoseTasksUserTaskDto>();
}

FLactoseTasksUserTaskDto ULactoseTasksBP::GetCurrentUserTask(const ULactoseTasksServiceSubsystem* Tasks, const FString& UserTaskId)
{
	if (!Tasks)
		return {};

	Sp<const FLactoseTasksUserTaskDto> FoundUserTask = Tasks->FindCurrentUserTask(UserTaskId);
	return FoundUserTask ? *FoundUserTask : FLactoseTasksUserTaskDto();
}

FLactoseTasksUserTaskDto ULactoseTasksBP::GetCurrentUserTaskUsingTaskId(const ULactoseTasksServiceSubsystem* Tasks, const FString& TaskId)
{
	if (!Tasks)
		return {};

	Sp<const FLactoseTasksUserTaskDto> FoundUserTask = Tasks->FindCurrentUserTaskWithTaskId(TaskId);
	return FoundUserTask ? *FoundUserTask : FLactoseTasksUserTaskDto();
}

bool ULactoseTasksBP::IsValidUserTask(const FLactoseTasksUserTaskDto& UserTask)
{
	return !UserTask.Id.IsEmpty();
}

void ULactoseTasksCurrentUserTasksLoadedDelegateWrapper::OnSubscribed()
{
	NativeDelegateHandle = Lactose::Tasks::Events::OnCurrentUserTasksLoaded.AddUObject(this, &ThisClass::HandleNativeEvent);
}

void ULactoseTasksCurrentUserTasksLoadedDelegateWrapper::OnUnsubscribed()
{
	Lactose::Tasks::Events::OnCurrentUserTasksLoaded.Remove(NativeDelegateHandle);
}

void ULactoseTasksCurrentUserTasksLoadedDelegateWrapper::HandleNativeEvent(const ULactoseTasksServiceSubsystem& Sender)
{
	OnExecuted();
}
