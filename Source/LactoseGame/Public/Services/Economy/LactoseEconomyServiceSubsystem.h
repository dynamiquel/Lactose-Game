#pragma once

#include "CoreMinimal.h"
#include "Services/LactoseServiceSubsystem.h"
#include "LactoseEconomyItemsRequests.h"
#include "LactoseEconomyShopItemsRequests.h"
#include "LactoseEconomyUserItemsRequests.h"
#include "LactoseEconomyServiceSubsystem.generated.h"

struct FMqttifyMessage;
class ULactoseIdentityServiceSubsystem;
struct FLactoseIdentityGetUserResponse;

UENUM()
enum class ELactoseEconomyAllItemsStatus
{
	None,
	Querying,
	Retrieving,
	Loaded
};

UENUM()
enum class ELactoseEconomyUserItemsStatus
{
	None,
	Retrieving,
	Loaded
};

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseEconomyServiceSubsystem : public ULactoseServiceSubsystem
{
	GENERATED_BODY()
	
	ULactoseEconomyServiceSubsystem();

	// Begin override ULactoseServiceSubsystem
	void Initialize(FSubsystemCollectionBase& Collection) override;
	// End override ULactoseServiceSubsystem
	
public:
	const TMap<FString, Sr<FLactoseEconomyItem>>& GetAllItems() const { return AllItems; }
	Sp<const FLactoseEconomyItem> GetItem(const FString& ItemId) const;
	ELactoseEconomyAllItemsStatus GetAllItemsStatus() const;

	void LoadAllItems();
	void ResetAllItems();

	TFuture<Sp<FGetEconomyUserItemsRequest::FResponseContext>> GetUserItems(const FString& UserId) const;
	const TMap<FString, Sr<FLactoseEconomyUserItem>>& GetCurrentUserItems() const;
	ELactoseEconomyUserItemsStatus GetCurrentUserItemsStatus() const;

	Sp<const FLactoseEconomyUserItem> FindCurrentUserItem(const FString& ItemId) const;
	int32 GetCurrentUserItemQuantity(const FString& ItemId) const;

	void LoadCurrentUserItems();

	void EnableGetCurrentUserItemsTicker();
	void DisableGetCurrentUserItemsTicker();
	bool IsAutoGetCurrentUserItemsTicking() const { return GetUserItemsTicker.IsValid(); }

	TFuture<Sp<FGetEconomyUserShopItemsRequest::FResponseContext>> GetUserShopItems(const FLactoseEconomyGetUserShopItemsRequest& Request) const;
	void PerformShopItemTrade(const FString& ShopItemId, int32 Quantity = 1);
	
protected:
	void OnAllItemsQueries(Sr<FQueryEconomyItemsRequest::FResponseContext> Context);
	void OnAllItemsRetrieved(Sr<FGetEconomyItemsRequest::FResponseContext> Context);

	void OnUserLoggedIn(
		const ULactoseIdentityServiceSubsystem& Sender,
		const Sr<FLactoseIdentityGetUserResponse>& User);

	void OnUserLoggedOut(const ULactoseIdentityServiceSubsystem& Sender);

	void OnGetCurrentUserItemsTick();

	void OnUserTransaction(const FMqttifyMessage& Message);

private:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bAutoRetrieveItems = true;

	// Very low since it uses events to update.
	// Poll all user items every so often for sanity.
	UPROPERTY(EditDefaultsOnly, Config)
	float GetUserItemsTickInterval = 60.f;
	
	TFuture<Sp<FQueryEconomyItemsRequest::FResponseContext>> QueryAllItemsFuture;
	TFuture<Sp<FGetEconomyItemsRequest::FResponseContext>> GetAllItemsFuture;
	TMap<FString, Sr<FLactoseEconomyItem>> AllItems;

	TFuture<Sp<FGetEconomyUserItemsRequest::FResponseContext>> GetCurrentUserItemsFuture;
	TMap<FString, Sr<FLactoseEconomyUserItem>> CurrentUserItems;

	FTimerHandle GetUserItemsTicker;
};

namespace Lactose::Economy::Events
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FAllItemsLoaded,
		const ULactoseEconomyServiceSubsystem& /* Sender */);

	DECLARE_MULTICAST_DELEGATE_OneParam(FCurrentUserItemsLoaded,
		const ULactoseEconomyServiceSubsystem& /* Sender */);

	inline FAllItemsLoaded OnAllItemsLoaded;
	inline FCurrentUserItemsLoaded OnCurrentUserItemsLoaded;
}

namespace Lactose::Economy::Types
{
	constexpr auto* Buy = TEXT("Buy");
	constexpr auto* Sell = TEXT("Sell");
	constexpr auto* Animal = TEXT("Animal");
}