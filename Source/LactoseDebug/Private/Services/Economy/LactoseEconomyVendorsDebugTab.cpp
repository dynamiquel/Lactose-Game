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
	EconomySubsystem = UGameInstance::GetSubsystem<ULactoseEconomyServiceSubsystem>(GetWorld()->GetGameInstance());
}

void ULactoseEconomyVendorsDebugTab::Render()
{
	if (!EconomySubsystem)
		return Debug::ImGui::Error("Identity Subsystem could not be found");

	// TODO: Move Vendors to Config Cloud or something.
	TArray<FString> StaticVendors;
	StaticVendors.Emplace(TEXT("vendor-test"));

	if (ImGui::Button("Refresh"))
	{
		VendorsItems.Reset();
		
		for (const auto& StaticVendor : StaticVendors)
		{
			EconomySubsystem->GetUserItems(StaticVendor).Next([WeakThis = MakeWeakObjectPtr(this)]
				(TSharedPtr<FGetEconomyUserItemsRequest::FResponseContext> Context)
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
	
	for (const TTuple<FString, TSharedRef<FLactoseEconomyGetUserItemsResponse>>& VendorItems : VendorsItems)
		DrawVendorItems(VendorItems.Key, VendorItems.Value);
}

void ULactoseEconomyVendorsDebugTab::DrawVendorItems(
	const FString& VendorId,
	const TSharedRef<FLactoseEconomyGetUserItemsResponse>& VendorItems)
{
	if (!ImGui::CollapsingHeader(STR_TO_ANSI(VendorId)))
		return;

	const FString VendorTableId = VendorId + TEXT("Table");

	constexpr int32 Columns = 3;
	constexpr int32 TableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
	if (!ImGui::BeginTable(STR_TO_ANSI(VendorTableId), Columns, TableFlags))
		return;

	ImGui::TableSetupColumn("Item ID");
	ImGui::TableSetupColumn("Item Name");
	ImGui::TableSetupColumn("Quantity");
	ImGui::TableSetupColumn("Buy");
	ImGui::TableHeadersRow();
	
	for (const FLactoseEconomyUserItem& VendorItem : VendorItems->Items)
	{
		FString ItemLabel = VendorItem.ItemId;
		const TSharedPtr<const FLactoseEconomyItem> FoundItem = EconomySubsystem->GetItem(VendorItem.ItemId);
		
		if (FoundItem)
			ItemLabel += FString::Printf(TEXT(" (%s)"), *FoundItem->Name);

		if (!ItemsSearchBox.PassesFilter(ItemLabel))
			continue;

		if (ImGui::TableNextColumn())
		{
			ImGui::Text("%s", STR_TO_ANSI(VendorItem.ItemId));
		}

		if (ImGui::TableNextColumn())
		{
			ImGui::Text("%s", FoundItem ? STR_TO_ANSI(FoundItem->Name) : "Not Found");
		}

		if (ImGui::TableNextColumn())
		{
			if (VendorItem.Quantity == -1)
				ImGui::Text("Infinite");
			else
				ImGui::Text("%d", VendorItem.Quantity);
		}

		if (ImGui::TableNextColumn())
		{
			// TODO: Hook into Lactose Simulation to know the cost of things.
		}
	}

	ImGui::EndTable();
}
