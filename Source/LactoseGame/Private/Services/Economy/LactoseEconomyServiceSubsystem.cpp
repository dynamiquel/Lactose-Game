// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Economy/LactoseEconomyServiceSubsystem.h"

#include "Services/LactoseServicesLog.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

ULactoseEconomyServiceSubsystem::ULactoseEconomyServiceSubsystem()
{
	SetServiceBaseUrl(TEXT("https://lactose.mookrata.ovh/economy"));
}

void ULactoseEconomyServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Lactose::Identity::Events::OnUserLoggedIn.AddUObject(this, &ThisClass::OnUserLoggedIn);
}

TSharedPtr<const FLactoseEconomyItem> ULactoseEconomyServiceSubsystem::GetItem(const FString& ItemId) const
{
	if (const TSharedRef<FLactoseEconomyItem>* FoundItem = GetAllItems().Find(ItemId))
		return TSharedPtr<const FLactoseEconomyItem>(*FoundItem);

	return nullptr;
}

ELactoseEconomyAllItemsStatus ULactoseEconomyServiceSubsystem::GetAllItemsStatus() const
{
	if (!GetAllItems().IsEmpty())
		return ELactoseEconomyAllItemsStatus::Loaded;

	if (GetAllItemsFuture.IsValid())
		return ELactoseEconomyAllItemsStatus::Retrieving;

	if (QueryAllItemsFuture.IsValid())
		return ELactoseEconomyAllItemsStatus::Querying;

	return ELactoseEconomyAllItemsStatus::None;
}

void ULactoseEconomyServiceSubsystem::LoadAllItems()
{
	ResetAllItems();
	
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FQueryEconomyItemsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("items/query"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnAllItemsQueries);

	auto QueryAllItemsRequest = MakeShared<FLactoseEconomyQueryItemsRequest>();
	QueryAllItemsFuture = RestRequest->SetContentAsJsonAndSendAsync(QueryAllItemsRequest);

	UE_LOG(LogLactoseEconomyService, Verbose, TEXT("Sent a Query All Items request"));
}

void ULactoseEconomyServiceSubsystem::ResetAllItems()
{
	QueryAllItemsFuture.Reset();
	GetAllItemsFuture.Reset();
	AllItems.Reset();
}

TFuture<TSharedPtr<FGetEconomyUserItemsRequest::FResponseContext>> ULactoseEconomyServiceSubsystem::GetUserItems(const FString& UserId) const
{
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetEconomyUserItemsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("useritems"));

	auto GetUserItemsRequest = MakeShared<FLactoseEconomyGetUserItemsRequest>();
	GetUserItemsRequest->UserId = UserId;
	auto Future = RestRequest->SetContentAsJsonAndSendAsync(GetUserItemsRequest);

	UE_LOG(LogLactoseEconomyService, Verbose, TEXT("Sent a Get User Items request for User ID '%s'"),
		*UserId);

	return Future;
}

const TMap<FString, TSharedRef<FLactoseEconomyUserItem>>& ULactoseEconomyServiceSubsystem::GetCurrentUserItems()
{
	return CurrentUserItems;
}

ELactoseEconomyUserItemsStatus ULactoseEconomyServiceSubsystem::GetCurrentUserItemsStatus() const
{
	if (!CurrentUserItems.IsEmpty())
		return ELactoseEconomyUserItemsStatus::Loaded;

	if (GetCurrentUserItemsFuture.IsValid())
		return ELactoseEconomyUserItemsStatus::Retrieving;

	return ELactoseEconomyUserItemsStatus::None;
}

void ULactoseEconomyServiceSubsystem::LoadCurrentUserItems()
{
	auto* IdentitySubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;
	
	CurrentUserItems.Reset();

	const TSharedPtr<FLactoseIdentityGetUserResponse> CurrentUser = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUser)
	{
		UE_LOG(LogLactoseEconomyService, Error, TEXT("Cannot load current user's items because the user is not logged in"));
		return;
	}

	GetCurrentUserItemsFuture = GetUserItems(CurrentUser->Id);
	GetCurrentUserItemsFuture.Next([WeakThis = MakeWeakObjectPtr(this)](TSharedPtr<FGetEconomyUserItemsRequest::FResponseContext> Context)
	{
		if (!Context.IsValid() || !Context->ResponseContent.IsValid())
			return;
		
		auto* ThisPinned = WeakThis.Get();
		if (!ThisPinned)
			return;

		ThisPinned->CurrentUserItems.Reset();
		ThisPinned->GetCurrentUserItemsFuture.Reset();
		
		for (const auto& UserItem : Context->ResponseContent->Items)
			ThisPinned->CurrentUserItems.Emplace(UserItem.ItemId, MakeShared<FLactoseEconomyUserItem>(UserItem));

		UE_LOG(LogLactoseEconomyService, Verbose, TEXT("Loaded All %d User Items"),
			ThisPinned->CurrentUserItems.Num());
	});
}

void ULactoseEconomyServiceSubsystem::OnAllItemsQueries(TSharedRef<FQueryEconomyItemsRequest::FResponseContext> Context)
{
	QueryAllItemsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->ItemIds.IsEmpty())
	{
		UE_LOG(LogLactoseEconomyService, Error, TEXT("Query Items request responed with no items"));
		return;
	}

	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetEconomyItemsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("items"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnAllItemsRetrieved);

	auto GetAllItemsRequest = MakeShared<FLactoseEconomyGetItemsRequest>();
	GetAllItemsRequest->ItemIds.Append(Context->ResponseContent->ItemIds);
	GetAllItemsFuture = RestRequest->SetContentAsJsonAndSendAsync(GetAllItemsRequest);

	UE_LOG(LogLactoseEconomyService, Verbose, TEXT("Sent a Get All Items request"));
}

void ULactoseEconomyServiceSubsystem::OnAllItemsRetrieved(TSharedRef<FGetEconomyItemsRequest::FResponseContext> Context)
{
	GetAllItemsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->Items.IsEmpty())
	{
		UE_LOG(LogLactoseEconomyService, Error, TEXT("Get Items request responed with no items"));
		return;
	}

	AllItems.Reset();

	for (const FLactoseEconomyItem& Item : Context->ResponseContent->Items)
		AllItems.Emplace(Item.Id, MakeShared<FLactoseEconomyItem>(Item));

	UE_LOG(LogLactoseEconomyService, Verbose, TEXT("Loaded All %d Items"),
		AllItems.Num());

	Lactose::Economy::Events::OnAllItemsLoaded.Broadcast(*this);
}

void ULactoseEconomyServiceSubsystem::OnUserLoggedIn(
	const ULactoseIdentityServiceSubsystem& Sender,
	const TSharedRef<FLactoseIdentityGetUserResponse>& User)
{
	if (bAutoRetrieveItems && GetAllItems().IsEmpty())
		LoadAllItems();

	LoadCurrentUserItems();
}
