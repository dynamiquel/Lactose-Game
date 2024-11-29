// Copyright Epic Games, Inc. All Rights Reserved.

#include "LactoseGameGameMode.h"
#include "LactoseGameCharacter.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"
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

	switch (ELactoseIdentityUserLoginStatus IdentityStatus = Identity->GetLoginStatus())
	{
		case ELactoseIdentityUserLoginStatus::NotLoggedIn:
			PendingConditions.Emplace(TEXT("You are not logged in"));
			break;
		case ELactoseIdentityUserLoginStatus::LoggingIn:
			PendingConditions.Emplace(TEXT("You are being logged in"));
			break;
		default: ;
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
