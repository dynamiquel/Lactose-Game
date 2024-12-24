// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <GameplayTagContainer.h>

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LactoseGame/LactoseGameCharacter.h"

#include "LactoseHUD.generated.h"

struct FGameplayTag;
class ALactoseGameCharacter;

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ALactoseHUD : public AHUD
{
	GENERATED_BODY()

public:
	void SetToolHUD(const FGameplayTag& ToolHUD);

protected:
	ALactoseHUD();
	
	void PostInitializeComponents() override;
	void BeginPlay() override;

	UFUNCTION()
	void OnMenuOpened(const APlayerController* PlayerController, const FGameplayTag& MenuTag);

	UFUNCTION()
	void OnMenuClosed(const APlayerController* PlayerController, const FGameplayTag& MenuTag);

	UFUNCTION()
	void OnItemStateChanged(
		ALactoseGameCharacter* Sender,
		ELactoseCharacterItemState NewItemState,
		ELactoseCharacterItemState OldItemState);

	static FGameplayTag GetToolTypeFromItemState(ELactoseCharacterItemState ItemState);
	UUserWidget* GetToolWidgetFromToolType(const FGameplayTag& ToolHUD) const;

public:
// Menu widgets
	UPROPERTY()
	TSubclassOf<UUserWidget> PlayerMenuWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> PlantCropWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> SeedCropWidgetClass;

// Tool HUD widgets
	UPROPERTY()
	TSubclassOf<UUserWidget> NoneToolWidgetClass;
	
	UPROPERTY()
	TSubclassOf<UUserWidget> PlotToolWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> TreeToolWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> AnimalToolWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> ServiceIssuesWidgetClass;
	
protected:
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PlayerMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PlantCropWidget;
	
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> SeedCropWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> NoneToolWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PlotToolWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> TreeToolWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> AnimalToolWidget;

	FGameplayTag ActiveToolHUD;
};
