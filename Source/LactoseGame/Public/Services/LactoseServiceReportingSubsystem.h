#pragma once

#include "Simp.h"
#include "CoreMinimal.h"
#include "Interfaces/IHttpResponse.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LactoseServiceReportingSubsystem.generated.h"

namespace Lactose::Rest
{
	class IRequest;
}

struct FServiceErrorReport
{
	FName ServiceName;
	std::variant<EHttpResponseCodes::Type> ErrorCode;
	FDateTime ErrorTime;
};

/**
 * Basic system used to report service-related errors.
 * Not the most efficient thing but works.
 */
UCLASS()
class LACTOSEGAME_API ULactoseServiceReportingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void HandleRestRequestFailed(const Sr<Lactose::Rest::IRequest>& Request);

	void Tick();

	static FString GetLactoseServiceNameFromUrl(const FString& Url);
	void ShowDebugWarningText(int32 Key, const FString& Text);
	void ShowDebugErrorText(int32 Key, const FString& Text);
	void RemoveDebugText(int32 Key);

protected:
	static constexpr int32 UnauthorisedWarningCount = 2;
	static constexpr int32 UnauthorisedErrorCount = 4;
	
	static constexpr int32 SameServiceWarningCount = 2;
	static constexpr int32 SameServiceErrorCount = 4;
	
	static constexpr int32 AnyServiceWarningCount = 4;
	static constexpr int32 AnyServiceErrorCount = 8;

	static constexpr float TickRate = 1.f;
	
	FTimespan ErrorExpiryTime = FTimespan::FromSeconds(30);

	FCriticalSection ErrorReportsCs;
	TArray<FServiceErrorReport> ErrorReports;

	TMap<FName, int> CachedServiceErrorCountMap;
};
