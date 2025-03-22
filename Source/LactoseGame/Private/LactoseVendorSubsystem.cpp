#include "LactoseVendorSubsystem.h"

#include "VendorActor.h"
#include "Kismet/GameplayStatics.h"
#include "LactoseGame/LactoseGame.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

void ULactoseVendorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	Lactose::Simulation::Events::OnCurrentUserCropsHarvested.AddUObject(this, &ThisClass::OnUserCropsHarvested);
}

AVendorActor* ULactoseVendorSubsystem::FindVendorById(const FString& VendorId) const
{
	const TObjectPtr<AVendorActor>* FoundVendor = ActiveVendors.FindByPredicate([&VendorId]
		(const TObjectPtr<AVendorActor>& VendorActor)
	{
		return VendorActor && VendorActor->VendorId == VendorId;
	});

	return FoundVendor ? *FoundVendor : nullptr;
}

void ULactoseVendorSubsystem::OnUserCropsHarvested(
	const ULactoseSimulationServiceSubsystem& Sender,
	const TArray<Sr<const FLactoseSimulationUserCropInstance>>& ModifiedUserCrops)
{
	// Temp. Ideally, we'll do some kind of statistic tracking for this.
	// Something like: "how many crops the player has harvested".

	SpawnBasicVendor();
}

void ULactoseVendorSubsystem::SpawnBasicVendor()
{
	if (FindVendorById(BasicVendorId))
		return;
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), BasicVendorId, OUT FoundActors);

	if (FoundActors.IsEmpty())
	{
		UE_LOG(LogLactose, Error, TEXT("Could not find actor with tag 'BasicVendor'"));
		return;
	}

	AVendorActor* VendorActor = GetWorldRef().SpawnActorDeferred<AVendorActor>(
		BasicVendorClass,
		FoundActors[0]->GetActorTransform(),
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	check(VendorActor);

	VendorActor->VendorId = BasicVendorId;
	VendorActor->FinishSpawning(FoundActors[0]->GetActorTransform());

	ActiveVendors.Add(VendorActor);

	UE_LOG(LogLactose, Log, TEXT("Spawned Basic Vendor"));
}
