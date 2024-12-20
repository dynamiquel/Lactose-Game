// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/Object.h>
#include "LactoseNativeDelegateWrapper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLactoseSimpleMulticastDelegate,
	const ULactoseNativeDelegateWrapper*, Sender);

/**
 * Simple object that wraps over a native delegate and exposes it to BP.
 */
UCLASS(BlueprintType)
class LACTOSEGAME_API ULactoseNativeDelegateWrapper : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Start();
	
	UFUNCTION(BlueprintCallable)
	void Stop();

	UFUNCTION(BlueprintCallable)
	void Reset();

	UFUNCTION(BlueprintPure)
	bool IsAssigned() const;

protected:
	void BeginDestroy() override;

	virtual void OnSubscribed() {}
	virtual void OnUnsubscribed() {}
	
	void OnExecuted();
	
public:
	UPROPERTY(BlueprintAssignable)
	FLactoseSimpleMulticastDelegate Delegate;

protected:
	FDelegateHandle NativeDelegateHandle;
};
