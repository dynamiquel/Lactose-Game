// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SimpShorts.h"

#include "LactoseMainMenuPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ALactoseMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

	ALactoseMainMenuPlayerController();
	
public:
	void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category="Lactose")
	void OpenFarm(bool bForceStart = false);
	
	UFUNCTION(BlueprintCallable, Category="Lactose")
	void TryLogin();

	UFUNCTION(BlueprintCallable, Category="Lactose")
	void TryLogout();

	UFUNCTION(BlueprintCallable, Category="Lactose")
	void LoginUsingBasicAuth(const FString& Email, const FString& Password);
	
	UFUNCTION(BlueprintPure, Category="Lactose")
	bool CanStart(UPARAM() TArray<FString>& PendingConditions) const;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MainMenuWidget;

	UPROPERTY(BlueprintAssignable)
	FSimpDynamicMCDelegate OnLoginUsingRefreshTokenFailed;
};
