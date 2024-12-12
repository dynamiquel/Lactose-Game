#include "CropActor.h"

#include "LactoseCropWorldSubsystem.h"
#include "LactoseInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"
#include "InputAction.h"

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
				TSharedPtr<const FLactoseEconomyUserItem> FoundUserItem = Economy->FindCurrentUserItem(CostItem.ItemId);
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
			TSharedPtr<const FLactoseEconomyUserItem> FoundUserItem = Economy->FindCurrentUserItem(FertiliserItemId);
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

	PlantMesh = CreateDefaultSubobject<UStaticMeshComponent>("PlantMesh");
	PlantMesh->SetupAttachment(GetRootComponent());

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
		check(CropSubsystem);

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
	const TSharedRef<const FLactoseSimulationCrop>& InCrop,
	const TSharedRef<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	Crop = InCrop;
	CropInstance = InCropInstance;
	
	CropInstance->OnLoaded.AddUObject(this, &ThisClass::OnLoaded);
	CropInstance->OnHarvested.AddUObject(this, &ThisClass::OnHarvested);
	CropInstance->OnFertilised.AddUObject(this, &ThisClass::OnFertilised);
	CropInstance->OnSeeded.AddUObject(this, &ThisClass::OnSeeded);
	CropInstance->OnDestroyed.AddUObject(this, &ThisClass::OnDestroyed);
}

void ACropActor::OnLoaded(const TSharedRef<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	UpdateBillboardText();
}

void ACropActor::OnHarvested(const TSharedRef<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	if (HarvestSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HarvestSound, GetActorLocation());
}

void ACropActor::OnFertilised(const TSharedRef<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	if (FertiliseSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FertiliseSound, GetActorLocation());
}

void ACropActor::OnSeeded(const TSharedRef<const FLactoseSimulationUserCropInstance>& InCropInstance)
{
	if (SeedSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SeedSound, GetActorLocation());
}

void ACropActor::OnDestroyed(const TSharedRef<const FLactoseSimulationUserCropInstance>& InCropInstance)
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

void ACropActor::OnInteracted(const ULactoseInteractionComponent& InteractionComponent)
{
	auto* SimulationSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseSimulationServiceSubsystem>();
	if (!ensure(SimulationSubsystem))
	{
		return;
	}

	if (!Crop || !CropInstance)
		return;
	
	if (CropInstance->State == Lactose::Simulation::States::Empty)
		SimulationSubsystem->SeedCropInstances({ CropInstance->Id }, Crop->Id);
	else if (CropInstance->State == Lactose::Simulation::States::Harvestable)
		SimulationSubsystem->HarvestCropInstances({ CropInstance->Id });
	else if (CropInstance->State == Lactose::Simulation::States::Growing)
		SimulationSubsystem->FertiliseCropInstances({ CropInstance->Id });
}

void ACropActor::OnDestroyInteracted(const ULactoseInteractionComponent& InteractionComponent)
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
	CropNameTextComponent->SetText(Crop.IsValid()
		? FText::FromString(Crop->Name)
		: INVTEXT("Crop Not Found"));

	CropStateTextComponent->SetText(FText::FromString(CropInstance->State));

	CropHarvestTimeTextComponent->SetText(CropInstance->RemainingHarvestSeconds > 0 && CropInstance->State != TEXT("Empty")
		? FText::AsTimespan(FTimespan::FromSeconds(CropInstance->RemainingHarvestSeconds))
		: FText());

	CropFertiliseTimeTextComponent->SetText(CropInstance->RemainingFertiliserSeconds > 0
		? FText::AsTimespan(FTimespan::FromSeconds(CropInstance->RemainingFertiliserSeconds))
		: FText());
}

