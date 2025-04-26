// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CatalystExampleClient.h"
#include "GameFramework/Actor.h"
#include "CatalystExampleActor.generated.h"

UCLASS()
class CATALYST_API ACatalystExampleActor : public AActor
{
	GENERATED_BODY()

public:
	ACatalystExampleActor();

protected:
	void BeginPlay() override;
	void Tick(float DeltaTime) override;

	void OnRolesReceived(TSharedRef<Catalyst::Example::FGetRoles> Operation);

protected:
	UPROPERTY(Transient)
	TObjectPtr<class UCatalystExampleClient> Client;
};
