// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "VendorActor.generated.h"

class UTextRenderComponent;
class ULactoseInteractionComponent;

UCLASS()
class LACTOSEGAME_API AVendorActor : public AActor
{
	GENERATED_BODY()

public:
	AVendorActor();

protected:
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;

	void OnInteracted(const ULactoseInteractionComponent& InteractionComponent, AController* Instigator);

	void UpdateBillboard();

public:
	UPROPERTY(EditAnywhere)
	FString VendorId;
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPrimitiveComponent> InteractionCollision;

	UPROPERTY(EditAnywhere)
	TObjectPtr<ULactoseInteractionComponent> Interaction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> BillboardComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTextRenderComponent> VendorIdTextComponent;

	UPROPERTY(EditAnywhere)
	FString VendorName;
};
