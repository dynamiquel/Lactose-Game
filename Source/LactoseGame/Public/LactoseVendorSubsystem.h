#pragma once

#include "Simp.h"
#include "VendorActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "LactoseVendorSubsystem.generated.h"

class FLactoseConfigCloudEntry;
class ULactoseConfigCloudServiceSubsystem;
class FLactoseConfigCloudConfig;
struct FLactoseVendorConfigs;
struct FLactoseVendorConfig;
class ULactoseTasksServiceSubsystem;
struct FLactoseTasksUserTaskDto;
class AVendorActor;
class ULactoseSimulationServiceSubsystem;
struct FLactoseSimulationUserCropInstance;

UCLASS(DefaultConfig, Config=Game)
class ULactoseVendorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void OnWorldBeginPlay(UWorld& InWorld) override;

	AVendorActor* FindVendorById(const FString& VendorId) const;

	const FLactoseVendorConfigs* FindVendorConfigs() const;
	const FLactoseVendorConfig* FindVendorConfig(const FString& VendorId) const;
	
protected:
	void OnConfigLoaded(
		const ULactoseConfigCloudServiceSubsystem& Sender,
		Sr<const FLactoseConfigCloudConfig> Config);
	
	void OnUserTaskUpdated(
		const ULactoseTasksServiceSubsystem& Sender,
		const Sr<const FLactoseTasksUserTaskDto>& UserTask);
	
	void SpawnVendor(const FString& VendorId);

protected:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AVendorActor>> ActiveVendors;

	UPROPERTY(EditDefaultsOnly, Config)
	TSubclassOf<AVendorActor> BasicVendorClass = AVendorActor::StaticClass();

	UPROPERTY(EditDefaultsOnly, Config)
	TSubclassOf<AVendorActor> WarmVendorClass = AVendorActor::StaticClass();

	UPROPERTY(EditDefaultsOnly, Config)
	TSubclassOf<AVendorActor> AnimalsVendorClass = AVendorActor::StaticClass();

	UPROPERTY(EditDefaultsOnly, Config)
	TSubclassOf<AVendorActor> MiscVendorClass = AVendorActor::StaticClass();
	
	static constexpr auto* BasicVendorId = TEXT("vendor-basic");
	static constexpr auto* WarmVendorId = TEXT("vendor-warm");
	static constexpr auto* AnimalsVendorId = TEXT("vendor-animals");
	static constexpr auto* MiscVendorId = TEXT("vendor-misc");

	Sp<const FLactoseConfigCloudEntry> VendorConfigEntry;
};

USTRUCT()
struct FLactoseVendorConfig
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString VendorId;

	UPROPERTY()
	FString VendorName;

	UPROPERTY()
	FString UnlockTask;
};

USTRUCT()
struct FLactoseVendorConfigs
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLactoseVendorConfig> Config;
};
