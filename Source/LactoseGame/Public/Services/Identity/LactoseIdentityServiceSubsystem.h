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

using FGetUserRequest = Lactose::Rest::TRequest<FLactoseIdentityGetUserRequest, FLactoseIdentityGetUserResponse>;

UENUM()
enum class ELactoseIdentityUserLoginStatus
{
	NotLoggedIn,
	LoggingIn,
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
	
	void Login();
	void Logout();
	
	const Sp<FLactoseIdentityGetUserResponse>& GetLoggedInUserInfo() const { return LoggedInUserInfo; }
	ELactoseIdentityUserLoginStatus GetLoginStatus() const;

protected:
	void OnUserLoggedIn(Sr<FGetUserRequest::FResponseContext> Context);
	
private:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bAutoLogin = true;
	
	TFuture<Sp<FGetUserRequest::FResponseContext>> LoggedInFuture;
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
