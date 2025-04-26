#include "CatalystOperation.h"

#include "CatalystSubsystem.h"

FCatalystOperation::FCatalystOperation(const TSharedRef<IHttpRequest>& InHttpRequest)
	: CreateTime(FDateTime::Now())
	, HttpRequest(InHttpRequest)
{
}

FCatalystOperation::~FCatalystOperation()
{
	UE_LOG(LogCatalyst, Verbose, TEXT("Freed Operation %s"), *ToString());
}

void FCatalystOperation::Init()
{
	if (!HttpRequest->ProcessRequest())
	{
		UE_LOG(LogCatalyst, Error, TEXT("Failed to create HTTP Request for Operation %s"), *ToString());
		return;
	}

	TSharedRef<FCatalystOperation> ThisShared = SharedThis(this);
	
	UCatalystSubsystem::Get().RegisterOperation(ThisShared);
	HttpRequest->OnProcessRequestComplete().BindSP(ThisShared, &FCatalystOperation::OnHttpResponse);

	UE_LOG(LogCatalyst, Verbose, TEXT("Sent Operation %s"), *ToString());
}

bool FCatalystOperation::IsPending() const
{
	return HttpRequest->GetStatus() == EHttpRequestStatus::Processing;
}

bool FCatalystOperation::IsError() const
{
	return !IsPending() && !IsSuccess();
}

TSharedPtr<IHttpResponse> FCatalystOperation::GetError() const
{
	return HttpRequest->GetResponse();
}

FDateTime FCatalystOperation::GetCreateTime() const
{
	return CreateTime;
}

FString FCatalystOperation::ToString() const
{
	return FString::Printf(TEXT("(%s %s [%s])"),
		*HttpRequest->GetVerb(),
		*HttpRequest->GetURL(),
		*GetCreateTime().ToString(TEXT("%H:%M:%S")));
}

bool FCatalystOperation::Cancel()
{
	if (!IsPending())
		return false;
		
	HttpRequest->CancelRequest();

	UE_LOG(LogCatalyst, Verbose, TEXT("Cancelled Operation %s"), *ToString());

	return true;
}

void FCatalystOperation::OnHttpResponse(
	FHttpRequestPtr HttpRequest,
	FHttpResponsePtr HttpResponse,
	bool bProcessedSuccessfully)
{
	UCatalystSubsystem::Get().UnregisterOperation(SharedThis(this));
}
