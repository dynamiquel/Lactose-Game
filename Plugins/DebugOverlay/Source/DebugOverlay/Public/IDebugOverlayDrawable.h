#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "IDebugOverlayDrawable.generated.h"

UINTERFACE()
class UDebugOverlayDrawable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DEBUGOVERLAY_API IDebugOverlayDrawable
{
	GENERATED_BODY()

public:
	virtual void Init() {}
	virtual void Open() {}
	virtual void Close() {}
	virtual void Render() {}
};
