#pragma once

#include "Simp.h"
#include "VendorActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "LactoseVendorSubsystem.generated.h"

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
	
protected:
	void OnUserCropsHarvested(
		const ULactoseSimulationServiceSubsystem& Sender,
		const TArray<Sr<const FLactoseSimulationUserCropInstance>>& ModifiedUserCrops);

	void SpawnBasicVendor();

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
};
