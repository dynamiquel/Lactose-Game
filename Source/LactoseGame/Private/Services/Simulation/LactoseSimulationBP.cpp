// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Simulation/LactoseSimulationBP.h"

#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

TMap<FString, FLactoseSimulationCrop> ULactoseSimulationBP::GetCrops(const ULactoseSimulationServiceSubsystem* Simulation)
{
	if (!Simulation)
		return {};

	const TMap<FString, Sr<FLactoseSimulationCrop>>& FoundCrops = Simulation->GetAllCrops();
	TMap<FString, FLactoseSimulationCrop> CopiedCrops;
	CopiedCrops.Reserve(FoundCrops.Num());
	
	for (const auto& Crop : FoundCrops)
		CopiedCrops.Add(Crop.Key, *Crop.Value);

	return CopiedCrops;
}

FLactoseSimulationCrop ULactoseSimulationBP::GetCrop(const ULactoseSimulationServiceSubsystem* Simulation, const FString& CropName)
{
	if (!Simulation)
		return {};

	const Sr<FLactoseSimulationCrop>* FoundCrop = Simulation->GetAllCrops().Find(CropName);
	return FoundCrop ? FoundCrop->Get() : FLactoseSimulationCrop();
}

void ULactoseSimulationBP::SeedCurrentUserCrops(ULactoseSimulationServiceSubsystem* Simulation, const TArray<FString>& CropInstanceIds, const FString& CropId)
{
	if (!Simulation)
		return;

	Simulation->SeedCropInstances(CropInstanceIds, CropId);

}

TArray<FLactoseEconomyUserItem> ULactoseSimulationBP::GetCropCostUserItems(
	const ULactoseSimulationServiceSubsystem* Simulation, const FString& CropId)
{
	TArray<FLactoseEconomyUserItem> UserCropCostItems;
	if (!Simulation)
		return UserCropCostItems;
	
	const Sp<const FLactoseSimulationCrop> FoundCrop =  Simulation->FindCrop(CropId);
	if (!FoundCrop)
		return UserCropCostItems;

	for (const FLactoseEconomyUserItem& CostItem : FoundCrop->CostItems)
	{
		const auto& EconomySubsystem = Lactose::GetService<ULactoseEconomyServiceSubsystem>(*Simulation);
		Sp<const FLactoseEconomyUserItem> FoundUserItem = EconomySubsystem.FindCurrentUserItem(CostItem.ItemId);
		UserCropCostItems.Add(FoundUserItem ? *FoundUserItem : FLactoseEconomyUserItem{.ItemId = CostItem.ItemId});
	}
	
	return UserCropCostItems;
}

bool ULactoseSimulationBP::CanCurrentUserAffordCrop(const ULactoseSimulationServiceSubsystem* Simulation, const FString& CropId)
{
	if (!Simulation)
		return false;

	return Simulation->CanCurrentUserAffordCrop(CropId);
}
