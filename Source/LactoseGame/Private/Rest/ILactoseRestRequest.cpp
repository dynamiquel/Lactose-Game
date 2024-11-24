#include "Rest/ILactoseRestRequest.h"

#include "Interfaces/IHttpResponse.h"

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

Lactose::Rest::IRequest& Lactose::Rest::IRequest::SetVerb(const FString& Verb)
{
	GetInternal()->SetVerb(Verb);
	return *this;
}

Lactose::Rest::IRequest& Lactose::Rest::IRequest::SetUrl(const FString& Url)
{
	GetInternal()->SetURL(Url);
	return *this;
}

Lactose::Rest::IRequest& Lactose::Rest::IRequest::SetContent(TArray<uint8>&& Bytes)
{
	GetInternal()->SetContent(Bytes);
	return *this;
}

TFuture<TSharedPtr<Lactose::Rest::IRequest::FResponseContext>> Lactose::Rest::IRequest::Send()
{
	if (HasBeenSent())
	{
		UE_LOG(LogTemp, Error, TEXT("Attempted to send a Request to '%s' but it has already been sent"),
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

	const ULactoseRestSubsystem* PinnedRestSubsystem = RestSubsystem.Get();
	if (!PinnedRestSubsystem)
		return {};
	
	if (!RestSubsystem->SendRequest(SharedThis(this)))
		return {};

	TimeRequestSent = FDateTime::UtcNow();
	return ResponsePromise.GetFuture(); 
}

void Lactose::Rest::IRequest::OnResponseReceived(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedRef<FResponseContext> ResponseContext = MakeShared<FResponseContext>();
	ResponseContext->HttpRequest = Request;
	ResponseContext->HttpResponse = Response;
	ResponseContext->RequestTime = GetRequestTime();
	ResponseContext->ResponseTime = FDateTime::UtcNow();

	if (!ResponseContext->IsSuccessful())
	{
		UE_CLOG(ResponseContext->HttpResponse, LogTemp, Error, TEXT("Received an unsuccessful response. Code %d; Reason: %d"),
			Response->GetResponseCode(),
			Response->GetFailureReason());
	}
	
	ResponsePromise.SetValue(ResponseContext);
	GetOnResponseReceived().Broadcast(ResponseContext);

	if (ULactoseRestSubsystem* PinnedRestSubsystem = RestSubsystem.Get())
	{
		PinnedRestSubsystem->RemoveRequest(SharedThis(this));
	}
}