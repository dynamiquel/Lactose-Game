#include "Example/CatalystExampleClient.h"

#include "CatalystSubsystem.h"
#include "Example/CatalystExampleRoles.h"
#include "Example/CatalystExampleUsers.h"

UCatalystExampleClient::UCatalystExampleClient()
{
	BaseUrl = "https://lactose.mookrata.ovh/identity";
}

TSharedRef<Catalyst::Example::FQueryUsers> UCatalystExampleClient::QueryUsers(
	const FCatalystExampleQueryUsersRequest& Request,
	float Timeout)
{
	TArray<uint8> RequestBytes = Request.ToBytes();

	auto Operation = UCatalystSubsystem::Get().CreateOperation<Catalyst::Example::FQueryUsers>(
		FPaths::Combine(BaseUrl, "users", "query"),
		Catalyst::Verbs::POST,
		MoveTemp(RequestBytes),
		Timeout == Timeout::Default ? DefaultTimeout : Timeout);

	return Operation;
}

TSharedRef<Catalyst::Example::FGetUser> UCatalystExampleClient::GetUser(
	const FCatalystExampleGetUserRequest& Request,
	float Timeout)
{
	TArray<uint8> RequestBytes = Request.ToBytes();

	auto Operation = UCatalystSubsystem::Get().CreateOperation<Catalyst::Example::FGetUser>(
		FPaths::Combine(BaseUrl, "users"),
		Catalyst::Verbs::POST,
		MoveTemp(RequestBytes),
		Timeout == Timeout::Default ? DefaultTimeout : Timeout);

	return Operation;
}

TSharedRef<Catalyst::Example::FQueryRoles> UCatalystExampleClient::QueryRoles(
	const FCatalystExampleQueryRolesRequest& Request,
	float Timeout)
{
	TArray<uint8> RequestBytes = Request.ToBytes();

	auto Operation = UCatalystSubsystem::Get().CreateOperation<Catalyst::Example::FQueryRoles>(
		FPaths::Combine(BaseUrl, "roles", "query"),
		Catalyst::Verbs::POST,
		MoveTemp(RequestBytes),
		Timeout == Timeout::Default ? DefaultTimeout : Timeout);

	return Operation;
}

TSharedRef<Catalyst::Example::FGetRoles> UCatalystExampleClient::GetRoles(
	const FCatalystExampleGetRolesRequest& Request,
	float Timeout)
{
	TArray<uint8> RequestBytes = Request.ToBytes();

	auto Operation = UCatalystSubsystem::Get().CreateOperation<Catalyst::Example::FGetRoles>(
		FPaths::Combine(BaseUrl, "roles"),
		Catalyst::Verbs::POST,
		MoveTemp(RequestBytes),
		Timeout == Timeout::Default ? DefaultTimeout : Timeout);

	return Operation;
}

TSharedRef<Catalyst::Example::FCreateRoles> UCatalystExampleClient::CreateRole(
	const FCatalystExampleCreateRoleRequest& Request,
	float Timeout)
{
	TArray<uint8> RequestBytes = Request.ToBytes();

	auto Operation = UCatalystSubsystem::Get().CreateOperation<Catalyst::Example::FCreateRoles>(
		FPaths::Combine(BaseUrl, "roles", "create"),
		Catalyst::Verbs::POST,
		MoveTemp(RequestBytes),
		Timeout == Timeout::Default ? DefaultTimeout : Timeout);

	return Operation;
}
