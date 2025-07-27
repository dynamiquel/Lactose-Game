// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CropActor.h"
#include "PlotCropActor.generated.h"

USTRUCT()
struct FLactosePlantMeshDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere)
	FTransform RelativeTransform;
};

UCLASS()
class LACTOSEGAME_API APlotCropActor : public ACropActor
{
	GENERATED_BODY()

public:
	APlotCropActor();

	void OnConstruction(const FTransform& Transform) override;

protected:
	void BeginPlay() override;
	void OnLoaded(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance) override;
	void SetPlantScaleBasedOnGrowth_Implementation() override;

	void SpawnPlantMeshes();
	void SetShadowBasedOnGrowth();

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> SoilMeshComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> PlantsComponent;
	
	UPROPERTY(EditAnywhere)
	float BoundsRadius = 25.f;
	
	UPROPERTY(EditAnywhere)
	int32 NumberOfPlantsX = 4;

	UPROPERTY(EditAnywhere)
	int32 NumberOfPlantsY = 4;

	UPROPERTY(EditAnywhere)
	TArray<FLactosePlantMeshDefinition> PlantMeshDefinitions;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0, ClampMax=1))
	float PlantGrowthEnableShadowThreshold = .5f;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> GeneratedPlantMeshes;

	TArray<TArray<FTransform>> InitialInstanceTransforms;
};