// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "LactoseGamePlayerController.generated.h"

struct FGameplayTag;

class UInputAction;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLactoseMenuDelegate,
	const APlayerController*, PlayerController,
	const FGameplayTag&, MenuTag);

/**
 *
 */
UCLASS()
class LACTOSEGAME_API ALactoseGamePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	ALactoseGamePlayerController();
	
public:
	UFUNCTION(BlueprintPure, Category="Lactose Player")
	bool IsMenuOpened(const FGameplayTag& MenuTag, bool bExactMenu = false) const;

	UFUNCTION(BlueprintPure, Category="Lactose Player")
	bool IsAnyMenuOpened() const;
	
	UFUNCTION(BlueprintCallable, Category="Lactose Player")
	void OpenMenu(const FGameplayTag& MenuTag);

	UFUNCTION(BlueprintCallable, Category="Lactose Player")
	void CloseActiveMenu();

	const TArray<FString>& GetCropInstanceIdsToSeed() const { return CropInstanceIdsToSeed; }

	UFUNCTION(BlueprintPure, Category="Lactose")
	TArray<FString>& GetCropInstanceIdsToSeed() { return CropInstanceIdsToSeed; }

	const TOptional<FString>& GetTreeCropIdToPlant() const { return TreeCropIdToPlant; }
	
	UFUNCTION(BlueprintPure, Category="Lactose")
	bool GetTreeCropIdToPlant(UPARAM() FString& CropId) const;

	UFUNCTION(BlueprintCallable, Category="Lactose")
	void SetTreeCropIdToPlant(const FString& CropId);

	UFUNCTION(BlueprintCallable, Category="Lactose")
	void ResetTreeCropIdToPlant();

	UFUNCTION(BlueprintPure, Category="Lactose")
	bool GetUserShopIdToBrowse(UPARAM() FString& CropId) const;
	
	UFUNCTION(BlueprintCallable, Category="Lactose")
	void SetUserShopIdToBrowse(const FString& UserId);


protected:
	void BeginPlay() override;
	void SetupInputComponent() override;

	void OnPlayerMenuActionPressed();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> CharacterMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> MenuMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> PlayerMenuAction;

	TOptional<FGameplayTag> OpenedMenuTag;
	
	TArray<FString> CropInstanceIdsToSeed;
	TOptional<FString> TreeCropIdToPlant;
	TOptional<FString> UserShopIdToBrowse;

public:
	UPROPERTY(BlueprintAssignable)
	FLactoseMenuDelegate OnMenuOpened;

	UPROPERTY(BlueprintAssignable)
	FLactoseMenuDelegate OnMenuClosed;
};
