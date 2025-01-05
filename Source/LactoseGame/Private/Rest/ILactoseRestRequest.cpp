#include "Rest/ILactoseRestRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Rest/LactoseRestLog.h"
#include "Rest/LactoseRestSubsystem.h"

bool Lactose::Rest::IRequest::FResponseContext::IsSuccessful() const
{
	return HttpResponse
		&& HttpResponse->GetStatus() == EHttpRequestStatus::Succeeded
		&& HttpResponse->GetResponseCode() >= 200
		&& HttpResponse->GetResponseCode() < 300;
}

double Lactose::Rest::IRequest::FResponseContext::GetLatencyMs() const
{
	return (ResponseTime - RequestTime).GetTotalMilliseconds();
}

Lactose::Rest::IRequest::IRequest(
	const TWeakObjectPtr<ULactoseRestSubsystem>& InRestSubsystem,
	const TSharedRef<IHttpRequest>& HttpRequest)
	: RestSubsystem(InRestSubsystem)
	, InternalHttpRequest(HttpRequest)
{
}

Lactose::Rest::IRequest::~IRequest()
{
	// Wish there was a better way of cancelling promises.
	const bool bPending = GetInternal()->GetStatus() == EHttpRequestStatus::Processing;
	if (bPending)
	{
		ResponsePromise.SetValue(nullptr);
		GetInternal()->CancelRequest();
	}
}

FString Lactose::Rest::IRequest::GetContentString() const
{
	return FString(StringCast<TCHAR>(
		reinterpret_cast<const UTF8CHAR*>(GetInternal()->GetContent().GetData()),
		GetInternal()->GetContent().Num()));
}

Lactose::Rest::IRequest& Lactose::Rest::IRequest::SetVerb(const FString& Verb)
{
	GetInternal()->SetVerb(Verb);
	return self;
}

Lactose::Rest::IRequest& Lactose::Rest::IRequest::SetUrl(const FString& Url)
{
	GetInternal()->SetURL(Url);
	return self;
}

Lactose::Rest::IRequest& Lactose::Rest::IRequest::SetContent(TArray<uint8>&& Bytes)
{
	GetInternal()->SetContent(Bytes);
	return self;
}

TFuture<Sp<Lactose::Rest::IRequest::FResponseContext>> Lactose::Rest::IRequest::Send()
{
	if (HasBeenSent())
	{
		Log::Error(LogLactoseRest,
			TEXT("Attempted to send a Request to '%s' but it has already been sent"),
			*GetInternal()->GetURL());
		
		return {};
	}
	
	GetInternal()->OnProcessRequestComplete().BindSP(
		SharedThis(this),
		&IRequest::OnResponseReceived);

	const bool bNeedsContentType = GetInternal()->GetContentType().IsEmpty() && !GetInternal()->GetContent().IsEmpty();
	if (bNeedsContentType)
	{
		// No Content Type is specified, assume JSON.
		// Unreal's HTTP module will perform an assertion if no Content Type is set (annoying).
		GetInternal()->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	}

	ULactoseRestSubsystem* PinnedRestSubsystem = RestSubsystem.Get();
	if (!PinnedRestSubsystem)
		return {};
	
	if (!PinnedRestSubsystem->SendRequest(SharedThis(this)))
		return {};

	TimeRequestSent = FDateTime::UtcNow();
	return ResponsePromise.GetFuture(); 
}

void Lactose::Rest::IRequest::OnResponseReceived(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	auto ResponseContext = CreateSr(FResponseContext
	{
		.HttpRequest = Request,
		.HttpResponse = Response,
		.RequestTime = GetRequestTime(),
		.ResponseTime = FDateTime::UtcNow()
	});

	if (!ResponseContext->IsSuccessful())
	{
		Log::Error(LogLactoseRest,
			TEXT("Received an unsuccessful response from %s. Code %d; Reason: %d"),
			*Response->GetURL(),
			Response->GetResponseCode(),
			Response->GetFailureReason());
	}
	else
	{
		Log::Verbose(LogLactoseRest,
			TEXT("Received a succesful response from %s. Code: %d"),
			*Response->GetURL(),
			Response->GetResponseCode());
				
		Log::Verbose(LogLactoseRest,
			TEXT("Content: %s"),
			*Response->GetContentAsString());
	}
	
	ResponsePromise.SetValue(ResponseContext);
	GetOnResponseReceived().Broadcast(ResponseContext);

	if (ULactoseRestSubsystem* PinnedRestSubsystem = RestSubsystem.Get())
		PinnedRestSubsystem->RemoveRequest(SharedThis(this));
}