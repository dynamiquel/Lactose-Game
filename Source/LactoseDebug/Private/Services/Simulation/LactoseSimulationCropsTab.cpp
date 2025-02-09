// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Simulation/LactoseSimulationCropsTab.h"

#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationDebugApp.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

ULactoseSimulationCropsTab::ULactoseSimulationCropsTab()
{
	SetOwningAppClass<ULactoseSimulationDebugApp>();
	SetTabName(TEXT("Crops"));
}

void ULactoseSimulationCropsTab::Init()
{
	SimulationSubsystem = Subsystems::Get<ULactoseSimulationServiceSubsystem>(self);
	EconomySubsystem = Subsystems::Get<ULactoseEconomyServiceSubsystem>(self);
}

void ULactoseSimulationCropsTab::Render()
{
	if (!SimulationSubsystem)
		return Debug::ImGui::Error("Simulation Subsystem could not be found");

	const ELactoseSimulationCropsStatus SimulationCropsStatus = SimulationSubsystem->GetAllCropsStatus();

	ImGui::Text("Status: ");
	ImGui::SameLine();
	
	switch (SimulationCropsStatus)
	{
		case ELactoseSimulationCropsStatus::None:
			ImGui::Text("None");
			if (ImGui::Button("Load Crops"))
				SimulationSubsystem->LoadAllCrops();
			break;
		case ELactoseSimulationCropsStatus::Querying:
			ImGui::Text("Querying");
			break;
		case ELactoseSimulationCropsStatus::Retrieving:
			ImGui::Text("Retrieving");
			break;
		case ELactoseSimulationCropsStatus::Loaded:
			ImGui::Text("Loaded");
			if (ImGui::Button("Load Crops"))
				SimulationSubsystem->LoadAllCrops();
			break;
		default:
			ImGui::Text("Unknown");
	}

	ImGui::Spacing();

	CropsSearchBox.Draw();

	ImGui::BeginChild("SimulationCrops", ImVec2(0, 0), /* bBorder*/ true);
	ON_SCOPE_EXIT
	{
		ImGui::EndChild();
	};

	const TMap<FString, Sr<FLactoseSimulationCrop>>& SimulationCrops = SimulationSubsystem->GetAllCrops();
	if (SimulationCrops.IsEmpty())
	{
		ImGui::Text("No Simulation Crops Found");
		return;
	}
	
	for (const auto& SimulationCrop : SimulationCrops)
	{
		const FString SimulationCropLabel = FString::Printf(TEXT("%s (%s)"), *SimulationCrop.Value->Id, *SimulationCrop.Value->Name);

		if (!CropsSearchBox.PassesFilter(SimulationCropLabel))
			continue;
		
		if (ImGui::CollapsingHeader(STR_TO_ANSI(SimulationCropLabel)))
		{
			ImGui::Indent();
			
			ImGui::Text("Id: %s", STR_TO_ANSI(SimulationCrop.Value->Id));
			ImGui::Text("Type: %s", STR_TO_ANSI(SimulationCrop.Value->Type));
			ImGui::Text("Name: %s", STR_TO_ANSI(SimulationCrop.Value->Name));
			ImGui::Text("Harvest Seconds: %f", SimulationCrop.Value->HarvestSeconds);

			if (!SimulationCrop.Value->CostItems.IsEmpty())
			{
				ImGui::Text("Cost Items:");
				ImGui::Indent();
				for (const FLactoseEconomyUserItem& CostItem : SimulationCrop.Value->CostItems)
				{
					FString CostItemLabel = CostItem.ItemId;
					
					if (EconomySubsystem)
						if (auto FoundItem = EconomySubsystem->GetItem(CostItem.ItemId))
							CostItemLabel += FString::Printf(TEXT(" (%s)"), *FoundItem->Name);
					
					ImGui::Text("%d x %s", CostItem.Quantity, STR_TO_ANSI(CostItemLabel));
				}
				ImGui::Unindent();
			}

			if (!SimulationCrop.Value->HarvestItems.IsEmpty())
			{
				ImGui::Text("Harvest Items:");
				ImGui::Indent();
				for (const FLactoseEconomyUserItem& HarvestItem : SimulationCrop.Value->HarvestItems)
				{
					FString HarvestItemLabel = HarvestItem.ItemId;
					
					if (EconomySubsystem)
						if (auto FoundItem = EconomySubsystem->GetItem(HarvestItem.ItemId))
							HarvestItemLabel += FString::Printf(TEXT(" (%s)"), *FoundItem->Name);
					
					ImGui::Text("%d x %s", HarvestItem.Quantity, STR_TO_ANSI(HarvestItemLabel));
				}
				ImGui::Unindent();
			}
			
			if (!SimulationCrop.Value->DestroyItems.IsEmpty())
			{
				ImGui::Text("Destroy Items:");
				ImGui::Indent();
				for (const FLactoseEconomyUserItem& DestroyItem : SimulationCrop.Value->DestroyItems)
				{
					FString DestroyItemLabel = DestroyItem.ItemId;
					
					if (EconomySubsystem)
						if (auto FoundItem = EconomySubsystem->GetItem(DestroyItem.ItemId))
							DestroyItemLabel += FString::Printf(TEXT(" (%s)"), *FoundItem->Name);
					
					ImGui::Text("%d x %s", DestroyItem.Quantity, STR_TO_ANSI(DestroyItemLabel));
				}
				ImGui::Unindent();
			}

			ImGui::Text("Fertiliser Item: %s", STR_TO_ANSI(SimulationCrop.Value->FertiliserItemId));

			ImGui::Text("Crop Actor Class: %s", STR_TO_ANSI(SimulationCrop.Value->GameCrop));
			
			ImGui::Unindent();
		}
	}
}
