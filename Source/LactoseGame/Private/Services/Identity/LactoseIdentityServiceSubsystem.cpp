#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

#include "Services/LactoseServicesLog.h"

ULactoseIdentityServiceSubsystem::ULactoseIdentityServiceSubsystem()
{
	SetServiceBaseUrl(TEXT("https://lactose.mookrata.ovh/identity"));
}

void ULactoseIdentityServiceSubsystem::Login()
{
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetUserRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("users"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnUserLoggedIn);

	auto GetUserRequest = MakeShared<FLactoseIdentityGetUserRequest>(TEXT("67026efde05aacf9d6c79af6"));
	RestRequest->SetContentAsJsonAndSendAsync(GetUserRequest);
}

void ULactoseIdentityServiceSubsystem::Logout()
{
	if (!GetLoggedInUserInfo().IsValid())
	{
		UE_LOG(LogLactoseIdentityService, Error, TEXT("Cannot log out the User as no User is logged in"));
		return;
	}

	LoggedInUserInfo.Reset();
}

void ULactoseIdentityServiceSubsystem::OnUserLoggedIn(TSharedRef<FGetUserRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent)
	{
		UE_CLOG(
			Context->RequestContent, LogLactoseIdentityService, Error,
			TEXT("Failed to log in User with ID '%s'"),
			*Context->RequestContent->UserId);
		
		UE_CLOG(
			!Context->RequestContent, LogLactoseIdentityService, Error,
			TEXT("Failed to log in User"));
		
		return;
	}
	
	LoggedInUserInfo = Context->ResponseContent;

	UE_LOG(
		LogLactoseIdentityService,
		Log,
		TEXT("User Logged In: ID '%s'; Name '%s'"),
		*Context->ResponseContent->Id,
		*Context->ResponseContent->DisplayName);
}
