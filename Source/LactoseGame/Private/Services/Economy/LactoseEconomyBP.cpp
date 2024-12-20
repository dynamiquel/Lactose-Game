// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Economy/LactoseEconomyBP.h"

#include "Services/Economy/LactoseEconomyServiceSubsystem.h"

TMap<FString, FLactoseEconomyItem> ULactoseEconomyBP::GetItems(const ULactoseEconomyServiceSubsystem* Economy)
{
	if (!Economy)
		return {};

	TMap<FString, TSharedRef<FLactoseEconomyItem>> FoundItems = Economy->GetAllItems();

	TMap<FString, FLactoseEconomyItem> CopiedItems;
	CopiedItems.Reserve(FoundItems.Num());

	for (const auto& FoundItem : FoundItems)
		CopiedItems.Add(FoundItem.Key, *FoundItem.Value);
	
	return CopiedItems;
}

FLactoseEconomyItem ULactoseEconomyBP::GetItem(const ULactoseEconomyServiceSubsystem* Economy, const FString& ItemId)
{
	if (!Economy)
		return FLactoseEconomyItem();

	const TSharedRef<FLactoseEconomyItem>* FoundItem = Economy->GetAllItems().Find(ItemId);
	return FoundItem ? FoundItem->Get() : FLactoseEconomyItem();
}

TMap<FString, FLactoseEconomyUserItem> ULactoseEconomyBP::GetCurrentUserItems(const ULactoseEconomyServiceSubsystem* Economy)
{
	if (!Economy)
		return {};

	const TMap<FString, TSharedRef<FLactoseEconomyUserItem>>& FoundCurrentUserItems = Economy->GetCurrentUserItems();
	TMap<FString, FLactoseEconomyUserItem> CopiedCurrentUserItems;
	CopiedCurrentUserItems.Reserve(FoundCurrentUserItems.Num());

	for (const auto& FoundCurrentUserItem : FoundCurrentUserItems)
		CopiedCurrentUserItems.Add(FoundCurrentUserItem.Key, *FoundCurrentUserItem.Value);

	return CopiedCurrentUserItems;
}

FLactoseEconomyUserItem ULactoseEconomyBP::GetCurrentUserItem(
	const ULactoseEconomyServiceSubsystem* Economy,
	const FString& ItemId)
{
	if (!Economy)
		return FLactoseEconomyUserItem();

	const TMap<FString, TSharedRef<FLactoseEconomyUserItem>>& FoundCurrentUserItems = Economy->GetCurrentUserItems();
	const TSharedRef<FLactoseEconomyUserItem>* FoundCurrentUserItem = FoundCurrentUserItems.Find(ItemId);
	return FoundCurrentUserItem ? FoundCurrentUserItem->Get() : FLactoseEconomyUserItem();
}

bool ULactoseEconomyBP::IsValidItem(const FLactoseEconomyItem& Item)
{
	return !Item.Id.IsEmpty();
}

bool ULactoseEconomyBP::IsValidUserItem(const FLactoseEconomyUserItem& Item)
{
	return !Item.ItemId.IsEmpty();
}

void ULactoseEconomyCurrentUserItemsLoadedDelegateWrapper::OnSubscribed()
{
	NativeDelegateHandle = Lactose::Economy::Events::OnCurrentUserItemsLoaded.AddUObject(this, &ThisClass::HandleNativeEvent);
}

void ULactoseEconomyCurrentUserItemsLoadedDelegateWrapper::OnUnsubscribed()
{
	Lactose::Economy::Events::OnCurrentUserItemsLoaded.Remove(NativeDelegateHandle);
}

void ULactoseEconomyCurrentUserItemsLoadedDelegateWrapper::HandleNativeEvent(const ULactoseEconomyServiceSubsystem& Sender)
{
	OnExecuted();
}

