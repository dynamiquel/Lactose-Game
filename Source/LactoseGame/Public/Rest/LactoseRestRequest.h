#pragma once

#include "ILactoseRestRequest.h"
#include "LactoseRestLog.h"
#include "LactoseRestSubsystem.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialisation/LactoseJsonSerialisationUtils.h"

namespace Lactose::Rest
{
	namespace Concepts
	{
		template<typename T>
		concept RequestType = requires { T::StaticStruct(); };
	}

	// TODO: Sort out this template specialisation vs generic bs. 
	template<typename TRequestContent = void, typename TResponseContent = void>
	class TRequest : public IRequest
	{
	public:
		class FResponseContext : public IRequest::FResponseContext
		{
		public:
			TSharedPtr<TRequestContent> RequestContent;
			TSharedPtr<TResponseContent> ResponseContent;
		};
	
		DECLARE_MULTICAST_DELEGATE_OneParam(FLactoseRestResponseReceived2Delegate, TSharedRef<FResponseContext>);

		TRequest(const TWeakObjectPtr<ULactoseRestSubsystem>& RestSubsystem, const TSharedRef<IHttpRequest>& HttpRequest)
			: IRequest(RestSubsystem, HttpRequest) { }
	
		virtual ~TRequest()
		{
			// Wish there was a better way of cancelling promises.
			const bool bPending = GetInternal()->GetStatus() == EHttpRequestStatus::Processing;
			if (bPending)
			{
				ResponsePromise2.SetValue(nullptr);
				GetInternal()->CancelRequest();
			}
		}

		static TSharedRef<TRequest> Create(ULactoseRestSubsystem& RestSubsystem)
		{
			return RestSubsystem.CreateRequest<TRequest>();
		}

		FLactoseRestResponseReceived2Delegate& GetOnResponseReceived2() { return ResponseReceived2; }

		TFuture<TSharedPtr<FResponseContext>> Send2()
		{
			if (IRequest::Send().IsValid())
				return ResponsePromise2.GetFuture();

			return {};
		}

		template<Concepts::RequestType TRequestContentEnabled = TRequestContent>
		bool SetContentAsJson(const TRequestContentEnabled& Request)
		{
			TArray<uint8> JsonBytes = Serialisation::Json::Serialise(Request);
			if (JsonBytes.IsEmpty())
			{
				return false;
			}
			
			GetInternal()->SetContent(MoveTemp(JsonBytes));
			GetInternal()->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
			return true;
		}

		template<Concepts::RequestType TRequestContentEnabled = TRequestContent>
		TFuture<TSharedPtr<FResponseContext>> SetContentAsJsonAndSendAsync(const TSharedRef<TRequestContentEnabled>& Request)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [This = SharedThis(this), Request]
			{
				if (This->SetContentAsJson(*Request))
				{
					TFuture<TSharedPtr<IRequest::FResponseContext>> FutureResponse = This->Send();
					if (/* bSent = */ FutureResponse.IsValid())
						return;
				}
				
				This->ResponsePromise2.SetValue(nullptr);
			});

			return ResponsePromise2.GetFuture();
		}

	protected:
		void OnResponseReceived(
			FHttpRequestPtr Request,
			FHttpResponsePtr Response,
			bool bConnectedSuccessfully) override
		{
			TSharedRef<FResponseContext> Context = MakeShared<FResponseContext>();
			Context->HttpRequest = Request;
			Context->HttpResponse = Response;
			Context->RequestTime = GetRequestTime();
			Context->ResponseTime = FDateTime::UtcNow();

			if (!Context->IsSuccessful())
			{
				UE_LOG(LogLactoseRest, Error, TEXT("Received an unsuccessful response from %s. Code %d; Reason: %d"),
					*Response->GetURL(),
					Response->GetResponseCode(),
					Response->GetFailureReason());

				ResponsePromise.SetValue(nullptr);
				ResponsePromise2.SetValue(nullptr);
			}
			else
			{
				UE_LOG(LogLactoseRest, Verbose, TEXT("Received a succesful response from %s. Code: %d"),
					*Response->GetURL(),
					Response->GetResponseCode());
				
				UE_LOG(LogLactoseRest, VeryVerbose, TEXT("Content: %s"),
					*Response->GetContentAsString());
				
				// Convert request and response from JSON on a background thread before dispatching back to game thread.
				AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [This = SharedThis(this), Context]
				{
					TRACE_CPUPROFILER_EVENT_SCOPE(Lactose::Rest::TRequest::Deserialise);

					if constexpr (Concepts::RequestType<TRequestContent>)
					{
						UE_LOG(LogLactoseRest, VeryVerbose, TEXT("Deserialising request content to a %s"),
							*TRequestContent::StaticStruct()->GetName());
						
						if (Context->HttpRequest && !Context->HttpRequest->GetContent().IsEmpty())
						{
							Context->RequestContent = Serialisation::Json::DeserialiseShared<TRequestContent>(
								Context->HttpRequest->GetContent());
						}
					}

					if constexpr (Concepts::RequestType<TResponseContent>)
					{
						if (Context->HttpResponse && !Context->HttpResponse->GetContent().IsEmpty())
						{
							UE_LOG(LogLactoseRest, VeryVerbose, TEXT("Deserialising response content to a %s"),
								*TResponseContent::StaticStruct()->GetName());
							
							Context->ResponseContent = Serialisation::Json::DeserialiseShared<TResponseContent>(
								Context->HttpResponse->GetContent());
						}
					}

					AsyncTask(ENamedThreads::GameThread, [This, Context]
					{
						TRACE_CPUPROFILER_EVENT_SCOPE(Lactose::Rest::TRequest::Despatch);

						This->ResponsePromise.SetValue(StaticCastSharedRef<IRequest::FResponseContext>(Context));
						This->GetOnResponseReceived().Broadcast(StaticCastSharedRef<IRequest::FResponseContext>(Context));
						This->ResponsePromise2.SetValue(Context);
						This->GetOnResponseReceived2().Broadcast(Context);
					});
				});
			}
			
			if (ULactoseRestSubsystem* PinnedRestSubsystem = RestSubsystem.Get())
				PinnedRestSubsystem->RemoveRequest(SharedThis(this));
		}

	private:
		TPromise<TSharedPtr<FResponseContext>> ResponsePromise2;
		FLactoseRestResponseReceived2Delegate ResponseReceived2;
	};

	using FRequest = TRequest<>;

	template<typename TRequestFuture>
	static bool IsPending(const TRequestFuture& RequestFuture)
	{
		return RequestFuture.IsValid() && !RequestFuture.IsReady();
	}
}
