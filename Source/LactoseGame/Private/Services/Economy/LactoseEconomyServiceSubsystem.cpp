// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Simp.h"
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
	Lactose::Identity::Events::OnUserLoggedOut.AddUObject(this, &ThisClass::OnUserLoggedOut);
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

const TMap<FString, TSharedRef<FLactoseEconomyUserItem>>& ULactoseEconomyServiceSubsystem::GetCurrentUserItems() const
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

TSharedPtr<const FLactoseEconomyUserItem> ULactoseEconomyServiceSubsystem::FindCurrentUserItem(
	const FString& ItemId) const
{
	const TSharedRef<FLactoseEconomyUserItem>* FoundUserItem = GetCurrentUserItems().Find(ItemId);
	return FoundUserItem ? TSharedPtr<const FLactoseEconomyUserItem>(*FoundUserItem) : TSharedPtr<const FLactoseEconomyUserItem>(nullptr);
}

int32 ULactoseEconomyServiceSubsystem::GetCurrentUserItemQuantity(const FString& ItemId) const
{
	const TSharedRef<FLactoseEconomyUserItem>* FoundCurrentUserItem = GetCurrentUserItems().Find(ItemId);
	return FoundCurrentUserItem ? FoundCurrentUserItem->Get().Quantity : 0;
}

void ULactoseEconomyServiceSubsystem::LoadCurrentUserItems()
{
	auto* IdentitySubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;
	
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

		ThisPinned->GetCurrentUserItemsFuture.Reset();

		TSet<FString> ExistingUserItemsIds;
		for (const TTuple<FString, TSharedRef<FLactoseEconomyUserItem>>& ExistingUserItem : ThisPinned->GetCurrentUserItems())
			ExistingUserItemsIds.Add(ExistingUserItem.Key);

		bool bAnyChanged = false;

		{
			TRACE_CPUPROFILER_EVENT_SCOPE_STR("Update User Items");
			ThisPinned->CurrentUserItems.Reserve(Context->ResponseContent->Items.Num());

			for (const FLactoseEconomyUserItem& UserItem : Context->ResponseContent->Items)
			{
				ExistingUserItemsIds.Remove(UserItem.ItemId);

				if (TSharedRef<FLactoseEconomyUserItem>* FoundExistingUserItem = ThisPinned->CurrentUserItems.Find(UserItem.ItemId))
				{
					if (FoundExistingUserItem->Get().Quantity != UserItem.Quantity)
					{
						UE_LOG(LogLactoseEconomyService, VeryVerbose, TEXT("Updated Current User Item '%s' Quantity: %d -> %d"),
							*UserItem.ItemId,
							FoundExistingUserItem->Get().Quantity,
							UserItem.Quantity);
						
						FoundExistingUserItem->Get().Quantity = UserItem.Quantity;
						bAnyChanged = true;
					}
				}
				else
				{					
					ThisPinned->CurrentUserItems.Emplace(UserItem.ItemId, MakeShared<FLactoseEconomyUserItem>(UserItem));

					UE_LOG(LogLactoseEconomyService, VeryVerbose, TEXT("Added Current User Item '%s' with Quantity: %d"),
						*UserItem.ItemId,
						UserItem.Quantity);
					
					bAnyChanged = true;
				}
			}
		}

		if (!ExistingUserItemsIds.IsEmpty())
		{
			// Anything in Existing User Items is an item that the player no longer has.
			for (const auto& ExistingUserItemsId : ExistingUserItemsIds)
			{
				ThisPinned->CurrentUserItems.Remove(ExistingUserItemsId);

				UE_LOG(LogLactoseEconomyService, VeryVerbose, TEXT("Removed Current User Item '%s"),
						*ExistingUserItemsId);
				
				bAnyChanged = true;
			}
		}
		
		UE_LOG(LogLactoseEconomyService, Verbose, TEXT("Loaded All %d User Items"),
			ThisPinned->CurrentUserItems.Num());

		// Only call the event if anything actually changed, otherwise it's pointless.
		if (bAnyChanged)
			Lactose::Economy::Events::OnCurrentUserItemsLoaded.Broadcast(*ThisPinned);

		if (!ThisPinned->GetUserItemsTicker.IsValid())
			ThisPinned->EnableGetCurrentUserItemsTicker();
	});
}

void ULactoseEconomyServiceSubsystem::EnableGetCurrentUserItemsTicker()
{
	GetGameInstance()->GetTimerManager().SetTimer(
		GetUserItemsTicker,
		this,
		&ThisClass::OnGetCurrentUserItemsTick,
		GetUserItemsTickInterval,
		/* bLoop */ true);
}

void ULactoseEconomyServiceSubsystem::DisableGetCurrentUserItemsTicker()
{
	GetGameInstance()->GetTimerManager().ClearTimer(GetUserItemsTicker);
	GetUserItemsTicker.Invalidate();
}

TFuture<TSharedPtr<FGetEconomyUserShopItemsRequest::FResponseContext>> ULactoseEconomyServiceSubsystem::GetUserShopItems(const FLactoseEconomyGetUserShopItemsRequest& Request) const
{
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetEconomyUserShopItemsRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("shopitems/usershop"));

	auto GetUserShopItemsRequest = MakeShared<FLactoseEconomyGetUserShopItemsRequest>(Request);
	auto Future = RestRequest->SetContentAsJsonAndSendAsync(GetUserShopItemsRequest);

	UE_LOG(LogLactoseEconomyService, Verbose, TEXT("Sent a Get User Shop Items request for User ID '%s'"),
		*Request.UserId);

	return Future;
}

void ULactoseEconomyServiceSubsystem::PerformShopItemTrade(const FString& ShopItemId, const int32 Quantity)
{
	auto* IdentitySubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	if (!IdentitySubsystem)
		return;
	
	const TSharedPtr<FLactoseIdentityGetUserResponse> CurrentUser = IdentitySubsystem->GetLoggedInUserInfo();
	if (!CurrentUser)
	{
		UE_LOG(LogLactoseEconomyService, Error, TEXT("Cannot load current user's items because the user is not logged in"));
		return;
	}
	
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FEconomyShopItemTradeRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("shopitems/trade"));

	TSharedRef<FLactoseEconomyShopItemTradeRequest> ShopItemTradeRequest = MakeShareable(new FLactoseEconomyShopItemTradeRequest
	{
		.UserId = CurrentUser->Id,
		.ShopItemId = ShopItemId,
		.Quantity = Quantity,
	});
	
	auto Future = RestRequest->SetContentAsJsonAndSendAsync(ShopItemTradeRequest);

	UE_LOG(LogLactoseEconomyService, Verbose, TEXT("Sent a Shop Item Trade request for User ID '%s' and Shop Item ID '%s'"),
		*CurrentUser->Id,
		*ShopItemId);
}

void ULactoseEconomyServiceSubsystem::OnAllItemsQueries(TSharedRef<FQueryEconomyItemsRequest::FResponseContext> Context)
{
	QueryAllItemsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
	{
		UE_LOG(LogLactoseEconomyService, Error, TEXT("Query Items request responded with no content"))
		return;
	}

	if (Context->ResponseContent->ItemIds.IsEmpty())
	{
		UE_LOG(LogLactoseEconomyService, Error, TEXT("Query Items request responded with no items"));
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

void ULactoseEconomyServiceSubsystem::OnUserLoggedOut(const ULactoseIdentityServiceSubsystem& Sender)
{
	CurrentUserItems.Reset();
}

void ULactoseEconomyServiceSubsystem::OnGetCurrentUserItemsTick()
{
	LoadCurrentUserItems();
}