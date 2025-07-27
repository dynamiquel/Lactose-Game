// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Economy/LactoseEconomyVendorsDebugTab.h"

#include "Services/Economy/LactoseEconomyDebugApp.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"


ULactoseEconomyVendorsDebugTab::ULactoseEconomyVendorsDebugTab()
{
	SetOwningAppClass<ULactoseEconomyDebugApp>();
	SetTabName(TEXT("Vendors"));
}

void ULactoseEconomyVendorsDebugTab::Init()
{
	EconomySubsystem = Subsystems::Get<ULactoseEconomyServiceSubsystem>(self);
}

void ULactoseEconomyVendorsDebugTab::Render()
{
	if (!EconomySubsystem)
		return Debug::ImGui::Error("Identity Subsystem could not be found");

	// TODO: Move Vendors to Config Cloud or something.
	TArray<FString> StaticVendors;
	StaticVendors.Emplace(TEXT("vendor-test"));
	StaticVendors.Emplace(TEXT("vendor-basic"));
	StaticVendors.Emplace(TEXT("vendor-warm"));
	StaticVendors.Emplace(TEXT("vendor-animals"));
	StaticVendors.Emplace(TEXT("vendor-misc"));
	StaticVendors.Emplace(TEXT(""));

	if (ImGui::Button("Refresh"))
	{
		VendorsItems.Reset();
		
		for (const auto& StaticVendor : StaticVendors)
		{
			auto ShopRequest = FLactoseEconomyGetUserShopItemsRequest
			{
				.UserId = StaticVendor,
				.RetrieveUserQuantity = true
			};
			
			EconomySubsystem->GetUserShopItems(ShopRequest).Next([WeakThis = MakeWeakObjectPtr(this)]
				(Sp<FGetEconomyUserShopItemsRequest::FResponseContext> Context)
			{
				if (!Context)
					return;

				if (!Context->RequestContent)
					return;

				if (!Context->ResponseContent)
					return;

				auto* PinnedThis = WeakThis.Get();
				if (!PinnedThis)
					return;
				
				PinnedThis->VendorsItems.Emplace(
					Context->RequestContent->UserId,
					Context->ResponseContent.ToSharedRef());
			});
		}
	}

	ItemsSearchBox.Draw();
	
	for (const TTuple<FString, Sr<FLactoseEconomyGetUserShopItemsResponse>>& VendorItems : VendorsItems)
		DrawVendorItems(VendorItems.Key, VendorItems.Value);
}

void ULactoseEconomyVendorsDebugTab::DrawVendorItems(
	const FString& VendorId,
	const Sr<FLactoseEconomyGetUserShopItemsResponse>& VendorItems)
{
	if (!ImGui::CollapsingHeader(STR_TO_ANSI(VendorId)))
		return;

	DrawSellSection(VendorId, VendorItems);
	DrawBuySection(VendorId, VendorItems);
}

void ULactoseEconomyVendorsDebugTab::DrawBuySection(
	const FString& VendorId,
	const Sr<FLactoseEconomyGetUserShopItemsResponse>& VendorItems)
{
	const FString VendorSectionId = VendorId + TEXT("BuySection");
	constexpr int32 NodeFlags = 0;
	if (!ImGui::TreeNodeEx(STR_TO_ANSI(VendorSectionId), NodeFlags, "Buying"))
		return;

	const FString VendorTableId = VendorId + TEXT("BuyTable");

	constexpr int32 Columns = 4;
	constexpr int32 TableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
	if (!ImGui::BeginTable(STR_TO_ANSI(VendorTableId), Columns, TableFlags))
		return;

	ImGui::TableSetupColumn("Item ID");
	ImGui::TableSetupColumn("Item Name");
	ImGui::TableSetupColumn("Receive");
	ImGui::TableSetupColumn("Sell");
	ImGui::TableHeadersRow();
	
	for (const FLactoseEconomyShopItem& ShopItem : VendorItems->ShopItems)
	{
		if (ShopItem.TransactionType != Lactose::Economy::Types::Buy)
			continue;
		
		FString ItemLabel = ShopItem.ItemId;
		const Sp<const FLactoseEconomyItem> FoundItem = EconomySubsystem->GetItem(ShopItem.ItemId);
		
		if (FoundItem)
			ItemLabel += FString::Printf(TEXT(" (%s)"), *FoundItem->Name);

		if (!ItemsSearchBox.PassesFilter(ItemLabel))
			continue;

		if (ImGui::TableNextColumn())
		{
			ImGui::Text("%s", STR_TO_ANSI(ShopItem.ItemId));
		}

		if (ImGui::TableNextColumn())
		{
			ImGui::Text("%s", FoundItem ? STR_TO_ANSI(FoundItem->Name) : "Not Found");
		}

		if (ImGui::TableNextColumn())
		{
			for (const FLactoseEconomyUserItem& TransactionItem : ShopItem.TransactionItems)
			{
				FString TransactionItemLabel = ShopItem.ItemId;
				if (const Sp<const FLactoseEconomyItem> TransactionFoundItem = EconomySubsystem->GetItem(TransactionItem.ItemId))
					TransactionItemLabel = TransactionFoundItem->Name;
				
				ImGui::BulletText("%d x %s", TransactionItem.Quantity, STR_TO_ANSI(TransactionItemLabel));
			}
		}

		if (ImGui::TableNextColumn())
		{
			check(EconomySubsystem);
			const int32 CurrentUserQuantity = EconomySubsystem->GetCurrentUserItemQuantity(ShopItem.ItemId);
			const bool bCanSell = CurrentUserQuantity > 0;

			bool bTrySell = false;
			if (bCanSell)
			{
				bTrySell = ImGui::Button(STR_TO_ANSI(FString::Printf(TEXT("Sell###%s"), *ShopItem.Id)));
			}
			else
			{
				ImGui::Text("You are missing the Item");
				bTrySell = ImGui::Button(STR_TO_ANSI(FString::Printf(TEXT("Try Sell Anyways###%s"), *ShopItem.Id)));
			}

			if (bTrySell)
			{
				EconomySubsystem->PerformShopItemTrade(ShopItem.Id);
			}
		}
	}

	ImGui::EndTable();
	ImGui::TreePop();
}

void ULactoseEconomyVendorsDebugTab::DrawSellSection(const FString& VendorId,
	const Sr<FLactoseEconomyGetUserShopItemsResponse>& VendorItems)
{
	const FString VendorSectionId = VendorId + TEXT("SellSection");
	constexpr int32 NodeFlags = 0;
	if (!ImGui::TreeNodeEx(STR_TO_ANSI(VendorSectionId), NodeFlags, "Selling"))
		return;
	
	const FString VendorTableId = VendorId + TEXT("SellTable");

	constexpr int32 Columns = 5;
	constexpr int32 TableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
	if (!ImGui::BeginTable(STR_TO_ANSI(VendorTableId), Columns, TableFlags))
		return;

	ImGui::TableSetupColumn("Item ID");
	ImGui::TableSetupColumn("Item Name");
	ImGui::TableSetupColumn("Quantity");
	ImGui::TableSetupColumn("Cost");
	ImGui::TableSetupColumn("Buy");
	ImGui::TableHeadersRow();
	
	for (const FLactoseEconomyShopItem& ShopItem : VendorItems->ShopItems)
	{
		if (ShopItem.TransactionType != Lactose::Economy::Types::Sell)
			continue;
		
		FString ItemLabel = ShopItem.ItemId;
		const Sp<const FLactoseEconomyItem> FoundItem = EconomySubsystem->GetItem(ShopItem.ItemId);
		
		if (FoundItem)
			ItemLabel += FString::Printf(TEXT(" (%s)"), *FoundItem->Name);

		if (!ItemsSearchBox.PassesFilter(ItemLabel))
			continue;

		if (ImGui::TableNextColumn())
		{
			ImGui::Text("%s", STR_TO_ANSI(ShopItem.ItemId));
		}

		if (ImGui::TableNextColumn())
		{
			ImGui::Text("%s", FoundItem ? STR_TO_ANSI(FoundItem->Name) : "Not Found");
		}

		if (ImGui::TableNextColumn())
		{
			if (ShopItem.Quantity == -1)
				ImGui::Text("Infinite");
			else
				ImGui::Text("%d", ShopItem.Quantity);
		}

		if (ImGui::TableNextColumn())
		{
			for (const FLactoseEconomyUserItem& TransactionItem : ShopItem.TransactionItems)
			{
				FString TransactionItemLabel = ShopItem.ItemId;
				if (const Sp<const FLactoseEconomyItem> TransactionFoundItem = EconomySubsystem->GetItem(TransactionItem.ItemId))
					TransactionItemLabel = TransactionFoundItem->Name;
				
				ImGui::BulletText("%d x %s", TransactionItem.Quantity, STR_TO_ANSI(TransactionItemLabel));
			}
		}

		if (ImGui::TableNextColumn())
		{
			TArray<FLactoseEconomyUserItem> MissingItems;
			
			check(EconomySubsystem);
			for (const FLactoseEconomyUserItem& TransactionItem : ShopItem.TransactionItems)
			{
				const int32 CurrentUserQuantity = EconomySubsystem->GetCurrentUserItemQuantity(TransactionItem.ItemId);
				if (CurrentUserQuantity < TransactionItem.Quantity)
				{
					MissingItems.Add(FLactoseEconomyUserItem
					{
						.ItemId = TransactionItem.ItemId,
						.Quantity = TransactionItem.Quantity - CurrentUserQuantity,
					});
				}
			}

			bool bTryBuy = false;
			if (MissingItems.IsEmpty())
			{
				bTryBuy = ImGui::Button(STR_TO_ANSI(FString::Printf(TEXT("Buy###%s"), *ShopItem.Id)));
			}
			else
			{
				ImGui::Text("You are missing:");
				for (const auto& MissingItem : MissingItems)
				{
					FString TransactionItemLabel = ShopItem.ItemId;
					if (const Sp<const FLactoseEconomyItem> TransactionFoundItem = EconomySubsystem->GetItem(MissingItem.ItemId))
						TransactionItemLabel = TransactionFoundItem->Name;
				
					ImGui::BulletText("%d x %s", MissingItem.Quantity, STR_TO_ANSI(TransactionItemLabel));
				}
				
				if (ImGui::Button(STR_TO_ANSI(FString::Printf(TEXT("Try Buy Anyways###%s"), *ShopItem.Id))))
				{
					bTryBuy = true;
				}
			}

			if (bTryBuy)
			{
				EconomySubsystem->PerformShopItemTrade(ShopItem.Id);
			}
		}
	}

	ImGui::EndTable();
	ImGui::TreePop();
}
