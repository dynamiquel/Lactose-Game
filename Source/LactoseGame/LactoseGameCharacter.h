// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "LactoseGameCharacter.generated.h"

class ACropActor;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

class ULactoseInteractionComponent;

UENUM()
enum class ELactoseCharacterItemState
{
	None,
	PlotTool,
	TreeTool
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLactoseCharacterItemStateChangedDelegate,
	class ALactoseGameCharacter*, Sender,
	ELactoseCharacterItemState, NewItemState,
	ELactoseCharacterItemState, OldItemState);

UCLASS(config=Game)
class ALactoseGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractSecondaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> NoneItemAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PlotToolItemAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TreeToolItemAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ChangeCropAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> UseItemAction;
	
public:
	ALactoseGameCharacter();

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

public:
	ULactoseInteractionComponent* FindInteractionForAction(const UInputAction& InputAction) const;
	ELactoseCharacterItemState GetCurrentItemState() const { return CurrentItemState; }
	FLactoseCharacterItemStateChangedDelegate& GetItemStateChanged() { return ItemStateChanged; }

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void InteractPrimary();
	void InteractSecondary();
	
	void UpdateClosestInteractions();
	void ResetAllInteractions();
	void SetClosestInteraction(const UInputAction& InputAction, ULactoseInteractionComponent* InteractionComponent);

	void RequestSwitchToNoneItem();
	void RequestSwitchToPlotToolItem();
	void RequestSwitchToTreeToolItem();
	void RequestUseItem();
	void RequestChangeCrop();
	void TryUsePlotTool();
	void TryUseTreeTool();

	void SetHoldableItemState(ELactoseCharacterItemState NewState);

	TOptional<FHitResult> PerformPlotToolTrace() const;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


protected:
	UPROPERTY(EditAnywhere, meta=(Units="cm"))
	float PlotToolMaxDistance = 100.f * 5.f; // 5m
	
private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULactoseInteractionComponent>> OverlappedInteractions;

	UPROPERTY(Transient)
	TMap<TObjectPtr<const UInputAction>, TObjectPtr<ULactoseInteractionComponent>> ClosestInteractions;

	ELactoseCharacterItemState CurrentItemState = ELactoseCharacterItemState::None;

	UPROPERTY(BlueprintAssignable, meta=(AllowPrivateAccess="true"))
	FLactoseCharacterItemStateChangedDelegate ItemStateChanged;
};

