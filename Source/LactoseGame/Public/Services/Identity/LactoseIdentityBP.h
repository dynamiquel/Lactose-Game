#pragma once
#include "LactoseGame/LactoseNativeDelegateWrapper.h"

#include "LactoseIdentityBP.generated.h"

class ULactoseIdentityServiceSubsystem;

UCLASS()
class LACTOSEGAME_API ULactoseIdentityBP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};

UCLASS()
class LACTOSEGAME_API ULactoseIdentityLoginFailedDelegateWrapper : public ULactoseNativeDelegateWrapper
{
	GENERATED_BODY()

public:
	void OnSubscribed() override;
	void OnUnsubscribed() override;
	
private:
	void HandleNativeEvent(const ULactoseIdentityServiceSubsystem& Sender);
};