#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Rest/LactoseRestRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "LactoseServiceSubsystem.generated.h"

USTRUCT()
struct FLactoseServiceInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString Description;

	UPROPERTY()
	TArray<FString> Dependencies;

	UPROPERTY()
	FString Version;

	UPROPERTY()
	FString BuildTime;

	UPROPERTY()
	int32 Status;
	
	UPROPERTY()
	FString Runtime;

	UPROPERTY()
	FString OperatingSystem;

	UPROPERTY()
	FString StartTime;

	UPROPERTY()
	FString Uptime;
};

using FGetServiceInfoRequest = Lactose::Rest::TRequest<void, FLactoseServiceInfo>;

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Config=Services, DefaultConfig)
class LACTOSEGAME_API ULactoseServiceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Lactose|Services")
	void GetServiceInfo();

	void GetServiceInfo2();
	void GetServiceInfo3();

protected:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	void OnGetServiceInfoResponse(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bConnectedSuccessfully);

	void OnGetServiceInfoResponse2(
		TSharedRef<Lactose::Rest::FRequest::FResponseContext> Context);

	void OnGetServiceInfoResponse3(
		TSharedRef<FGetServiceInfoRequest::FResponseContext> Context);
};
