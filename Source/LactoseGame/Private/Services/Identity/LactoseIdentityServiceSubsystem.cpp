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

	/*Lactose::Identity::Events::OnUserLoginFailed.AddLambda([WeakThis = MakeWeakObjectPtr(this)](const ULactoseIdentityServiceSubsystem& Sender)
	{
		if (auto* RestSubsystem = Subsystems::Get<ULactoseRestSubsystem>(WeakThis.Get()))
		{
			RestSubsystem->RemoveAuthorization();
		}
	});*/
}

void ULactoseIdentityServiceSubsystem::SignupUsingBasicAuth(
	const FString& DisplayName,
	const FString& Email,
	const FString& Password,
	TFunction<void()>&& SignupFailed)
{
	if (GetLoginStatus() != ELactoseIdentityUserLoginStatus::NotLoggedIn)
	{
		UE_LOG(LogLactoseIdentityService, Error, TEXT("Cannot register as the User is already logged in or logging in"));
		return;
	}

	auto SignupRequest = CreateSr(FLactoseIdentitySignupRequest
	{
		.DisplayName = DisplayName,
		.Email = Email,
		.Password = Password
	});

	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	RestSubsystem.RemoveAuthorization();
	
	auto RestRequest = FSignupRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("auth/signup"));
		
	SignupBasicAuthFuture = RestRequest->SetContentAsJsonAndSendAsync(SignupRequest);
	SignupBasicAuthFuture.Next([WeakThis = MakeWeakObjectPtr(this), SignupFailed = MoveTemp(SignupFailed)]
		(Sp<FSignupRequest::FResponseContext> Context)
	{
		auto* ThisPinned = WeakThis.Get();
		if (!ThisPinned)
			return;

		ThisPinned->SignupBasicAuthFuture.Reset();

		if (!Context.IsValid() || !Context->ResponseContent.IsValid())
		{
			Lactose::Identity::Events::OnUserLoginFailed.Broadcast(*ThisPinned);
			SignupFailed();
			return;
		}

		UE_LOG(LogLactoseIdentityService, Log,
			TEXT("Registered as '%s'. Access Token: %s\n"),
			*Context->ResponseContent->Id,
			*Context->ResponseContent->AccessToken);

		auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(*ThisPinned);
		RestSubsystem.AddAuthorization(
			Context->ResponseContent->AccessToken,
			Context->ResponseContent->RefreshToken.IsEmpty() ? nullptr : &Context->ResponseContent->RefreshToken);

		ThisPinned->StartRefreshTokenTimer();
		ThisPinned->LoadCurrentUser(Context->ResponseContent->Id);
	});
}

void ULactoseIdentityServiceSubsystem::LoginUsingBasicAuth(const FString& Username, const FString& Password)
{
	if (GetLoginStatus() != ELactoseIdentityUserLoginStatus::NotLoggedIn)
	{
		Log::Error(LogLactoseIdentityService, TEXT("Cannot log in as the User is already logged in or logging in"));
		return;
	}
	
	auto LoginRequest = CreateSr(FLactoseIdentityLoginRequest
	{
		.Email = Username,
		.Password = Password
	});

	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	RestSubsystem.RemoveAuthorization();

	auto RestRequest = FLoginRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("auth/login"));
		
	LoginUsingBasicAuthFuture = RestRequest->SetContentAsJsonAndSendAsync(LoginRequest);
	LoginUsingBasicAuthFuture.Next([WeakThis = MakeWeakObjectPtr(this)](Sp<FLoginRequest::FResponseContext> Context)
	{
		auto* ThisPinned = WeakThis.Get();
		if (!ThisPinned)
			return;

		ThisPinned->LoginUsingBasicAuthFuture.Reset();

		if (!Context.IsValid() || !Context->ResponseContent.IsValid())
		{
			Lactose::Identity::Events::OnUserLoginFailed.Broadcast(*ThisPinned);
			return;
		}

		UE_LOG(LogLactoseIdentityService, Log,
			TEXT("Logged in as '%s'. Access Token: %s\n"),
			*Context->ResponseContent->Id,
			*Context->ResponseContent->AccessToken);
			
		auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(*ThisPinned);
		RestSubsystem.AddAuthorization(
			Context->ResponseContent->AccessToken,
			Context->ResponseContent->RefreshToken.IsEmpty() ? nullptr : &Context->ResponseContent->RefreshToken);

		ThisPinned->StartRefreshTokenTimer();
		ThisPinned->LoadCurrentUser(Context->ResponseContent->Id);
	});
}

void ULactoseIdentityServiceSubsystem::LoginUsingRefreshToken(TFunction<void()>&& LoginFailed)
{
	if (GetLoginStatus() == ELactoseIdentityUserLoginStatus::LoggingIn || GetLoginStatus() == ELactoseIdentityUserLoginStatus::GettingUserInfo)
	{
		Log::Error(LogLactoseIdentityService, TEXT("Cannot log in as the User is already logging in"));
		return;
	}

	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	auto RestRequest = FRefreshTokenRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("auth/refresh"));

	const TOptional<FString>& RefreshToken = RestSubsystem.GetRefreshToken();

	auto Request = CreateSr(FLactoseIdentityRefreshTokenRequest
	{
		.RefreshToken = RefreshToken.IsSet() ? *RefreshToken : FString()
	});
	
	LoginUsingRefreshFuture = RestRequest->SetContentAsJsonAndSendAsync(Request);

	LoginUsingRefreshFuture.Next([WeakThis = MakeWeakObjectPtr(this), LoginFailed = MoveTemp(LoginFailed)](TSharedPtr<FRefreshTokenRequest::FResponseContext> Context)
	{		
		auto* ThisPinned = WeakThis.Get();
		if (!ThisPinned)
			return;

		ThisPinned->LoginUsingRefreshFuture.Reset();
		
		auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(ThisPinned);

		if (!Context.IsValid() || !Context->ResponseContent.IsValid())
		{
			Lactose::Identity::Events::OnUserLoginFailed.Broadcast(*ThisPinned);
			LoginFailed();
			return;
		}

		UE_LOG(LogLactoseIdentityService, Log,
			TEXT("Logged in as '%s'. Access Token: %s\n"),
			*Context->ResponseContent->Id,
			*Context->ResponseContent->AccessToken);

		RestSubsystem.AddAuthorization(
			Context->ResponseContent->AccessToken,
			Context->ResponseContent->RefreshToken.IsEmpty() ? nullptr : &Context->ResponseContent->RefreshToken);

		if (ThisPinned->LoggedInUserInfo && ThisPinned->LoggedInUserInfo->Id == Context->ResponseContent->Id)
		{
			// Already logged-in with this user. No need to do anything else as this was just an auth refresh.
			// Technically, it does mean Display Name and Roles could be out of date but don't worry about it.
			ThisPinned->StartRefreshTokenTimer();
			return;
		}
		
		// Just reset current user info so it gets reloaded.
		ThisPinned->LoggedInUserInfo.Reset();

		ThisPinned->StartRefreshTokenTimer();
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

	UE_LOG(LogLactoseIdentityService, Log,
		TEXT("Sent a User Login Request for User ID '%s'"),
		*GetUserRequest->UserId);
}

void ULactoseIdentityServiceSubsystem::Logout()
{
	if (!GetLoggedInUserInfo().IsValid())
	{
		UE_LOG(LogLactoseIdentityService, Error,
			TEXT("Cannot log out the User as no User is logged in"));
		return;
	}

	LoggedInUserInfo.Reset();
	
	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	RestSubsystem.RemoveAuthorization();
	StopRefreshTokenTimer();

	Lactose::Identity::Events::OnUserLoggedOut.Broadcast(self);
}

ELactoseIdentityUserLoginStatus ULactoseIdentityServiceSubsystem::GetLoginStatus() const
{
	if (LoggedInUserInfo.IsValid())
		return ELactoseIdentityUserLoginStatus::LoggedIn;
	
	if (LoginUsingRefreshFuture.IsValid() || LoginUsingBasicAuthFuture.IsValid() || SignupBasicAuthFuture.IsValid())
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

		auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
		RestSubsystem.RemoveAuthorization();

		Lactose::Identity::Events::OnUserLoginFailed.Broadcast(self);
		return;
	}
	
	LoggedInUserInfo = Context->ResponseContent;

	UE_LOG(LogLactoseIdentityService, Log,
		TEXT("User Logged In: ID '%s'; Name '%s'"),
		*Context->ResponseContent->Id,
		*Context->ResponseContent->DisplayName);

	Lactose::Identity::Events::OnUserLoggedIn.Broadcast(self, LoggedInUserInfo.ToSharedRef());
}

void ULactoseIdentityServiceSubsystem::StartRefreshTokenTimer()
{
	GetGameInstance()->GetTimerManager().SetTimer(
		RefreshTokenTimer,
		this, &ThisClass::OnRefreshTokenTimer,
		RefreshTokenInterval);

	UE_LOG(LogLactoseIdentityService, Log, TEXT("Next reauthentication scheduled in %.0f minutes"), RefreshTokenInterval / 60.f);
}

void ULactoseIdentityServiceSubsystem::StopRefreshTokenTimer()
{
	GetGameInstance()->GetTimerManager().ClearTimer(RefreshTokenTimer);
	UE_LOG(LogLactoseIdentityService, Log, TEXT("Stopped reauthentication schedule"));
}

void ULactoseIdentityServiceSubsystem::OnRefreshTokenTimer()
{
	/**
	 * TODO: Could be better.
	 * I.e.
	 *   - reschdule based on expiry time
	 *   - some kind of retry policy
	 */
	
	if (GetLoginStatus() != ELactoseIdentityUserLoginStatus::LoggedIn)
		return;

	UE_LOG(LogLactoseIdentityService, Log, TEXT("Reauthenticating User %s..."), *GetLoggedInUserInfo()->Id);

	LoginUsingRefreshToken([WeakThis = MakeWeakObjectPtr(this)]
	{
		// OnLoginFailed
		auto* This = WeakThis.Get();
		if (!This)
			return;

		This->StopRefreshTokenTimer();
		
		UE_LOG(LogLactoseIdentityService, Error, TEXT("Failed to reauthenticate using refresh token. Returning to Main Menu..."));
		This->GetGameInstance()->ReturnToMainMenu();
	});
}
