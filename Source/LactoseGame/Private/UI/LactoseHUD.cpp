// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LactoseHUD.h"

#include "Core.h"
#include "LactoseMenuTags.h"
#include "Blueprint/UserWidget.h"
#include "LactoseGame/LactoseGame.h"
#include "LactoseGame/LactoseGamePlayerController.h"
#include "LactoseGame/LactoseGameCharacter.h"
#include "LactoseGame/LactoseHUDTags.h"

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

void ALactoseHUD::SetToolHUD(const FGameplayTag& ToolHUD)
{
	if (ToolHUD == ActiveToolHUD)
		return;

	if (UUserWidget* CurrentToolHUD = GetToolWidgetFromToolType(ActiveToolHUD))
		CurrentToolHUD->RemoveFromParent();

	if (UUserWidget* NewToolHUD = GetToolWidgetFromToolType(ToolHUD))
		NewToolHUD->AddToPlayerScreen();
	
	ActiveToolHUD = ToolHUD;
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
	static ConstructorHelpers::FClassFinder<UUserWidget> PlotToolWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_PlotTool"));
	static ConstructorHelpers::FClassFinder<UUserWidget> TreeToolWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_TreeTool"));
	static ConstructorHelpers::FClassFinder<UUserWidget> AnimalToolWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_AnimalTool"));

	NoneToolWidgetClass = NoneToolWidgetClassFinder.Class;
	PlotToolWidgetClass = PlotToolWidgetClassFinder.Class;
	TreeToolWidgetClass = TreeToolWidgetClassFinder.Class;
	AnimalToolWidgetClass = AnimalToolWidgetClassFinder.Class;
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

	if (LIKELY(PlotToolWidgetClass))
	{
		PlotToolWidget = CreateWidget(GetOwningPlayerController(), PlotToolWidgetClass);
	}
	else
	{
		UE_LOG(LogLactose, Error, TEXT("HUD Plot Tool Widget Class was not set"));
	}

	if (LIKELY(TreeToolWidgetClass))
	{
		TreeToolWidget = CreateWidget(GetOwningPlayerController(), TreeToolWidgetClass);
	}
	else
	{
		UE_LOG(LogLactose, Error, TEXT("HUD Tree Tool Widget Class was not set"));
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
}

void ALactoseHUD::BeginPlay()
{
	Super::BeginPlay();

	auto* LactoseChar = Cast<ALactoseGameCharacter>(GetOwningPawn());
	if (!ensure(LactoseChar))
	{
		return;
	}
	
	LactoseChar->GetItemStateChanged().AddUniqueDynamic(this, &ThisClass::OnItemStateChanged);

	const ELactoseCharacterItemState CharItemState = LactoseChar->GetCurrentItemState();
	const FGameplayTag DesiredToolHUD = GetToolTypeFromItemState(CharItemState);
	SetToolHUD(DesiredToolHUD);
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
	const FGameplayTag ToolHUDType = GetToolTypeFromItemState(NewItemState);
	if (!ensure(ToolHUDType.IsValid()))
		return;
	
	SetToolHUD(ToolHUDType);
}

FGameplayTag ALactoseHUD::GetToolTypeFromItemState(const ELactoseCharacterItemState ItemState)
{
	switch (ItemState)
	{
		case ELactoseCharacterItemState::None:
			return Lactose::HUD::Tool::None;
		case ELactoseCharacterItemState::PlotTool:
			return Lactose::HUD::Tool::Plot;
		case ELactoseCharacterItemState::TreeTool:
			return Lactose::HUD::Tool::Tree;
		default:
			return FGameplayTag();
	}
}

UUserWidget* ALactoseHUD::GetToolWidgetFromToolType(const FGameplayTag& ToolHUD) const
{
	if (ToolHUD.MatchesTag(Lactose::HUD::Tool::None))
		return NoneToolWidget;
	if (ToolHUD.MatchesTag(Lactose::HUD::Tool::Plot))
		return PlotToolWidget;
	if (ToolHUD.MatchesTag(Lactose::HUD::Tool::Tree))
		return TreeToolWidget;
	if (ToolHUD.MatchesTag(Lactose::HUD::Tool::Animal))
		return AnimalToolWidget;

	return nullptr;
}
