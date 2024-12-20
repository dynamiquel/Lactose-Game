// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LactoseHUD.h"

#include "LactoseMenuTags.h"
#include "Blueprint/UserWidget.h"
#include "LactoseGame/LactoseGame.h"
#include "LactoseGame/LactoseGamePlayerController.h"

ALactoseHUD::ALactoseHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> PauseWidgetClassFinder(TEXT("/Game/UI/WBP_PlayerMenu"));
	static ConstructorHelpers::FClassFinder<UUserWidget> PlantCropWidgetClassFinder(TEXT("/Game/UI/WBP_PlantCrop"));
	static ConstructorHelpers::FClassFinder<UUserWidget> SeedCropWidgetClassFinder(TEXT("/Game/UI/WBP_SeedCrop"));

	PlayerMenuWidgetClass = PauseWidgetClassFinder.Class;
	PlantCropWidgetClass = PlantCropWidgetClassFinder.Class;
	SeedCropWidgetClass = SeedCropWidgetClassFinder.Class;
}

void ALactoseHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (LIKELY(PlayerMenuWidgetClass))
	{
		PlayerMenuWidget = CreateWidget(GetOwningPlayerController(), PlayerMenuWidgetClass);
	}
	else
	{
		UE_LOG(LogLactose, Error, TEXT("HUD Player Menu Widget Class was not set"));
	}

	if (LIKELY(PlantCropWidgetClass))
	{
		PlantCropWidget = CreateWidget(GetOwningPlayerController(), PlantCropWidgetClass);
	}
	else
	{
		UE_LOG(LogLactose, Error, TEXT("HUD Plant Crop Widget Class was not set"));
	}

	if (LIKELY(SeedCropWidgetClass))
	{
		SeedCropWidget = CreateWidget(GetOwningPlayerController(), SeedCropWidgetClass);
	}
	else
	{
		UE_LOG(LogLactose, Error, TEXT("HUD Seed Crop Widget Class was not set"));
	}

	auto* LactosePC = Cast<ALactoseGamePlayerController>(GetOwningPlayerController());
	if (!ensure(LactosePC))
	{
		return;
	}

	LactosePC->OnMenuOpened.AddUniqueDynamic(this, &ThisClass::OnMenuOpened);
	LactosePC->OnMenuClosed.AddUniqueDynamic(this, &ThisClass::OnMenuClosed);
}

void ALactoseHUD::OnMenuOpened(const APlayerController* PlayerController, const FGameplayTag& MenuTag)
{
	if (MenuTag.MatchesTag(Lactose::Menus::Player))
	{
		if (LIKELY(PlayerMenuWidget))
		{
			PlayerMenuWidget->AddToPlayerScreen();
		}
	}
	else if (MenuTag.MatchesTag(Lactose::Menus::PlantCrop))
	{
		if (LIKELY(PlantCropWidget))
			PlantCropWidget->AddToPlayerScreen();
	}
	else if (MenuTag.MatchesTag(Lactose::Menus::SeedCrop))
	{
		if (LIKELY(SeedCropWidget))
			SeedCropWidget->AddToPlayerScreen();
	}
}

void ALactoseHUD::OnMenuClosed(const APlayerController* PlayerController, const FGameplayTag& MenuTag)
{
	if (MenuTag.MatchesTag(Lactose::Menus::Player))
	{
		if (LIKELY(PlayerMenuWidget))
			PlayerMenuWidget->RemoveFromParent();
	}
	else if (MenuTag.MatchesTag(Lactose::Menus::PlantCrop))
	{
		if (LIKELY(PlantCropWidget))
			PlantCropWidget->RemoveFromParent();
	}
	else if (MenuTag.MatchesTag(Lactose::Menus::SeedCrop))
	{
		if (LIKELY(SeedCropWidget))
			SeedCropWidget->RemoveFromParent();
	}
}
