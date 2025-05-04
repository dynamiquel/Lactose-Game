#include "Services/Economy/LactoseEconomyBP.h"

#include "LactoseGame/LactosePathUtils.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"

TMap<FString, FLactoseEconomyItem> ULactoseEconomyBP::GetItems(const ULactoseEconomyServiceSubsystem* Economy)
{
	if (!Economy)
		return {};

	TMap<FString, Sr<FLactoseEconomyItem>> FoundItems = Economy->GetAllItems();

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

	const Sr<FLactoseEconomyItem>* FoundItem = Economy->GetAllItems().Find(ItemId);
	return FoundItem ? FoundItem->Get() : FLactoseEconomyItem();
}

TMap<FString, FLactoseEconomyUserItem> ULactoseEconomyBP::GetCurrentUserItems(const ULactoseEconomyServiceSubsystem* Economy)
{
	if (!Economy)
		return {};

	const TMap<FString, Sr<FLactoseEconomyUserItem>>& FoundCurrentUserItems = Economy->GetCurrentUserItems();
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

	const TMap<FString, Sr<FLactoseEconomyUserItem>>& FoundCurrentUserItems = Economy->GetCurrentUserItems();
	const Sr<FLactoseEconomyUserItem>* FoundCurrentUserItem = FoundCurrentUserItems.Find(ItemId);
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

void ULactoseEconomyBP::PerformShopItemTrade(
	ULactoseEconomyServiceSubsystem* Economy,
	const FString& ShopItemId,
	int32 Quantity)
{
	if (!Economy)
		return;

	Economy->PerformShopItemTrade(ShopItemId, Quantity);
}

TSoftObjectPtr<UTexture2D> ULactoseEconomyBP::GetItemImage(const FString& ItemImageId)
{
	FString SoftPathStr = ItemImageId;
		SoftPathStr = Lactose::Paths::GetObjectPackagePathWithSelf(SoftPathStr);
	
	return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(SoftPathStr));
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

ULactoseEconomyGetUserShopItemsAsyncNode* ULactoseEconomyGetUserShopItemsAsyncNode::LactoseEconomyGetUserShopItemsAsyncNode(
	UObject* InWorldContext,
	const FString& InUserId)
{
	auto* Node = NewObject<ULactoseEconomyGetUserShopItemsAsyncNode>();
	Node->UserId = InUserId;
	Node->WorldContext = InWorldContext;
	return Node;
}

void ULactoseEconomyGetUserShopItemsAsyncNode::Activate()
{
	if (!IsValid(WorldContext))
		return;
	
	auto* World = WorldContext->GetWorld();
	if (!World)
		return;

	auto& Economy = Subsystems::GetRef<ULactoseEconomyServiceSubsystem>(*World);
	auto Request = FLactoseEconomyGetUserShopItemsRequest
	{
		.UserId = UserId,
		.RetrieveUserQuantity = true
	};

	TFuture<Sp<FGetEconomyUserShopItemsRequest::FResponseContext>> Future = Economy.GetUserShopItems(Request);
	Future.Next<>([WeakThis = MakeWeakObjectPtr(this)](Sp<FGetEconomyUserShopItemsRequest::FResponseContext> Response)
	{
		auto* ThisPinned = WeakThis.Get();
		if (!ThisPinned)
			return;

		if (!Response || !Response->ResponseContent)
		{
			ThisPinned->OnFailed.Broadcast();
			ThisPinned->SetReadyToDestroy();
			return;
		}

		ThisPinned->OnLoaded.Broadcast(*Response->ResponseContent);
		ThisPinned->SetReadyToDestroy();
	});
}
