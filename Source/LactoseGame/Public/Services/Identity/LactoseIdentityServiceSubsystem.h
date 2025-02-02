#pragma once

#include <CoreMinimal.h>

#include "Services/LactoseServiceSubsystem.h"

#include "LactoseIdentityServiceSubsystem.generated.h"

USTRUCT()
struct FLactoseIdentityGetUserRequest
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString UserId;
};

USTRUCT()
struct FLactoseIdentityGetUserResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString Id;

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	TArray<FString> Roles;

	UPROPERTY()
	FDateTime TimeCreated;
	
	UPROPERTY()
	FDateTime TimeLastLoggedIn;
};

USTRUCT()
struct FLactoseIdentityLoginRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString Email;

	UPROPERTY()
	FString Password;
};

USTRUCT()
struct FLactoseIdentityLoginResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString Id;

	UPROPERTY()
	FString AccessToken;

	UPROPERTY()
	FString RefreshToken;
};

USTRUCT()
struct FLactoseIdentitySignupRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString DisplayName;

	UPROPERTY()
	FString Email;

	UPROPERTY()
	FString Password;
};

USTRUCT()
struct FLactoseIdentityRefreshTokenRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString RefreshToken;
};

using FGetUserRequest = Lactose::Rest::TRequest<FLactoseIdentityGetUserRequest, FLactoseIdentityGetUserResponse>;
using FLoginRequest = Lactose::Rest::TRequest<FLactoseIdentityLoginRequest, FLactoseIdentityLoginResponse>;
using FSignupRequest = Lactose::Rest::TRequest<FLactoseIdentitySignupRequest, FLactoseIdentityLoginResponse>;
using FRefreshTokenRequest = Lactose::Rest::TRequest<FLactoseIdentityRefreshTokenRequest, FLactoseIdentityLoginResponse>;

UENUM(BlueprintType)
enum class ELactoseIdentityUserLoginStatus : uint8
{
	NotLoggedIn,
	LoggingIn,
	GettingUserInfo,
	LoggedIn
};

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseIdentityServiceSubsystem : public ULactoseServiceSubsystem
{
	GENERATED_BODY()

	ULactoseIdentityServiceSubsystem();

public:
	// Begin override ULactoseServiceSubsystem
	void Initialize(FSubsystemCollectionBase& Collection) override;
	// End override ULactoseServiceSubsystem

	void LoginUsingBasicAuth(const FString& Username, const FString& Password);
	void LoginUsingRefreshToken(TFunction<void()>&& LoginFailed);
	void LoadCurrentUser(const FString& UserId);
	void Logout();
	
	const Sp<FLactoseIdentityGetUserResponse>& GetLoggedInUserInfo() const { return LoggedInUserInfo; }

	UFUNCTION(BlueprintPure, Category = "Identity")
	ELactoseIdentityUserLoginStatus GetLoginStatus() const;

protected:
	void OnUserLoggedIn(Sr<FGetUserRequest::FResponseContext> Context);
	
private:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bAutoLogin = true;

	TFuture<Sp<FLoginRequest::FResponseContext>> LoginUsingBasicAuthFuture;
	TFuture<Sp<FRefreshTokenRequest::FResponseContext>> LoginUsingRefreshFuture;
	TFuture<Sp<FGetUserRequest::FResponseContext>> CurrentUserInfoFuture;
	Sp<FLactoseIdentityGetUserResponse> LoggedInUserInfo;
};

namespace Lactose::Identity::Events
{
	DECLARE_MULTICAST_DELEGATE_TwoParams(FUserLoggedIn,
		const ULactoseIdentityServiceSubsystem& /* Sender */,
		const Sr<FLactoseIdentityGetUserResponse>& /* User */);

	DECLARE_MULTICAST_DELEGATE_OneParam(FGeneric,
		const ULactoseIdentityServiceSubsystem& /* Sender */);

	inline FUserLoggedIn OnUserLoggedIn;
	inline FGeneric OnUserLoggedOut;
	inline FGeneric OnUserLoginFailed;
}
