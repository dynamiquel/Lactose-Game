// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Economy/LactoseEconomyUserItemsTab.h"

#include "Services/Economy/LactoseEconomyDebugApp.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"

ULactoseEconomyUserItemsTab::ULactoseEconomyUserItemsTab()
{
	SetOwningAppClass<ULactoseEconomyDebugApp>();
	SetTabName(TEXT("User Items"));
}

void ULactoseEconomyUserItemsTab::Init()
{
	EconomySubsystem = Subsystems::Get<ULactoseEconomyServiceSubsystem>(self);
}

void ULactoseEconomyUserItemsTab::Render()
{
	if (!EconomySubsystem)
		return Debug::ImGui::Error("Identity Subsystem could not be found");

	constexpr int32 Columns = 2;
	constexpr int32 Flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
	if (!ImGui::BeginTable("EconomyUserItemsTable", Columns, Flags))
		return;

	ImGui::TableSetupColumn("Current User");
	ImGui::TableSetupColumn("Search User");
	ImGui::TableHeadersRow();
	
	if (ImGui::TableNextColumn())
	{
		ImGui::Text("Status: ");
		ImGui::SameLine();

		switch (EconomySubsystem->GetCurrentUserItemsStatus())
		{
			case ELactoseEconomyUserItemsStatus::None:
				ImGui::Text("None");
				if (ImGui::Button("Load"))
					EconomySubsystem->LoadCurrentUserItems();
				break;
			case ELactoseEconomyUserItemsStatus::Retrieving:
				ImGui::Text("Retrieving");
				break;
			case ELactoseEconomyUserItemsStatus::Loaded:
				ImGui::Text("Loaded");
				if (ImGui::Button("Refresh"))
					EconomySubsystem->LoadCurrentUserItems();
				break;
			default:
				ImGui::Text("Unknown");
		}

		ImGui::SameLine();
		if (EconomySubsystem->IsAutoGetCurrentUserItemsTicking())
		{
			if (ImGui::Button("Disable Get Current User Items Tick"))
				EconomySubsystem->DisableGetCurrentUserItemsTicker();
		}
		else
		{
			if (ImGui::Button("Enable Get Current User Items Tick"))
				EconomySubsystem->EnableGetCurrentUserItemsTicker();
		}

		ImGui::Spacing();
		
		const TMap<FString, Sr<FLactoseEconomyUserItem>>& CurrentUserItems = EconomySubsystem->GetCurrentUserItems();
		if (CurrentUserItems.IsEmpty())
		{
			ImGui::Text("No Items found for the Current User");
		}
		else
		{
			CurrentUserItemsSearchBox.Draw();

			ImGui::Spacing();
			
			for (const auto& CurrentUserItem : CurrentUserItems)
			{
				FString ItemLabel = CurrentUserItem.Key;
				if (Sp<const FLactoseEconomyItem> FoundItem = EconomySubsystem->GetItem(CurrentUserItem.Value->ItemId))
					ItemLabel += FString::Printf(TEXT(" (%s)"), *FoundItem->Name);

				if (!CurrentUserItemsSearchBox.PassesFilter(ItemLabel))
					continue;
					
				if (ImGui::CollapsingHeader(STR_TO_ANSI(ItemLabel)))
				{
					ImGui::Indent();
					ImGui::Text("Item Id: %s", STR_TO_ANSI(CurrentUserItem.Value->ItemId));
					if (CurrentUserItem.Value->Quantity == -1)
						ImGui::Text("Quantity: Infinite");
					else
						ImGui::Text("Quantity: %d", CurrentUserItem.Value->Quantity);
					ImGui::Unindent();
				}
			}
		}
	}

	if (ImGui::TableNextColumn())
	{
		ImGui::Text("User ID:");
		ImGui::SameLine();
		ImGui::InputText("###OtherUserIdInput", OtherUserIdBuffer.data(), OtherUserIdBuffer.size());
		
		const bool bPendingOtherUserItems = GetOtherUserItemsFuture.IsValid() && !GetOtherUserItemsFuture.IsReady();
		if (bPendingOtherUserItems)
		{
			ImGui::Text("Status: Retrieving");
		}
		else
		{
			if (ImGui::Button("Load"))
			{
				FString UserIdStr = ANSI_TO_TCHAR(OtherUserIdBuffer.data());
				if (!UserIdStr.IsEmpty())
					GetOtherUserItemsFuture = EconomySubsystem->GetUserItems(UserIdStr);
			}
		}

		ImGui::Spacing();

		if (GetOtherUserItemsFuture.IsReady())
		{
			const Sp<FGetEconomyUserItemsRequest::FResponseContext>& Context = GetOtherUserItemsFuture.Get();
			if (!Context.IsValid())
			{
				ImGui::Text("Received unsuccessful response");
			}
			else
			{
				const Sp<FLactoseEconomyGetUserItemsResponse>& Response = Context->ResponseContent;
				if (!Response)
				{
					ImGui::Text("Received unsuccessful response");
				}
				else
				{
					if (Response->Items.IsEmpty())
					{
						ImGui::Text("User has no items");
					}
					else
					{
						OtherUserItemsSearchBox.Draw();

						ImGui::Spacing();

						for (const FLactoseEconomyUserItem& CurrentUserItem : Response->Items)
						{
							FString ItemLabel = CurrentUserItem.ItemId;
							if (Sp<const FLactoseEconomyItem> FoundItem = EconomySubsystem->GetItem(CurrentUserItem.ItemId))
								ItemLabel += FString::Printf(TEXT(" (%s)"), *FoundItem->Name);
							
							if (!OtherUserItemsSearchBox.PassesFilter(ItemLabel))
								continue;
							
							if (ImGui::CollapsingHeader(STR_TO_ANSI(ItemLabel)))
							{
								ImGui::Indent();
								ImGui::Text("Item Id: %s", STR_TO_ANSI(CurrentUserItem.ItemId));
								if (CurrentUserItem.Quantity == -1)
									ImGui::Text("Quantity: Infinite");
								else
									ImGui::Text("Quantity: %d", CurrentUserItem.Quantity);
								ImGui::Unindent();
							}
						}
					}
				}
			}
		}
	}
	
	ImGui::EndTable();
}