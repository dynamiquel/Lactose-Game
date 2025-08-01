//
// This file was generated by Catalyst's Unreal compiler at 02/05/2025 17:42:48.
// It is recommended not to modify this file. Modify the source spec file instead.
//

#pragma once

#include "CatalystClient.h"
#include "CatalystOperation.h"
#include "LactoseIdentityRolesDtosRoles.h"
#include "Templates/SharedPointer.h"

#include "LactoseIdentityRolesDtosRolesClient.generated.h"


namespace Lactose::Identity::Roles::Dtos::Roles
{
    using FQueryRoles = TCatalystOperation<FLactoseIdentityRolesDtosQueryRolesResponse>;
    using FGetRoles = TCatalystOperation<FLactoseIdentityRolesDtosGetRolesResponse>;
    using FCreateRoleRequest = TCatalystOperation<FLactoseIdentityRolesDtosGetRoleResponse>;
}

UCLASS(Config=Catalyst, DefaultConfig)
class ULactoseIdentityRolesDtosRolesClient : public UCatalystClient
{
    GENERATED_BODY()

public:
    ULactoseIdentityRolesDtosRolesClient();

    TSharedRef<Lactose::Identity::Roles::Dtos::Roles::FQueryRoles> QueryRoles(
        const FLactoseIdentityRolesDtosQueryRolesRequest& Request,
        float Timeout = Timeout::Default);

    TSharedRef<Lactose::Identity::Roles::Dtos::Roles::FGetRoles> GetRoles(
        const FLactoseIdentityRolesDtosGetRolesRequest& Request,
        float Timeout = Timeout::Default);

    TSharedRef<Lactose::Identity::Roles::Dtos::Roles::FCreateRoleRequest> CreateRoleRequest(
        const FLactoseIdentityRolesDtosCreateRoleRequest& Request,
        float Timeout = Timeout::Default);
};
