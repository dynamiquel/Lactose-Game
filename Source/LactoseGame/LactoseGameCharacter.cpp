// Copyright Epic Games, Inc. All Rights Reserved.

#include "LactoseGameCharacter.h"

#include "CropActor.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "LactoseCropWorldSubsystem.h"
#include "LactoseGame.h"
#include "LactoseGamePlayerController.h"
#include "LactoseInteractionComponent.h"
#include "LactoseMenuTags.h"
#include "Landscape.h"
#include "Engine/LocalPlayer.h"
#include "LandscapeStreamingProxy.h"
#include "Components/SphereComponent.h"
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

	CropCullCollider = CreateDefaultSubobject<USphereComponent>("CropCullCollider");
	CropCullCollider->SetupAttachment(GetCapsuleComponent());
	CropCullCollider->SetSphereRadius(450.f, /* bUpdateOverlaps */ false);
	CropCullCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Using Vehicle channel coz cba messing around making my own.
	CropCullCollider->SetCollisionResponseToChannel(Crops::CropCullChannel, ECR_Overlap);
	CropCullCollider->SetCollisionObjectType(Crops::CropCullChannel);
}

void ALactoseGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	CropCullCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnCropCullColliderOverlap);
	CropCullCollider->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnCropCullColliderOverlapEnd);

	GetCapsuleComponent()->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnCapsuleOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnCapsuleOverlapEnd);
}

void ALactoseGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetCurrentItemState() == ELactoseCharacterItemState::PlotTool || GetCurrentItemState() == ELactoseCharacterItemState::TreeTool)
	{
		ResetAllInteractions();

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
		
		TOptional<TTuple<FHitResult, bool>> HitResult = PerformPlotToolTrace();
		if (!HitResult)
			return;

		constexpr double HalfHeight = 5.f;

		if (HitResult->Value)
		{
			DrawDebugBox(
				GetWorld(),
				HitResult->Key.Location + FVector(0, 0, HalfHeight - .5f),
				FVector(Crops::PlotHalfExtentCm, Crops::PlotHalfExtentCm, HalfHeight),
				FColor::Red,
				false,
				-1,
				0,
				1);
		}
		else
		{
			DrawDebugBox(
				GetWorld(),
				HitResult->Key.Location + FVector(0, 0, HalfHeight - .5f),
				FVector(Crops::PlotHalfExtentCm, Crops::PlotHalfExtentCm, HalfHeight),
				FColor::Blue,
				false,
				-1,
				0,
				1);
		}
	}
	else
	{
		UpdateClosestInteractions();
	}
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

TArray<ULactoseInteractionComponent*> ALactoseGameCharacter::GetClosestInteractions() const
{
	TArray<ULactoseInteractionComponent*> Array;
	Array.Reserve(ClosestInteractions.Num());
	
	for (const TTuple<TObjectPtr<const UInputAction>, TObjectPtr<ULactoseInteractionComponent>>& ClosestInteraction : ClosestInteractions)
		Array.Add(ClosestInteraction.Value);

	return Array;
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
		Log::Verbose(LogLactose,
			TEXT("Player's closest Interaction for '%s' is for Actor '%s' (%s)"),
			*InputAction.GetName(),
			*InteractionComponent->GetOwner()->GetActorNameOrLabel(),
			*InteractionComponent->GetInteractionText());
	}
	else
	{
		Log::Verbose(LogLactose,
			TEXT("Player is no longer interacting with anything for '%s'"),
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
	TOptional<TTuple<FHitResult, bool>> HitResult = PerformPlotToolTrace();
	if (!HitResult || HitResult->Value)
		return;

	auto& SimulationSubsystem = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);

	const FRotator SpawnRotation = FRotationMatrix::MakeFromZ(HitResult->Key.Normal).Rotator();
	SimulationSubsystem.CreateEmptyPlot(HitResult->Key.Location, SpawnRotation);
}

void ALactoseGameCharacter::TryUseTreeTool()
{
	auto* PC = Cast<ALactoseGamePlayerController>(GetController());
	if (!ensure(PC))
		return;

	const TOptional<FString>& TreeCropToPlant = PC->GetTreeCropIdToPlant();
	if (!TreeCropToPlant.IsSet())
		return;

	auto& Simulation = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);
	if (!Simulation.CanCurrentUserAffordCrop(*TreeCropToPlant))
	{
		PC->ResetTreeCropIdToPlant();
		return;
	}
	
	TOptional<TTuple<FHitResult, bool>> HitResult = PerformPlotToolTrace();
	if (!HitResult || HitResult->Value)
		return;

	const FRotator SpawnRotation = FRotationMatrix::MakeFromZ(HitResult->Key.Normal).Rotator();
	Simulation.CreateCrop(*TreeCropToPlant, HitResult->Key.Location, SpawnRotation);
}

void ALactoseGameCharacter::SetHoldableItemState(ELactoseCharacterItemState NewState)
{
	if (CurrentItemState == NewState)
		return;

	const ELactoseCharacterItemState OldState = CurrentItemState;
	CurrentItemState = NewState;

	Log::Log(LogLactose,
		TEXT("Character's Item State: %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(CurrentItemState));

	ItemStateChanged.Broadcast(this, CurrentItemState, OldState);
}

TOptional<TTuple<FHitResult, bool>> ALactoseGameCharacter::PerformPlotToolTrace() const
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

	if (!bHit)
		return {};

	AActor* HitActor = HitResult.GetActor();
	bool bObstructed = !HitActor || !(HitActor->IsA(ALandscape::StaticClass()) || HitActor->IsA(ALandscapeStreamingProxy::StaticClass()));
	if (bObstructed)
	{
		UE_LOG(LogLactose, Log, TEXT("Obstructed by actor '%s' of type '%s'"), *HitActor->GetActorNameOrLabel(), *HitActor->GetClass()->GetName())
		return TTuple<FHitResult, bool>(HitResult, true);
	}

	auto& CropSubsystem = Subsystems::GetRef<ULactoseCropWorldSubsystem>(self);
	HitResult.Location = CropSubsystem.GetMagnetizedPlotLocation(HitResult.Location);
	
	bObstructed = CropSubsystem.IsLocationObstructed(HitResult.Location);
	if (bObstructed)
		return TTuple<FHitResult, bool>(HitResult, true);

	return TTuple<FHitResult, bool>(HitResult, false);
}

void ALactoseGameCharacter::OnCropCullColliderOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (auto* CropActor = Cast<ACropActor>(OtherActor))
	{
		CropActor->TurnOnRendering();
	}
}

void ALactoseGameCharacter::OnCropCullColliderOverlapEnd(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (auto* CropActor = Cast<ACropActor>(OtherActor))
	{
		CropActor->TurnOffRendering();
	}
}

void ALactoseGameCharacter::OnCapsuleOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;
	
	TInlineComponentArray<ULactoseInteractionComponent*> FoundInteractionComps;
	OtherActor->GetComponents<ULactoseInteractionComponent>(OUT FoundInteractionComps);
	
	for (ULactoseInteractionComponent* FoundInteractionComp : FoundInteractionComps)
	{
		OverlappedInteractions.AddUnique(FoundInteractionComp);

		Log::Verbose(LogLactose,
			TEXT("Player is overlapping with Interaction for Actor '%s'"),
			*FoundInteractionComp->GetOwner()->GetActorNameOrLabel());
	}
}

void ALactoseGameCharacter::OnCapsuleOverlapEnd(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		TInlineComponentArray<ULactoseInteractionComponent*> FoundInteractionComps;
		OtherActor->GetComponents<ULactoseInteractionComponent>(OUT FoundInteractionComps);
	
		for (ULactoseInteractionComponent* FoundInteractionComp : FoundInteractionComps)
		{
			OverlappedInteractions.Remove(FoundInteractionComp);

			Log::Verbose(LogLactose,
				TEXT("Player is no longer overlapping with Interaction for Actor '%s'"),
				*FoundInteractionComp->GetOwner()->GetActorNameOrLabel());
		}
	}
}
