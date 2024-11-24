#pragma once

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "Rest/LactoseRestRequest.h"

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
	FDateTime BuildTime;

	UPROPERTY()
	int32 Status;
	
	UPROPERTY()
	FString Runtime;

	UPROPERTY()
	FString OperatingSystem;

	UPROPERTY()
	FDateTime StartTime;

	UPROPERTY()
	FString Uptime;
};

using FGetServiceStatusRequest = Lactose::Rest::TRequest<void, FLactoseServiceInfo>;

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Config=Services, DefaultConfig)
class LACTOSEGAME_API ULactoseServiceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	const FString& GetServiceBaseUrl() const { return ServiceBaseUrl; }
	TFuture<TSharedPtr<FGetServiceStatusRequest::FResponseContext>> GetServiceInfo();

protected:
	// Begin override ULactoseServiceSubsystem
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	// End override ULactoseServiceSubsystem

	void SetServiceBaseUrl(FString&& InUrl);
	
	void OnGetServiceInfoResponse(
		TSharedRef<FGetServiceStatusRequest::FResponseContext> Context);

private:
	UPROPERTY(EditDefaultsOnly, Config)
	FString ServiceBaseUrl; 
};
