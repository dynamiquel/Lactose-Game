// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Simp.h"
#include "LactoseInteractionComponent.h"
#include "GameFramework/Actor.h"
#include "CropActor.generated.h"

class ULactoseInteractionComponent;
class UTextRenderComponent;
class FLactoseSimulationUserCrops;
class ULactoseSimulationServiceSubsystem;
struct FLactoseSimulationUserCropInstance;
struct FLactoseSimulationCrop;

UCLASS()
class LACTOSEGAME_API ACropActor : public AActor
{
	GENERATED_BODY()

public:
	ACropActor();

protected:
	// Begin override AActor
	void PostInitializeComponents() override;
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End override AActor

public:
	virtual void Init(
		const Sr<const FLactoseSimulationCrop>& InCrop,
		const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance);

	Sp<const FLactoseSimulationCrop> GetCrop() const { return Crop; };
	Sp<const FLactoseSimulationUserCropInstance> GetCropInstance() const { return CropInstance; };

	void TurnOnRendering();
	void TurnOffRendering();

protected:
	virtual void OnLoaded(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance);
	virtual void OnHarvested(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance);
	virtual void OnFertilised(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance);
	virtual void OnSeeded(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance);
	virtual void OnDestroyed(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance);

	virtual void OnInteracted(const ULactoseInteractionComponent& InteractionComponent, AController* InInstigator);
	virtual void OnDestroyInteracted(const ULactoseInteractionComponent& InteractionComponent, AController* InInstigator);

	void UpdateBillboard();
	void UpdateBillboardText();

	UFUNCTION(BlueprintPure)
	float GetCropGrowthProgress() const;

	UFUNCTION(BlueprintNativeEvent)
	void SetPlantScaleBasedOnGrowth();
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPrimitiveComponent> InteractionCollision;

	UPROPERTY(EditAnywhere)
	TObjectPtr<ULactoseInteractionComponent> Interaction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<ULactoseInteractionComponent> DestroyInteraction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UPrimitiveComponent> CullCollider;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UMeshComponent> PlantMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMeshComponent> GroundMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> BillboardComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTextRenderComponent> CropNameTextComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTextRenderComponent> CropStateTextComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTextRenderComponent> CropHarvestTimeTextComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTextRenderComponent> CropFertiliseTimeTextComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> HarvestSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> SeedSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> FertiliseSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> DestroySound;

	UPROPERTY(EditAnywhere)
	float DestroySeconds = .3f;

	UPROPERTY(EditAnywhere)
	bool bUsePlantGrowthScale = true;
	
private:
	Sp<const FLactoseSimulationCrop> Crop;
	Sp<const FLactoseSimulationUserCropInstance> CropInstance;

	// Used to know when the Crop for this Crop Instance has been changed,
	// so we can replace the Actor with the correct one.
	FString OriginalCropId;
};
