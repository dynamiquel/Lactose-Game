// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Simulation/LactoseSimulationBP.h"

#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

TMap<FString, FLactoseSimulationCrop> ULactoseSimulationBP::GetCrops(const ULactoseSimulationServiceSubsystem* Simulation)
{
	if (!Simulation)
		return {};

	const TMap<FString, TSharedRef<FLactoseSimulationCrop>>& FoundCrops = Simulation->GetAllCrops();
	TMap<FString, FLactoseSimulationCrop> CopiedCrops;
	CopiedCrops.Reserve(FoundCrops.Num());
	
	for (const auto& Crop : FoundCrops)
		CopiedCrops.Add(Crop.Key, *Crop.Value);

	return CopiedCrops;
}

TOptional<FLactoseSimulationCrop> ULactoseSimulationBP::GetCrop(const ULactoseSimulationServiceSubsystem* Simulation, const FString& CropName)
{
	if (!Simulation)
		return {};

	const TSharedRef<FLactoseSimulationCrop>* FoundCrop = Simulation->GetAllCrops().Find(CropName);
	return FoundCrop ? FoundCrop->Get() : TOptional<FLactoseSimulationCrop>();
}
