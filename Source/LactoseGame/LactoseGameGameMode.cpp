// Copyright Epic Games, Inc. All Rights Reserved.

#include "LactoseGameGameMode.h"
#include "LactoseGameCharacter.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"
#include "UObject/ConstructorHelpers.h"

ALactoseGameGameMode::ALactoseGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void ALactoseGameGameMode::StartPlay()
{
	// Keep ticking until all conditions have been met.
	GetWorldTimerManager().SetTimer(
		BeginPlayConditionCheckTimer,
		this, &ThisClass::ProcessBeginPlayConditions,
		/* Rate */ .25f,
		/* bLoop */ true);
}

void ALactoseGameGameMode::ProcessBeginPlayConditions()
{
	PendingConditions.Reset();

	auto* Identity = GetGameInstance()->GetSubsystem<ULactoseIdentityServiceSubsystem>();
	check(Identity);

	switch (Identity->GetLoginStatus())
	{
		case ELactoseIdentityUserLoginStatus::NotLoggedIn:
			PendingConditions.Emplace(TEXT("You are not logged in"));
			break;
		case ELactoseIdentityUserLoginStatus::LoggingIn:
			PendingConditions.Emplace(TEXT("You are being logged in"));
			break;
		default:
	}

	auto* Economy = GetGameInstance()->GetSubsystem<ULactoseEconomyServiceSubsystem>();
	check(Economy);
	
	switch (Economy->GetAllItemsStatus())
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

	switch (Economy->GetCurrentUserItemsStatus())
	{
		case ELactoseEconomyUserItemsStatus::None:
			PendingConditions.Emplace(TEXT("Your items have not been loaded"));
			break;
		case ELactoseEconomyUserItemsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Your items are being loaded"));
			break;
		default:
	}

	auto* Simulation = GetGameInstance()->GetSubsystem<ULactoseSimulationServiceSubsystem>();
	check(Simulation);

	switch (Simulation->GetAllCropsStatus())
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

	switch (Simulation->GetCurrentUserCropsStatus())
	{
		case ELactoseSimulationUserCropsStatus::None:
			PendingConditions.Emplace(TEXT("Your crops have not been loaded"));
			break;
		case ELactoseSimulationUserCropsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Your crops are being loaded"));
			break;
		default:
	}
	
	if (PendingConditions.IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(BeginPlayConditionCheckTimer);
		Super::StartPlay();
	}
	else
	{
		FString PendingConditionsFlattened;
		for (const FString& Condition : PendingConditions)
			PendingConditionsFlattened.Appendf(TEXT("\n%s"), *Condition);
			
		UE_LOG(LogGameMode, Verbose, TEXT("Game not starting because preconditions have not been met:%s"),
			*PendingConditionsFlattened);
	}
}
