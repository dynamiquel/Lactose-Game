#include "CropActor.h"

#include "LactoseCropWorldSubsystem.h"
#include "LactoseInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"
#include "InputAction.h"
#include "LactoseMenuTags.h"
#include "LactoseGame/LactoseGamePlayerController.h"

ACropActor::ACropActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	InteractionCollision = CreateDefaultSubobject<UBoxComponent>("InteractionCollision");
	InteractionCollision->SetupAttachment(GetRootComponent());
	Interaction = CreateDefaultSubobject<ULactoseInteractionComponent>(TEXT("Interaction"));
	Interaction->OnInteractionComplete.AddUObject(this, &ThisClass::OnInteracted);
	Interaction->InteractionTextFunc = [WeakThis = MakeWeakObjectPtr(this)]
	{
		auto* This = WeakThis.Get();
		if (!This || !This->CropInstance || !This->Crop)
			return FString();

		const FString CropName = This->Crop->Name;

		if (This->CropInstance->State == Lactose::Simulation::States::Empty)
			return FString::Printf(TEXT("Seed your %s"), *CropName);
		if (This->CropInstance->State == Lactose::Simulation::States::Harvestable)
			return FString::Printf(TEXT("Harvest your %s"), *CropName);
		if (This->CropInstance->State == Lactose::Simulation::States::Growing)
			return FString::Printf(TEXT("Fertilise your %s"), *CropName);
		
		return FString();
	};
	Interaction->CanInteractFunc = [WeakThis = MakeWeakObjectPtr(this)]
	{
		auto* This = WeakThis.Get();
		if (!This || !This->CropInstance || !This->Crop)
			return false;

		auto* Economy = This->GetWorld()->GetGameInstance()->GetSubsystem<ULactoseEconomyServiceSubsystem>();
		
		if (This->CropInstance->State == Lactose::Simulation::States::Empty)
		{
			for (const auto& CostItem : This->Crop->CostItems)
			{
				Sp<const FLactoseEconomyUserItem> FoundUserItem = Economy->FindCurrentUserItem(CostItem.ItemId);
				if (!FoundUserItem || FoundUserItem->Quantity < CostItem.Quantity)
					return false;
			}

			return true;
		}
		if (This->CropInstance->State == Lactose::Simulation::States::Harvestable)
		{
			return true;
		}
		if (This->CropInstance->State == Lactose::Simulation::States::Growing)
		{
			const FString& FertiliserItemId = This->Crop->FertiliserItemId;
			Sp<const FLactoseEconomyUserItem> FoundUserItem = Economy->FindCurrentUserItem(FertiliserItemId);
			return FoundUserItem && FoundUserItem->Quantity > 0;
		}

		return false;
	};

	DestroyInteraction = CreateDefaultSubobject<ULactoseInteractionComponent>(TEXT("DestroyInteraction"));
	DestroyInteraction->InteractionText = TEXT("Destroy");
	DestroyInteraction->OnInteractionComplete.AddUObject(this, &ThisClass::OnDestroyInteracted);

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractSecondaryInputAction(TEXT("/Game/FirstPerson/Input/Actions/IA_InteractSecondary.IA_InteractSecondary"));
	DestroyInteraction->InputAction = InteractSecondaryInputAction.Object;

	if (auto* InteractionCollisionBox = Cast<UBoxComponent>(InteractionCollision))
		InteractionCollisionBox->InitBoxExtent(FVector(75., 75., 75.));

	GroundMesh = CreateDefaultSubobject<UStaticMeshComponent>("GroundMesh");
	GroundMesh->SetupAttachment(GetRootComponent());
	GroundMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PlantMesh = CreateDefaultSubobject<UStaticMeshComponent>("PlantMesh");
	PlantMesh->SetupAttachment(GetRootComponent());
	PlantMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BillboardComponent = CreateDefaultSubobject<USceneComponent>("Billboard");
	BillboardComponent->SetupAttachment(GetRootComponent());
	CropNameTextComponent = CreateDefaultSubobject<UTextRenderComponent>("CropNameText");
	CropNameTextComponent->SetupAttachment(BillboardComponent);
	CropNameTextComponent->SetRelativeLocation(FVector(0., 0., 100.));
	CropNameTextComponent->HorizontalAlignment = EHTA_Center;
	CropStateTextComponent = CreateDefaultSubobject<UTextRenderComponent>("CropStateText");
	CropStateTextComponent->SetupAttachment(BillboardComponent);
	CropStateTextComponent->SetRelativeLocation(FVector(0., 0., 70.));
	CropStateTextComponent->HorizontalAlignment = EHTA_Center;
	CropStateTextComponent->SetWorldSize(20.f);
	CropHarvestTimeTextComponent = CreateDefaultSubobject<UTextRenderComponent>("CropHarvestTimeText");
	CropHarvestTimeTextComponent->SetupAttachment(BillboardComponent);
	CropHarvestTimeTextComponent->SetRelativeLocation(FVector(0., 0., 40.));
	CropHarvestTimeTextComponent->HorizontalAlignment = EHTA_Center;
	CropHarvestTimeTextComponent->SetWorldSize(20.f);
	CropFertiliseTimeTextComponent = CreateDefaultSubobject<UTextRenderComponent>("CropFertiliseTimeText");
	CropFertiliseTimeTextComponent->SetupAttachment(BillboardComponent);
	CropFertiliseTimeTextComponent->SetRelativeLocation(FVector(0., 0., 10.));
	CropFertiliseTimeTextComponent->HorizontalAlignment = EHTA_Center;
	CropFertiliseTimeTextComponent->SetWorldSize(20.f);
}

void ACropActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		auto* CropSubsystem = GetWorld()->GetSubsystem<ULactoseCropWorldSubsystem>();
		if (!CropSubsystem)
			return;

		CropSubsystem->RegisterCropActor(*this);
		UpdateBillboardText();
	}
}

void ACropActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACropActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (auto* CropSubsystem = GetWorld()->GetSubsystem<ULactoseCropWorldSubsystem>())
		CropSubsystem->DeregisterCropActor(*this);
}

void ACropActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (WasRecentlyRendered())
		UpdateBillboard();
}

void ACropActor::Init(
	const Sr<const FLactoseSimulationCrop>& InCrop,
	const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	Crop = InCrop;
	CropInstance = InCropInstance;
	
	CropInstance->OnLoaded.AddUObject(this, &ThisClass::OnLoaded);
	CropInstance->OnHarvested.AddUObject(this, &ThisClass::OnHarvested);
	CropInstance->OnFertilised.AddUObject(this, &ThisClass::OnFertilised);
	CropInstance->OnSeeded.AddUObject(this, &ThisClass::OnSeeded);
	CropInstance->OnDestroyed.AddUObject(this, &ThisClass::OnDestroyed);
}

void ACropActor::OnLoaded(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	UpdateBillboardText();
}

void ACropActor::OnHarvested(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	if (HarvestSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HarvestSound, GetActorLocation());
}

void ACropActor::OnFertilised(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	if (FertiliseSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FertiliseSound, GetActorLocation());
}

void ACropActor::OnSeeded(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	if (SeedSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SeedSound, GetActorLocation());
}

void ACropActor::OnDestroyed(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	if (DestroySound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestroySound, GetActorLocation());

	UpdateBillboardText();

	FTimerHandle DestroyTimer;
	GetWorldTimerManager().SetTimer(DestroyTimer, [WeakThis = MakeWeakObjectPtr(this)]()
		{
			if (auto* This = WeakThis.Get())
				This->Destroy();
		},
		DestroySeconds,
		/* bLoop */ false);
}

void ACropActor::OnInteracted(const ULactoseInteractionComponent& InteractionComponent, AController* Instigator)
{
	auto* SimulationSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseSimulationServiceSubsystem>();
	if (!ensure(SimulationSubsystem))
	{
		return;
	}
	
	if (!Crop || !CropInstance)
		return;
	
	if (CropInstance->State == Lactose::Simulation::States::Harvestable)
		SimulationSubsystem->HarvestCropInstances({ CropInstance->Id });
	else if (CropInstance->State == Lactose::Simulation::States::Growing)
		SimulationSubsystem->FertiliseCropInstances({ CropInstance->Id });
	else if (CropInstance->State == Lactose::Simulation::States::Empty)
	{
		if (auto* LactosePC = Cast<ALactoseGamePlayerController>(Instigator))
		{
			LactosePC->GetCropInstanceIdsToSeed().Reset();
			LactosePC->GetCropInstanceIdsToSeed().Add(CropInstance->Id);
			LactosePC->OpenMenu(Lactose::Menus::SeedCrop);
		}
	}
}

void ACropActor::OnDestroyInteracted(const ULactoseInteractionComponent& InteractionComponent, AController* Instigator)
{
	auto* SimulationSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseSimulationServiceSubsystem>();
	if (!ensure(SimulationSubsystem))
	{
		return;
	}

	if (!Crop || !CropInstance)
		return;

	SimulationSubsystem->DestroyCropInstances({ CropInstance->Id} );
}

void ACropActor::UpdateBillboard()
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

void ACropActor::UpdateBillboardText()
{
	if (!CropInstance)
		return;

	if (!Crop.IsValid())
		CropNameTextComponent->SetText(FText::FromString(CropInstance->CropId));
	else if (CropInstance->State == Lactose::Simulation::States::Empty)
		CropNameTextComponent->SetText(FText::GetEmpty());
	else
		CropNameTextComponent->SetText(FText::FromString(Crop->Name));
	
	CropStateTextComponent->SetText(FText::FromString(CropInstance->State));

	CropHarvestTimeTextComponent->SetText(CropInstance->RemainingHarvestSeconds > 0 && CropInstance->State != Lactose::Simulation::States::Empty
		? FText::AsTimespan(FTimespan::FromSeconds(CropInstance->RemainingHarvestSeconds))
		: FText());

	CropFertiliseTimeTextComponent->SetText(CropInstance->RemainingFertiliserSeconds > 0
		? FText::AsTimespan(FTimespan::FromSeconds(CropInstance->RemainingFertiliserSeconds))
		: FText());
}

