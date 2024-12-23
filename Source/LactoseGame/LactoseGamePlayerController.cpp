// Copyright Epic Games, Inc. All Rights Reserved.


#include "LactoseGamePlayerController.h"

#include <EnhancedInputComponent.h>

#include "EnhancedInputSubsystems.h"
#include "LactoseGame.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"

#include "LactoseMenuTags.h"

void ALactoseGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (auto* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		EnhancedInputComponent->BindAction(PlayerMenuAction, ETriggerEvent::Completed, this, &ThisClass::OnPlayerMenuActionPressed);
}

void ALactoseGamePlayerController::OnPlayerMenuActionPressed()
{
	const bool bOpened = IsAnyMenuOpened();
	bOpened ? CloseActiveMenu() : OpenMenu(Lactose::Menus::Player);
}

ALactoseGamePlayerController::ALactoseGamePlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultCharacterInputMapping(TEXT("/Game/FirstPerson/Input/IMC_Default.IMC_Default"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMenuInputMapping(TEXT("/Game/FirstPerson/Input/IMC_Menu.IMC_Menu"));
	static ConstructorHelpers::FObjectFinder<UInputAction> DefaultPlayerMenuAction(TEXT("/Game/FirstPerson/Input/Actions/IA_PlayerMenu.IA_PlayerMenu"));

	CharacterMappingContext = DefaultCharacterInputMapping.Object;
	MenuMappingContext = DefaultMenuInputMapping.Object;
	PlayerMenuAction = DefaultPlayerMenuAction.Object;
}

bool ALactoseGamePlayerController::IsMenuOpened(const FGameplayTag& MenuTag, const bool bExactMenu) const
{
	return OpenedMenuTag && (MenuTag.MatchesTagExact(*OpenedMenuTag) || (!bExactMenu && MenuTag.MatchesTag(*OpenedMenuTag)));
}

bool ALactoseGamePlayerController::IsAnyMenuOpened() const
{
	return OpenedMenuTag.IsSet();
}

void ALactoseGamePlayerController::OpenMenu(const FGameplayTag& MenuTag)
{
	if (!ensure(MenuTag.IsValid()))
	{
		return;
	}
	
	if (OpenedMenuTag)
	{
		// Menu is already open.
		if (MenuTag.MatchesTagExact(*OpenedMenuTag))
			return;

		// A menu is already open, and it's not a parent menu.
		const bool bExistingMenuNeedsClosing = !MenuTag.MatchesTag(*OpenedMenuTag);
		if (bExistingMenuNeedsClosing)
			CloseActiveMenu();
	}

	OpenedMenuTag = MenuTag;
	OnMenuOpened.Broadcast(this, *OpenedMenuTag);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(CharacterMappingContext);
		Subsystem->AddMappingContext(MenuMappingContext, 0);
	}

	/*
	 * We don't use UI Only because it doesn't seem to work with
	 * the newer Enhanced Input system.
	 * WBP_WBP_
	 * This is likely because `Common UI Input is supposed to handle
	 * UI input and Enhanced Input for game input. But my question is:
	 * WHY DOES UNREAL HAVE THREE DIFFERENT INPUT SYSTEMS!!!!
	 */
	SetInputMode(FInputModeGameAndUI());
	SetShowMouseCursor(true);

	UE_LOG(LogLactose, Log, TEXT("Menu Opened: '%s'"), *OpenedMenuTag->ToString());
}

void ALactoseGamePlayerController::CloseActiveMenu()
{
	if (!OpenedMenuTag)
		return;
	
	const FGameplayTag CopiedTag = *OpenedMenuTag;
	OpenedMenuTag.Reset();

	OnMenuClosed.Broadcast(this, CopiedTag);

	if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->RemoveMappingContext(MenuMappingContext);
		InputSubsystem->AddMappingContext(CharacterMappingContext, 0);
	}
	
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);

	UE_LOG(LogLactose, Log, TEXT("Menu Closed: '%s'"), *CopiedTag.ToString());
}

bool ALactoseGamePlayerController::GetTreeCropIdToPlant(FString& CropId) const
{
	if (const TOptional<FString>& FoundCropId = GetTreeCropIdToPlant())
	{
		CropId = *FoundCropId;
		return true;
	}

	return false;
}

void ALactoseGamePlayerController::SetTreeCropIdToPlant(const FString& CropId)
{
	TreeCropIdToPlant = CropId;

	UE_LOG(LogLactose, Log, TEXT("Player has selected Tree Crop '%s' to plant"), *CropId);
}

void ALactoseGamePlayerController::ResetTreeCropIdToPlant()
{
	TreeCropIdToPlant.Reset();

	UE_LOG(LogLactose, Log, TEXT("Player has unselected Tree to plant"));
}

void ALactoseGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		InputSubsystem->AddMappingContext(CharacterMappingContext, 0);
}
