// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LactoseInteractionComponent.generated.h"

class UInputAction;

DECLARE_MULTICAST_DELEGATE_TwoParams(FLactoseInteractionDelegate,
	const class ULactoseInteractionComponent& /* Sender */,
	class AController* /* Instigator */);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LACTOSEGAME_API ULactoseInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

	ULactoseInteractionComponent();

public:
	UFUNCTION(BlueprintPure, Category="Lactose Interaction")
	virtual bool CanBeInteracted() const;

	UFUNCTION(BlueprintPure, Category="Lactose Interaction")
	virtual FString GetInteractionText() const;
	
	virtual void Interact(AController* Instigator);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditAnywhere)
	int32 Priority = 0;

	UPROPERTY(EditAnywhere)
	float InteractionCooldown = .5f;

	UPROPERTY(EditAnywhere)
	FString InteractionText;

	FLactoseInteractionDelegate OnInteractionComplete;
	TOptional<TFunction<FString()>> InteractionTextFunc;
	TOptional<TFunction<bool()>> CanInteractFunc;
	
	FTimerHandle InteractionCooldownTimer;
};
