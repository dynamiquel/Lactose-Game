// Fill out your copyright notice in the Description page of Project Settings.


#include "PlotCropActor.h"

#include "LactoseCropWorldSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationUserCropsRequests.h"


APlotCropActor::APlotCropActor()
{
	SoilMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Soil");
	SoilMeshComponent->SetupAttachment(GroundMesh);
	SoilMeshComponent->SetCollisionObjectType(ECC_WorldStatic);
	SoilMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SoilMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SoilMeshComponent->SetCollisionResponseToChannel(Crops::CropTraceChannel, ECR_Block);
	SoilMeshComponent->SetCastShadow(false);
	
	PlantsComponent = CreateDefaultSubobject<USceneComponent>("Plants");
	PlantsComponent->SetupAttachment(PlantMesh);
}

void APlotCropActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SpawnPlantMeshes();
}

void APlotCropActor::BeginPlay()
{
	Super::BeginPlay();

	if (bUsePlantGrowthScale)
		SetPlantScaleBasedOnGrowth();
}

void APlotCropActor::OnLoaded(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	Super::OnLoaded(InCropInstance);

	if (bUsePlantGrowthScale)
	{
		SetPlantScaleBasedOnGrowth();
		SetShadowBasedOnGrowth();
	}
}

float APlotCropActor::GetCropGrowthProgress() const
{
	if (GetCropInstance()->State == Lactose::Simulation::States::Harvestable)
		return 1.f;
	
	if (GetCropInstance()->State == Lactose::Simulation::States::Empty)
		return 0.f;

	if (!GetCrop())
		return 0.f;

	if (GetCrop()->HarvestSeconds <= 0.)
		return 0.f;

	return (GetCrop()->HarvestSeconds - GetCropInstance()->RemainingHarvestSeconds) / GetCrop()->HarvestSeconds;
}

void APlotCropActor::SpawnPlantMeshes()
{
	{
		// Reset any lingering Generated Plant Meshes.
		for (UInstancedStaticMeshComponent* GeneratedPlantMesh : GeneratedPlantMeshes)
			if (IsValid(GeneratedPlantMesh))
				GeneratedPlantMesh->DestroyComponent();
	
		GeneratedPlantMeshes.Reset();
		InitialInstanceTransforms.SetNum(PlantMeshDefinitions.Num());
	}
	
	if (PlantMeshDefinitions.IsEmpty())
		return;

	// Procedurally spawn in the desired plant meshes based on configuration.
	// I tried setting up the meshes manually for my first crop and that was a huge time killer.
	// Never again!

	for (const FLactosePlantMeshDefinition& PlantMeshDefinition : PlantMeshDefinitions)
	{
		// Create an Instanced Mesh Component for every Plant Mesh type.
		auto* NewPlantMesh = Cast<UInstancedStaticMeshComponent>(AddComponentByClass(
			UInstancedStaticMeshComponent::StaticClass(),
			/* bManualAttachment */ true,
			FTransform(),
			/* bDeferredFinish */ true));

		NewPlantMesh->SetupAttachment(PlantsComponent);
		NewPlantMesh->SetStaticMesh(PlantMeshDefinition.StaticMesh);
		NewPlantMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		NewPlantMesh->bDisableCollision = true;
		
		FinishAddComponent(
			NewPlantMesh,
			/* bManualAttachment */ true,
			FTransform());
		
		GeneratedPlantMeshes.Add(NewPlantMesh);
	}

	check(PlantMeshDefinitions.Num() == GeneratedPlantMeshes.Num());
	
	const float PlantPaddingX = BoundsRadius / static_cast<float>(NumberOfPlantsX) * 2.f;
	const float PlantPaddingY = BoundsRadius / static_cast<float>(NumberOfPlantsY) * 2.f;

	const float StartX = -BoundsRadius + PlantPaddingX * .5f;
	const float StartY = -BoundsRadius + PlantPaddingY * .5f;

	for (int32 PlantXIdx = 0; PlantXIdx < NumberOfPlantsX; PlantXIdx++)
	{
		for (int32 PlantYIdx = 0; PlantYIdx < NumberOfPlantsY; PlantYIdx++)
		{
			const int32 RandomPlantMeshDefinitionIdx = FMath::RandRange(0, GeneratedPlantMeshes.Num() - 1);
			FLactosePlantMeshDefinition& PlantMeshDefinition = PlantMeshDefinitions[RandomPlantMeshDefinitionIdx];
			UInstancedStaticMeshComponent* PlantInstancedMeshComp = GeneratedPlantMeshes[RandomPlantMeshDefinitionIdx];
			
			const FVector PlantPaddingLocation = FVector(
				StartX + PlantPaddingX * PlantXIdx,
				StartY + PlantPaddingY * PlantYIdx,
				0.);
			
			const FTransform PlantPaddingTransform = FTransform(PlantPaddingLocation);
			const FTransform PlantRelativeTransform = PlantMeshDefinition.RelativeTransform * PlantPaddingTransform;
			
			PlantInstancedMeshComp->AddInstance(PlantRelativeTransform);
			InitialInstanceTransforms[RandomPlantMeshDefinitionIdx].Add(PlantRelativeTransform);
		}
	}
}

void APlotCropActor::SetPlantScaleBasedOnGrowth()
{
	if (!ensure(PlantsComponent))
	{
		return;
	}

	const float CurrentGrowthScale = GetCropGrowthProgress();

	for (int PlantInstancedMeshIdx = 0; PlantInstancedMeshIdx < GeneratedPlantMeshes.Num(); PlantInstancedMeshIdx++)
    {
		UInstancedStaticMeshComponent* PlantInstancedMeshComp = GeneratedPlantMeshes[PlantInstancedMeshIdx];
		TArray<FTransform>& StoredInitialTransforms = InitialInstanceTransforms[PlantInstancedMeshIdx];
		const FTransform& PlantDefinitionTransform = PlantMeshDefinitions[PlantInstancedMeshIdx].RelativeTransform;

        if (!IsValid(PlantInstancedMeshComp) || StoredInitialTransforms.IsEmpty())
            continue;

        TArray<FTransform> UpdatedTransformsBatch;
        UpdatedTransformsBatch.Reserve(StoredInitialTransforms.Num()); 

        for (const FTransform& InitialTransform : StoredInitialTransforms)
        {
            FTransform CurrentInstanceTransform = InitialTransform;
            CurrentInstanceTransform.SetScale3D(PlantDefinitionTransform.GetScale3D() * CurrentGrowthScale);
            UpdatedTransformsBatch.Emplace(CurrentInstanceTransform);
        }
		
        PlantInstancedMeshComp->BatchUpdateInstancesTransforms(
            0,
            UpdatedTransformsBatch,
            false,
            true,
            false 
        );
    }

}

void APlotCropActor::SetShadowBasedOnGrowth()
{
	if (!GetCropInstance())
		return;

	const bool bShouldCastShadow = GetCropGrowthProgress() > PlantGrowthEnableShadowThreshold;
	
	for (UInstancedStaticMeshComponent* GeneratedPlantMesh : GeneratedPlantMeshes)
		if (IsValid(GeneratedPlantMesh))
			GeneratedPlantMesh->SetCastShadow(bShouldCastShadow);
}
