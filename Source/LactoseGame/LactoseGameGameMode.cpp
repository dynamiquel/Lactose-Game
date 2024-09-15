// Copyright Epic Games, Inc. All Rights Reserved.

#include "LactoseGameGameMode.h"
#include "LactoseGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALactoseGameGameMode::ALactoseGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
