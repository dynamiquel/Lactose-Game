#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "IDebugOverlayDrawable.h"

#include "DebugAppTab.generated.h"

class UDebugApp;

/**
 * 
 */
UCLASS(Abstract, Config=Debug, DefaultConfig)
class DEBUGOVERLAY_API UDebugAppTab : public UObject, public IDebugOverlayDrawable
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateTab(const UDebugApp& App) const { return bEnabled; }

	const FString& GetTabName() const { return TabName; }
	TSubclassOf<UDebugApp> GetOwningAppClass() const { return OwningAppClass; }
	
	void SetTabName(FString&& InTabName);

	template<typename TApp> requires (TIsDerivedFrom<TApp, UDebugApp>::IsDerived)
	void SetOwningAppClass()
	{
		OwningAppClass = TApp::StaticClass();
	}
	
protected:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bEnabled = true;

private:
	UPROPERTY(EditDefaultsOnly, Config)
	FString TabName;

	UPROPERTY(EditDefaultsOnly, Config)
	TSubclassOf<UDebugApp> OwningAppClass;
};
