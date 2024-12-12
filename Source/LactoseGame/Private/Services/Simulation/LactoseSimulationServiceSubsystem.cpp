// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

#include "Services/LactoseServicesLog.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

TConstArrayView<TSharedRef<FLactoseSimulationUserCropInstance>> FLactoseSimulationUserCrops::GetAllCropInstances() const
{
	return Database;
}

TSharedPtr<const FLactoseSimulationUserCropInstance> FLactoseSimulationUserCrops::FindCropInstance(
	const FString& CropInstanceId) const
{
	return const_cast<FLactoseSimulationUserCrops*>(this)->FindCropInstance(CropInstanceId);
}

TSharedRef<const FLactoseSimulationUserCropInstance> FLactoseSimulationUserCrops::UpdateCropInstance(
	const FLactoseSimulationUserCropInstance& NewCropInstanceData)
{
	auto FoundCropInstance = FindCropInstance(NewCropInstanceData.Id);
	if (!FoundCropInstance)
	{
		auto NewCropInstance = MakeShared<FLactoseSimulationUserCropInstance>(NewCropInstanceData);
		Database.Add(NewCropInstance);
		return NewCropInstance;
	}

	*FoundCropInstance = NewCropInstanceData;
	return FoundCropInstance.ToSharedRef();
}

bool FLactoseSimulationUserCrops::DeleteCropInstance(const FString& CropInstanceId)
{
	const int32 FoundCropInstanceIndex = Database.IndexOfByPredicate([&CropInstanceId]
	(const TSharedRef<FLactoseSimulationUserCropInstance>& CropInstance)
		{
			return CropInstance->Id == CropInstanceId;
		});

	if (FoundCropInstanceIndex == INDEX_NONE)
		return false;

	Database.RemoveAtSwap(FoundCropInstanceIndex, EAllowShrinking::No);
	return true;
}

TArray<TSharedRef<const FLactoseSimulationUserCropInstance>> FLactoseSimulationUserCrops::FindCropInstances(
	TConstArrayView<FString> CropInstanceIds) const
{
	TArray<TSharedRef<const FLactoseSimulationUserCropInstance>> FoundCropsInstances;
	for (const FString& CropInstanceId : CropInstanceIds)
	{
		TSharedPtr<const FLactoseSimulationUserCropInstance> FoundCropInstance = FindCropInstance(CropInstanceId);
		if (!FoundCropInstance)
			continue;

		FoundCropsInstances.Emplace(FoundCropInstance.ToSharedRef());	
	}

	return FoundCropsInstances;
}

TSharedPtr<FLactoseSimulationUserCropInstance> FLactoseSimulationUserCrops::FindCropInstance(
	const FString& CropInstanceId)
{
	auto* FoundCropInstance = Database.FindByPredicate([&CropInstanceId]
	(const TSharedRef<FLactoseSimulationUserCropInstance>& CropInstance)
		{
			return CropInstance->Id == CropInstanceId;
		});

	return FoundCropInstance ? FoundCropInstance->ToSharedPtr() : nullptr;
}

void FLactoseSimulationUserCrops::EmplaceCropInstance(const TSharedRef<FLactoseSimulationUserCropInstance>& ExistingCropInstance)
{
	Database.Add(ExistingCropInstance);
}

ULactoseSimulationServiceSubsystem::ULactoseSimulationServiceSubsystem()
{
	SetServiceBaseUrl(TEXT("https://lactose.mookrata.ovh/simulation"));
}

void ULactoseSimulationServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Lactose::Identity::Events::OnUserLoggedIn.AddUObject(this, &ThisClass::OnUserLoggedIn);
	Lactose::Identity::Events::OnUserLoggedOut.AddUObject(this, &ThisClass::OnUserLoggedOut);
}

ELactoseSimulationCropsStatus ULactoseSimulationServiceSubsystem::GetAllCropsStatus() const
{
	if (!GetAllCrops().IsEmpty())
		return ELactoseSimulationCropsStatus::Loaded;

	if (GetAllCropsFuture.IsValid())
		return ELactoseSimulationCropsStatus::Retrieving;

	if (QueryAllCropsFuture.IsValid())
		return ELactoseSimulationCropsStatus::Querying;

	return ELactoseSimulationCropsStatus::None;
}

TSharedPtr<const FLactoseSimulationCrop> ULactoseSimulationServiceSubsystem::FindCrop(const FString& CropId) const
{
	const TSharedRef<FLactoseSimulationCrop>* FoundCrop = GetAllCrops().Find(CropId);
	return FoundCrop ? FoundCrop->ToSharedPtr() : nullptr;
}

void ULactoseSimulationServiceSubsystem::LoadAllCrops()
{
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FQuerySimulationCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("crops/query"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnAllCropsQueried);

	auto QueryAllItemsRequest = MakeShared<FLactoseSimulationQueryCropsRequest>();
	QueryAllCropsFuture = RestRequest->SetContentAsJsonAndSendAsync(QueryAllItemsRequest);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Sent a Query All Crops request"));
}

ELactoseSimulationUserCropsStatus ULactoseSimulationServiceSubsystem::GetCurrentUserCropsStatus() const
{
	if (GetCurrentUserCrops().IsValid())
		return ELactoseSimulationUserCropsStatus::Loaded;

	if (GetCurrentUserCropsFuture.IsValid())
		return ELactoseSimulationUserCropsStatus::Retrieving;

	return ELactoseSimulationUserCropsStatus::None;
}

TSharedPtr<const FLactoseSimulationUserCrops> ULactoseSimulationServiceSubsystem::GetCurrentUserCrops() const
{
	return CurrentUserCrops;
}

void ULactoseSimulationServiceSubsystem::LoadCurrentUserCrops()
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	auto CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		UE_LOG(LogLactoseSimulationService, Error, TEXT("Cannot load current user's crops because the user is not logged in"));
		return;
	}
	
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsRetrieved);

	auto GetCurrentUserCropsRequest = MakeShared<FLactoseSimulationGetUserCropsRequest>();
	GetCurrentUserCropsRequest->UserId = CurrentUserInfo->Id;
	GetCurrentUserCropsFuture = RestRequest->SetContentAsJsonAndSendAsync(GetCurrentUserCropsRequest);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Sent a Get User Crops request"));

	if (!SimulateTicker.IsValid())
		EnableSimulateTicker();
}

void ULactoseSimulationServiceSubsystem::Simulate()
{
	// Simulate Request is still pending. Wait until it is finished before sending a new one.
	if (IsPending(SimulateCurrentUserCropsFuture))
		return;
	
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	auto CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		UE_LOG(LogLactoseSimulationService, Error, TEXT("Cannot simulate current user's crops because the user is not logged in"));
		return;
	}
	
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FSimulateSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/simulate"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsSimulated);

	auto SimulateRequest = MakeShared<FLactoseSimulationSimulateUserCropsRequest>();
	SimulateRequest->UserId = CurrentUserInfo->Id;
	SimulateCurrentUserCropsFuture = RestRequest->SetContentAsJsonAndSendAsync(SimulateRequest);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Sent a Simulate User Crops request"));
}

void ULactoseSimulationServiceSubsystem::HarvestCropInstances(TConstArrayView<FString> CropInstanceIds)
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	TSharedPtr<FLactoseIdentityGetUserResponse> CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		UE_LOG(LogLactoseSimulationService, Error, TEXT("Cannot harvest current user's crops because the user is not logged in"));
		return;
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FHarvestSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/harvest"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsHarvested);

	auto Request = MakeShared<FLactoseSimulationHarvestUserCropsRequest>();
	Request->UserId = CurrentUserInfo->Id;
	Request->CropInstanceIds.Append(CropInstanceIds);
	RestRequest->SetContentAsJsonAndSendAsync(Request);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Sent a Harvest User Crops request"));
}

void ULactoseSimulationServiceSubsystem::SeedCropInstances(
	TConstArrayView<FString> CropInstanceIds,
	const FString& CropId)
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	TSharedPtr<FLactoseIdentityGetUserResponse> CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		UE_LOG(LogLactoseSimulationService, Error, TEXT("Cannot seed current user's crops because the user is not logged in"));
		return;
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FSeedSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/seed"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsSeeded);

	auto Request = MakeShared<FLactoseSimulationSeedUserCropsRequest>();
	Request->UserId = CurrentUserInfo->Id;
	Request->CropInstanceIds.Append(CropInstanceIds);
	Request->CropId = CropId;
	RestRequest->SetContentAsJsonAndSendAsync(Request);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Sent a Seed User Crops request"));
}

void ULactoseSimulationServiceSubsystem::FertiliseCropInstances(TConstArrayView<FString> CropInstanceIds)
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	TSharedPtr<FLactoseIdentityGetUserResponse> CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		UE_LOG(LogLactoseSimulationService, Error, TEXT("Cannot fertilise current user's crops because the user is not logged in"));
		return;
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FFertiliseSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/fertilise"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsFertilised);

	auto Request = MakeShared<FLactoseSimulationFertiliseUserCropsRequest>();
	Request->UserId = CurrentUserInfo->Id;
	Request->CropInstanceIds.Append(CropInstanceIds);
	RestRequest->SetContentAsJsonAndSendAsync(Request);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Sent a Fertilise User Crops request"));
}

void ULactoseSimulationServiceSubsystem::DestroyCropInstances(TConstArrayView<FString> CropInstanceIds)
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	TSharedPtr<FLactoseIdentityGetUserResponse> CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		UE_LOG(LogLactoseSimulationService, Error, TEXT("Cannot destroy current user's crops because the user is not logged in"));
		return;
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FDeleteSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/destroy"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsDestroyed);

	auto Request = MakeShared<FLactoseSimulationDeleteUserCropsRequest>();
	Request->UserId = CurrentUserInfo->Id;
	Request->CropInstanceIds.Append(CropInstanceIds);
	RestRequest->SetContentAsJsonAndSendAsync(Request);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Sent a Destroy User Crops request"));
}

void ULactoseSimulationServiceSubsystem::EnableSimulateTicker()
{
	GetGameInstance()->GetTimerManager().SetTimer(
		SimulateTicker,
		this,
		&ThisClass::OnSimulateTick,
		SimulateTickInterval,
		/* bLoop */ true);
}

void ULactoseSimulationServiceSubsystem::DisableSimulateTicker()
{
	GetGameInstance()->GetTimerManager().ClearTimer(SimulateTicker);
	SimulateTicker.Invalidate();
}

void ULactoseSimulationServiceSubsystem::OnAllCropsQueried(TSharedRef<FQuerySimulationCropsRequest::FResponseContext> Context)
{
	QueryAllCropsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->CropIds.IsEmpty())
	{
		UE_LOG(LogLactoseSimulationService, Error, TEXT("Query Crops request responed with no items"));
		return;
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetSimulationCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("crops"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnAllCropsRetrieved);

	auto GetAllItemsRequest = MakeShared<FLactoseSimulationGetCropsRequest>();
	GetAllItemsRequest->CropIds.Append(Context->ResponseContent->CropIds);
	GetAllCropsFuture = RestRequest->SetContentAsJsonAndSendAsync(GetAllItemsRequest);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Sent a Get All Crops request"));
}

void ULactoseSimulationServiceSubsystem::OnAllCropsRetrieved(TSharedRef<FGetSimulationCropsRequest::FResponseContext> Context)
{
	GetAllCropsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->Crops.IsEmpty())
	{
		UE_LOG(LogLactoseSimulationService, Error, TEXT("Get Crops request responed with no items"));
		return;
	}

	// Add or update crops.
	for (const FLactoseSimulationCrop& Crop : Context->ResponseContent->Crops)
	{
		if (TSharedRef<FLactoseSimulationCrop>* ExistingCrop = AllCrops.Find(Crop.Id))
		{
			ExistingCrop->Get() = Crop;
		}
		else
		{
			AllCrops.Emplace(Crop.Id, MakeShared<FLactoseSimulationCrop>(Crop));
		}
	}

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Added or updated %d Crops"),
		Context->ResponseContent->Crops.Num());

	Lactose::Simulation::Events::OnAllCropsLoaded.Broadcast(*this);
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsRetrieved(TSharedRef<FGetSimulationUserCropsRequest::FResponseContext> Context)
{
	GetCurrentUserCropsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
		return;

	PreviousUserSimulationTime = Context->ResponseContent->PreviousSimulationTime;

	if (!CurrentUserCrops)
		CurrentUserCrops = MakeShared<FLactoseSimulationUserCrops>();

	TSet<FString> ExistingCropInstanceIds;
	for (const TSharedRef<FLactoseSimulationUserCropInstance>& ExistingCrop : GetCurrentUserCrops()->GetAllCropInstances())
		ExistingCropInstanceIds.Add(ExistingCrop->Id);

	for (const FLactoseSimulationUserCropInstance& CropInstance : Context->ResponseContent->CropInstances)
	{
		ExistingCropInstanceIds.Remove(CropInstance.Id);
		auto SharedCropInstance = CurrentUserCrops->UpdateCropInstance(CropInstance);
		SharedCropInstance->OnLoaded.Broadcast(SharedCropInstance);
	}
	
	if (!ExistingCropInstanceIds.IsEmpty())
	{
		// Anything in Existing Crops is a crop that no longer exists.
		// Forward this to the Delete function.
		
		auto DestroyRequest = MakeShared<FDeleteSimulationUserCropsRequest::FResponseContext>();
		DestroyRequest->ResponseContent = MakeShared<FLactoseSimulationDeleteUserCropsResponse>();
		DestroyRequest->ResponseContent->DeletedCropInstanceIds.Append(ExistingCropInstanceIds.Array());
		OnCurrentUserCropsDestroyed(DestroyRequest);
	}

	Lactose::Simulation::Events::OnCurrentUserCropsLoaded.Broadcast(*this, CurrentUserCrops.ToSharedRef());
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsSimulated(TSharedRef<FSimulateSimulationUserCropsRequest::FResponseContext> Context)
{
	SimulateCurrentUserCropsFuture.Reset();
	
	if (!Context->ResponseContent.IsValid())
		return;
	
	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("User Crops have been simulated"));

	Lactose::Simulation::Events::OnCurrentUserCropsSimulated.Broadcast(
		*this,
		Context->ResponseContent->PreviousSimulationTime,
		Context->ResponseContent->NewSimulationTime);

	if (bRefreshUserCropsOnSimulated)
		LoadCurrentUserCrops();
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsHarvested(TSharedRef<FHarvestSimulationUserCropsRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->HarvestedCropInstanceIds.IsEmpty())
	{
		UE_LOG(LogLactoseSimulationService, Warning, TEXT("No User Crops were harvested"));
		return;
	}

	const TArray<TSharedRef<const FLactoseSimulationUserCropInstance>> HarvestedCropInstances =
		GetCurrentUserCrops()->FindCropInstances(Context->ResponseContent->HarvestedCropInstanceIds);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Harvested %d User Crops"),
		Context->ResponseContent->HarvestedCropInstanceIds.Num());

	Lactose::Simulation::Events::OnCurrentUserCropsHarvested.Broadcast(*this, HarvestedCropInstances);

	for (const auto& HarvestedCrop : HarvestedCropInstances)
		HarvestedCrop->OnHarvested.Broadcast(HarvestedCrop);
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsSeeded(TSharedRef<FSeedSimulationUserCropsRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->SeededCropInstanceIds.IsEmpty())
	{
		UE_LOG(LogLactoseSimulationService, Warning, TEXT("No User Crops were seeded"));
		return;
	}

	const TArray<TSharedRef<const FLactoseSimulationUserCropInstance>> SeededCropInstances =
		GetCurrentUserCrops()->FindCropInstances(Context->ResponseContent->SeededCropInstanceIds);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Seeded %d User Crops"),
		Context->ResponseContent->SeededCropInstanceIds.Num());

	Lactose::Simulation::Events::OnCurrentUserCropsSeeded.Broadcast(*this, SeededCropInstances);

	for (const auto& SeededCrop : SeededCropInstances)
		SeededCrop->OnSeeded.Broadcast(SeededCrop);
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsFertilised(TSharedRef<FFertiliseSimulationUserCropsRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->FertilisedCropInstanceIds.IsEmpty())
	{
		UE_LOG(LogLactoseSimulationService, Warning, TEXT("No User Crops were fertilised"));
		return;
	}

	const TArray<TSharedRef<const FLactoseSimulationUserCropInstance>> FertilisedCropInstances =
		GetCurrentUserCrops()->FindCropInstances(Context->ResponseContent->FertilisedCropInstanceIds);

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Fertilised %d User Crops"),
		Context->ResponseContent->FertilisedCropInstanceIds.Num());

	Lactose::Simulation::Events::OnCurrentUserCropsFertilised.Broadcast(*this, FertilisedCropInstances);

	for (const auto& FertilisedCrop : FertilisedCropInstances)
		FertilisedCrop->OnFertilised.Broadcast(FertilisedCrop);
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsDestroyed(TSharedRef<FDeleteSimulationUserCropsRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->DeletedCropInstanceIds.IsEmpty())
	{
		UE_LOG(LogLactoseSimulationService, Warning, TEXT("No User Crops were destroyed"));
		return;
	}

	
	const TArray<TSharedRef<const FLactoseSimulationUserCropInstance>> DestroyedCropInstances =
		GetCurrentUserCrops()->FindCropInstances(Context->ResponseContent->DeletedCropInstanceIds);

	for (const auto& DestroyedCropInstance : DestroyedCropInstances)
	{
		CurrentUserCrops->DeleteCropInstance(DestroyedCropInstance->Id);
		DestroyedCropInstance->OnDestroyed.Broadcast(DestroyedCropInstance);
	}

	UE_LOG(LogLactoseSimulationService, Verbose, TEXT("Deleted %d User Crops"),
		DestroyedCropInstances.Num());

	Lactose::Simulation::Events::OnCurrentUserCropsDestroyed.Broadcast(*this, DestroyedCropInstances);

	for (const auto& DestroyedCrop : DestroyedCropInstances)
		DestroyedCrop->OnDestroyed.Broadcast(DestroyedCrop);
}

void ULactoseSimulationServiceSubsystem::OnUserLoggedIn(
	const ULactoseIdentityServiceSubsystem& Sender,
	const TSharedRef<FLactoseIdentityGetUserResponse>& User)
{
	if (bAutoRetrieveCrops)
		LoadAllCrops();

	LoadCurrentUserCrops();
}

void ULactoseSimulationServiceSubsystem::OnUserLoggedOut(const ULactoseIdentityServiceSubsystem& Sender)
{
	DisableSimulateTicker();
	CurrentUserCrops.Reset();
}

void ULactoseSimulationServiceSubsystem::OnSimulateTick()
{	
	Simulate();
}
