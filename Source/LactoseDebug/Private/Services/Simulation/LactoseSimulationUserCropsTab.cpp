#include "Services/Simulation/LactoseSimulationUserCropsTab.h"

#include <string>
#include "imgui_stdlib.h"

#include "Services/Simulation/LactoseSimulationDebugApp.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"

ULactoseSimulationUserCropsTab::ULactoseSimulationUserCropsTab()
{
	SetOwningAppClass<ULactoseSimulationDebugApp>();
	SetTabName(TEXT("User Crops"));
}

void ULactoseSimulationUserCropsTab::Init()
{
	SimulationSubsystem = Subsystems::Get<ULactoseSimulationServiceSubsystem>(self);
	EconomySubsystem = Subsystems::Get<ULactoseEconomyServiceSubsystem>(self);
}

void ULactoseSimulationUserCropsTab::Render()
{
	if (!SimulationSubsystem)
		return Debug::ImGui::Error("Simulation Subsystem could not be found");

	const ELactoseSimulationUserCropsStatus SimulationUserCropsStatus = SimulationSubsystem->GetCurrentUserCropsStatus();

	ImGui::Text("Status: ");
	ImGui::SameLine();
	
	switch (SimulationUserCropsStatus)
	{
		case ELactoseSimulationUserCropsStatus::None:
			ImGui::Text("None");
			if (ImGui::Button("Load Crops"))
				SimulationSubsystem->LoadCurrentUserCrops();
			break;
		case ELactoseSimulationUserCropsStatus::Retrieving:
			ImGui::Text("Retrieving");
			break;
		case ELactoseSimulationUserCropsStatus::Loaded:
			ImGui::Text("Loaded");
			if (ImGui::Button("Load Crops"))
				SimulationSubsystem->LoadCurrentUserCrops();
			break;
		default:
			ImGui::Text("Unknown");
	}

	auto UserCrops = SimulationSubsystem->GetCurrentUserCrops();
	if (!UserCrops)
		return;
	
	ImGui::Spacing();
	
	if (ImGui::Button("Simulate"))
		SimulationSubsystem->Simulate();

	ImGui::SameLine();
	
	if (SimulationSubsystem->IsAutoSimulateTicking())
	{
		if (ImGui::Button("Disable Simulate Tick"))
			SimulationSubsystem->DisableSimulateTicker();
	}
	else
	{
		if (ImGui::Button("Enable Simulate Tick"))
			SimulationSubsystem->EnableSimulateTicker();
	}
	
	ImGui::SameLine();
	
	ImGui::Text("Previous Simulation Time: %s", STR_TO_ANSI(SimulationSubsystem->GetCurrentUserPreviousSimulationTime().ToString()));
	
	ImGui::Spacing();

	const bool bAnyCropsSelected = !SelectedUserCrops.IsEmpty();

	if (SelectedUserCrops.Num() < UserCrops->GetAllCropInstances().Num())
	{
		if (ImGui::Button("Select All"))
		{
			SelectedUserCrops.Reset();
		
			for (const Sr<FLactoseSimulationUserCropInstance>& UserCrop : UserCrops->GetAllCropInstances())
				SelectedUserCrops.Add(UserCrop->Id);
		}

		if (bAnyCropsSelected)
			ImGui::SameLine();
	}

	if (bAnyCropsSelected)
	{
		if (ImGui::Button("Deselect All"))
			SelectedUserCrops.Reset();

		ImGui::SameLine();
		ImGui::Text("Selected %d Crops", SelectedUserCrops.Num());

		ImGui::Spacing();

		if (ImGui::Button("Harvest"))
			SimulationSubsystem->HarvestCropInstances(SelectedUserCrops.Array());
		
		ImGui::SameLine();

		if (ImGui::Button("Fertilise"))
			SimulationSubsystem->FertiliseCropInstances(SelectedUserCrops.Array());

		ImGui::SameLine();

		if (ImGui::Button("Destroy"))
			SimulationSubsystem->DestroyCropInstances(SelectedUserCrops.Array());

		ImGui::SameLine();
		
		if (ImGui::Button("Seed"))
		{
			if (!OverrideSeedCropId.empty())
			{
				const FString SeedCropIdStr = ANSI_TO_TCHAR(OverrideSeedCropId.data());
				SimulationSubsystem->SeedCropInstances(SelectedUserCrops.Array(), SeedCropIdStr);
			}
			else
			{
				/*
				 * Go through each Selected Crop and reseed it with its last known crop.
				 * Batch process the requests based on the seed crop.
				 *
				 * This is done because the Simulation API can only do batch requests on a per-seed basis.
				 *
				 * TMap<SeedCropId, UserCropInstanceId
				 */
				TMap<FString, TArray<FString>> CropsToSeedBySeed;
				
				for (const FString& SelectedUserCropId : SelectedUserCrops)
					if (auto UserCropInstance = UserCrops->FindCropInstance(SelectedUserCropId))
						CropsToSeedBySeed.FindOrAdd(UserCropInstance->CropId).Add(UserCropInstance->Id);

				for (const auto& CropsToSeed : CropsToSeedBySeed)
					SimulationSubsystem->SeedCropInstances(CropsToSeed.Value, CropsToSeed.Key);
			}
		}
		ImGui::SameLine();
		ImGui::Text("Override Seed Crop Id: ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.f);
		ImGui::InputText("###SeedCropIdInput", &OverrideSeedCropId);
	}

	ImGui::Spacing();

	ImGui::SetNextItemWidth(200.f);
	UserCropsSearchBox.Draw();

	constexpr int32 Columns = 8;
	constexpr int32 TableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
	if (ImGui::BeginTable("UserCropsTable", Columns, TableFlags))
	{
		ImGui::TableSetupColumn("Select");
		ImGui::TableSetupColumn("ID");
		ImGui::TableSetupColumn("Crop ID");
		ImGui::TableSetupColumn("Crop Name");
		ImGui::TableSetupColumn("State");
		ImGui::TableSetupColumn("Created");
		ImGui::TableSetupColumn("Harvest");
		ImGui::TableSetupColumn("Fertilise");
		ImGui::TableHeadersRow();
		
		for (const Sr<FLactoseSimulationUserCropInstance>& UserCrop : UserCrops->GetAllCropInstances())
		{
			FString CropName;
			if (const Sp<const FLactoseSimulationCrop> FoundCrop = SimulationSubsystem->FindCrop(UserCrop->CropId))
				CropName = FoundCrop->Name;

			// Allow the search box to filter by any of the below values.
			const FString CropSearchIdentifier = FString::Printf(TEXT("%s %s %s %s"),
				*UserCrop->Id,
				*UserCrop->CropId,
				*CropName,
				*UserCrop->State);
			
			if (!UserCropsSearchBox.PassesFilter(CropSearchIdentifier))
				continue;
			
			if (ImGui::TableNextColumn())
			{
				const FString CheckboxId = TEXT("###") + UserCrop->Id;
				bool bSelected = SelectedUserCrops.Contains(UserCrop->Id);
				if (ImGui::Checkbox(STR_TO_ANSI(CheckboxId), &bSelected))
				{
					if (bSelected)
						SelectedUserCrops.Add(UserCrop->Id);
					else
						SelectedUserCrops.Remove(UserCrop->Id);
				}
			}

			if (ImGui::TableNextColumn())
				ImGui::Text("%s", STR_TO_ANSI(UserCrop->Id));

			if (ImGui::TableNextColumn())
				ImGui::Text("%s", STR_TO_ANSI(UserCrop->CropId));

			if (ImGui::TableNextColumn())
				ImGui::Text("%s", STR_TO_ANSI(CropName));

			if (ImGui::TableNextColumn())
				ImGui::Text("%s", STR_TO_ANSI(UserCrop->State));

			if (ImGui::TableNextColumn())
			{
				const FString CreationTimeStr = UserCrop->CreationTime.ToString();
				ImGui::Text("%s", STR_TO_ANSI(CreationTimeStr));
			}

			if (ImGui::TableNextColumn())
				ImGui::Text("%.0f s", UserCrop->RemainingHarvestSeconds);

			if (ImGui::TableNextColumn())
				ImGui::Text("%.0f s", UserCrop->RemainingFertiliserSeconds);
		}

		ImGui::EndTable();
	}
}
