#pragma once

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "Simp.h"
#include "SimpConcepts.h"
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
	FString Hostname;

	UPROPERTY()
	FDateTime StartTime;

	UPROPERTY()
	FString Uptime;
};

using FGetServiceStatusRequest = Lactose::Rest::TRequest<void, FLactoseServiceInfo>;

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Config=Services)
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
		return Subsystems::GetRef<TLactoseService>(Context);
	}

	/**
	 * Converts a map with Shared Pointer/Ref values into a map with the pointer's
	 * type. This is useful for temporarily passing Shared Pointer data to Blueprint.
	 */
	template<
		typename TMapKey,
		typename TMapValue,
		typename TMapValueRaw = typename TMapValue::ElementType>
	TMap<TMapKey, TMapValueRaw> ExtractSpMap(const TMap<TMapKey, TMapValue>& Map)
	{
		TMap<TMapKey, TMapValueRaw> CopiedItems;
		CopiedItems.Reserve(Map.Num());

		for (const auto& Item : Map)
			CopiedItems.Add(Item.Key, *Item.Value);
	
		return CopiedItems;
	}
}
