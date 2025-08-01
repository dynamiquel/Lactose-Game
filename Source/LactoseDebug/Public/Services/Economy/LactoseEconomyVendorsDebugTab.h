// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "DebugImGuiHelpers.h"
#include "LactoseEconomyVendorsDebugTab.generated.h"

struct FLactoseEconomyGetUserShopItemsResponse;
struct FLactoseEconomyGetUserItemsResponse;
class ULactoseEconomyServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseEconomyVendorsDebugTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseEconomyVendorsDebugTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

	void DrawVendorItems(
		const FString& VendorId,
		const TSharedRef<FLactoseEconomyGetUserShopItemsResponse>& VendorItems);

	void DrawBuySection(
		const FString& VendorId,
		const TSharedRef<FLactoseEconomyGetUserShopItemsResponse>& VendorItems);

	void DrawSellSection(
		const FString& VendorId,
		const TSharedRef<FLactoseEconomyGetUserShopItemsResponse>& VendorItems);
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseEconomyServiceSubsystem> EconomySubsystem;
	
	Debug::ImGui::FSearchBox ItemsSearchBox { TEXT("###VendorItemsSearchBox") };

	TMap<FString, TSharedRef<FLactoseEconomyGetUserShopItemsResponse>> VendorsItems;
};
