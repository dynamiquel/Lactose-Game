// Copyright Epic Games, Inc. All Rights Reserved.

#include "LactoseGameGameMode.h"
#include "SimpLog.h"
#include "UI/LactoseHUD.h"
#include "Services/ConfigCloud/LactoseConfigCloudServiceSubsystem.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"
#include "UObject/ConstructorHelpers.h"

ALactoseGameGameMode::ALactoseGameGameMode()
	: Super()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	HUDClass = ALactoseHUD::StaticClass();
}

void ALactoseGameGameMode::StartPlay()
{
	Log::Hello();
	
	// Keep ticking until all conditions have been met.
	GetWorldTimerManager().SetTimer(
		BeginPlayConditionCheckTimer,
		this, &ThisClass::ProcessBeginPlayConditions,
		/* Rate */ .25f,
		/* bLoop */ true);

	// This only prevents BeginPlay being called on Actors and Components.
	// It doesn't prevent the World from calling Begin Play or its Subsystems.
	// That seems to require an engine change but another hack around could be to just reload the World when
	// everything is ready.
}

void ALactoseGameGameMode::ProcessBeginPlayConditions()
{
	PendingConditions.Reset();

	auto& Identity = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);

	switch (Identity.GetLoginStatus())
	{
		case ELactoseIdentityUserLoginStatus::NotLoggedIn:
			PendingConditions.Emplace(TEXT("You are not logged in"));
			break;
		case ELactoseIdentityUserLoginStatus::LoggingIn:
			PendingConditions.Emplace(TEXT("You are being logged in"));
			break;
		default:
	}

	auto& Economy = Subsystems::GetRef<ULactoseEconomyServiceSubsystem>(self);
	
	switch (Economy.GetAllItemsStatus())
	{
		case ELactoseEconomyAllItemsStatus::None:
			PendingConditions.Emplace(TEXT("Economy items have not been loaded"));
			break;
		case ELactoseEconomyAllItemsStatus::Querying:
			PendingConditions.Emplace(TEXT("Economy items are being queried"));
			break;
		case ELactoseEconomyAllItemsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Economy items are being loaded"));
			break;
		default:
	}

	switch (Economy.GetCurrentUserItemsStatus())
	{
		case ELactoseEconomyUserItemsStatus::None:
			PendingConditions.Emplace(TEXT("Your items have not been loaded"));
			break;
		case ELactoseEconomyUserItemsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Your items are being loaded"));
			break;
		default:
	}

	auto& Simulation = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);

	switch (Simulation.GetAllCropsStatus())
	{
		case ELactoseSimulationCropsStatus::None:
			PendingConditions.Emplace(TEXT("Crops have not been loaded"));
			break;
		case ELactoseSimulationCropsStatus::Querying:
			PendingConditions.Emplace(TEXT("Crops are being queried"));
			break;
		case ELactoseSimulationCropsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Crops are being loaded"));
			break;
		default:
	}

	switch (Simulation.GetCurrentUserCropsStatus())
	{
		case ELactoseSimulationUserCropsStatus::None:
			PendingConditions.Emplace(TEXT("Your crops have not been loaded"));
			break;
		case ELactoseSimulationUserCropsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Your crops are being loaded"));
			break;
		default:
	}

	auto& ConfigCloud = Subsystems::GetRef<ULactoseConfigCloudServiceSubsystem>(self);

	switch (ConfigCloud.GetStatus())
	{
		case ELactoseConfigCloudStatus::None:
			PendingConditions.Emplace(TEXT("Config has not been loaded"));
			break;
		case ELactoseConfigCloudStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Config is being loaded"));
			break;
		default:
	}
	
	if (PendingConditions.IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(BeginPlayConditionCheckTimer);
		Log::Log(LogGameMode, TEXT("Game started"));
		Super::StartPlay();
	}
	else
	{
		FString PendingConditionsFlattened;
		for (const FString& Condition : PendingConditions)
			PendingConditionsFlattened.Appendf(TEXT("\n%s"), *Condition);

		Log::Verbose(LogGameMode,
			TEXT("Game not starting because preconditions have not been met:%s"),
			*PendingConditionsFlattened);
	}
}