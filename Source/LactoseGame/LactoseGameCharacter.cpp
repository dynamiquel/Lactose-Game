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
#include "LactoseInteractionComponent.h"
#include "Engine/LocalPlayer.h"

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
}

void ALactoseGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

void ALactoseGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateClosestInteraction();
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
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::Interact);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
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

void ALactoseGameCharacter::Interact(const FInputActionValue& Value)
{
	if (!IsValid(ClosestInteraction))
		return;

	ClosestInteraction->Interact();
}

void ALactoseGameCharacter::UpdateClosestInteraction()
{
	if (OverlappedInteractions.IsEmpty())
	{
		SetClosestInteraction(nullptr);
		return;
	}

	float ClosestDistanceSq = TNumericLimits<float>::Max();
	ULactoseInteractionComponent* NewClosestInteraction = nullptr;

	for (ULactoseInteractionComponent* Interaction : OverlappedInteractions)
	{
		if (!IsValid(Interaction) || !Interaction->CanBeInteracted())
			continue;
		
		const FVector InteractionActorLocation = Interaction->GetOwner()->GetActorLocation();
		const float Distance = FVector::DistSquared(InteractionActorLocation, GetActorLocation());
		if (Distance < ClosestDistanceSq)
		{
			ClosestDistanceSq = Distance;
			NewClosestInteraction = Interaction;
		}
	}

	SetClosestInteraction(NewClosestInteraction);
}

void ALactoseGameCharacter::SetClosestInteraction(ULactoseInteractionComponent* InteractionComponent)
{
	if (ClosestInteraction == InteractionComponent)
		return;
	
	ClosestInteraction = InteractionComponent;

	if (ClosestInteraction)
	{
		UE_LOG(LogLactose, Verbose, TEXT("Player's closest Interaction is for Actor '%s' (%s)"),
			*ClosestInteraction->GetOwner()->GetActorNameOrLabel(),
			*ClosestInteraction->GetInteractionText());
	}
	else
	{
		UE_LOG(LogLactose, Verbose, TEXT("Player is no longer interacting with anything"));
	}
}
