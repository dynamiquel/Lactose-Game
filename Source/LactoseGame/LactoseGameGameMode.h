// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LactoseGameGameMode.generated.h"

UCLASS(minimalapi)
class ALactoseGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALactoseGameGameMode();

	// Begin override AGameModeBase
	void StartPlay() override;
	// End override AGameModeBase

	void ProcessBeginPlayConditions();
	
private:
	TArray<FString> PendingConditions;
	FTimerHandle BeginPlayConditionCheckTimer;
};



