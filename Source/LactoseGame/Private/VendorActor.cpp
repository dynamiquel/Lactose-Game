// Fill out your copyright notice in the Description page of Project Settings.


#include "VendorActor.h"

#include <Components/BoxComponent.h>
#include <Components/TextRenderComponent.h>

#include "LactoseInteractionComponent.h"
#include "LactoseMenuTags.h"
#include "LactoseGame/LactoseGamePlayerController.h"


AVendorActor::AVendorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	InteractionCollision = CreateDefaultSubobject<UBoxComponent>("InteractionCollision");
	InteractionCollision->SetupAttachment(GetRootComponent());
	if (auto* BoxCollision = Cast<UBoxComponent>(InteractionCollision))
		BoxCollision->SetBoxExtent(FVector(50., 50., 100.));

	Interaction = CreateDefaultSubobject<ULactoseInteractionComponent>(TEXT("Interaction"));
	Interaction->OnInteractionComplete.AddUObject(this, &ThisClass::OnInteracted);
	Interaction->InteractionText = VendorName;

	BillboardComponent = CreateDefaultSubobject<USceneComponent>("Billboard");
	BillboardComponent->SetupAttachment(GetRootComponent());
	VendorIdTextComponent = CreateDefaultSubobject<UTextRenderComponent>("VendorIdText");
	VendorIdTextComponent->SetupAttachment(BillboardComponent);
}

void AVendorActor::BeginPlay()
{
	Super::BeginPlay();

	VendorIdTextComponent->SetText(FText::FromString(VendorId));
}

void AVendorActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (WasRecentlyRendered())
		UpdateBillboard();
}

void AVendorActor::OnInteracted(const ULactoseInteractionComponent& InteractionComponent, AController* Instigator)
{
	if (VendorId.IsEmpty())
		return;
	
	if (auto* LactosePC = Cast<ALactoseGamePlayerController>(Instigator))
	{
		LactosePC->SetUserShopIdToBrowse(VendorId);
		LactosePC->OpenMenu(Lactose::Menus::UserShop);
	}
}

void AVendorActor::UpdateBillboard()
{
	if (!BillboardComponent)
		return;

	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
		return;

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(OUT CameraLocation, OUT CameraRotation);
	const FRotator NewBillboardRotation = (CameraLocation - BillboardComponent->GetComponentLocation()).Rotation();
	BillboardComponent->SetWorldRotation(NewBillboardRotation);
}
