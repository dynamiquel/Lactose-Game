#include "LactoseVendorSubsystem.h"

#include "VendorActor.h"
#include "Api/Tasks/LactoseTasksUserTasks.h"
#include "Kismet/GameplayStatics.h"
#include "LactoseGame/LactoseGame.h"
#include "Services/ConfigCloud/LactoseConfigCloudServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"
#include "Services/Tasks/LactoseTasksServiceSubsystem.h"

void ULactoseVendorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	auto& ConfigCloud = Subsystems::GetRef<ULactoseConfigCloudServiceSubsystem>(self);
	if (Sp<const FLactoseConfigCloudConfig> Config = ConfigCloud.GetConfig())
		OnConfigLoaded(ConfigCloud, Config.ToSharedRef());

	Lactose::Config::Events::OnConfigLoaded.AddUObject(this, &ThisClass::OnConfigLoaded);
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

const FLactoseVendorConfigs* ULactoseVendorSubsystem::FindVendorConfigs() const
{	
	auto* VendorConfigs = VendorConfigEntry->Get<FLactoseVendorConfigs>();
	if (!VendorConfigs)
	{
		UE_LOG(LogLactose, Error, TEXT("Unable to parse Vendor Config Entry. Json:%s"),
			*VendorConfigEntry->GetString());
		return nullptr;
	}

	return VendorConfigs;
}

const FLactoseVendorConfig* ULactoseVendorSubsystem::FindVendorConfig(const FString& VendorId) const
{
	const FLactoseVendorConfigs* VendorConfigs = FindVendorConfigs();
	return VendorConfigs->Config.FindByPredicate([VendorId](const FLactoseVendorConfig& Config)
	{
		return Config.VendorId == VendorId;
	});
}

void ULactoseVendorSubsystem::OnConfigLoaded(
	const ULactoseConfigCloudServiceSubsystem& Sender,
	Sr<const FLactoseConfigCloudConfig> Config)
{
	VendorConfigEntry = Config->FindEntry(TEXT("vendors"));
	if (!VendorConfigEntry)
	{
		UE_LOG(LogLactose, Error, TEXT("Vendors Config Entry not found"));
		return;
	}

	const FLactoseVendorConfigs* VendorConfigs = FindVendorConfigs();
	if (!VendorConfigs)
	{
		VendorConfigEntry.Reset();
		return;
	}

	UE_LOG(LogLactose, Log, TEXT("Looking for Vendors to spawn"));

	// Attempt to spawn any vendors that should already be unlocked.
	auto& Tasks = Subsystems::GetRef<ULactoseTasksServiceSubsystem>(self);
	for (const FLactoseVendorConfig& VendorConfig : VendorConfigs->Config)
	{
		UE_LOG(LogLactose, Log, TEXT("Vendor '%s' - Name '%s' - Unlock Task '%s'"),
			*VendorConfig.VendorId, *VendorConfig.VendorName, *VendorConfig.UnlockTask);
		
		Sp<const FLactoseTasksUserTaskDto> UserTask = Tasks.FindCurrentUserTaskWithTaskId(VendorConfig.UnlockTask);
		if (UserTask && UserTask->Completed)
			SpawnVendor(VendorConfig.VendorId);
	}

	// Listen for future task completions to unlock any vendors later.
	Lactose::Tasks::Events::OnCurrentUserTaskUpdated.AddUObject(this, &ThisClass::OnUserTaskUpdated);
}

void ULactoseVendorSubsystem::OnUserTaskUpdated(
	const ULactoseTasksServiceSubsystem& Sender,
	const Sr<const FLactoseTasksUserTaskDto>& UserTask)
{
	// Finds out which vendors to unlock based on the task completed.
	if (!UserTask->Completed)
		return;
	
	const FLactoseVendorConfigs* VendorConfigs = FindVendorConfigs();
	if (!VendorConfigs)
		return;

	for (const FLactoseVendorConfig& VendorConfig : VendorConfigs->Config)
		if (UserTask->TaskId == VendorConfig.UnlockTask)
			SpawnVendor(VendorConfig.VendorId);
}

void ULactoseVendorSubsystem::SpawnVendor(const FString& VendorId)
{
	if (FindVendorById(VendorId))
		return;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(VendorId), OUT FoundActors);

	if (FoundActors.IsEmpty())
	{
		UE_LOG(LogLactose, Error, TEXT("Could not find actor with tag '%s'"), *VendorId);
		return;
	}

	TSubclassOf<AVendorActor> ActorClass;
	if (VendorId == BasicVendorId)
		ActorClass = BasicVendorClass;
	else if (VendorId == WarmVendorId)
		ActorClass = WarmVendorClass;
	else if (VendorId == AnimalsVendorId)
		ActorClass = AnimalsVendorClass;
	else if (VendorId == MiscVendorId)
		ActorClass = MiscVendorClass;
	else
	{
		UE_LOG(LogLactose, Error, TEXT("Cannot find an actor class for Vendor '%s'"), *VendorId);
		return;
	}

	if (!ActorClass)
	{
		UE_LOG(LogLactose, Error, TEXT("Actor class for Vendor '%s' is null"), *VendorId);
		return;
	}
	
	AVendorActor* VendorActor = GetWorldRef().SpawnActorDeferred<AVendorActor>(
		ActorClass,
		FoundActors[0]->GetActorTransform(),
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	check(VendorActor);

	VendorActor->VendorId = VendorId;
	VendorActor->FinishSpawning(FoundActors[0]->GetActorTransform());

	ActiveVendors.Add(VendorActor);

	UE_LOG(LogLactose, Log, TEXT("Spawned Vendor '%s'"), *VendorId);
}