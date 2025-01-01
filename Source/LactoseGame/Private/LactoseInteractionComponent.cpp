// Fill out your copyright notice in the Description page of Project Settings.


#include "LactoseInteractionComponent.h"
#include "InputAction.h"
#include "LactoseGame/LactoseGame.h"

ULactoseInteractionComponent::ULactoseInteractionComponent()
{
	static ConstructorHelpers::FObjectFinder<UInputAction> DefaultInputAction(TEXT("/Game/FirstPerson/Input/Actions/IA_Interact.IA_Interact"));
	InputAction = DefaultInputAction.Object;
}

bool ULactoseInteractionComponent::CanBeInteracted() const
{
	const UWorld* World = GetWorld();
	bool bCanInteract =
		IsValid(InputAction)
		&& World
		&& !World->GetTimerManager().TimerExists(InteractionCooldownTimer)
		&& (!CanInteractFunc.IsSet() || Invoke(*CanInteractFunc));
	
	return bCanInteract;
}

FString ULactoseInteractionComponent::GetInteractionText() const
{
	if (InteractionTextFunc.IsSet())
		return Invoke(*InteractionTextFunc);

	return InteractionText;
}

void ULactoseInteractionComponent::Interact(AController* Instigator)
{
	if (!GetOwner())
		return;
	
	OnInteractionComplete.Broadcast(*this, Instigator);

	Log::Verbose(LogLactose,
		TEXT("Actor '%s' Interacted (%s)"),
		*GetOwner()->GetActorNameOrLabel(),
		*GetInteractionText());

	GetWorld()->GetTimerManager().SetTimer(InteractionCooldownTimer, InteractionCooldown, false);
}
