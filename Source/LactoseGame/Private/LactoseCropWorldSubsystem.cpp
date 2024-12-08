// Fill out your copyright notice in the Description page of Project Settings.


#include "LactoseCropWorldSubsystem.h"

#include "CropActor.h"
#include "LactoseGame/LactoseGame.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

void ULactoseCropWorldSubsystem::RegisterCropActor(ACropActor& CropActor)
{
	TSharedPtr<const FLactoseSimulationUserCropInstance> CropInstance = CropActor.GetCropInstance();
	if (!ensure(CropInstance))
	{
		return;
	}
	
	RegisteredCropActors.Add(CropInstance->Id, &CropActor);
}

void ULactoseCropWorldSubsystem::DeregisterCropActor(const ACropActor& CropActor)
{
	TSharedPtr<const FLactoseSimulationUserCropInstance> CropInstance = CropActor.GetCropInstance();
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

	if (CanCreateCrops())
		CreateRequiredUserCrops();
}

void ULactoseCropWorldSubsystem::OnAllCropsLoaded(const ULactoseSimulationServiceSubsystem& Sender)
{
	if (bWaitingForCrops)
	{
		bWaitingForCrops = false;

		if (CanCreateCrops())
			CreateRequiredUserCrops();
	}

	// Don't care about future events.
	Lactose::Simulation::Events::OnAllCropsLoaded.RemoveAll(this);
}

void ULactoseCropWorldSubsystem::OnUserCropsLoaded(
	const ULactoseSimulationServiceSubsystem& Sender,
	TSharedRef<FLactoseSimulationUserCrops> UserCrops)
{
	if (bWaitingForUserCrops)
	{
		bWaitingForUserCrops = false;
		UE_LOG(LogLactose, Verbose, TEXT("Crop Subsystem: Received User Crops"));
		
		if (CanCreateCrops())
			CreateRequiredUserCrops();
	}
}

void ULactoseCropWorldSubsystem::CreateRequiredUserCrops()
{
	auto* Simulation = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseSimulationServiceSubsystem>();
	check(Simulation);

	const TMap<FString, TSharedRef<FLactoseSimulationCrop>>& AllCrops = Simulation->GetAllCrops();
	if (AllCrops.IsEmpty())
		return;

	TSharedPtr<const FLactoseSimulationUserCrops> UserCrops = Simulation->GetCurrentUserCrops();
	if (!UserCrops)
		return;

	for (const TSharedRef<FLactoseSimulationUserCropInstance>& UserCrop : UserCrops->GetAllCropInstances())
	{
		if (FindCropActorForCropInstance(UserCrop->Id))
			continue;
		
		auto Crop = Simulation->FindCrop(UserCrop->CropId);
		if (!Crop)
			continue;

		CreateUserCrop(Crop.ToSharedRef(), UserCrop);
	}
}

bool ULactoseCropWorldSubsystem::CreateUserCrop(
	const TSharedRef<const FLactoseSimulationCrop>& Crop,
	const TSharedRef<const FLactoseSimulationUserCropInstance>& CropInstance)
{
	if (!ensureMsgf(CropInstance->CropId == Crop->Id, TEXT("%s != %s"), *CropInstance->CropId, *Crop->Id))
	{
		return false;
	}

	if (!ensure(!FindCropActorForCropInstance(Crop->Id)))
	{
		return false;
	}

	// TODO: Find Crop Actor Class for Crop.
	TSubclassOf<ACropActor> CropActorClass = ACropActor::StaticClass();
	if (!CropActorClass)
	{
		UE_LOG(LogLactose, Error, TEXT("Could not find a Crop Actor class for Crop '%s'"),
			*Crop->Id);
		return false;
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
