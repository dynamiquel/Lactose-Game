//
// This file was generated by Catalyst's Unreal compiler at 02/05/2025 17:42:48.
// It is recommended not to modify this file. Modify the source spec file instead.
//

#pragma once

#include "CatalystClient.h"
#include "CatalystOperation.h"
#include "LactoseTasksUserTasks.h"
#include "Templates/SharedPointer.h"

#include "LactoseTasksUserTasksClient.generated.h"


namespace Lactose::Tasks::UserTasks
{
    using FQuery = TCatalystOperation<FLactoseTasksQueryUserTasksResponse>;
    using FGet = TCatalystOperation<FLactoseTasksGetUserTasksResponse>;
    using FGetById = TCatalystOperation<FLactoseTasksGetUserTasksResponse>;
}

UCLASS(Config=Catalyst, DefaultConfig)
class ULactoseTasksUserTasksClient : public UCatalystClient
{
    GENERATED_BODY()

public:
    ULactoseTasksUserTasksClient();

    TSharedRef<Lactose::Tasks::UserTasks::FQuery> Query(
        const FLactoseTasksQueryUserTasksRequest& Request,
        float Timeout = Timeout::Default);

    TSharedRef<Lactose::Tasks::UserTasks::FGet> Get(
        const FLactoseTasksGetUserTasksRequest& Request,
        float Timeout = Timeout::Default);

    TSharedRef<Lactose::Tasks::UserTasks::FGetById> GetById(
        const FLactoseTasksGetUserTasksFromTaskIdRequest& Request,
        float Timeout = Timeout::Default);
};
