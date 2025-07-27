#pragma once

#include "CoreMinimal.h"
#include "Simp.h"
#include "Api/Tasks/LactoseTasks.h"
#include "Api/Tasks/LactoseTasksUserTasks.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LactoseGame/LactoseNativeDelegateWrapper.h"
#include "LactoseTasksBP.generated.h"

class ULactoseTasksServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseTasksBP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, Category = "Lactose Tasks", meta=(WorldContext="WorldContextObject"))
	static TMap<FString, FLactoseTasksGetTaskResponse> GetTasks(const ULactoseTasksServiceSubsystem* Tasks);

	UFUNCTION(BlueprintPure, Category = "Lactose Tasks", meta=(WorldContext="WorldContextObject"))
	static FLactoseTasksGetTaskResponse GetTask(const ULactoseTasksServiceSubsystem* Tasks, const FString& TaskId);

	UFUNCTION(BlueprintPure, Category = "Lactose Tasks")
	static bool IsValidTask(const FLactoseTasksGetTaskResponse& Task);

	// BP doesn't support optional, which is what Description is.
	UFUNCTION(BlueprintPure, Category = "Lactose Tasks")
	static FString GetTaskDescription(const FLactoseTasksGetTaskResponse& Task);
	
	UFUNCTION(BlueprintPure, Category = "Lactose Tasks", meta=(WorldContext="WorldContextObject"))
	static TMap<FString, FLactoseTasksUserTaskDto> GetCurrentUserTasks(const ULactoseTasksServiceSubsystem* Tasks);

	UFUNCTION(BlueprintPure, Category = "Lactose Tasks", meta=(WorldContext="WorldContextObject"))
	static FLactoseTasksUserTaskDto GetCurrentUserTask(const ULactoseTasksServiceSubsystem* Tasks, const FString& UserTaskId);

	UFUNCTION(BlueprintPure, Category = "Lactose Tasks", meta=(WorldContext="WorldContextObject"))
	static FLactoseTasksUserTaskDto GetCurrentUserTaskUsingTaskId(const ULactoseTasksServiceSubsystem* Tasks, const FString& TaskId);

	UFUNCTION(BlueprintPure, Category = "Lactose Tasks")
	static bool IsValidUserTask(const FLactoseTasksUserTaskDto& UserTask);

	// BP doesn't support optional, which is what CompleteTime is.
	UFUNCTION(BlueprintPure, Category = "Lactose Tasks")
	static FDateTime GetCompleteTime(const FLactoseTasksUserTaskDto& UserTask);
};


UCLASS()
class ULactoseTasksCurrentUserTasksLoadedDelegateWrapper : public ULactoseNativeDelegateWrapper
{
	GENERATED_BODY()

public:
	void OnSubscribed() override;
	void OnUnsubscribed() override;
	
private:
	void HandleNativeEvent(const ULactoseTasksServiceSubsystem& Sender);
};

UCLASS()
class ULactoseTasksCurrentUserTaskUpdatedDelegateWrapper : public ULactoseNativeDelegateWrapper
{
	GENERATED_BODY()

public:
	void OnSubscribed() override;
	void OnUnsubscribed() override;

	UFUNCTION(BlueprintPure)
	FLactoseTasksUserTaskDto GetUserTask() const;

	UFUNCTION(BlueprintPure)
	FLactoseTasksGetTaskResponse GetTask() const;
	
private:
	void HandleNativeEvent(
		const ULactoseTasksServiceSubsystem& Sender,
		const Sr<const FLactoseTasksUserTaskDto>& UserTask);

	Sp<const FLactoseTasksUserTaskDto> LastUserTask;
	Sp<const FLactoseTasksGetTaskResponse> LastTask;
};