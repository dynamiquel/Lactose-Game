#include "Services/Economy/LactoseEconomyItemsTab.h"

#include "DebugImGuiHelpers.h"

#include "Services/Economy/LactoseEconomyDebugApp.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"

ULactoseEconomyItemsTab::ULactoseEconomyItemsTab()
{
	SetOwningAppClass<ULactoseEconomyDebugApp>();
	SetTabName(TEXT("Items"));
}

void ULactoseEconomyItemsTab::Init()
{
	EconomySubsystem = UGameInstance::GetSubsystem<ULactoseEconomyServiceSubsystem>(GetWorld()->GetGameInstance());
}

void ULactoseEconomyItemsTab::Render()
{
	if (!EconomySubsystem)
		return Debug::ImGui::Error("Identity Subsystem could not be found");

	const ELactoseEconomyAllItemsStatus EconomyItemsStatus = EconomySubsystem->GetAllItemsStatus();

	ImGui::Text("Status: ");
	ImGui::SameLine();
	
	switch (EconomyItemsStatus)
	{
		case ELactoseEconomyAllItemsStatus::None:
			ImGui::Text("None");
			if (ImGui::Button("Load Items"))
				EconomySubsystem->LoadAllItems();
			break;
		case ELactoseEconomyAllItemsStatus::Querying:
			ImGui::Text("Querying");
			break;
		case ELactoseEconomyAllItemsStatus::Retrieving:
			ImGui::Text("Retrieving");
			break;
		case ELactoseEconomyAllItemsStatus::Loaded:
			ImGui::Text("Loaded");
			if (ImGui::Button("Load Items"))
				EconomySubsystem->LoadAllItems();
			break;
		default:
			ImGui::Text("Unknown");
	}

	ImGui::Spacing();

	ImGui::BeginChild("EconomyItems", ImVec2(0, 0), /* bBorder*/ true);
	ON_SCOPE_EXIT
	{
		ImGui::EndChild();
	};

	const TMap<FString, Sr<FLactoseEconomyItem>>& EconomyItems = EconomySubsystem->GetAllItems();
	if (EconomyItems.IsEmpty())
	{
		ImGui::Text("No Economy Items Found");
		return;
	}

	ItemsSearchBox.Draw();

	for (const auto& EconomyItem : EconomyItems)
	{
		const FString EconomyItemLabel = FString::Printf(TEXT("%s (%s)"), *EconomyItem.Value->Id, *EconomyItem.Value->Name);

		if (!ItemsSearchBox.PassesFilter(EconomyItemLabel))
			continue;
		
		if (ImGui::CollapsingHeader(STR_TO_ANSI(EconomyItemLabel)))
		{
			ImGui::Indent();
			
			ImGui::Text("Id: %s", STR_TO_ANSI(EconomyItem.Value->Id));
			ImGui::Text("Type: %s", STR_TO_ANSI(EconomyItem.Value->Type));
			ImGui::Text("Name: %s", STR_TO_ANSI(EconomyItem.Value->Name));

			ImGui::Text("Description:");
			ImGui::Indent();
			ImGui::TextWrapped("%s", STR_TO_ANSI(EconomyItem.Value->Description));
			ImGui::Unindent();

			ImGui::Unindent();
		}
	}
}
