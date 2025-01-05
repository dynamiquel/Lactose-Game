// Fill out your copyright notice in the Description page of Project Settings.


#include "LactoseMainMenuGameMode.h"

#include "LactoseMainMenuPlayerController.h"

ALactoseMainMenuGameMode::ALactoseMainMenuGameMode()
{
	PlayerControllerClass = ALactoseMainMenuPlayerController::StaticClass();
}
