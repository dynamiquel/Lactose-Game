#pragma once

#include "CoreMinimal.h"
#include "Services/LactoseServiceSubsystem.h"
#include "LactoseTasksServiceSubsystem.generated.h"

struct FMqttifyMessage;
struct FLactoseTasksGetTaskResponse;
struct FLactoseTasksUserTaskDto;
struct FLactoseIdentityGetUserResponse;
class ULactoseIdentityServiceSubsystem;
class ULactoseTasksTasksClient;
class ULactoseTasksUserTasksClient;

UENUM()
enum class ELactoseTasksTasksStatus
{
	None,
	Querying,
	Retrieving,
	Loaded
};

UENUM()
enum class ELactoseTasksUserTasksStatus
{
	None,
	Querying,
	Retrieving,
	Loaded
};

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseTasksServiceSubsystem : public ULactoseServiceSubsystem
{
	GENERATED_BODY()

	ULactoseTasksServiceSubsystem();
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	ELactoseTasksTasksStatus GetTasksStatus() const { return TasksStatus; }
	const TMap<FString, Sr<FLactoseTasksGetTaskResponse>>& GetAllTasks() const { return AllTasks; }
	Sp<const FLactoseTasksGetTaskResponse> FindTask(const FString& TaskId) const;

	ELactoseTasksUserTasksStatus GetCurrentUserTasksStatus() const { return UserTasksStatus; }
	const TMap<FString, Sr<FLactoseTasksUserTaskDto>>& GetCurrentUserTasks() const { return CurrentUserTasks; }
	Sp<const FLactoseTasksUserTaskDto> FindCurrentUserTask(const FString& UserTaskId) const;
	Sp<const FLactoseTasksUserTaskDto> FindCurrentUserTaskWithTaskId(const FString& TaskId) const;

	void LoadTasks();
	void LoadCurrentUserTasks();
	
protected:
	void OnUserLoggedIn(
		const ULactoseIdentityServiceSubsystem& Sender,
		const Sr<FLactoseIdentityGetUserResponse>& User);

	void OnUserLoggedOut(const ULactoseIdentityServiceSubsystem& Sender);
	
	void EnableGetUserTasksTicker();
	void DisableGetUserTasksTicker();
	bool IsAutoGetUserTasksTicking() const { return GetUserTasksTicker.IsValid(); }
	void OnGetUserTasksTick();

	void OnCurrentUserTaskUpdated(const FMqttifyMessage& Message);

protected:
	// Very low since it uses events to update.
	// Poll all user tasks every so often for sanity.
	UPROPERTY(EditDefaultsOnly, Config)
	float GetUserTasksTickInterval = 60.f;
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseTasksTasksClient> TasksClient;

	UPROPERTY(Transient)
	TObjectPtr<ULactoseTasksUserTasksClient> UserTasksClient;

	ELactoseTasksTasksStatus TasksStatus = ELactoseTasksTasksStatus::None;
	ELactoseTasksUserTasksStatus UserTasksStatus = ELactoseTasksUserTasksStatus::None;

	TMap<FString, Sr<FLactoseTasksGetTaskResponse>> AllTasks;
	TMap<FString, Sr<FLactoseTasksUserTaskDto>> CurrentUserTasks;

	FTimerHandle GetUserTasksTicker;
};

namespace Lactose::Tasks::Events
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FAllTasksLoaded,
		const ULactoseTasksServiceSubsystem& /* Sender */);

	DECLARE_MULTICAST_DELEGATE_OneParam(FCurrentUserTasksLoaded,
		const ULactoseTasksServiceSubsystem& /* Sender */);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FCurrentUserTaskUpdated,
		const ULactoseTasksServiceSubsystem& /* Sender */,
		const Sr<const FLactoseTasksUserTaskDto>& /* UserTask */);

	inline FAllTasksLoaded OnAllTasksLoaded;
	inline FCurrentUserTasksLoaded OnCurrentUserTasksLoaded;
	inline FCurrentUserTaskUpdated OnCurrentUserTaskUpdated;
}
