// Copyright Epic Games, Inc. All Rights Reserved.

#include "LactoseGameCharacter.h"

#include "CropActor.h"
#include "LactoseGameProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "LactoseGame.h"
#include "LactoseGamePlayerController.h"
#include "LactoseInteractionComponent.h"
#include "LactoseMenuTags.h"
#include "Landscape.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "LandscapeProxy.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ALactoseGameCharacter

ALactoseGameCharacter::ALactoseGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	static ConstructorHelpers::FObjectFinder<UInputAction> DefaultInteractInputAction(TEXT("/Game/FirstPerson/Input/Actions/IA_Interact.IA_Interact"));
	static ConstructorHelpers::FObjectFinder<UInputAction> DefaultInteractSecondaryInputAction(TEXT("/Game/FirstPerson/Input/Actions/IA_InteractSecondary.IA_InteractSecondary"));
	static ConstructorHelpers::FObjectFinder<UInputAction> DefaultNoItemInputAction(TEXT("/Game/FirstPerson/Input/Actions/IA_NoItem.IA_NoItem"));
	static ConstructorHelpers::FObjectFinder<UInputAction> DefaultPlotToolInputAction(TEXT("/Game/FirstPerson/Input/Actions/IA_PlotTool.IA_PlotTool"));
	static ConstructorHelpers::FObjectFinder<UInputAction> DefaultTreeToolInputAction(TEXT("/Game/FirstPerson/Input/Actions/IA_TreeTool.IA_TreeTool"));
	static ConstructorHelpers::FObjectFinder<UInputAction> DefaultUseItemInputAction(TEXT("/Game/FirstPerson/Input/Actions/IA_Shoot.IA_Shoot"));
	static ConstructorHelpers::FObjectFinder<UInputAction> DefaultChangeCropInputAction(TEXT("/Game/FirstPerson/Input/Actions/IA_ChangeCrop.IA_ChangeCrop"));

	InteractAction = DefaultInteractInputAction.Object;
	InteractSecondaryAction = DefaultInteractSecondaryInputAction.Object;
	NoneItemAction = DefaultNoItemInputAction.Object;
	PlotToolItemAction = DefaultPlotToolInputAction.Object;
	TreeToolItemAction = DefaultTreeToolInputAction.Object;
	UseItemAction = DefaultUseItemInputAction.Object;
	ChangeCropAction = DefaultChangeCropInputAction.Object;
}

void ALactoseGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

void ALactoseGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateClosestInteractions();

	if (GetCurrentItemState() == ELactoseCharacterItemState::PlotTool || GetCurrentItemState() == ELactoseCharacterItemState::TreeTool)
	{
		if (GetCurrentItemState() == ELactoseCharacterItemState::TreeTool)
		{
			auto* PC = Cast<ALactoseGamePlayerController>(GetController());
			if (!ensure(PC))
			{
				return;
			}

			if (!PC->GetTreeCropIdToPlant().IsSet())
				return;
		}
		
		TOptional<FHitResult> HitResult = PerformPlotToolTrace();
		if (!HitResult)
			return;

		DrawDebugPoint(
			GetWorld(),
			HitResult->Location,
			20.f,
			FColor::Blue);
	}
}

void ALactoseGameCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!OtherActor)
		return;

	TInlineComponentArray<ULactoseInteractionComponent*> FoundInteractionComps;
	OtherActor->GetComponents<ULactoseInteractionComponent>(OUT FoundInteractionComps);
	
	for (ULactoseInteractionComponent* FoundInteractionComp : FoundInteractionComps)
	{
		OverlappedInteractions.AddUnique(FoundInteractionComp);
		
		UE_LOG(LogLactose, Verbose, TEXT("Player is overlapping with Interaction for Actor '%s'"),
			*FoundInteractionComp->GetOwner()->GetActorNameOrLabel());
	}
}

void ALactoseGameCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	if (OtherActor)
	{
		TInlineComponentArray<ULactoseInteractionComponent*> FoundInteractionComps;
		OtherActor->GetComponents<ULactoseInteractionComponent>(OUT FoundInteractionComps);
	
		for (ULactoseInteractionComponent* FoundInteractionComp : FoundInteractionComps)
		{
			OverlappedInteractions.Remove(FoundInteractionComp);

			UE_LOG(LogLactose, Verbose, TEXT("Player is no longer overlapping with Interaction for Actor '%s'"),
				*FoundInteractionComp->GetOwner()->GetActorNameOrLabel());
		}
	}
	
	Super::NotifyActorEndOverlap(OtherActor);
}

//////////////////////////////////////////////////////////////////////////// Input

void ALactoseGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALactoseGameCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALactoseGameCharacter::Look);

		// Interaction
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::InteractPrimary);
		EnhancedInputComponent->BindAction(InteractSecondaryAction, ETriggerEvent::Triggered, this, &ThisClass::InteractSecondary);

		// Holdable Items
		EnhancedInputComponent->BindAction(NoneItemAction, ETriggerEvent::Completed, this, &ThisClass::RequestSwitchToNoneItem);
		EnhancedInputComponent->BindAction(PlotToolItemAction, ETriggerEvent::Completed, this, &ThisClass::RequestSwitchToPlotToolItem);
		EnhancedInputComponent->BindAction(TreeToolItemAction, ETriggerEvent::Completed, this, &ThisClass::RequestSwitchToTreeToolItem);
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Completed, this, &ThisClass::RequestUseItem);
		EnhancedInputComponent->BindAction(ChangeCropAction, ETriggerEvent::Completed, this, &ThisClass::RequestChangeCrop);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


ULactoseInteractionComponent* ALactoseGameCharacter::FindInteractionForAction(const UInputAction& InputAction) const
{
	return ClosestInteractions.FindRef(&InputAction);
}

void ALactoseGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ALactoseGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ALactoseGameCharacter::InteractPrimary()
{
	if (!IsValid(InteractAction))
		return;
	
	auto* FoundPrimaryInteraction = FindInteractionForAction(*InteractAction);
	if (!FoundPrimaryInteraction)
		return;

	FoundPrimaryInteraction->Interact(GetController());
}

void ALactoseGameCharacter::InteractSecondary()
{
	if (!IsValid(InteractSecondaryAction))
		return;
	
	auto* FoundSecondaryInteraction = FindInteractionForAction(*InteractSecondaryAction);
	if (!FoundSecondaryInteraction)
		return;

	FoundSecondaryInteraction->Interact(GetController());
}

void ALactoseGameCharacter::UpdateClosestInteractions()
{
	if (OverlappedInteractions.IsEmpty())
	{
		ResetAllInteractions();
		return;
	}
	
	typedef TTuple<ULactoseInteractionComponent*, float> FClosestInteractionComponent;
	TMap<const UInputAction*, FClosestInteractionComponent> NewClosestInteractionComponents;
	
	for (ULactoseInteractionComponent* Interaction : OverlappedInteractions)
	{
		if (!IsValid(Interaction) || !Interaction->CanBeInteracted())
			continue;
		
		const FVector InteractionActorLocation = Interaction->GetOwner()->GetActorLocation();
		const float Distance = FVector::DistSquared(InteractionActorLocation, GetActorLocation());

		auto* ExistingClosestInteractionComp = NewClosestInteractionComponents.Find(Interaction->InputAction);
		if (!ExistingClosestInteractionComp)
		{
			NewClosestInteractionComponents.Emplace(
				Interaction->InputAction,
				FClosestInteractionComponent(Interaction, Distance));
		}
		else if (Distance < ExistingClosestInteractionComp->Value)
		{
			*ExistingClosestInteractionComp = FClosestInteractionComponent(Interaction, Distance);
		}
	}

	// Reset Actions that are no longer being referenced.
	for (auto& ClosestInteraction : ClosestInteractions)
	{
		const UInputAction* InputAction = ClosestInteraction.Key;
		if (!NewClosestInteractionComponents.Contains(InputAction))
			SetClosestInteraction(*InputAction, nullptr);
	}

	for (auto& NewClosestInteractionComponent : NewClosestInteractionComponents)
	{
		const UInputAction* InputAction = NewClosestInteractionComponent.Key;
		ULactoseInteractionComponent* InteractionComponent = NewClosestInteractionComponent.Value.Key;
		SetClosestInteraction(*InputAction, InteractionComponent);
	}
}

void ALactoseGameCharacter::ResetAllInteractions()
{
	for (TTuple<TObjectPtr<const UInputAction>, TObjectPtr<ULactoseInteractionComponent>>& Interaction : ClosestInteractions)
		if (Interaction.Key)
			SetClosestInteraction(*Interaction.Key, nullptr);

	ClosestInteractions.Reset();
}

void ALactoseGameCharacter::SetClosestInteraction(const UInputAction& InputAction, ULactoseInteractionComponent* InteractionComponent)
{
	TObjectPtr<ULactoseInteractionComponent>& ExistingClosestInteraction = ClosestInteractions.FindOrAdd(&InputAction);
	if (ExistingClosestInteraction == InteractionComponent)
		return;

	ExistingClosestInteraction = InteractionComponent;
	
	if (InteractionComponent)
	{
		UE_LOG(LogLactose, Verbose, TEXT("Player's closest Interaction for '%s' is for Actor '%s' (%s)"),
			*InputAction.GetName(),
			*InteractionComponent->GetOwner()->GetActorNameOrLabel(),
			*InteractionComponent->GetInteractionText());
	}
	else
	{
		UE_LOG(LogLactose, Verbose, TEXT("Player is no longer interacting with anything for '%s'"),
			*InputAction.GetName());
	}
}

void ALactoseGameCharacter::RequestSwitchToNoneItem()
{
	SetHoldableItemState(ELactoseCharacterItemState::None);
}

void ALactoseGameCharacter::RequestSwitchToPlotToolItem()
{
	SetHoldableItemState(ELactoseCharacterItemState::PlotTool);
}

void ALactoseGameCharacter::RequestSwitchToTreeToolItem()
{
	SetHoldableItemState(ELactoseCharacterItemState::TreeTool);
}

void ALactoseGameCharacter::RequestUseItem()
{
	switch (GetCurrentItemState())
	{
		case ELactoseCharacterItemState::None:
			break;
		case ELactoseCharacterItemState::PlotTool:
			TryUsePlotTool();
			break;
		case ELactoseCharacterItemState::TreeTool:
			TryUseTreeTool();
	}
}

void ALactoseGameCharacter::RequestChangeCrop()
{
	if (GetCurrentItemState() == ELactoseCharacterItemState::TreeTool)
	{
		auto* PC = Cast<ALactoseGamePlayerController>(GetController());
		if (!ensure(PC))
			return;

		PC->OpenMenu(Lactose::Menus::PlantCrop);
	}
}

void ALactoseGameCharacter::TryUsePlotTool()
{
	TOptional<FHitResult> HitResult = PerformPlotToolTrace();
	
	if (!HitResult)
		return;

	AActor* HitActor = HitResult->GetActor();

	// If anything but the ground was hit, don't plant.
	if (!(HitActor && (HitActor->IsA(ALandscapeProxy::StaticClass()) || HitActor->IsA(ALandscape::StaticClass()))))
		return;

	// TODO: Perform box trace to ensure the plot isn't blocking anything.
	// TODO: If near an existing crop, lock on to it.

	auto* SimulationSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseSimulationServiceSubsystem>();
	if (!ensure(SimulationSubsystem))
	{
		return;
	}

	const FRotator SpawnRotation = FRotationMatrix::MakeFromZ(HitResult->Normal).Rotator();
	SimulationSubsystem->CreateEmptyPlot(HitResult->Location, SpawnRotation);
}

void ALactoseGameCharacter::TryUseTreeTool()
{
	auto* PC = Cast<ALactoseGamePlayerController>(GetController());
	if (!ensure(PC))
		return;

	const TOptional<FString>& TreeCropToPlant = PC->GetTreeCropIdToPlant();
	if (!TreeCropToPlant.IsSet())
		return;

	auto& Simulation = Lactose::GetService<ULactoseSimulationServiceSubsystem>(*this);
	if (!Simulation.CanCurrentUserAffordCrop(*TreeCropToPlant))
	{
		PC->ResetTreeCropIdToPlant();
		return;
	}
	
	TOptional<FHitResult> HitResult = PerformPlotToolTrace();
	
	if (!HitResult)
		return;

	AActor* HitActor = HitResult->GetActor();

	// If anything but the ground was hit, don't plant.
	if (!(HitActor && (HitActor->IsA(ALandscapeProxy::StaticClass()) || HitActor->IsA(ALandscape::StaticClass()))))
		return;

	// TODO: Perform box trace to ensure the plot isn't blocking anything.
	// TODO: If near an existing crop, lock on to it.

	const FRotator SpawnRotation = FRotationMatrix::MakeFromZ(HitResult->Normal).Rotator();
	Simulation.CreateCrop(*TreeCropToPlant, HitResult->Location, SpawnRotation);
}

void ALactoseGameCharacter::SetHoldableItemState(ELactoseCharacterItemState NewState)
{
	if (CurrentItemState == NewState)
		return;

	UE_LOG(LogLactose, Log, TEXT("Character's Item State: %s -> %s"),
		*UEnum::GetValueAsString(CurrentItemState),
		*UEnum::GetValueAsString(NewState));

	CurrentItemState = NewState;
}

TOptional<FHitResult> ALactoseGameCharacter::PerformPlotToolTrace() const
{
	// Fire raycast from player screen centre to determine if can make a new
	// plot at location.

	const APlayerController* PlayerController = GetLocalViewingPlayerController();
	if (!PlayerController)
		return {};
	
	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(OUT CameraLocation, OUT CameraRotation);

	const FVector TraceEnd = CameraLocation + CameraRotation.Vector() * PlotToolMaxDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OUT HitResult,
		CameraLocation,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	return bHit ? HitResult : TOptional<FHitResult>();
}
