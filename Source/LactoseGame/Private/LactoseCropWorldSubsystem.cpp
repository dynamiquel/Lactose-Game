#include "LactoseCropWorldSubsystem.h"

#include <array>
#include "HAL/IConsoleManager.h"
#include "CropActor.h"
#include "PlotCropActor.h"
#include "Engine/OverlapResult.h"
#include "LactoseGame/LactoseGame.h"
#include "LactoseGame/LactosePathUtils.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

TAutoConsoleVariable CVarEnableCropWorldSubsystem
(
	TEXT("Lactose.EnableCropWorldSubsystem"),
	true,
	TEXT("")
);

ULactoseCropWorldSubsystem::ULactoseCropWorldSubsystem()
{
	static ConstructorHelpers::FClassFinder<APlotCropActor> DefaultEmptyPlotCropActor(TEXT("/Game/Crops/BP_Crop_EmptyPlot"));
	EmptyPlotCropActor = DefaultEmptyPlotCropActor.Class;
}


bool ULactoseCropWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && CVarEnableCropWorldSubsystem.GetValueOnGameThread() == true;
}

void ULactoseCropWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	Lactose::Simulation::Events::OnCurrentUserCropsLoaded.AddUObject(this, &ULactoseCropWorldSubsystem::OnUserCropsLoaded);

	auto& Simulation = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);
	
	if (Simulation.GetAllCropsStatus() == ELactoseSimulationCropsStatus::Loaded)
	{
		bWaitingForCrops = false;
		LoadCropActorClasses();
	}
	else
	{
		// Be notified once all crops are loaded.
		Lactose::Simulation::Events::OnAllCropsLoaded.AddUObject(this, &ThisClass::OnAllCropsLoaded);
	}

	if (Simulation.GetCurrentUserCropsStatus() == ELactoseSimulationUserCropsStatus::Loaded)
		bWaitingForUserCrops = false;

	if (CanCreateCrops())
		CreateRequiredUserCrops();
}

bool ULactoseCropWorldSubsystem::CanCreateCrops() const
{
	return !bWaitingForCrops && !bWaitingForUserCrops && !bWaitingForCropActorClassMap;
}

void ULactoseCropWorldSubsystem::RegisterCropActor(ACropActor& CropActor)
{
	Sp<const FLactoseSimulationUserCropInstance> CropInstance = CropActor.GetCropInstance();
	if (!ensure(CropInstance))
	{
		return;
	}
	
	RegisteredCropActors.Add(CropInstance->Id, &CropActor);
}

void ULactoseCropWorldSubsystem::DeregisterCropActor(const ACropActor& CropActor)
{
	Sp<const FLactoseSimulationUserCropInstance> CropInstance = CropActor.GetCropInstance();
	if (!ensure(CropInstance))
	{
		return;
	}
	
	RegisteredCropActors.Remove(CropInstance->Id);
}

ACropActor* ULactoseCropWorldSubsystem::FindCropActorForCropInstance(const FString& CropInstanceId) const
{
	return RegisteredCropActors.FindRef(CropInstanceId);
}

TSubclassOf<ACropActor> ULactoseCropWorldSubsystem::FindCropActorClassForCrop(const FString& CropId) const
{
	return CropIdToCropActorMap.FindRef(CropId);
}

ACropActor* ULactoseCropWorldSubsystem::ResetCropActor(ACropActor& CropActor)
{
	Sp<const FLactoseSimulationCrop> ExistingCrop = CropActor.GetCrop();
	Sp<const FLactoseSimulationUserCropInstance> ExistingCropInstance = CropActor.GetCropInstance();
	CropActor.Destroy();

	auto& Simulation = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);
	Sp<const FLactoseSimulationCrop> NewCrop = Simulation.FindCrop(ExistingCropInstance->CropId);
	if (!ensure(NewCrop))
	{
		return nullptr;
	}
	
	ACropActor* NewCropActor = CreateUserCrop(NewCrop.ToSharedRef(), ExistingCropInstance.ToSharedRef());
	
	return NewCropActor;
}

bool ULactoseCropWorldSubsystem::IsLocationObstructed(const FVector& ProposedLocation, const AActor* ActorToIgnore) const
{
	// Reduce the extents in a tiny bit to allow 'perfect fits' to pass.
    FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(Crops::PlotHalfExtentCm - .1, Crops::PlotHalfExtentCm - .1, 10.0)); 
	
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
	
	if (ActorToIgnore)
		QueryParams.AddIgnoredActor(ActorToIgnore);
	
    bool bOverlap = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        ProposedLocation,
        FQuat::Identity,
        Crops::CropTraceChannel,
        BoxShape
    );

    if (bOverlap)
    {
        for (const FOverlapResult& Overlap : OverlapResults)
        {
            if (auto* Actor = Cast<APlotCropActor>(Overlap.GetActor()))
            {
            	UE_LOG(LogLactose, Log, TEXT("Location '%s' is obstructed by Actor '%s'"),
            		*ProposedLocation.ToCompactString(), *Actor->GetActorNameOrLabel());
	            return true;
            }
        }
    }
	
    return false;
}

FVector ULactoseCropWorldSubsystem::GetMagnetizedPlotLocation(const FVector& ProposedLocation) const
{
	if (MagnetType == ECropMagnetType::None)
		return ProposedLocation;
	
    FVector BestSnapLocation = ProposedLocation;
    float ClosestSnapDistanceSq = FMath::Square(Crops::MagnetiseSearchRadiusCm + Crops::MagnetiseToleranceCm + 1.0f); 

    TArray<FOverlapResult> NearbyOverlapResults;
    FCollisionShape SearchShape = FCollisionShape::MakeSphere(Crops::MagnetiseSearchRadiusCm); 
    FCollisionQueryParams SearchQueryParams;
    SearchQueryParams.bTraceComplex = false;
	
    GetWorld()->OverlapMultiByChannel(
        NearbyOverlapResults,
        ProposedLocation,
        FQuat::Identity,
        Crops::CropTraceChannel,
        SearchShape,
        SearchQueryParams
    );

    TArray<ACropActor*> NearbyCrops;
    NearbyCrops.Reserve(NearbyOverlapResults.Num());
    for (const FOverlapResult& Overlap : NearbyOverlapResults)
    {
	    if (ACropActor* PlotActor = Cast<ACropActor>(Overlap.GetActor()))
	    	NearbyCrops.Add(PlotActor);
    }

    // Define potential snap offsets from an EXISTING plot's center to get to an ADJACENT plot's center
    // These cover cardinal and diagonal neighbors around an existing 2x2m plot.
	const std::array SnapOffsets = {
		FVector(Crops::PlotSizeCm, 0., 0.), // Right
		FVector(-Crops::PlotSizeCm, 0., 0.), // Left
		FVector(0., Crops::PlotSizeCm, 0.), // Forward
		FVector(0., -Crops::PlotSizeCm, 0.), // Back
		FVector(Crops::PlotSizeCm, Crops::PlotSizeCm, 0.), // Right-Forward (Diagonal)
		FVector(Crops::PlotSizeCm, -Crops::PlotSizeCm, 0.), // Right-Back (Diagonal)
		FVector(-Crops::PlotSizeCm, Crops::PlotSizeCm, 0.), // Left-Forward (Diagonal)
		FVector(-Crops::PlotSizeCm, -Crops::PlotSizeCm, 0.) // Left-Back (Diagonal)
	};

    // Iterate only through the efficiently found nearby plots (much fewer than all plots)
    for (ACropActor* NearbyCrop : NearbyCrops) 
    {
        for (const FVector& Offset : SnapOffsets)
        {
            FVector PotentialSnapLocation = NearbyCrop->GetActorLocation() + Offset;
        	
            // Check if this potential snap location is "close enough" to where the player is trying to plant
            if (FVector::DistSquaredXY(ProposedLocation, PotentialSnapLocation) < FMath::Square(Crops::MagnetiseToleranceCm))
            {
                // Check if this PotentialSnapLocation is NOT OBSTRUCTED by any other plot.
                // We pass the 'ExistingPlot' as ActorToIgnore so that IsLocationObstructed doesn't
                // count the plot we are trying to snap *adjacent* to as an obstruction.
                if (!IsLocationObstructed(PotentialSnapLocation, NearbyCrop))
                {
                    // If it's a valid, non-obstructed snap point, see if it's the closest one found so far
                    float CurrentSnapDistanceSq = FVector::DistSquaredXY(ProposedLocation, PotentialSnapLocation);
                    if (CurrentSnapDistanceSq < ClosestSnapDistanceSq)
                    {
                        ClosestSnapDistanceSq = CurrentSnapDistanceSq;
                        BestSnapLocation = PotentialSnapLocation;
                    }
                }
            }
        }
    }
	
    // Fallback: If no adjacent plot was found to magnetize to, snap the initial proposed location
    // to the nearest 2m grid point. This ensures all plots are aligned on a consistent grid.
    if (MagnetType == ECropMagnetType::Grid && BestSnapLocation == ProposedLocation)
    {
        BestSnapLocation.X = FMath::RoundToFloat(BestSnapLocation.X / Crops::PlotSizeCm) * Crops::PlotSizeCm;
        BestSnapLocation.Y = FMath::RoundToFloat(BestSnapLocation.Y / Crops::PlotSizeCm) * Crops::PlotSizeCm;
    }
    
    return BestSnapLocation;
}

void ULactoseCropWorldSubsystem::OnAllCropsLoaded(const ULactoseSimulationServiceSubsystem& Sender)
{
	if (bWaitingForCrops)
	{
		bWaitingForCrops = false;
		Log::Verbose(LogLactose, TEXT("Crop Subsystem: Received Crops"));
		
		LoadCropActorClasses();

		if (CanCreateCrops())
			CreateRequiredUserCrops();
	}

	// Don't care about future events.
	Lactose::Simulation::Events::OnAllCropsLoaded.RemoveAll(this);
}

void ULactoseCropWorldSubsystem::OnUserCropsLoaded(
	const ULactoseSimulationServiceSubsystem& Sender,
	Sr<FLactoseSimulationUserCrops> UserCrops)
{
	if (bWaitingForUserCrops)
	{
		bWaitingForUserCrops = false;
		Log::Verbose(LogLactose, TEXT("Crop Subsystem: Received User Crops"));
	}

	if (CanCreateCrops())
		CreateRequiredUserCrops();
}

void ULactoseCropWorldSubsystem::CreateRequiredUserCrops()
{
	auto& Simulation = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);

	const TMap<FString, Sr<FLactoseSimulationCrop>>& AllCrops = Simulation.GetAllCrops();
	if (AllCrops.IsEmpty())
		return;

	Sp<const FLactoseSimulationUserCrops> UserCrops = Simulation.GetCurrentUserCrops();
	if (!UserCrops)
		return;

	for (const Sr<FLactoseSimulationUserCropInstance>& UserCrop : UserCrops->GetAllCropInstances())
	{
		if (FindCropActorForCropInstance(UserCrop->Id))
			continue;

		Sp<const FLactoseSimulationCrop> Crop = Simulation.FindCrop(UserCrop->CropId);
		if (!Crop)
		{
			Log::Error(LogLactose,
				TEXT("Could not find a Crop with ID: '%s'"),
				*UserCrop->CropId);
			
			continue;
		}

		CreateUserCrop(Crop.ToSharedRef(), UserCrop);
	}
}

ACropActor* ULactoseCropWorldSubsystem::CreateUserCrop(
	const Sr<const FLactoseSimulationCrop>& Crop,
	const Sr<const FLactoseSimulationUserCropInstance>& CropInstance)
{
	if (!ensureMsgf(CropInstance->CropId == Crop->Id, TEXT("%s != %s"), *CropInstance->CropId, *Crop->Id))
	{
		return nullptr;
	}

	if (!ensure(!FindCropActorForCropInstance(CropInstance->Id)))
	{
		return nullptr;
	}

	TSubclassOf<ACropActor> CropActorClass = FindCropActorClassForCrop(Crop->Id);
	if (!CropActorClass)
	{
		Log::Error(LogLactose,
			TEXT("Could not find a Crop Actor class for Crop '%s'"),
			*Crop->Id);
		
#if UE_BUILD_SHIPPING || UE_BUILD_TEST 
		return nullptr;
#else
		Log::Warning(LogLactose, TEXT("Fallbacking to default Crop Actor class for development purposes"));
		CropActorClass = ACropActor::StaticClass();
#endif // UE_BUILD_SHIPPING || UE_BUILD_TEST 
	}

	const auto CropRotation = FRotator(
		/* Pitch */ CropInstance->Rotation.X,
		/* Yaw   */ CropInstance->Rotation.Y,
		/* Roll  */ CropInstance->Rotation.Z);
	
	FTransform CropTransform;
	CropTransform.SetLocation(CropInstance->Location);
	CropTransform.SetRotation(CropRotation.Quaternion());

	ACropActor* NewCropActor = GetWorld()->SpawnActorDeferred<ACropActor>(
		CropActorClass,
		CropTransform,
		/* Owner */ nullptr,
		/* Instigator */ nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!NewCropActor)
	{
		Log::Error(LogLactose,
			TEXT("Could not create a Crop Actor with class '%s'"),
			*CropActorClass->GetName());
		
		return nullptr;
	}

	NewCropActor->Init(Crop, CropInstance);
	NewCropActor->FinishSpawning(CropTransform);

	Log::Verbose(LogLactose,
		TEXT("Created Crop Actor '%s' for Crop Instance '%s'"),
		*NewCropActor->GetActorNameOrLabel(),
		*CropInstance->Id);
	
	return NewCropActor;
}

void ULactoseCropWorldSubsystem::LoadCropActorClasses()
{
	auto& SimulationSubsystem = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);

	const TMap<FString, Sr<FLactoseSimulationCrop>>& AllCrops = SimulationSubsystem.GetAllCrops();
	for (auto& Crop : AllCrops)
	{
		const FString CropClassPath = Lactose::Paths::GetClassPackagePath(Crop.Value->GameCrop);

		Log::Verbose(LogLactose,
			TEXT("Attempting to load Crop Actor Class '%s' for Crop '%s'"),
			*CropClassPath,
			*Crop.Value->Id);
		
		TSoftClassPtr<ACropActor> CropActorSoftClass(CropClassPath);
		if (CropActorSoftClass.IsNull())
		{
			Log::Warning(LogLactose,
				TEXT("Crop Subsystem: Could not find a Crop Actor Class for '%s' (%s)"),
				*Crop.Value->Id,
				*Crop.Value->Name);
			
			continue;
		}

		TSubclassOf<ACropActor> CropActorClass = CropActorSoftClass.LoadSynchronous();
		if (!CropActorClass)
		{
			Log::Error(LogLactose,
				TEXT("Crop Subsystem: Could not load Actor Class with path '%s'"),
				*CropActorSoftClass.ToString());
			
			continue;
		}

		CropIdToCropActorMap.Add(Crop.Value->Id, CropActorClass);
	}

	if (TSubclassOf<ACropActor> EmptyPlotActorClass = EmptyPlotCropActor.LoadSynchronous())
	{
		CropIdToCropActorMap.Add(FString(), EmptyPlotActorClass);
	}
	else
	{
		Log::Error(LogLactose,
			TEXT("Crop Subsystem: Could not load the Empty Crop Actor Class with path '%s'"),
			*EmptyPlotCropActor.ToString());
	}
	
	bWaitingForCropActorClassMap = false;
}
