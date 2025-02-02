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
		LoginUsingRefreshToken([]{});
}

void ULactoseIdentityServiceSubsystem::LoginUsingBasicAuth(const FString& Username, const FString& Password)
{
	if (GetLoginStatus() != ELactoseIdentityUserLoginStatus::NotLoggedIn)
	{
		Log::Error(LogLactoseIdentityService, TEXT("Cannot log in as the User is already logged in"));
		return;
	}

	if (LoginUsingBasicAuthFuture.IsValid() || LoginUsingRefreshFuture.IsValid())
	{
		Log::Error(LogLactoseIdentityService, TEXT("Cannot log in as the User is already logging in"));
		return;
	}

	auto LoginRequest = CreateSr(FLactoseIdentityLoginRequest
		{
			.Email = Username,
			.Password = Password
		});

	LoginUsingRefreshToken([this, LoginRequest]()
	{
		auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
		auto RestRequest = FLoginRequest::Create(RestSubsystem);
		RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
		RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("auth/login"));
		
		LoginUsingBasicAuthFuture = RestRequest->SetContentAsJsonAndSendAsync(LoginRequest);
		LoginUsingBasicAuthFuture.Next([WeakThis = MakeWeakObjectPtr(this)](TSharedPtr<FLoginRequest::FResponseContext> Context)
		{
			auto* ThisPinned = WeakThis.Get();
			if (!ThisPinned)
				return;

			ThisPinned->LoginUsingBasicAuthFuture.Reset();

			if (!Context.IsValid() || !Context->ResponseContent.IsValid())
				return;

			Log::Log(LogLactoseIdentityService,
			TEXT("Logged in as '%s'. Access Token: %s\n"),
			*Context->ResponseContent->Id,
			*Context->ResponseContent->Token);

			auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(*ThisPinned);
			RestSubsystem.AddAuthorization(Context->ResponseContent->Token);
			
			ThisPinned->LoadCurrentUser(Context->ResponseContent->Id);
		});
	});
}

void ULactoseIdentityServiceSubsystem::LoginUsingRefreshToken(TFunction<void()>&& LoginFailed)
{
	if (GetLoginStatus() != ELactoseIdentityUserLoginStatus::NotLoggedIn)
	{
		Log::Error(LogLactoseIdentityService, TEXT("Cannot log in as the User is already logging in"));
		return;
	}

	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	auto RestRequest = FRefreshTokenRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("auth/refresh"));
	LoginUsingRefreshFuture = RestRequest->SetContentAsJsonAndSendAsync(CreateSr<FLactoseIdentityRefreshTokenRequest>());

	LoginUsingRefreshFuture.Next([WeakThis = MakeWeakObjectPtr(this), LoginFailed = MoveTemp(LoginFailed)](TSharedPtr<FRefreshTokenRequest::FResponseContext> Context)
	{		
		auto* ThisPinned = WeakThis.Get();
		if (!ThisPinned)
			return;

		ThisPinned->LoginUsingRefreshFuture.Reset();

		if (!Context.IsValid() || !Context->ResponseContent.IsValid())
		{
			LoginFailed();
			return;
		}

		Log::Log(LogLactoseIdentityService,
			TEXT("Logged in as '%s'. Access Token: %s\n"),
			*Context->ResponseContent->Id,
			*Context->ResponseContent->Token);

		auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(ThisPinned);
		RestSubsystem.AddAuthorization(Context->ResponseContent->Token);
		
		// Just reset current user info so it gets reloaded.
		ThisPinned->LoggedInUserInfo.Reset();
		
		ThisPinned->LoadCurrentUser(Context->ResponseContent->Id);
	});
}

void ULactoseIdentityServiceSubsystem::LoadCurrentUser(const FString& UserId)
{
	if (GetLoginStatus() == ELactoseIdentityUserLoginStatus::LoggedIn)
	{
		Log::Error(LogLactoseIdentityService, TEXT("Cannot log in as the User is already logged in"));
		return;
	}
	
	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	auto RestRequest = FGetUserRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("users"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnUserLoggedIn);

	auto GetUserRequest = CreateSr(FLactoseIdentityGetUserRequest
	{
		.UserId = UserId
	});
	
	CurrentUserInfoFuture = RestRequest->SetContentAsJsonAndSendAsync(GetUserRequest);

	Log::Verbose(LogLactoseIdentityService,
		TEXT("Sent a User Login Request for User ID '%s'"),
		*GetUserRequest->UserId);
}

void ULactoseIdentityServiceSubsystem::Logout()
{
	if (!GetLoggedInUserInfo().IsValid())
	{
		Log::Error(LogLactoseIdentityService, TEXT("Cannot log out the User as no User is logged in"));
		return;
	}

	LoggedInUserInfo.Reset();
	
	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	RestSubsystem.RemoveAuthorization();

	Lactose::Identity::Events::OnUserLoggedOut.Broadcast(self);
}

ELactoseIdentityUserLoginStatus ULactoseIdentityServiceSubsystem::GetLoginStatus() const
{
	if (LoggedInUserInfo.IsValid())
		return ELactoseIdentityUserLoginStatus::LoggedIn;
	
	if (LoginUsingRefreshFuture.IsValid() || LoginUsingBasicAuthFuture.IsValid())
		return ELactoseIdentityUserLoginStatus::LoggingIn;

	if (CurrentUserInfoFuture.IsValid())
		return ELactoseIdentityUserLoginStatus::GettingUserInfo;
	
	return ELactoseIdentityUserLoginStatus::NotLoggedIn;
}

void ULactoseIdentityServiceSubsystem::OnUserLoggedIn(Sr<FGetUserRequest::FResponseContext> Context)
{
	// Atm, I'm only really using the future to know the status of a request.
	// I couldn't care less about its contents.
	CurrentUserInfoFuture.Reset();
	
	if (!Context->ResponseContent)
	{
		UE_CLOG(
			Context->RequestContent, LogLactoseIdentityService, Error,
			TEXT("Failed to log in User with ID '%s'"),
			*Context->RequestContent->UserId);
		
		UE_CLOG(
			!Context->RequestContent, LogLactoseIdentityService, Error,
			TEXT("Failed to log in User"));

		Lactose::Identity::Events::OnUserLoginFailed.Broadcast(self);
		return;
	}
	
	LoggedInUserInfo = Context->ResponseContent;

	Log::Log(LogLactoseIdentityService,
		TEXT("User Logged In: ID '%s'; Name '%s'"),
		*Context->ResponseContent->Id,
		*Context->ResponseContent->DisplayName);

	Lactose::Identity::Events::OnUserLoggedIn.Broadcast(self, LoggedInUserInfo.ToSharedRef());
}
