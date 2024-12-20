// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "LactoseHUD.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ALactoseHUD : public AHUD
{
	GENERATED_BODY()

protected:
	ALactoseHUD();

	void PostInitializeComponents() override;

	UFUNCTION()
	void OnMenuOpened(const APlayerController* PlayerController, const FGameplayTag& MenuTag);

	UFUNCTION()
	void OnMenuClosed(const APlayerController* PlayerController, const FGameplayTag& MenuTag);

public:
	UPROPERTY()
	TSubclassOf<UUserWidget> PlayerMenuWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> PlantCropWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> SeedCropWidgetClass;
	
protected:
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PlayerMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PlantCropWidget;
	
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> SeedCropWidget;
};
