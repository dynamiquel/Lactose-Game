#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

#include "Services/LactoseServicesLog.h"

ULactoseIdentityServiceSubsystem::ULactoseIdentityServiceSubsystem()
{
	SetServiceBaseUrl(TEXT("https://lactose.mookrata.ovh/identity"));
}

void ULactoseIdentityServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (bAutoLogin)
		Login();
}

void ULactoseIdentityServiceSubsystem::Login()
{
	if (GetLoggedInUserInfo().IsValid())
	{
		UE_LOG(LogLactoseIdentityService, Error, TEXT("Cannot log in as the User is already logged in"));
		return;
	}
	
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetUserRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("users"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnUserLoggedIn);

	auto GetUserRequest = MakeShared<FLactoseIdentityGetUserRequest>(TEXT("67026efde05aacf9d6c79af6"));
	LoggedInFuture = RestRequest->SetContentAsJsonAndSendAsync(GetUserRequest);

	UE_LOG(LogLactoseIdentityService, Verbose,
		TEXT("Sent a User Login Request for User ID '%s'"),
		*GetUserRequest->UserId);
}

void ULactoseIdentityServiceSubsystem::Logout()
{
	if (!GetLoggedInUserInfo().IsValid())
	{
		UE_LOG(LogLactoseIdentityService, Error, TEXT("Cannot log out the User as no User is logged in"));
		return;
	}

	LoggedInUserInfo.Reset();

	Lactose::Identity::Events::OnUserLoggedOut.Broadcast(*this);
}

ELactoseIdentityUserLoginStatus ULactoseIdentityServiceSubsystem::GetLoginStatus() const
{
	if (LoggedInUserInfo.IsValid())
		return ELactoseIdentityUserLoginStatus::LoggedIn;
	
	if (LoggedInFuture.IsValid())
		return ELactoseIdentityUserLoginStatus::LoggingIn;
	
	return ELactoseIdentityUserLoginStatus::NotLoggedIn;
}

void ULactoseIdentityServiceSubsystem::OnUserLoggedIn(TSharedRef<FGetUserRequest::FResponseContext> Context)
{
	// Atm, I'm only really using the future to know the status of a request.
	// I couldn't care less about its contents.
	LoggedInFuture.Reset();
	
	if (!Context->ResponseContent)
	{
		UE_CLOG(
			Context->RequestContent, LogLactoseIdentityService, Error,
			TEXT("Failed to log in User with ID '%s'"),
			*Context->RequestContent->UserId);
		
		UE_CLOG(
			!Context->RequestContent, LogLactoseIdentityService, Error,
			TEXT("Failed to log in User"));

		Lactose::Identity::Events::OnUserLoginFailed.Broadcast(*this);
		return;
	}
	
	LoggedInUserInfo = Context->ResponseContent;

	UE_LOG(
		LogLactoseIdentityService,
		Log,
		TEXT("User Logged In: ID '%s'; Name '%s'"),
		*Context->ResponseContent->Id,
		*Context->ResponseContent->DisplayName);

	Lactose::Identity::Events::OnUserLoggedIn.Broadcast(*this, LoggedInUserInfo.ToSharedRef());
}
