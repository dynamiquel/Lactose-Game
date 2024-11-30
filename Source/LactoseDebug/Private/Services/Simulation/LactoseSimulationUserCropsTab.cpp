#include "Services/Simulation/LactoseSimulationUserCropsTab.h"

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
	SimulationSubsystem = UGameInstance::GetSubsystem<ULactoseSimulationServiceSubsystem>(GetWorld()->GetGameInstance());
	EconomySubsystem = UGameInstance::GetSubsystem<ULactoseEconomyServiceSubsystem>(GetWorld()->GetGameInstance());
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
	ImGui::Text("Previous Simulation Time: %s", STR_TO_ANSI(SimulationSubsystem->GetCurrentUserPreviousSimulationTime().ToString()));
	ImGui::Spacing();

	const bool bAnyCropsSelected = !SelectedUserCrops.IsEmpty();

	if (SelectedUserCrops.Num() < UserCrops->GetAllCropInstances().Num())
	{
		if (ImGui::Button("Select All"))
		{
			SelectedUserCrops.Reset();
		
			for (const TSharedRef<FLactoseSimulationUserCropInstance>& UserCrop : UserCrops->GetAllCropInstances())
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

		if (ImGui::Button("Harvest"))
		{
			
		}
		
		ImGui::SameLine();
		
		if (ImGui::Button("Seed"))
		{
			
		}

		ImGui::SameLine();

		if (ImGui::Button("Fertilise"))
		{
			
		}

		ImGui::SameLine();

		if (ImGui::Button("Destroy"))
		{
			
		}
	}

	ImGui::Spacing();
	
	UserCropsSearchBox.Draw();

	constexpr int32 Columns = 9;
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
		ImGui::TableSetupColumn("Actions");
		ImGui::TableHeadersRow();
		
		for (const TSharedRef<FLactoseSimulationUserCropInstance>& UserCrop : UserCrops->GetAllCropInstances())
		{
			FString CropName;
			if (const TSharedPtr<const FLactoseSimulationCrop> FoundCrop = SimulationSubsystem->FindCrop(UserCrop->CropId))
				CropName = FoundCrop->Name;

			// Allow the search box to filter by any of the below values.
			const FString CropSearchIdentifier = FString::Printf(TEXT("%s %s %s %s"),
				*UserCrop->Id,
				*UserCrop->CropId,
				*CropName,
				*UserCrop->State);
			
			if (!UserCropsSearchBox.PassesFilter(CropSearchIdentifier))
				return;
			
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
				ImGui::Text("%f.0 s", UserCrop->RemainingHarvestSeconds);

			if (ImGui::TableNextColumn())
				ImGui::Text("%f.0 s", UserCrop->RemainingFertiliserSeconds);

			if (ImGui::TableNextColumn())
			{
				if (UserCrop->State == Lactose::Simulation::States::Empty)
				{
					if (ImGui::Button("Seed"))
					{
						
					}

					ImGui::SameLine();
				}
				else if (UserCrop->State == Lactose::Simulation::States::Growing)
				{
					if (ImGui::Button("Fertilise"))
					{
						
					}

					ImGui::SameLine();
				}
				else if (UserCrop->State == Lactose::Simulation::States::Harvestable)
				{
					if (ImGui::Button("Harvest"))
					{
						
					}

					ImGui::SameLine();
				}
				
				if (ImGui::Button("Destroy"))
				{
					
				}
			}
		}

		ImGui::EndTable();
	}
}
