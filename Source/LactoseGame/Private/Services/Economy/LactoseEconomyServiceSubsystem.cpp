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

Sp<const FLactoseEconomyItem> ULactoseEconomyServiceSubsystem::GetItem(const FString& ItemId) const
{
	if (const Sr<FLactoseEconomyItem>* FoundItem = GetAllItems().Find(ItemId))
		return Sp<const FLactoseEconomyItem>(*FoundItem);

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
	
	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	auto RestRequest = FQueryEconomyItemsRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("items/query"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnAllItemsQueries);

	auto QueryAllItemsRequest = CreateSr<FLactoseEconomyQueryItemsRequest>();
	QueryAllItemsFuture = RestRequest->SetContentAsJsonAndSendAsync(QueryAllItemsRequest);

	Log::Verbose(LogLactoseEconomyService, TEXT("Sent a Query All Items request"));
}

void ULactoseEconomyServiceSubsystem::ResetAllItems()
{
	QueryAllItemsFuture.Reset();
	GetAllItemsFuture.Reset();
	AllItems.Reset();
}

TFuture<Sp<FGetEconomyUserItemsRequest::FResponseContext>> ULactoseEconomyServiceSubsystem::GetUserItems(const FString& UserId) const
{
	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	auto RestRequest = FGetEconomyUserItemsRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("useritems"));

	auto GetUserItemsRequest = CreateSr<FLactoseEconomyGetUserItemsRequest>();
	GetUserItemsRequest->UserId = UserId;
	auto Future = RestRequest->SetContentAsJsonAndSendAsync(GetUserItemsRequest);

	Log::Verbose(LogLactoseEconomyService,
		TEXT("Sent a Get User Items request for User ID '%s'"),
		*UserId);

	return Future;
}

const TMap<FString, Sr<FLactoseEconomyUserItem>>& ULactoseEconomyServiceSubsystem::GetCurrentUserItems() const
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

Sp<const FLactoseEconomyUserItem> ULactoseEconomyServiceSubsystem::FindCurrentUserItem(
	const FString& ItemId) const
{
	const Sr<FLactoseEconomyUserItem>* FoundUserItem = GetCurrentUserItems().Find(ItemId);
	return FoundUserItem ? Sp<const FLactoseEconomyUserItem>(*FoundUserItem) : Sp<const FLactoseEconomyUserItem>(nullptr);
}

int32 ULactoseEconomyServiceSubsystem::GetCurrentUserItemQuantity(const FString& ItemId) const
{
	const Sr<FLactoseEconomyUserItem>* FoundCurrentUserItem = GetCurrentUserItems().Find(ItemId);
	return FoundCurrentUserItem ? FoundCurrentUserItem->Get().Quantity : 0;
}

void ULactoseEconomyServiceSubsystem::LoadCurrentUserItems()
{
	auto& IdentitySubsystem = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);
	
	const Sp<FLactoseIdentityGetUserResponse> CurrentUser = IdentitySubsystem.GetLoggedInUserInfo();
	if (!CurrentUser)
	{
		return Log::Error(LogLactoseEconomyService, TEXT("Cannot load current user's items because the user is not logged in"));
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
		for (const TTuple<FString, Sr<FLactoseEconomyUserItem>>& ExistingUserItem : ThisPinned->GetCurrentUserItems())
			ExistingUserItemsIds.Add(ExistingUserItem.Key);

		bool bAnyChanged = false;

		{
			TRACE_CPUPROFILER_EVENT_SCOPE_STR("Update User Items");
			ThisPinned->CurrentUserItems.Reserve(Context->ResponseContent->Items.Num());

			for (const FLactoseEconomyUserItem& UserItem : Context->ResponseContent->Items)
			{
				ExistingUserItemsIds.Remove(UserItem.ItemId);

				if (Sr<FLactoseEconomyUserItem>* FoundExistingUserItem = ThisPinned->CurrentUserItems.Find(UserItem.ItemId))
				{
					if (FoundExistingUserItem->Get().Quantity != UserItem.Quantity)
					{
						Log::VeryVerbose(LogLactoseEconomyService,
							TEXT("Updated Current User Item '%s' Quantity: %d -> %d"),
							*UserItem.ItemId,
							FoundExistingUserItem->Get().Quantity,
							UserItem.Quantity);
						
						FoundExistingUserItem->Get().Quantity = UserItem.Quantity;
						bAnyChanged = true;
					}
				}
				else
				{					
					ThisPinned->CurrentUserItems.Emplace(UserItem.ItemId, CreateSr(UserItem));

					Log::VeryVerbose(LogLactoseEconomyService,
						TEXT("Added Current User Item '%s' with Quantity: %d"),
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

				Log::VeryVerbose(LogLactoseEconomyService,
					TEXT("Removed Current User Item '%s"),
						*ExistingUserItemsId);
				
				bAnyChanged = true;
			}
		}
		
		Log::Verbose(LogLactoseEconomyService,
			TEXT("Loaded All %d User Items"),
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

TFuture<Sp<FGetEconomyUserShopItemsRequest::FResponseContext>> ULactoseEconomyServiceSubsystem::GetUserShopItems(const FLactoseEconomyGetUserShopItemsRequest& Request) const
{
	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	auto RestRequest = FGetEconomyUserShopItemsRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("shopitems/usershop"));

	auto GetUserShopItemsRequest = CreateSr(Request);
	auto Future = RestRequest->SetContentAsJsonAndSendAsync(GetUserShopItemsRequest);

	Log::Verbose(LogLactoseEconomyService,
		TEXT("Sent a Get User Shop Items request for User ID '%s'"),
		*Request.UserId);

	return Future;
}

void ULactoseEconomyServiceSubsystem::PerformShopItemTrade(const FString& ShopItemId, const int32 Quantity)
{
	auto& IdentitySubsystem = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);
	
	const Sp<FLactoseIdentityGetUserResponse> CurrentUser = IdentitySubsystem.GetLoggedInUserInfo();
	if (!CurrentUser)
	{
		return Log::Error(LogLactoseEconomyService, TEXT("Cannot load current user's items because the user is not logged in"));
	}
	
	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	auto RestRequest = FEconomyShopItemTradeRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("shopitems/trade"));

	Sr<FLactoseEconomyShopItemTradeRequest> ShopItemTradeRequest = MakeShareable(new FLactoseEconomyShopItemTradeRequest
	{
		.UserId = CurrentUser->Id,
		.ShopItemId = ShopItemId,
		.Quantity = Quantity,
	});
	
	auto Future = RestRequest->SetContentAsJsonAndSendAsync(ShopItemTradeRequest);

	Log::Verbose(LogLactoseEconomyService,
		TEXT("Sent a Shop Item Trade request for User ID '%s' and Shop Item ID '%s'"),
		*CurrentUser->Id,
		*ShopItemId);
}

void ULactoseEconomyServiceSubsystem::OnAllItemsQueries(Sr<FQueryEconomyItemsRequest::FResponseContext> Context)
{
	QueryAllItemsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
	{
		return Log::Error(LogLactoseEconomyService, TEXT("Query Items request responded with no content"));
	}

	if (Context->ResponseContent->ItemIds.IsEmpty())
	{
		return Log::Error(LogLactoseEconomyService, TEXT("Query Items request responded with no items"));
	}

	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	auto RestRequest = FGetEconomyItemsRequest::Create(RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("items"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnAllItemsRetrieved);

	auto GetAllItemsRequest = CreateSr<FLactoseEconomyGetItemsRequest>();
	GetAllItemsRequest->ItemIds.Append(Context->ResponseContent->ItemIds);
	GetAllItemsFuture = RestRequest->SetContentAsJsonAndSendAsync(GetAllItemsRequest);

	Log::Verbose(LogLactoseEconomyService, TEXT("Sent a Get All Items request"));
}

void ULactoseEconomyServiceSubsystem::OnAllItemsRetrieved(Sr<FGetEconomyItemsRequest::FResponseContext> Context)
{
	GetAllItemsFuture.Reset();

	if (!Context->ResponseContent.IsValid())
		return;

	if (Context->ResponseContent->Items.IsEmpty())
	{
		return Log::Error(LogLactoseEconomyService, TEXT("Get Items request responed with no items"));
	}

	AllItems.Reset();

	for (const FLactoseEconomyItem& Item : Context->ResponseContent->Items)
		AllItems.Emplace(Item.Id, CreateSr(Item));

	Log::Verbose(LogLactoseEconomyService,
		TEXT("Loaded All %d Items"),
		AllItems.Num());

	Lactose::Economy::Events::OnAllItemsLoaded.Broadcast(self);
}

void ULactoseEconomyServiceSubsystem::OnUserLoggedIn(
	const ULactoseIdentityServiceSubsystem& Sender,
	const Sr<FLactoseIdentityGetUserResponse>& User)
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