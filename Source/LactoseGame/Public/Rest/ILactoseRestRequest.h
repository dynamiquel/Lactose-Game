#pragma once

#include "Interfaces/IHttpRequest.h"

class ULactoseRestSubsystem;

namespace Lactose::Rest
{
	namespace Verbs
	{
		constexpr auto* GET = TEXT("GET");
		constexpr auto* POST = TEXT("POST");
		constexpr auto* HEAD = TEXT("HEAD");
		constexpr auto* PUT = TEXT("PUT");
		constexpr auto* DELETE = TEXT("DELETE");
	}

	class LACTOSEGAME_API IRequest : public TSharedFromThis<IRequest>
	{
	public:
		class FResponseContext
		{
		public:
			bool IsSuccessful() const;

			FHttpRequestPtr HttpRequest;
			FHttpResponsePtr HttpResponse;
			FDateTime RequestTime;
			FDateTime ResponseTime;

			double GetLatencyMs() const;
		};

		DECLARE_MULTICAST_DELEGATE_OneParam(FLactoseRestResponseReceivedDelegate, TSharedRef<FResponseContext>);
		
		IRequest(const TWeakObjectPtr<ULactoseRestSubsystem>& InRestSubsystem, const TSharedRef<IHttpRequest>& HttpRequest);
		virtual ~IRequest();

		FDateTime GetRequestTime() const { return TimeRequestSent; }
		bool HasBeenSent() const { return TimeRequestSent > 0; }
		FLactoseRestResponseReceivedDelegate& GetOnResponseReceived() { return ResponseReceived; }
		
		IRequest& SetVerb(const FString& Verb);
		IRequest& SetUrl(const FString& Url);
		IRequest& SetContent(TArray<uint8>&& Bytes);
	
		TFuture<TSharedPtr<FResponseContext>> Send();
	
		const TSharedRef<IHttpRequest>& GetInternal() const { return InternalHttpRequest; }

	protected:
		virtual void OnResponseReceived(
			FHttpRequestPtr Request,
			FHttpResponsePtr Response,
			bool bConnectedSuccessfully);

	protected:
		TWeakObjectPtr<ULactoseRestSubsystem> RestSubsystem;
		TPromise<TSharedPtr<FResponseContext>> ResponsePromise;
		FLactoseRestResponseReceivedDelegate ResponseReceived;
		
	private:
		TSharedRef<IHttpRequest> InternalHttpRequest;
		FDateTime TimeRequestSent;
	};
}