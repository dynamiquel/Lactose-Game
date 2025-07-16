#include "Services/Tasks/LactoseTasksServiceSubsystem.h"

#include "Api/Tasks/LactoseTasksClient.h"
#include "Api/Tasks/LactoseTasksUserTasksClient.h"
#include "Mqtt/LactoseMqttSubsystem.h"
#include "Mqtt/MqttifyMessage.h"
#include "Services/LactoseServicesLog.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

ULactoseTasksServiceSubsystem::ULactoseTasksServiceSubsystem()
{
	SetServiceBaseUrl(TEXT("https://lactose.mookrata.ovh/tasks"));
}

void ULactoseTasksServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TasksClient = NewObject<ULactoseTasksTasksClient>(this);
	TasksClient->BaseUrl = GetServiceBaseUrl();
	UserTasksClient = NewObject<ULactoseTasksUserTasksClient>(this);
	UserTasksClient->BaseUrl = GetServiceBaseUrl();

	Lactose::Identity::Events::OnUserLoggedIn.AddUObject(this, &ThisClass::OnUserLoggedIn);
	Lactose::Identity::Events::OnUserLoggedOut.AddUObject(this, &ThisClass::OnUserLoggedOut);
}

Sp<const FLactoseTasksGetTaskResponse> ULactoseTasksServiceSubsystem::FindTask(const FString& TaskId) const
{
	const Sr<FLactoseTasksGetTaskResponse>* FoundTask = GetAllTasks().Find(TaskId);
	return FoundTask ? Sp<const FLactoseTasksGetTaskResponse>(*FoundTask) : nullptr;
}

Sp<const FLactoseTasksUserTaskDto> ULactoseTasksServiceSubsystem::FindCurrentUserTask(const FString& UserTaskId) const
{
	const Sr<FLactoseTasksUserTaskDto>* FoundUserTask = GetCurrentUserTasks().Find(UserTaskId);
	return FoundUserTask ? Sp<const FLactoseTasksUserTaskDto>(*FoundUserTask) : nullptr;
}

Sp<const FLactoseTasksUserTaskDto> ULactoseTasksServiceSubsystem::FindCurrentUserTaskWithTaskId(const FString& TaskId) const
{
	for (const TPair<FString, Sr<FLactoseTasksUserTaskDto>>& CurrentUserTask : GetCurrentUserTasks())
	{
		if (CurrentUserTask.Value->TaskId == TaskId)
			return CurrentUserTask.Value;
	}

	return nullptr;
}

void ULactoseTasksServiceSubsystem::LoadTasks()
{
	check(TasksClient);

	if (TasksStatus == ELactoseTasksTasksStatus::Querying || TasksStatus == ELactoseTasksTasksStatus::Retrieving)
	{
		UE_LOG(LogLactoseTasksService, Warning, TEXT("Requested to load Tasks but they are already being queried or retrieved"));
		return;
	}
	
	TasksStatus = ELactoseTasksTasksStatus::Querying;
	
	TSharedRef<Lactose::Tasks::FQuery> QueryOp = TasksClient->Query({});
	QueryOp->Then([WeakThis = MakeWeakObjectPtr(this)](TSharedRef<Lactose::Tasks::FQuery> Operation)
	{
		auto* This = WeakThis.Get();
		if (!This)
			return;

		if (Operation->IsError())
		{
			This->TasksStatus = ELactoseTasksTasksStatus::None;
			return;
		}

		UE_LOG(LogLactoseTasksService, Log, TEXT("Queried %d Tasks"), Operation->GetResponse()->TaskIds.Num());

		This->TasksStatus = ELactoseTasksTasksStatus::Retrieving;
		
		TSharedRef<Lactose::Tasks::FGet> GetOp = This->TasksClient->Get({.TaskIds = Operation->GetResponse()->TaskIds});
		GetOp->Then([WeakThis = MakeWeakObjectPtr(This)](TSharedRef<Lactose::Tasks::FGet> Operation)
		{
			auto* This = WeakThis.Get();
			if (!This)
				return;

			if (Operation->IsError())
			{
				This->TasksStatus = ELactoseTasksTasksStatus::None;
				return;
			}
			
			for (const FLactoseTasksGetTaskResponse& Task : Operation->GetResponse()->Tasks)
			{
				if (auto* ExistingTask = This->AllTasks.Find(Task.Id))
				{
					ExistingTask->Get() = Task;
					UE_LOG(LogLactoseTasksService, Log, TEXT("Updated Task with ID: %s"), *Task.Id);
				}
				else
				{
					This->AllTasks.Emplace(Task.Id, CreateSr(Task));
					UE_LOG(LogLactoseTasksService, Log, TEXT("Added Task with ID: %s"), *Task.Id);
				}
			}

			This->TasksStatus = ELactoseTasksTasksStatus::Loaded;
			Lactose::Tasks::Events::OnAllTasksLoaded.Broadcast(*This);
		});
	});
}

void ULactoseTasksServiceSubsystem::LoadCurrentUserTasks()
{
	check(TasksClient);

	if (UserTasksStatus == ELactoseTasksUserTasksStatus::Querying || UserTasksStatus == ELactoseTasksUserTasksStatus::Retrieving)
	{
		UE_LOG(LogLactoseTasksService, Warning, TEXT("Requested to load Current User Tasks but they are already being queried or retrieved"));
		return;
	}

	auto& IdentitySubsystem = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);
	if (IdentitySubsystem.GetLoginStatus() != ELactoseIdentityUserLoginStatus::LoggedIn)
	{
		UE_LOG(LogLactoseTasksService, Error, TEXT("Cannot load Current User Tasks as user not logged in"));
		return;
	}
	
	UserTasksStatus = ELactoseTasksUserTasksStatus::Querying;

	TSharedRef<Lactose::Tasks::UserTasks::FQuery> QueryOp = UserTasksClient->Query({
		.UserId = IdentitySubsystem.GetLoggedInUserInfo()->Id
	});
	QueryOp->Then([WeakThis = MakeWeakObjectPtr(this)](TSharedRef<Lactose::Tasks::UserTasks::FQuery> Operation)
	{
		auto* This = WeakThis.Get();
		if (!This)
			return;

		if (Operation->IsError())
		{
			This->UserTasksStatus = ELactoseTasksUserTasksStatus::None;
			return;
		}

		This->UserTasksStatus = ELactoseTasksUserTasksStatus::Retrieving;
		
		TSharedRef<Lactose::Tasks::UserTasks::FGet> GetOp = This->UserTasksClient->Get({
			.UserTaskIds = Operation->GetResponse()->UserTaskIds
		});
		GetOp->Then([WeakThis = MakeWeakObjectPtr(This)](TSharedRef<Lactose::Tasks::UserTasks::FGet> Operation)
		{
			auto* This = WeakThis.Get();
			if (!This)
				return;

			if (Operation->IsError())
			{
				This->UserTasksStatus = ELactoseTasksUserTasksStatus::None;
				return;
			}

			bool bAnyChanged = false;
			for (const FLactoseTasksUserTaskDto& UserTask : Operation->GetResponse()->UserTasks)
			{
				if (auto* ExistingTask = This->CurrentUserTasks.Find(UserTask.Id))
				{
					const bool bChanged = ExistingTask->Get().Completed != UserTask.Completed || !FMath::IsNearlyEqual(ExistingTask->Get().Progress, UserTask.Progress);
					if (bChanged)
					{
						bAnyChanged = true;
						ExistingTask->Get() = UserTask;
						UE_LOG(LogLactoseTasksService, Log, TEXT("Updated User Task with ID: %s"), *UserTask.Id);
						Lactose::Tasks::Events::OnCurrentUserTaskUpdated.Broadcast(*This, *ExistingTask);
					}
				}
				else
				{
					bAnyChanged = true;
					Sr<FLactoseTasksUserTaskDto> NewUserTask = CreateSr(UserTask);
					This->CurrentUserTasks.Emplace(UserTask.Id, NewUserTask);
					UE_LOG(LogLactoseTasksService, Log, TEXT("Added User Task with ID: %s"), *UserTask.Id);
					Lactose::Tasks::Events::OnCurrentUserTaskUpdated.Broadcast(*This, NewUserTask);
				}
			}

			This->UserTasksStatus = ELactoseTasksUserTasksStatus::Loaded;

			if (bAnyChanged)
				Lactose::Tasks::Events::OnCurrentUserTasksLoaded.Broadcast(*This);
		});
	});
}

void ULactoseTasksServiceSubsystem::OnUserLoggedIn(
	const ULactoseIdentityServiceSubsystem& Sender,
	const Sr<FLactoseIdentityGetUserResponse>& User)
{
	LoadTasks();
	LoadCurrentUserTasks();
	EnableGetUserTasksTicker();

	const FString& UserTaskUpdateTopic = FString::Printf(TEXT("/tasks/usertasks/%s/updated"), *User->Id);
	auto& Mqtt = Subsystems::GetRef<ULactoseMqttSubsystem>(self);
	Mqtt.RouteSubscription(UserTaskUpdateTopic, FMqttDelegate::CreateUObject(this, &ThisClass::OnCurrentUserTaskUpdated));
}

void ULactoseTasksServiceSubsystem::OnUserLoggedOut(const ULactoseIdentityServiceSubsystem& Sender)
{
	CurrentUserTasks.Reset();
}

void ULactoseTasksServiceSubsystem::EnableGetUserTasksTicker()
{
	GetGameInstance()->GetTimerManager().SetTimer(
		GetUserTasksTicker,
		this,
		&ThisClass::OnGetUserTasksTick,
		GetUserTasksTickInterval,
		/* bLoop */ true);
}

void ULactoseTasksServiceSubsystem::DisableGetUserTasksTicker()
{
	GetGameInstance()->GetTimerManager().ClearTimer(GetUserTasksTicker);
	GetUserTasksTicker.Invalidate();
}

void ULactoseTasksServiceSubsystem::OnGetUserTasksTick()
{
	if (GetCurrentUserTasksStatus() == ELactoseTasksUserTasksStatus::Querying || GetCurrentUserTasksStatus() == ELactoseTasksUserTasksStatus::Retrieving)
		return;

	LoadCurrentUserTasks();
}

void ULactoseTasksServiceSubsystem::OnCurrentUserTaskUpdated(const FMqttifyMessage& Message)
{
	TOptional<FLactoseTasksUserTaskUpdatedEvent> Event = FLactoseTasksUserTaskUpdatedEvent::FromBytes(Message.Payload);
	if (!Event)
	{
		UE_LOG(LogLactoseTasksService, Error, TEXT("Received incorrect event type for message. Topic: %s"), *Message.Topic);
		return;
	}

	if (UserTasksStatus == ELactoseTasksUserTasksStatus::Querying || UserTasksStatus == ELactoseTasksUserTasksStatus::Retrieving)
	{
		// Already doing a full load, no point.
		return;
	}

	UE_LOG(LogLactoseTasksService, Log, TEXT("Received User Task Update Event for User Task '%s'. Grabbing latest"), *Event->UserTaskId);

	check(UserTasksClient);

	TSharedRef<Lactose::Tasks::UserTasks::FGet> Operation = UserTasksClient->Get({
		.UserTaskIds = { Event->UserTaskId }
	});
	
	Operation->Then(ECallbackOn::Success, [WeakThis = MakeWeakObjectPtr(this)]
		(TSharedRef<Lactose::Tasks::UserTasks::FGet> Operation)
	{
		auto* This = WeakThis.Get();
		if (!This)
			return;

		// Bit copy and pasty but whatever.

		// Should only ever be one but jic.
		for (const FLactoseTasksUserTaskDto& UserTask : Operation->GetResponse()->UserTasks)
		{
			if (auto* ExistingTask = This->CurrentUserTasks.Find(UserTask.Id))
			{
				const bool bChanged = ExistingTask->Get().Completed != UserTask.Completed || !FMath::IsNearlyEqual(ExistingTask->Get().Progress, UserTask.Progress);
				if (bChanged)
				{
					ExistingTask->Get() = UserTask;
					UE_LOG(LogLactoseTasksService, Log, TEXT("Updated User Task with ID: %s"), *UserTask.Id);
					Lactose::Tasks::Events::OnCurrentUserTaskUpdated.Broadcast(*This, *ExistingTask);
				}
			}
			else
			{
				Sr<FLactoseTasksUserTaskDto> NewUserTask = CreateSr(UserTask);
				This->CurrentUserTasks.Emplace(UserTask.Id, NewUserTask);
				UE_LOG(LogLactoseTasksService, Log, TEXT("Added User Task with ID: %s"), *UserTask.Id);
				Lactose::Tasks::Events::OnCurrentUserTaskUpdated.Broadcast(*This, NewUserTask);
			}
		}

		Lactose::Tasks::Events::OnCurrentUserTasksLoaded.Broadcast(*This);
	});
}