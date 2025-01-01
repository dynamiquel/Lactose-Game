// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

#include "Services/LactoseServicesLog.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

TConstArrayView<Sr<FLactoseSimulationUserCropInstance>> FLactoseSimulationUserCrops::GetAllCropInstances() const
{
	return Database;
}

Sp<const FLactoseSimulationUserCropInstance> FLactoseSimulationUserCrops::FindCropInstance(
	const FString& CropInstanceId) const
{
	return const_cast<FLactoseSimulationUserCrops*>(this)->FindMutableCropInstance(CropInstanceId);
}

Sr<const FLactoseSimulationUserCropInstance> FLactoseSimulationUserCrops::UpdateCropInstance(
	const FLactoseSimulationUserCropInstance& NewCropInstanceData)
{
	auto FoundCropInstance = FindMutableCropInstance(NewCropInstanceData.Id);
	if (!FoundCropInstance)
	{
		auto NewCropInstance = CreateSr(NewCropInstanceData);
		Database.Add(NewCropInstance);
		return NewCropInstance;
	}

	*FoundCropInstance = NewCropInstanceData;
	return FoundCropInstance.ToSharedRef();
}

bool FLactoseSimulationUserCrops::DeleteCropInstance(const FString& CropInstanceId)
{
	const int32 FoundCropInstanceIndex = Database.IndexOfByPredicate([&CropInstanceId]
		(const Sr<FLactoseSimulationUserCropInstance>& CropInstance)
		{
			return CropInstance->Id == CropInstanceId;
		});

	if (FoundCropInstanceIndex == INDEX_NONE)
		return false;

	Database.RemoveAtSwap(FoundCropInstanceIndex, EAllowShrinking::No);
	return true;
}

TArray<Sr<const FLactoseSimulationUserCropInstance>> FLactoseSimulationUserCrops::FindCropInstances(
	TConstArrayView<FString> CropInstanceIds) const
{
	TArray<Sr<const FLactoseSimulationUserCropInstance>> FoundCropsInstances;
	for (const FString& CropInstanceId : CropInstanceIds)
	{
		Sp<const FLactoseSimulationUserCropInstance> FoundCropInstance = FindCropInstance(CropInstanceId);
		if (!FoundCropInstance)
			continue;

		FoundCropsInstances.Emplace(FoundCropInstance.ToSharedRef());	
	}

	return FoundCropsInstances;
}

TArray<Sr<FLactoseSimulationUserCropInstance>> FLactoseSimulationUserCrops::FindMutableCropInstances(
	TConstArrayView<FString> CropInstanceIds)
{
	TArray<Sr<FLactoseSimulationUserCropInstance>> FoundCropsInstances;
	for (const FString& CropInstanceId : CropInstanceIds)
	{
		Sp<FLactoseSimulationUserCropInstance> FoundCropInstance = FindMutableCropInstance(CropInstanceId);
		if (!FoundCropInstance)
			continue;

		FoundCropsInstances.Emplace(FoundCropInstance.ToSharedRef());	
	}

	return FoundCropsInstances;
}

Sp<FLactoseSimulationUserCropInstance> FLactoseSimulationUserCrops::FindMutableCropInstance(
	const FString& CropInstanceId)
{
	auto* FoundCropInstance = Database.FindByPredicate([&CropInstanceId]
		(const Sr<FLactoseSimulationUserCropInstance>& CropInstance)
		{
			return CropInstance->Id == CropInstanceId;
		});

	return FoundCropInstance ? FoundCropInstance->ToSharedPtr() : nullptr;
}

void FLactoseSimulationUserCrops::EmplaceCropInstance(const Sr<FLactoseSimulationUserCropInstance>& ExistingCropInstance)
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

Sp<const FLactoseSimulationCrop> ULactoseSimulationServiceSubsystem::FindCrop(const FString& CropId) const
{
	const Sr<FLactoseSimulationCrop>* FoundCrop = GetAllCrops().Find(CropId);
	return FoundCrop ? FoundCrop->ToSharedPtr() : nullptr;
}

void ULactoseSimulationServiceSubsystem::LoadAllCrops()
{
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FQuerySimulationCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("crops/query"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnAllCropsQueried);

	auto QueryAllItemsRequest = CreateSr<FLactoseSimulationQueryCropsRequest>();
	QueryAllCropsFuture = RestRequest->SetContentAsJsonAndSendAsync(QueryAllItemsRequest);

	Log::Verbose(LogLactoseSimulationService, TEXT("Sent a Query All Crops request"));
}

ELactoseSimulationUserCropsStatus ULactoseSimulationServiceSubsystem::GetCurrentUserCropsStatus() const
{
	if (GetCurrentUserCrops().IsValid())
		return ELactoseSimulationUserCropsStatus::Loaded;

	if (GetCurrentUserCropsFuture.IsValid())
		return ELactoseSimulationUserCropsStatus::Retrieving;

	return ELactoseSimulationUserCropsStatus::None;
}

Sp<const FLactoseSimulationUserCrops> ULactoseSimulationServiceSubsystem::GetCurrentUserCrops() const
{
	return CurrentUserCrops;
}

bool ULactoseSimulationServiceSubsystem::CanCurrentUserAffordCrop(const FString& CropId) const
{
	const Sp<const FLactoseSimulationCrop> FoundCrop = FindCrop(CropId);
	if (!FoundCrop)
		return false;

	for (const FLactoseEconomyUserItem& CostItem : FoundCrop->CostItems)
	{
		const auto& EconomySubsystem = Lactose::GetService<ULactoseEconomyServiceSubsystem>(*this);
		Sp<const FLactoseEconomyUserItem> FoundUserItem = EconomySubsystem.FindCurrentUserItem(CostItem.ItemId);
		if (!FoundUserItem)
			return false;

		if (FoundUserItem->Quantity < CostItem.Quantity)
			return false;
	}
	
	return true;
}

Sp<FLactoseSimulationUserCrops> ULactoseSimulationServiceSubsystem::GetMutableCurrentUserCrops()
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
		return Log::Error(LogLactoseSimulationService, TEXT("Cannot load current user's crops because the user is not logged in"));
	}
	
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsRetrieved);

	auto GetCurrentUserCropsRequest = CreateSr<FLactoseSimulationGetUserCropsRequest>();
	GetCurrentUserCropsRequest->UserId = CurrentUserInfo->Id;
	GetCurrentUserCropsFuture = RestRequest->SetContentAsJsonAndSendAsync(GetCurrentUserCropsRequest);

	Log::Verbose(LogLactoseSimulationService, TEXT("Sent a Get User Crops request"));

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
		return Log::Error(LogLactoseSimulationService, TEXT("Cannot simulate current user's crops because the user is not logged in"));
	}
	
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FSimulateSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/simulate"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsSimulated);

	auto SimulateRequest = CreateSr<FLactoseSimulationSimulateUserCropsRequest>();
	SimulateRequest->UserId = CurrentUserInfo->Id;
	SimulateCurrentUserCropsFuture = RestRequest->SetContentAsJsonAndSendAsync(SimulateRequest);

	Log::Verbose(LogLactoseSimulationService, TEXT("Sent a Simulate User Crops request"));
}

void ULactoseSimulationServiceSubsystem::HarvestCropInstances(TConstArrayView<FString> CropInstanceIds)
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	Sp<FLactoseIdentityGetUserResponse> CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		return Log::Error(LogLactoseSimulationService, TEXT("Cannot harvest current user's crops because the user is not logged in"));
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FHarvestSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/harvest"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsHarvested);

	auto Request = CreateSr<FLactoseSimulationHarvestUserCropsRequest>();
	Request->UserId = CurrentUserInfo->Id;
	Request->CropInstanceIds.Append(CropInstanceIds);
	RestRequest->SetContentAsJsonAndSendAsync(Request);

	Log::Verbose(LogLactoseSimulationService, TEXT("Sent a Harvest User Crops request"));
}

void ULactoseSimulationServiceSubsystem::SeedCropInstances(
	TConstArrayView<FString> CropInstanceIds,
	const FString& CropId)
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	Sp<FLactoseIdentityGetUserResponse> CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		return Log::Error(LogLactoseSimulationService, TEXT("Cannot seed current user's crops because the user is not logged in"));
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FSeedSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/seed"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsSeeded);

	auto Request = CreateSr<FLactoseSimulationSeedUserCropsRequest>();
	Request->UserId = CurrentUserInfo->Id;
	Request->CropInstanceIds.Append(CropInstanceIds);
	Request->CropId = CropId;
	RestRequest->SetContentAsJsonAndSendAsync(Request);

	Log::Verbose(LogLactoseSimulationService, TEXT("Sent a Seed User Crops request"));
}

void ULactoseSimulationServiceSubsystem::FertiliseCropInstances(TConstArrayView<FString> CropInstanceIds)
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	Sp<FLactoseIdentityGetUserResponse> CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		return Log::Error(LogLactoseSimulationService, TEXT("Cannot fertilise current user's crops because the user is not logged in"));
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FFertiliseSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/fertilise"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsFertilised);

	auto Request = CreateSr<FLactoseSimulationFertiliseUserCropsRequest>();
	Request->UserId = CurrentUserInfo->Id;
	Request->CropInstanceIds.Append(CropInstanceIds);
	RestRequest->SetContentAsJsonAndSendAsync(Request);

	Log::Verbose(LogLactoseSimulationService, TEXT("Sent a Fertilise User Crops request"));
}

void ULactoseSimulationServiceSubsystem::DestroyCropInstances(TConstArrayView<FString> CropInstanceIds)
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	Sp<FLactoseIdentityGetUserResponse> CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		return Log::Error(LogLactoseSimulationService, TEXT("Cannot destroy current user's crops because the user is not logged in"));
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FDeleteSimulationUserCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/destroy"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsDestroyed);

	auto Request = CreateSr<FLactoseSimulationDeleteUserCropsRequest>();
	Request->UserId = CurrentUserInfo->Id;
	Request->CropInstanceIds.Append(CropInstanceIds);
	RestRequest->SetContentAsJsonAndSendAsync(Request);

	Log::Verbose(LogLactoseSimulationService, TEXT("Sent a Destroy User Crops request"));
}

void ULactoseSimulationServiceSubsystem::CreateEmptyPlot(const FVector& Location, const FRotator& Rotation)
{
	CreateCrop(TEXT(""), Location, Rotation);
}

void ULactoseSimulationServiceSubsystem::CreateCrop(const FString& CropId, const FVector& Location, const FRotator& Rotation)
{
	auto IdentitySubsystem = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;

	Sp<FLactoseIdentityGetUserResponse> CurrentUserInfo = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUserInfo)
	{
		return Log::Error(LogLactoseSimulationService, TEXT("Cannot destroy current user's crops because the user is not logged in"));
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FCreateSimulationUserCropRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("usercrops/create"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnCurrentUserCropsCreated);

	auto Request = CreateSr<FLactoseSimulationCreateUserCropRequest>();
	Request->UserId = CurrentUserInfo->Id;
	Request->CropId = CropId;
	Request->CropLocation = Location;
	Request->CropRotation = FVector(Rotation.Pitch, Rotation.Yaw, Rotation.Roll);
	RestRequest->SetContentAsJsonAndSendAsync(Request);

	Log::Verbose(LogLactoseSimulationService, TEXT("Sent a Create User Crop request"));
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

void ULactoseSimulationServiceSubsystem::OnAllCropsQueried(Sr<FQuerySimulationCropsRequest::FResponseContext> Context)
{
	QueryAllCropsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->CropIds.IsEmpty())
	{
		return Log::Error(LogLactoseSimulationService, TEXT("Query Crops request responed with no items"));
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetSimulationCropsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("crops"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnAllCropsRetrieved);

	auto GetAllItemsRequest = CreateSr<FLactoseSimulationGetCropsRequest>();
	GetAllItemsRequest->CropIds.Append(Context->ResponseContent->CropIds);
	GetAllCropsFuture = RestRequest->SetContentAsJsonAndSendAsync(GetAllItemsRequest);

	Log::Verbose(LogLactoseSimulationService, TEXT("Sent a Get All Crops request"));
}

void ULactoseSimulationServiceSubsystem::OnAllCropsRetrieved(Sr<FGetSimulationCropsRequest::FResponseContext> Context)
{
	GetAllCropsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->Crops.IsEmpty())
	{
		return Log::Error(LogLactoseSimulationService, TEXT("Get Crops request responed with no items"));
	}

	// Add or update crops.
	for (const FLactoseSimulationCrop& Crop : Context->ResponseContent->Crops)
	{
		if (Sr<FLactoseSimulationCrop>* ExistingCrop = AllCrops.Find(Crop.Id))
		{
			ExistingCrop->Get() = Crop;
		}
		else
		{
			AllCrops.Emplace(Crop.Id, CreateSr(Crop));
		}
	}

	if (!FindCrop(TEXT("")))
	{
		// Create the placeholder 'empty' crop to make the rest of the game happy.
		auto EmptyCrop = CreateSr(FLactoseSimulationCrop
		{
			.Type = Lactose::Simulation::Types::Plot,
			.Name =  TEXT("Empty"),
		});
		
		AllCrops.Add(TEXT(""), EmptyCrop);
	}

	Log::Verbose(LogLactoseSimulationService,
		TEXT("Added or updated %d Crops"),
		Context->ResponseContent->Crops.Num());

	Lactose::Simulation::Events::OnAllCropsLoaded.Broadcast(*this);
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsRetrieved(Sr<FGetSimulationUserCropsRequest::FResponseContext> Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ULactoseSimulationServiceSubsystem::OnCurrentUserCropsRetrieved);
	
	GetCurrentUserCropsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
		return;

	PreviousUserSimulationTime = Context->ResponseContent->PreviousSimulationTime;

	if (!CurrentUserCrops)
		CurrentUserCrops = CreateSr<FLactoseSimulationUserCrops>();

	TSet<FString> ExistingCropInstanceIds;
	for (const Sr<FLactoseSimulationUserCropInstance>& ExistingCrop : GetMutableCurrentUserCrops()->GetAllCropInstances())
	{
		ExistingCropInstanceIds.Add(ExistingCrop->Id);
	}
	
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Update Crop Instances");

		for (const FLactoseSimulationUserCropInstance& CropInstance : Context->ResponseContent->CropInstances)
		{
			ExistingCropInstanceIds.Remove(CropInstance.Id);
			auto SharedCropInstance = CurrentUserCrops->UpdateCropInstance(CropInstance);
			SharedCropInstance->OnLoaded.Broadcast(SharedCropInstance);
		}
	}
	
	if (!ExistingCropInstanceIds.IsEmpty())
	{
		// Anything in Existing Crops is a crop that no longer exists.
		// Forward this to the Delete function.
		
		auto DestroyRequest = CreateSr<FDeleteSimulationUserCropsRequest::FResponseContext>();
		DestroyRequest->ResponseContent = CreateSr<FLactoseSimulationDeleteUserCropsResponse>();
		DestroyRequest->ResponseContent->DestroyedCropInstanceIds.Append(ExistingCropInstanceIds.Array());
		OnCurrentUserCropsDestroyed(DestroyRequest);
	}

	Lactose::Simulation::Events::OnCurrentUserCropsLoaded.Broadcast(*this, CurrentUserCrops.ToSharedRef());
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsSimulated(Sr<FSimulateSimulationUserCropsRequest::FResponseContext> Context)
{
	SimulateCurrentUserCropsFuture.Reset();
	
	if (!Context->ResponseContent.IsValid())
		return;
	
	Log::Verbose(LogLactoseSimulationService, TEXT("User Crops have been simulated"));

	Lactose::Simulation::Events::OnCurrentUserCropsSimulated.Broadcast(
		*this,
		Context->ResponseContent->PreviousSimulationTime,
		Context->ResponseContent->NewSimulationTime);

	if (bRefreshUserCropsOnSimulated)
		LoadCurrentUserCrops();
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsHarvested(Sr<FHarvestSimulationUserCropsRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->HarvestedCropInstanceIds.IsEmpty())
	{
		return Log::Warning(LogLactoseSimulationService, TEXT("No User Crops were harvested"));
	}

	TArray<Sr<FLactoseSimulationUserCropInstance>> HarvestedCropInstances =
		GetMutableCurrentUserCrops()->FindMutableCropInstances(Context->ResponseContent->HarvestedCropInstanceIds);

	Log::Verbose(LogLactoseSimulationService,
		TEXT("Harvested %d User Crops"),
		Context->ResponseContent->HarvestedCropInstanceIds.Num());

	Lactose::Simulation::Events::OnCurrentUserCropsHarvested.Broadcast(
		*this,
		static_cast<TArray<Sr<const FLactoseSimulationUserCropInstance>>>(HarvestedCropInstances));

	for (const auto& HarvestedCrop : HarvestedCropInstances)
		HarvestedCrop->OnHarvested.Broadcast(HarvestedCrop);

	if (bClientSidePrediction)
	{
		// Client-side prediction until next simulation tick happens.
		for (const auto& HarvestedCropInstance : HarvestedCropInstances)
		{
			Sp<const FLactoseSimulationCrop> FoundCrop = FindCrop(HarvestedCropInstance->CropId);
			if (!FoundCrop)
				continue;

			if (FoundCrop->Type == Lactose::Simulation::Types::Plot)
			{
				HarvestedCropInstance->State = Lactose::Simulation::States::Empty;
			}
			else
			{
				HarvestedCropInstance->State = Lactose::Simulation::States::Growing;
				HarvestedCropInstance->RemainingHarvestSeconds = FoundCrop->HarvestSeconds;
			}

			HarvestedCropInstance->OnLoaded.Broadcast(HarvestedCropInstance);
		}
	}
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsSeeded(Sr<FSeedSimulationUserCropsRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->SeededCropInstanceIds.IsEmpty())
	{
		return Log::Warning(LogLactoseSimulationService, TEXT("No User Crops were seeded"));
	}

	const TArray<Sr<FLactoseSimulationUserCropInstance>> SeededCropInstances =
		GetMutableCurrentUserCrops()->FindMutableCropInstances(Context->ResponseContent->SeededCropInstanceIds);

	Log::Verbose(LogLactoseSimulationService,
		TEXT("Seeded %d User Crops"),
		Context->ResponseContent->SeededCropInstanceIds.Num());

	Lactose::Simulation::Events::OnCurrentUserCropsSeeded.Broadcast(
		*this,
		static_cast<TArray<Sr<const FLactoseSimulationUserCropInstance>>>(SeededCropInstances));

	for (const auto& SeededCrop : SeededCropInstances)
		SeededCrop->OnSeeded.Broadcast(SeededCrop);

	if (bClientSidePrediction)
	{
		// Client-side prediction until next simulation tick happens.
		for (const auto& SeededCropInstance : SeededCropInstances)
		{
			Sp<const FLactoseSimulationCrop> FoundCrop = FindCrop(SeededCropInstance->CropId);
			if (!FoundCrop)
				continue;
			
			SeededCropInstance->State = Lactose::Simulation::States::Growing;
			SeededCropInstance->RemainingHarvestSeconds = FoundCrop->HarvestSeconds;
			SeededCropInstance->OnLoaded.Broadcast(SeededCropInstance);
		}
	}
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsFertilised(Sr<FFertiliseSimulationUserCropsRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->FertilisedCropInstanceIds.IsEmpty())
	{
		return Log::Warning(LogLactoseSimulationService, TEXT("No User Crops were fertilised"));
	}

	const TArray<Sr<const FLactoseSimulationUserCropInstance>> FertilisedCropInstances =
		GetMutableCurrentUserCrops()->FindCropInstances(Context->ResponseContent->FertilisedCropInstanceIds);

	Log::Verbose(LogLactoseSimulationService,
		TEXT("Fertilised %d User Crops"),
		Context->ResponseContent->FertilisedCropInstanceIds.Num());

	Lactose::Simulation::Events::OnCurrentUserCropsFertilised.Broadcast(*this, FertilisedCropInstances);

	for (const auto& FertilisedCrop : FertilisedCropInstances)
		FertilisedCrop->OnFertilised.Broadcast(FertilisedCrop);
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsDestroyed(Sr<FDeleteSimulationUserCropsRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->DestroyedCropInstanceIds.IsEmpty())
	{
		return Log::Warning(LogLactoseSimulationService, TEXT("No User Crops were destroyed"));
	}
	
	const TArray<Sr<const FLactoseSimulationUserCropInstance>> DestroyedCropInstances =
		GetMutableCurrentUserCrops()->FindCropInstances(Context->ResponseContent->DestroyedCropInstanceIds);

	for (const auto& DestroyedCropInstance : DestroyedCropInstances)
	{
		CurrentUserCrops->DeleteCropInstance(DestroyedCropInstance->Id);
		DestroyedCropInstance->OnDestroyed.Broadcast(DestroyedCropInstance);
	}

	Log::Verbose(LogLactoseSimulationService,
		TEXT("Deleted %d User Crops"),
		DestroyedCropInstances.Num());

	Lactose::Simulation::Events::OnCurrentUserCropsDestroyed.Broadcast(*this, DestroyedCropInstances);
}

void ULactoseSimulationServiceSubsystem::OnCurrentUserCropsCreated(
	Sr<FCreateSimulationUserCropRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->UserCropInstanceId.IsEmpty())
	{
		return Log::Warning(LogLactoseSimulationService, TEXT("No User Crop was created"));
	}
	
	Lactose::Simulation::Events::OnCurrentUserCropsCreated.Broadcast(*this, {});

	// TODO: Client-side prediction.
}

void ULactoseSimulationServiceSubsystem::OnUserLoggedIn(
	const ULactoseIdentityServiceSubsystem& Sender,
	const Sr<FLactoseIdentityGetUserResponse>& User)
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
