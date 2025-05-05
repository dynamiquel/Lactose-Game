#pragma once

#include "Simp.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "LactoseMotdsSubsystem.generated.h"

class ULactoseConfigCloudServiceSubsystem;
class FLactoseConfigCloudConfig;

USTRUCT(BlueprintType)
struct FLactoseMotd
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Title;

	UPROPERTY(BlueprintReadOnly)
	FString Body;
};

USTRUCT(BlueprintType)
struct FLactoseMotds
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FLactoseMotd> Motds;
};

UCLASS(BlueprintType)
class ULactoseMotdsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	void Initialize(FSubsystemCollectionBase& Collection) override;

	void OnConfigCloudLoaded(
		const ULactoseConfigCloudServiceSubsystem& Sender,
		Sr<const FLactoseConfigCloudConfig> Config);

public:
	Sp<const FLactoseMotds> GetMotds() const { return Motds; }

	UFUNCTION(BlueprintPure, Category="Lactose")
	FLactoseMotds GetMotdsCopy() const;

public:
	Sp<FLactoseMotds> Motds;
};
