// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

protected:
	ALactoseHUD();
	
	void PostInitializeComponents() override;

	UFUNCTION()
	void OnMenuOpened(const APlayerController* PlayerController, const FGameplayTag& MenuTag);

	UFUNCTION()
	void OnMenuClosed(const APlayerController* PlayerController, const FGameplayTag& MenuTag);

	UFUNCTION()
	void OnItemStateChanged(
		ALactoseGameCharacter* Sender,
		ELactoseCharacterItemState NewItemState,
		ELactoseCharacterItemState OldItemState);

	UUserWidget* GetToolWidgetFromToolType(ELactoseCharacterItemState ItemState) const;

public:
	UPROPERTY()
	TSubclassOf<UUserWidget> PlayerMenuWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> PlantCropWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> SeedCropWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> NoneToolWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> SeedToolWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> CropToolWidgetClass;

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
	TObjectPtr<UUserWidget> SeedToolWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> TreeToolWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> AnimalToolWidget;
};
