#include "LactoseCropWorldSubsystem.h"

#include "HAL/IConsoleManager.h"
#include "CropActor.h"
#include "LactoseGame/LactoseGame.h"
#include "Services/ConfigCloud/LactoseConfigCloudServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

TAutoConsoleVariable CVarEnableCropWorldSubsystem
(
	TEXT("Lactose.EnableCropWorldSubsystem"),
	true,
	TEXT("")
);

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

bool ULactoseCropWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && CVarEnableCropWorldSubsystem.GetValueOnGameThread() == true;
}

void ULactoseCropWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	Lactose::Simulation::Events::OnCurrentUserCropsLoaded.AddUObject(this, &ULactoseCropWorldSubsystem::OnUserCropsLoaded);

	auto* Simulation = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseSimulationServiceSubsystem>();
	check(Simulation);
	
	if (Simulation->GetAllCropsStatus() == ELactoseSimulationCropsStatus::Loaded)
	{
		bWaitingForCrops = false;
	}
	else
	{
		// Be notified once all crops are loaded.
		Lactose::Simulation::Events::OnAllCropsLoaded.AddUObject(this, &ThisClass::OnAllCropsLoaded);
	}

	if (Simulation->GetCurrentUserCropsStatus() == ELactoseSimulationUserCropsStatus::Loaded)
		bWaitingForUserCrops = false;

	auto* Config = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseConfigCloudServiceSubsystem>();
	check(Config);

	if (Config->GetStatus() == ELactoseConfigCloudStatus::Loaded)
	{
		LoadCropActorClasses();
	}
	else
	{
		Lactose::Config::Events::OnConfigLoaded.AddUObject(this, &ThisClass::OnConfigCloudLoaded);
	}

	if (CanCreateCrops())
		CreateRequiredUserCrops();
}

void ULactoseCropWorldSubsystem::OnAllCropsLoaded(const ULactoseSimulationServiceSubsystem& Sender)
{
	if (bWaitingForCrops)
	{
		bWaitingForCrops = false;
		UE_LOG(LogLactose, Verbose, TEXT("Crop Subsystem: Received Crops"));

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
		UE_LOG(LogLactose, Verbose, TEXT("Crop Subsystem: Received User Crops"));
	}

	if (CanCreateCrops())
		CreateRequiredUserCrops();
}

void ULactoseCropWorldSubsystem::OnConfigCloudLoaded(
	const ULactoseConfigCloudServiceSubsystem& Sender,
   Sr<FLactoseConfigCloudConfig> Config)
{
	if (bWaitingForCropActorClassMap)
	{
		UE_LOG(LogLactose, Verbose, TEXT("Crop Subsystem: Received Config"));

		LoadCropActorClasses();
		
		if (CanCreateCrops())
			CreateRequiredUserCrops();
	}

	// Don't care about future events.
	Lactose::Config::Events::OnConfigLoaded.RemoveAll(this);
}

void ULactoseCropWorldSubsystem::CreateRequiredUserCrops()
{
	auto* Simulation = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseSimulationServiceSubsystem>();
	check(Simulation);

	const TMap<FString, Sr<FLactoseSimulationCrop>>& AllCrops = Simulation->GetAllCrops();
	if (AllCrops.IsEmpty())
		return;

	Sp<const FLactoseSimulationUserCrops> UserCrops = Simulation->GetCurrentUserCrops();
	if (!UserCrops)
		return;

	for (const Sr<FLactoseSimulationUserCropInstance>& UserCrop : UserCrops->GetAllCropInstances())
	{
		if (FindCropActorForCropInstance(UserCrop->Id))
			continue;
		
		auto Crop = Simulation->FindCrop(UserCrop->CropId);
		if (!Crop)
		{
			UE_LOG(LogLactose, Error, TEXT("Could not find a Crop with ID: '%s'"),
				*UserCrop->CropId);
			
			continue;
		}

		CreateUserCrop(Crop.ToSharedRef(), UserCrop);
	}
}

bool ULactoseCropWorldSubsystem::CreateUserCrop(
	const Sr<const FLactoseSimulationCrop>& Crop,
	const Sr<const FLactoseSimulationUserCropInstance>& CropInstance)
{
	if (!ensureMsgf(CropInstance->CropId == Crop->Id, TEXT("%s != %s"), *CropInstance->CropId, *Crop->Id))
	{
		return false;
	}

	if (!ensure(!FindCropActorForCropInstance(Crop->Id)))
	{
		return false;
	}

	TSubclassOf<ACropActor> CropActorClass = FindCropActorClassForCrop(Crop->Id);
	if (!CropActorClass)
	{
		UE_LOG(LogLactose, Error, TEXT("Could not find a Crop Actor class for Crop '%s'"),
			*Crop->Id);

#if UE_BUILD_SHIPPING || UE_BUILD_TEST 
		return false;
#else
		UE_LOG(LogLactose, Warning, TEXT("Fallbacking to default Crop Actor class for development purposes"));
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
		UE_LOG(LogLactose, Error, TEXT("Could not create a Crop Actor with class '%s'"),
			*CropActorClass->GetName());
		return false;
	}

	NewCropActor->Init(Crop, CropInstance);
	NewCropActor->FinishSpawning(CropTransform);

	UE_LOG(LogLactose, Verbose, TEXT("Created Crop Actor '%s' for Crop Instance '%s'"),
		*NewCropActor->GetActorNameOrLabel(),
		*CropInstance->Id);
	
	return true;
}

void ULactoseCropWorldSubsystem::LoadCropActorClasses()
{
	auto* ConfigSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseConfigCloudServiceSubsystem>();
	check(ConfigSubsystem);

	Sp<const FLactoseConfigCloudConfig> Config = ConfigSubsystem->GetConfig();
	if (!Config)
	{
		UE_LOG(LogLactose, Error, TEXT("Crop Subsystem: Config Subsystem does not have a Config"));
		return;
	}

	Sp<const FLactoseConfigCloudEntry> FoundCropIdToCropActorClassMap = Config->FindEntry(CropIdToCropActorMapEntryId);
	if (!FoundCropIdToCropActorClassMap)
	{
		UE_LOG(LogLactose, Error, TEXT("Crop Subsystem: Could not find the Crop Actor Classes Map in the Config (%s)"),
			*CropIdToCropActorMapEntryId);
		return;
	}

	const FLactoseCropActorClassesDatabase* DeserialisedMap = FoundCropIdToCropActorClassMap->Get<FLactoseCropActorClassesDatabase>();
	if (!DeserialisedMap)
	{
		UE_LOG(LogLactose, Error, TEXT("Crop Subsystem: The Crop Actor Classes Map in the Config (%s) could not be deserialised"),
			*CropIdToCropActorMapEntryId);
		return;
	}

	if (DeserialisedMap->Items.IsEmpty())
	{
		UE_LOG(LogLactose, Error, TEXT("Crop Subsystem: The Crop Actor Classes Map in the Config (%s) is empty"),
			*CropIdToCropActorMapEntryId);
		return;
	}

	for (const TTuple<FString, TSoftClassPtr<ACropActor>>& CropIdToCropActorMapping : DeserialisedMap->Items)
	{
		TSubclassOf<ACropActor> CropActorClass = CropIdToCropActorMapping.Value.LoadSynchronous();
		if (!CropActorClass)
		{
			UE_LOG(LogLactose, Error, TEXT("Crop Subsystem: Could not load Actor Class with path '%s'"),
				*CropIdToCropActorMapping.Value.ToString());
			continue;
		}

		CropIdToCropActorMap.Add(CropIdToCropActorMapping.Key, CropActorClass);
	}
	
	bWaitingForCropActorClassMap = false;
}
