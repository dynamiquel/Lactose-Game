#include "Example/CatalystExampleActor.h"

#include "Example/CatalystExampleClient.h"
#include "Example/CatalystExampleRoles.h"

ACatalystExampleActor::ACatalystExampleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 5.f;
}

void ACatalystExampleActor::BeginPlay()
{
	Super::BeginPlay();

	Client = NewObject<UCatalystExampleClient>(this);
}

void ACatalystExampleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	check(Client);

	TSharedRef<Catalyst::Example::FQueryRoles> QueryRolesOp = Client->QueryRoles(FCatalystExampleQueryRolesRequest());
	QueryRolesOp->Then([this](TSharedRef<Catalyst::Example::FQueryRoles> Operation)
	{
		TSharedPtr<FCatalystExampleQueryRolesResponse> Response = Operation->GetResponse();
		if (!Response.IsValid())
		{
			// Invalid Response == Error
			check(Operation->IsError());
			TSharedPtr<IHttpResponse> Error = Operation->GetError();
			check(Error);
			
			UE_LOG(LogTemp, Error, TEXT("Received an invalid response from Query Users"));
			UE_LOG(LogTemp, Error, TEXT("Code: %d - Str: %s"), Error->GetResponseCode(), *Error->GetContentAsString());
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("Queried %d roles"), Response->RoleIds.Num());
		
		TSharedRef<Catalyst::Example::FGetRoles> GetRolesOp = Client->GetRoles(FCatalystExampleGetRolesRequest{
			.RoleIds = Response->RoleIds
		});
		GetRolesOp->Then(ECallbackOn::Success, this, &ACatalystExampleActor::OnRolesReceived);
	});
}

void ACatalystExampleActor::OnRolesReceived(TSharedRef<Catalyst::Example::FGetRoles> Operation)
{
	// Should always be success do the prior callback condition.
	check(Operation->IsSuccess());

	TSharedPtr<FCatalystExampleGetRolesResponse> Response = Operation->GetResponse();

	// Success = Valid Response
	check(Response.IsValid());

	UE_LOG(LogTemp, Log, TEXT("Received %d roles"), Response->Roles.Num());
	for (const FCatalystExampleGetRoleResponse& Role : Response->Roles)
		UE_LOG(LogTemp, Log, TEXT("Role: %s"), *Role.RoleName);
}

