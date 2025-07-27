#include "Services/LactoseServiceReportingSubsystem.h"

#include "Rest/LactoseRestSubsystem.h"

void ULactoseServiceReportingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Lactose::Rest::OnRequestFailed.AddUObject(this, &ThisClass::HandleRestRequestFailed);

	FTimerHandle TickTimer;
	GetGameInstance()->GetTimerManager().SetTimer(TickTimer, this, &ThisClass::Tick, TickRate, true);
}

void ULactoseServiceReportingSubsystem::HandleRestRequestFailed(const Sr<Lactose::Rest::IRequest>& Request)
{
	FScopeLock Lock(&ErrorReportsCs);

	ErrorReports.Emplace(FServiceErrorReport
	{
		.ServiceName = FName(GetLactoseServiceNameFromUrl(Request->GetInternal()->GetURL())),
		.ErrorCode = Request->GetResponseCode(),
		.ErrorTime = FDateTime::Now(),
	});
}

void ULactoseServiceReportingSubsystem::Tick()
{
	ErrorReportsCs.Lock();

	FDateTime TimeOfRemoval = FDateTime::Now() - ErrorExpiryTime;
	ErrorReports.RemoveAllSwap([TimeOfRemoval](const FServiceErrorReport& ErrorReport)
	{
		return ErrorReport.ErrorTime < TimeOfRemoval;
	});
	
	int32 UnauthorisedCount = 0;
	int32 AnyServiceIssuesCount = 0;
	CachedServiceErrorCountMap.Reset();

	for (const FServiceErrorReport& ErrorReport : ErrorReports)
	{
		const auto ResponseCode = std::get<EHttpResponseCodes::Type>(ErrorReport.ErrorCode);
		if (ResponseCode && (ResponseCode == EHttpResponseCodes::Denied || ResponseCode == EHttpResponseCodes::Forbidden))
			UnauthorisedCount++;

		CachedServiceErrorCountMap.FindOrAdd(ErrorReport.ServiceName)++;
		AnyServiceIssuesCount++;
	}

	ErrorReportsCs.Unlock();

	const bool bShouldThrowUnauthorisedWarning = UnauthorisedCount >= UnauthorisedWarningCount;
	const bool bShouldThrowUnauthorisedError = UnauthorisedCount >= UnauthorisedErrorCount;
	const bool bShouldThrowAnyServiceWarning = AnyServiceIssuesCount >= AnyServiceWarningCount;
	const bool bShouldThrowAnyServiceError = AnyServiceIssuesCount >= AnyServiceErrorCount;
	bool bAnyErrors = bShouldThrowUnauthorisedError || bShouldThrowAnyServiceError;
	
	constexpr int32 UnauthorisedIssueKey = 5001;
	constexpr int32 AnyServiceIssueKey = 5002;

	if (bShouldThrowUnauthorisedError)
	{
		ShowDebugErrorText(UnauthorisedIssueKey, TEXT("Encountering serious issues with authentication"));

		// Go back to the main menu to redo the authentication flow.
		GetGameInstance()->ReturnToMainMenu();
	}
	else if (bShouldThrowUnauthorisedWarning)
	{
		ShowDebugWarningText(UnauthorisedIssueKey, TEXT("Encountering issues with authentication"));
	}
	else
	{
		RemoveDebugText(UnauthorisedIssueKey);
	}

	if (bShouldThrowAnyServiceError)
	{
		ShowDebugErrorText(AnyServiceIssueKey, TEXT("Encountering serious issues with Lactose Services"));
	}
	else if (bShouldThrowAnyServiceWarning)
	{
		ShowDebugWarningText(AnyServiceIssueKey, TEXT("Encountering issues with Lactose Services"));
	}
	else
	{
		RemoveDebugText(AnyServiceIssueKey);
	}

	for (const TPair<FName, int>& ServiceErrorCount : CachedServiceErrorCountMap)
	{
		const int32 ServiceKey = ServiceErrorCount.Key.ToUnstableInt();
		const FString ServiceName = ServiceErrorCount.Key.ToString().ToUpper();
		const bool bShouldThrowServiceWarning = ServiceErrorCount.Value >= SameServiceWarningCount;
		const bool bShouldThrowServiceError = ServiceErrorCount.Value >= SameServiceErrorCount;

		bAnyErrors |= bShouldThrowServiceError;
		
		if (bShouldThrowServiceError)
		{
			ShowDebugErrorText(ServiceKey,
				FString::Printf(TEXT("Encountering serious issues with Lactose %s Service"), *ServiceName));
		}
		else if (bShouldThrowServiceWarning)
		{
			ShowDebugWarningText(ServiceKey,
				FString::Printf(TEXT("Encountering issues with Lactose %s Service"), *ServiceName));
		}
		else
		{
			RemoveDebugText(ServiceKey);
		}
	}
}

FString ULactoseServiceReportingSubsystem::GetLactoseServiceNameFromUrl(const FString& Url)
{
	// Currently, the Lactose Service name is always in between the first and second forward slash.
	
	FString Path;
	Url.Split(TEXT(".ovh/"), nullptr, &Path);

	FString FirstPath;
	Path.Split(TEXT("/"), &FirstPath, nullptr);

	return FirstPath;
}

void ULactoseServiceReportingSubsystem::ShowDebugWarningText(int32 Key, const FString& Text)
{
	GEngine->AddOnScreenDebugMessage(
		Key,
		TNumericLimits<float>::Max(),
		FColor::Yellow,
		Text);
}

void ULactoseServiceReportingSubsystem::ShowDebugErrorText(int32 Key, const FString& Text)
{
	GEngine->AddOnScreenDebugMessage(
		Key,
		TNumericLimits<float>::Max(),
		FColor::Red,
		Text,
		false,
		FVector2D(1.25,1.25));
}

void ULactoseServiceReportingSubsystem::RemoveDebugText(int32 Key)
{
	GEngine->RemoveOnScreenDebugMessage(Key);
}
