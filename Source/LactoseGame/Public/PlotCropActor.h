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

	void OnLoaded(const TSharedRef<const FLactoseSimulationUserCropInstance>& InCropInstance) override;

	void SetPlantScaleBasedOnGrowth();

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

	UPROPERTY(EditAnywhere)
	bool bUsePlantGrowthScale = true;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> GeneratedPlantMeshes;
};
