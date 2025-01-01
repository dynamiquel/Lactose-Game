#pragma once

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "Simp.h"
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
	int32 Status = 0;
	
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
	TFuture<Sp<FGetServiceStatusRequest::FResponseContext>> GetServiceInfo();

protected:
	// Begin override ULactoseServiceSubsystem
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	// End override ULactoseServiceSubsystem

	void SetServiceBaseUrl(FString&& InUrl);
	
	void OnGetServiceInfoResponse(
		Sr<FGetServiceStatusRequest::FResponseContext> Context);

private:
	UPROPERTY(EditDefaultsOnly, Config)
	FString ServiceBaseUrl; 
};

namespace Lactose
{
	template<typename TLactoseService>
	static TLactoseService& GetService(const UObject& Context)
	{
		const UWorld* World = Context.GetWorld();
		check(World);
		const UGameInstance* GameInstance = World->GetGameInstance();
		check(GameInstance);
		auto* Service = GameInstance->GetSubsystem<TLactoseService>();
		return *Service;
	}
}
