// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LactoseHUD.h"

#include "LactoseMenuTags.h"
#include "Blueprint/UserWidget.h"
#include "LactoseGame/LactoseGame.h"
#include "LactoseGame/LactoseGamePlayerController.h"
#include "Core.h"
#include "LactoseGame/LactoseGameCharacter.h"

constexpr auto BaseWidgetShowFunctionName = TEXT("Show");
constexpr auto BaseWidgetHideFunctionName = TEXT("Hide");

void CallBPFunction(UObject& Object, const TCHAR* FunctionName)
{
	auto OutputDeviceNull = FOutputDeviceNull();
	Object.CallFunctionByNameWithArguments(
		FunctionName,
		OutputDeviceNull,
		nullptr);
}

ALactoseHUD::ALactoseHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> PauseWidgetClassFinder(TEXT("/Game/UI/WBP_PlayerMenu"));
	static ConstructorHelpers::FClassFinder<UUserWidget> PlantCropWidgetClassFinder(TEXT("/Game/UI/WBP_SeedTreeCrop"));
	static ConstructorHelpers::FClassFinder<UUserWidget> SeedCropWidgetClassFinder(TEXT("/Game/UI/WBP_SeedPlotCrop"));

	PlayerMenuWidgetClass = PauseWidgetClassFinder.Class;
	PlantCropWidgetClass = PlantCropWidgetClassFinder.Class;
	SeedCropWidgetClass = SeedCropWidgetClassFinder.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> NoneToolWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_NoneTool"));
	static ConstructorHelpers::FClassFinder<UUserWidget> SeedToolWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_SeedTool"));
	static ConstructorHelpers::FClassFinder<UUserWidget> CropToolWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_CropTool"));
	static ConstructorHelpers::FClassFinder<UUserWidget> AnimalToolWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_AnimalTool"));

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

	if (LIKELY(NoneToolWidgetClass))
	{
		NoneToolWidget = CreateWidget(GetOwningPlayerController(), NoneToolWidgetClass);
	}
	else
	{
		UE_LOG(LogLactose, Error, TEXT("HUD None Tool Widget Class was not set"));
	}

	if (LIKELY(SeedToolWidgetClass))
	{
		SeedToolWidget = CreateWidget(GetOwningPlayerController(), SeedToolWidgetClass);
	}
	else
	{
		UE_LOG(LogLactose, Error, TEXT("HUD Seed Tool Widget Class was not set"));
	}

	if (LIKELY(CropToolWidgetClass))
	{
		TreeToolWidget = CreateWidget(GetOwningPlayerController(), CropToolWidgetClass);
	}
	else
	{
		UE_LOG(LogLactose, Error, TEXT("HUD Crop Tool Widget Class was not set"));
	}

	if (LIKELY(AnimalToolWidgetClass))
	{
		AnimalToolWidget = CreateWidget(GetOwningPlayerController(), AnimalToolWidgetClass);
	}
	else
	{
		UE_LOG(LogLactose, Error, TEXT("HUD Animal Tool Widget Class was not set"));
	}

	auto* LactosePC = Cast<ALactoseGamePlayerController>(GetOwningPlayerController());
	if (!ensure(LactosePC))
	{
		return;
	}

	LactosePC->OnMenuOpened.AddUniqueDynamic(this, &ThisClass::OnMenuOpened);
	LactosePC->OnMenuClosed.AddUniqueDynamic(this, &ThisClass::OnMenuClosed);

	auto* LactoseChar = Cast<ALactoseGameCharacter>(LactosePC->GetPawn());
	if (!ensure(LactoseChar))
	{
		return;
	}
	
	LactoseChar->GetItemStateChanged().AddUniqueDynamic(this, &ThisClass::OnItemStateChanged);
}

void ALactoseHUD::OnMenuOpened(const APlayerController* PlayerController, const FGameplayTag& MenuTag)
{
	if (MenuTag.MatchesTag(Lactose::Menus::Player))
	{
		if (LIKELY(PlayerMenuWidget))
		{
			PlayerMenuWidget->AddToPlayerScreen();
			CallBPFunction(*PlayerMenuWidget, BaseWidgetShowFunctionName);
		}
	}
	else if (MenuTag.MatchesTag(Lactose::Menus::PlantCrop))
	{
		if (LIKELY(PlantCropWidget))
		{
			PlantCropWidget->AddToPlayerScreen();
			CallBPFunction(*PlantCropWidget, BaseWidgetShowFunctionName);
		}
	}
	else if (MenuTag.MatchesTag(Lactose::Menus::SeedCrop))
	{
		if (LIKELY(SeedCropWidget))
		{
			SeedCropWidget->AddToPlayerScreen();
			CallBPFunction(*SeedCropWidget, BaseWidgetShowFunctionName);
		}
	}
}

void ALactoseHUD::OnMenuClosed(const APlayerController* PlayerController, const FGameplayTag& MenuTag)
{
	if (MenuTag.MatchesTag(Lactose::Menus::Player))
	{
		if (LIKELY(PlayerMenuWidget))
		{
			CallBPFunction(*PlayerMenuWidget, BaseWidgetHideFunctionName);
			PlayerMenuWidget->RemoveFromParent();
		}
	}
	else if (MenuTag.MatchesTag(Lactose::Menus::PlantCrop))
	{
		if (LIKELY(PlantCropWidget))
		{
			CallBPFunction(*PlantCropWidget, BaseWidgetHideFunctionName);
			PlantCropWidget->RemoveFromParent();
		}
	}
	else if (MenuTag.MatchesTag(Lactose::Menus::SeedCrop))
	{
		if (LIKELY(SeedCropWidget))
		{
			CallBPFunction(*SeedCropWidget, BaseWidgetHideFunctionName);
			SeedCropWidget->RemoveFromParent();
		}
	}
}

void ALactoseHUD::OnItemStateChanged(
	ALactoseGameCharacter* Sender,
	const ELactoseCharacterItemState NewItemState,
	const ELactoseCharacterItemState OldItemState)
{
	if (UUserWidget* WidgetToRemove = GetToolWidgetFromToolType(OldItemState))
		WidgetToRemove->RemoveFromParent();

	if (UUserWidget* WidgetToAdd = GetToolWidgetFromToolType(NewItemState))
		WidgetToAdd->AddToPlayerScreen();
}

UUserWidget* ALactoseHUD::GetToolWidgetFromToolType(const ELactoseCharacterItemState ItemState) const
{
	switch (ItemState)
	{
		case ELactoseCharacterItemState::None:
			return NoneToolWidget;
		case ELactoseCharacterItemState::PlotTool:
			return SeedToolWidget;
		case ELactoseCharacterItemState::TreeTool:
			return TreeToolWidget;
		default:
			return nullptr;
	}
}
