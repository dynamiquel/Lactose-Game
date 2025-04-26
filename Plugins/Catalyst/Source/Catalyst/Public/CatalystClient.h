#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CatalystClient.generated.h"

namespace Timeout
{
	static constexpr float Default = 0.f;
	static constexpr float Never = TNumericLimits<float>::Max();
}

/**
 * Base class for a Catalyst HTTP Client.
 */
UCLASS(Config=Catalyst, DefaultConfig)
class CATALYST_API UCatalystClient : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config)
	FString BaseUrl = "https://localhost";

	UPROPERTY(EditAnywhere, Config, meta=(Units="s"))
	float DefaultTimeout = 15.f;
};
