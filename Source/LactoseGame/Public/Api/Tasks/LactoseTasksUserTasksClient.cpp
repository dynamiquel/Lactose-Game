//
// This file was generated by Catalyst's Unreal compiler at 02/05/2025 17:42:48.
// It is recommended not to modify this file. Modify the source spec file instead.
//

#include "LactoseTasksUserTasksClient.h"

#include "CatalystSubsystem.h"
#include "LactoseTasksUserTasks.h"

ULactoseTasksUserTasksClient::ULactoseTasksUserTasksClient()
{
    BaseUrl = TEXT("https://YouNeedToSet.Me");
}

TSharedRef<Lactose::Tasks::UserTasks::FQuery> ULactoseTasksUserTasksClient::Query(
    const FLactoseTasksQueryUserTasksRequest& Request,
    float Timeout)
{
    TArray<uint8> RequestBytes = Request.ToBytes();

    auto Operation = UCatalystSubsystem::Get().CreateOperation<Lactose::Tasks::UserTasks::FQuery>(
        BaseUrl + TEXT("/userTasks") + TEXT("/query"),
        Catalyst::Verbs::POST,
        MoveTemp(RequestBytes),
        Timeout == Timeout::Default ? DefaultTimeout : Timeout
    );

    return Operation;
}

TSharedRef<Lactose::Tasks::UserTasks::FGet> ULactoseTasksUserTasksClient::Get(
    const FLactoseTasksGetUserTasksRequest& Request,
    float Timeout)
{
    TArray<uint8> RequestBytes = Request.ToBytes();

    auto Operation = UCatalystSubsystem::Get().CreateOperation<Lactose::Tasks::UserTasks::FGet>(
        BaseUrl + TEXT("/userTasks") + TEXT("/get"),
        Catalyst::Verbs::POST,
        MoveTemp(RequestBytes),
        Timeout == Timeout::Default ? DefaultTimeout : Timeout
    );

    return Operation;
}

TSharedRef<Lactose::Tasks::UserTasks::FGetById> ULactoseTasksUserTasksClient::GetById(
    const FLactoseTasksGetUserTasksFromTaskIdRequest& Request,
    float Timeout)
{
    TArray<uint8> RequestBytes = Request.ToBytes();

    auto Operation = UCatalystSubsystem::Get().CreateOperation<Lactose::Tasks::UserTasks::FGetById>(
        BaseUrl + TEXT("/userTasks") + TEXT("/getById"),
        Catalyst::Verbs::POST,
        MoveTemp(RequestBytes),
        Timeout == Timeout::Default ? DefaultTimeout : Timeout
    );

    return Operation;
}
