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
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CullCollider = CreateDefaultSubobject<UBoxComponent>("CullCollider");
	CullCollider->SetupAttachment(GetRootComponent());
	CullCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	CullCollider->SetCollisionResponseToChannel(Crops::CropCullChannel, ECR_Overlap);
	CullCollider->SetCollisionObjectType(Crops::CropCullChannel);

	if (auto* CullColliderBox = Cast<UBoxComponent>(CullCollider))
		CullColliderBox->InitBoxExtent(FVector(75., 75., 75.));
	
	InteractionCollision = CreateDefaultSubobject<UBoxComponent>("InteractionCollision");
	InteractionCollision->SetupAttachment(GetRootComponent());
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionCollision->SetCollisionResponseToChannel(Crops::CropTraceChannel, ECR_Overlap);

	Interaction = CreateDefaultSubobject<ULactoseInteractionComponent>(TEXT("Interaction"));
	Interaction->OnInteractionComplete.AddUObject(this, &ThisClass::OnInteracted);
	Interaction->InteractionTextFunc = [WeakThis = MakeWeakObjectPtr(this)]
	{
		auto* This = WeakThis.Get();
		if (!This || !This->CropInstance || !This->Crop)
			return FString();

		const FString CropName = This->Crop->Name;

		if (This->CropInstance->State == Lactose::Simulation::States::Empty)
			return FString::Printf(TEXT("Plant a seed"));
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

		auto& Economy = Subsystems::GetRef<ULactoseEconomyServiceSubsystem>(*This);
		
		if (This->CropInstance->State == Lactose::Simulation::States::Empty)
		{
			return true;
		}
		if (This->CropInstance->State == Lactose::Simulation::States::Harvestable)
		{
			return true;
		}
		if (This->CropInstance->State == Lactose::Simulation::States::Growing)
		{
			const FString& FertiliserItemId = This->Crop->FertiliserItemId;
			Sp<const FLactoseEconomyUserItem> FoundUserItem = Economy.FindCurrentUserItem(FertiliserItemId);
			return FoundUserItem && FoundUserItem->Quantity > 0;
		}

		return false;
	};

	DestroyInteraction = CreateDefaultSubobject<ULactoseInteractionComponent>(TEXT("DestroyInteraction"));
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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
		auto* CropSubsystem = Subsystems::Get<ULactoseCropWorldSubsystem>(self);
		if (!CropSubsystem)
			return;

		CropSubsystem->RegisterCropActor(self);
		UpdateBillboardText();

		BillboardComponent->SetHiddenInGame(true, true);
	}
}

void ACropActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (bUsePlantGrowthScale)
		SetPlantScaleBasedOnGrowth();
}

void ACropActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (auto* CropSubsystem = Subsystems::Get<ULactoseCropWorldSubsystem>(self))
		CropSubsystem->DeregisterCropActor(self);
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
	OriginalCropId = InCropInstance->CropId;
	
	CropInstance->OnLoaded.AddUObject(this, &ThisClass::OnLoaded);
	CropInstance->OnHarvested.AddUObject(this, &ThisClass::OnHarvested);
	CropInstance->OnFertilised.AddUObject(this, &ThisClass::OnFertilised);
	CropInstance->OnSeeded.AddUObject(this, &ThisClass::OnSeeded);
	CropInstance->OnDestroyed.AddUObject(this, &ThisClass::OnDestroyed);
}

void ACropActor::OnLoaded(const Sr<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	if (InCropInstance->CropId != OriginalCropId)
	{
		if (auto* Crops = Subsystems::Get<ULactoseCropWorldSubsystem>(self))
		{
			Crops->ResetCropActor(self);
			return;
		}
	}
	
	UpdateBillboardText();

	if (bUsePlantGrowthScale)
		SetPlantScaleBasedOnGrowth();
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

void ACropActor::OnInteracted(const ULactoseInteractionComponent& InteractionComponent, AController* InInstigator)
{
	auto& SimulationSubsystem = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);
	
	if (!Crop || !CropInstance)
		return;
	
	if (CropInstance->State == Lactose::Simulation::States::Harvestable)
		SimulationSubsystem.HarvestCropInstances({ CropInstance->Id });
	else if (CropInstance->State == Lactose::Simulation::States::Growing)
		SimulationSubsystem.FertiliseCropInstances({ CropInstance->Id });
	else if (CropInstance->State == Lactose::Simulation::States::Empty)
	{
		if (auto* LactosePC = Cast<ALactoseGamePlayerController>(InInstigator))
		{
			LactosePC->GetCropInstanceIdsToSeed().Reset();
			LactosePC->GetCropInstanceIdsToSeed().Add(CropInstance->Id);
			LactosePC->OpenMenu(Lactose::Menus::SeedCrop);
		}
	}
}

void ACropActor::OnDestroyInteracted(const ULactoseInteractionComponent& InteractionComponent, AController* InInstigator)
{
	auto& SimulationSubsystem = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);

	if (!Crop || !CropInstance)
		return;

	SimulationSubsystem.DestroyCropInstances({ CropInstance->Id} );
}

void ACropActor::UpdateBillboard()
{
	if (!BillboardComponent)
		return;

	BillboardComponent->SetHiddenInGame(false);

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

float ACropActor::GetCropGrowthProgress() const
{
	if (!GetCropInstance().IsValid())
		return 0.f;
	
	if (GetCropInstance()->State == Lactose::Simulation::States::Harvestable)
		return 1.f;
	
	if (GetCropInstance()->State == Lactose::Simulation::States::Empty)
		return 0.f;

	if (!GetCrop())
		return 0.f;

	if (GetCrop()->HarvestSeconds <= 0.)
		return 0.f;

	return (GetCrop()->HarvestSeconds - GetCropInstance()->RemainingHarvestSeconds) / GetCrop()->HarvestSeconds;
}

void ACropActor::SetPlantScaleBasedOnGrowth_Implementation()
{
}

void ACropActor::TurnOnRendering()
{
	SetActorTickEnabled(true);
	BillboardComponent->SetHiddenInGame(false, true);
}

void ACropActor::TurnOffRendering()
{
	SetActorTickEnabled(false);
	BillboardComponent->SetHiddenInGame(true, true);
}

