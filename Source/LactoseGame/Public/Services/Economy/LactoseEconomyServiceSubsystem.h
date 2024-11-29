#pragma once

#include "CoreMinimal.h"
#include "Services/LactoseServiceSubsystem.h"
#include "LactoseEconomyItemsRequests.h"
#include "LactoseEconomyUserItemsRequests.h"
#include "LactoseEconomyServiceSubsystem.generated.h"

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

public:
	// Begin override ULactoseServiceSubsystem
	void Initialize(FSubsystemCollectionBase& Collection) override;
	// End override ULactoseServiceSubsystem

	const TMap<FString, TSharedRef<FLactoseEconomyItem>>& GetAllItems() const { return AllItems; }
	TSharedPtr<const FLactoseEconomyItem> GetItem(const FString& ItemId) const;
	ELactoseEconomyAllItemsStatus GetAllItemsStatus() const;

	void LoadAllItems();
	void ResetAllItems();

	TFuture<TSharedPtr<FGetEconomyUserItemsRequest::FResponseContext>> GetUserItems(const FString& UserId) const;
	const TMap<FString, TSharedRef<FLactoseEconomyUserItem>>& GetCurrentUserItems();
	ELactoseEconomyUserItemsStatus GetCurrentUserItemsStatus() const;

	void LoadCurrentUserItems();
	
protected:
	void OnAllItemsQueries(TSharedRef<FQueryEconomyItemsRequest::FResponseContext> Context);
	void OnAllItemsRetrieved(TSharedRef<FGetEconomyItemsRequest::FResponseContext> Context);

	void OnUserLoggedIn(
		const ULactoseIdentityServiceSubsystem& Sender,
		const TSharedRef<FLactoseIdentityGetUserResponse>& User);

private:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bAutoRetrieveItems = true;
	
	TFuture<TSharedPtr<FQueryEconomyItemsRequest::FResponseContext>> QueryAllItemsFuture;
	TFuture<TSharedPtr<FGetEconomyItemsRequest::FResponseContext>> GetAllItemsFuture;
	TMap<FString, TSharedRef<FLactoseEconomyItem>> AllItems;

	TFuture<TSharedPtr<FGetEconomyUserItemsRequest::FResponseContext>> GetCurrentUserItemsFuture;
	TMap<FString, TSharedRef<FLactoseEconomyUserItem>> CurrentUserItems;
};

namespace Lactose::Economy::Events
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FAllItemsLoaded,
		const ULactoseEconomyServiceSubsystem& /* Sender */);

	inline FAllItemsLoaded OnAllItemsLoaded;
}
