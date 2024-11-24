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

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseIdentityServiceSubsystem : public ULactoseServiceSubsystem
{
	GENERATED_BODY()

	ULactoseIdentityServiceSubsystem();

public:
	void Login();
	void Logout();
	
	const TSharedPtr<FLactoseIdentityGetUserResponse>& GetLoggedInUserInfo() const { return LoggedInUserInfo; }

protected:
	void OnUserLoggedIn(TSharedRef<FGetUserRequest::FResponseContext> Context);
	
private:
	TSharedPtr<FLactoseIdentityGetUserResponse> LoggedInUserInfo;
};
