#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "LactoseServiceSubsystem.generated.h"

class UHello;
struct FGrpcLactoseCommonHelloResponse;
struct FGrpcResult;
struct FGrpcContextHandle;
class UHelloClient;

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Config=Services, DefaultConfig)
class LACTOSEGAME_API ULactoseServiceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Lactose|Services")
	void SayHello();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION()
	void OnHelloResponse(
		FGrpcContextHandle Handle,
		const FGrpcResult& GrpcResult,
		const FGrpcLactoseCommonHelloResponse& Response);

private:
	UPROPERTY(GlobalConfig)
	FString HelloServiceName = TEXT("Hello");

	UPROPERTY(Transient)
	TObjectPtr<UHelloClient> HelloClient = nullptr;
};
