// Fill out your copyright notice in the Description page of Project Settings.


#include "PlotCropActor.h"

#include "Components/StaticMeshComponent.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationUserCropsRequests.h"


APlotCropActor::APlotCropActor()
{
	SoilMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Soil");
	SoilMeshComponent->SetupAttachment(GroundMesh);

	PlantsComponent = CreateDefaultSubobject<USceneComponent>("Plants");
	PlantsComponent->SetupAttachment(PlantMesh);	
}

void APlotCropActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	{
		// Reset any lingering Generated Plant Meshes.
		for (UStaticMeshComponent* GeneratedPlantMesh : GeneratedPlantMeshes)
			if (IsValid(GeneratedPlantMesh))
				GeneratedPlantMesh->DestroyComponent();
	
		GeneratedPlantMeshes.Reset();
	}
	
	if (PlantMeshDefinitions.IsEmpty())
		return;

	// Procedurally spawn in the desired plant meshes based on configuration.
	// I tried setting up the meshes manually for my first crop and that was a huge time killer.
	// Never again!
	
	const float PlantPaddingX = BoundsRadius / static_cast<float>(NumberOfPlantsX) * 2.f;
	const float PlantPaddingY = BoundsRadius / static_cast<float>(NumberOfPlantsY) * 2.f;

	const float StartX = -BoundsRadius + PlantPaddingX * .5f;
	const float StartY = -BoundsRadius + PlantPaddingY * .5f;

	for (int32 PlantXIdx = 0; PlantXIdx < NumberOfPlantsX; PlantXIdx++)
	{
		for (int32 PlantYIdx = 0; PlantYIdx < NumberOfPlantsY; PlantYIdx++)
		{
			const int32 RandomPlantMeshDefinitionIdx = FMath::RandRange(0, PlantMeshDefinitions.Num() - 1);
			FLactosePlantMeshDefinition& PlantMeshDefinition = PlantMeshDefinitions[RandomPlantMeshDefinitionIdx];

			const FVector PlantPaddingLocation = FVector(
				StartX + PlantPaddingX * PlantXIdx,
				StartY + PlantPaddingY * PlantYIdx,
				0);
			
			const FTransform PlantPaddingTransform = FTransform(PlantPaddingLocation);
			const FTransform PlantRelativeTransform = PlantMeshDefinition.RelativeTransform * PlantPaddingTransform;
			
			UStaticMeshComponent* NewPlantMesh = Cast<UStaticMeshComponent>(AddComponentByClass(
				UStaticMeshComponent::StaticClass(),
				/* bManualAttachment */ true,
				PlantRelativeTransform,
				/* bDeferredFinish */ true));

			if (!NewPlantMesh)
				continue;

			NewPlantMesh->SetupAttachment(PlantsComponent);
			NewPlantMesh->SetStaticMesh(PlantMeshDefinition.StaticMesh);

			FinishAddComponent(
				NewPlantMesh,
				true,
				PlantRelativeTransform);

			GeneratedPlantMeshes.Add(NewPlantMesh);
		}
	}
}

void APlotCropActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlotCropActor::OnLoaded(const TSharedRef<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	Super::OnLoaded(InCropInstance);

	if (bUsePlantGrowthScale)
		SetPlantScaleBasedOnGrowth();
}

void APlotCropActor::SetPlantScaleBasedOnGrowth()
{
	if (!ensure(PlantsComponent))
	{
		return;
	}

	if (!GetCropInstance())
		return;

	float CropGrowthProgress;

	if (GetCropInstance()->State == Lactose::Simulation::States::Harvestable)
	{
		CropGrowthProgress = 1.f;
	}
	else if (GetCropInstance()->State == Lactose::Simulation::States::Empty)
	{
		CropGrowthProgress = 0.f;
	}
	else
	{
		if (!GetCrop())
			return;

		if (GetCrop()->HarvestSeconds <= 0.)
			return;

		CropGrowthProgress = (GetCrop()->HarvestSeconds - GetCropInstance()->RemainingHarvestSeconds) / GetCrop()->HarvestSeconds;
	}

	PlantsComponent->SetRelativeScale3D(FVector(CropGrowthProgress));
}
