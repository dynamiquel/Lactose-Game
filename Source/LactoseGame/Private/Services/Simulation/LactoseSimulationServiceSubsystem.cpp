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

ULactoseSimulationServiceSubsystem::ULactoseSimulationServiceSubsystem()
{
	SetServiceBaseUrl(TEXT("https://lactose.mookrata.ovh/simulation"));
}

void ULactoseSimulationServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Lactose::Identity::Events::OnUserLoggedIn.AddUObject(this, &ThisClass::OnUserLoggedIn);

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
		UE_LOG(LogLactoseSimulationService, Error, TEXT("Cannot load current user's items because the user is not logged in"));
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
		CurrentUserCrops->UpdateCropInstance(CropInstance);
	}

	// Anything in Existing Crops is a crop that no longer exists.
	for (const FString& LingeringCropInstanceId : ExistingCropInstanceIds)
		CurrentUserCrops->DeleteCropInstance(LingeringCropInstanceId);

	Lactose::Simulation::Events::OnCurrentUserCropsLoaded.Broadcast(*this, CurrentUserCrops.ToSharedRef());
}

void ULactoseSimulationServiceSubsystem::OnUserLoggedIn(
	const ULactoseIdentityServiceSubsystem& Sender,
	const TSharedRef<FLactoseIdentityGetUserResponse>& User)
{
	if (bAutoRetrieveCrops)
		LoadAllCrops();

	LoadCurrentUserCrops();
}
