// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>
#include "DebugAppTab.h"
#include "LactoseIdentityUserTab.generated.h"

class ULactoseIdentityServiceSubsystem;

UCLASS()
class LACTOSEDEBUG_API ULactoseIdentityUserTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseIdentityUserTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab

private:
	UPROPERTY(Transient)
	TObjectPtr<ULactoseIdentityServiceSubsystem> IdentitySubsystem = nullptr;
};
