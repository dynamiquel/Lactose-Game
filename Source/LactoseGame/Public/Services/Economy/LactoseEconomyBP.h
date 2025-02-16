// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Simp.h"
#include <Kismet/BlueprintFunctionLibrary.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "LactoseEconomyItemsRequests.h"
#include "LactoseEconomyShopItemsRequests.h"
#include "LactoseEconomyUserItemsRequests.h"
#include "LactoseGame/LactoseNativeDelegateWrapper.h"
#include "LactoseEconomyBP.generated.h"

class ULactoseEconomyServiceSubsystem;

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseEconomyBP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Lactose Economy")
	static TMap<FString, FLactoseEconomyItem> GetItems(const ULactoseEconomyServiceSubsystem* Economy);

	UFUNCTION(BlueprintPure, Category = "Lactose Economy")
	static FLactoseEconomyItem GetItem(const ULactoseEconomyServiceSubsystem* Economy, const FString& ItemId);

	UFUNCTION(BlueprintPure, Category = "Lactose Economy")
	static TMap<FString, FLactoseEconomyUserItem> GetCurrentUserItems(const ULactoseEconomyServiceSubsystem* Economy);

	UFUNCTION(BlueprintPure, Category = "Lactose Economy")
	static FLactoseEconomyUserItem GetCurrentUserItem(const ULactoseEconomyServiceSubsystem* Economy, const FString& ItemId);

	// BP doesn't support pointers or even TOptional for structs so we must
	// do some primitive valid checks.
	//
	// I wish BP did support optionals tho...
	
	UFUNCTION(BlueprintPure, Category = "Lactose Economy")
	static bool IsValidItem(const FLactoseEconomyItem& Item);

	UFUNCTION(BlueprintPure, Category = "Lactose Economy")
	static bool IsValidUserItem(const FLactoseEconomyUserItem& Item);

	UFUNCTION(BlueprintCallable, Category = "Lactose Economy")
	static void PerformShopItemTrade(ULactoseEconomyServiceSubsystem* Economy, const FString& ShopItemId, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Lactose Economy")
	static TSoftObjectPtr<UTexture2D> GetItemImage(const FString& ItemImageId);

};

UCLASS()
class ULactoseEconomyCurrentUserItemsLoadedDelegateWrapper : public ULactoseNativeDelegateWrapper
{
	GENERATED_BODY()

public:
	void OnSubscribed() override;
	void OnUnsubscribed() override;
	
private:
	void HandleNativeEvent(const ULactoseEconomyServiceSubsystem& Sender);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLactoseEconomyGetUserShopItemsSuccessPin,
	const FLactoseEconomyGetUserShopItemsResponse&, Result);

UCLASS(BlueprintType)
class ULactoseEconomyGetUserShopItemsAsyncNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext="InWorldContext"))
	static ULactoseEconomyGetUserShopItemsAsyncNode* LactoseEconomyGetUserShopItemsAsyncNode(
		UObject* InWorldContext,
		const FString& InUserId);

	void Activate() override;
	
	UPROPERTY(BlueprintAssignable)
	FLactoseEconomyGetUserShopItemsSuccessPin OnLoaded;

	UPROPERTY(BlueprintAssignable)
	FSimpDynamicMCDelegate OnFailed;

protected:
	UPROPERTY()
	TObjectPtr<UObject> WorldContext;

	FString UserId;
};