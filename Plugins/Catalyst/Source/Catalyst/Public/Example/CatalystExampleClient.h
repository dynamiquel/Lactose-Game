#pragma once

#include "CoreMinimal.h"
#include "CatalystClient.h"
#include "CatalystOperation.h"
#include "CatalystExampleClient.generated.h"

struct FCatalystExampleGetRoleResponse;
struct FCatalystExampleGetRolesResponse;
struct FCatalystExampleQueryRolesResponse;
struct FCatalystExampleGetUserResponse;
struct FCatalystExampleQueryUsersResponse;

namespace Catalyst::Example
{
	using FQueryUsers = TCatalystOperation<FCatalystExampleQueryUsersResponse>;
	using FGetUser = TCatalystOperation<FCatalystExampleGetUserResponse>;
	using FQueryRoles = TCatalystOperation<FCatalystExampleQueryRolesResponse>;
	using FGetRoles = TCatalystOperation<FCatalystExampleGetRolesResponse>;
	using FCreateRoles = TCatalystOperation<FCatalystExampleGetRoleResponse>;
}

/**
 * 
 */
UCLASS(Config=Catalyst, DefaultConfig)
class CATALYST_API UCatalystExampleClient : public UCatalystClient
{
	GENERATED_BODY()

public:
	UCatalystExampleClient();
	
	TSharedRef<Catalyst::Example::FQueryUsers> QueryUsers(const struct FCatalystExampleQueryUsersRequest& Request, float Timeout = Timeout::Default);
	
	TSharedRef<Catalyst::Example::FGetUser> GetUser(const struct FCatalystExampleGetUserRequest& Request, float Timeout = Timeout::Default);
	
	TSharedRef<Catalyst::Example::FQueryRoles> QueryRoles(const struct FCatalystExampleQueryRolesRequest& Request, float Timeout = Timeout::Default);
	
	TSharedRef<Catalyst::Example::FGetRoles> GetRoles(const struct FCatalystExampleGetRolesRequest& Request, float Timeout = Timeout::Default);
	
	TSharedRef<Catalyst::Example::FCreateRoles> CreateRole(const struct FCatalystExampleCreateRoleRequest& Request, float Timeout = Timeout::Default);
};
