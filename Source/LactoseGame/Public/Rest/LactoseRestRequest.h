#pragma once

#include "ILactoseRestRequest.h"
#include "LactoseRestSubsystem.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialisation/LactoseJsonSerialisationUtils.h"

namespace Lactose::Rest
{
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
	
		virtual ~TRequest() = default;

		static TSharedRef<TRequest> Create(ULactoseRestSubsystem& RestSubsystem)
		{
			return RestSubsystem.CreateRequest<TRequest>();
		}

		FLactoseRestResponseReceived2Delegate& GetOnResponseReceived2() { return ResponseReceived2; }

		template<typename TRequestContentEnabled = TRequestContent, std::enable_if_t<!std::is_same_v<TRequestContentEnabled, void>>>
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

		template<typename TRequestContentEnabled = TRequestContent, std::enable_if_t<!std::is_same_v<TRequestContentEnabled, void>>>
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
			IRequest::OnResponseReceived(Request, Response, bConnectedSuccessfully);
			
			TSharedRef<FResponseContext> Context = MakeShared<FResponseContext>();
			Context->HttpRequest = Request;
			Context->HttpResponse = Response;
			
			if (Context->IsSuccessful())
			{
				// Convert request and response to Json.

				if constexpr (!std::is_same_v<TRequestContent, void>)
				{
					if (Request && !Request->GetContent().IsEmpty())
					{
						Context->RequestContent = Serialisation::Json::DeserialiseShared<TRequestContent>(Request->GetContent());
					}
				}

				if constexpr (!std::is_same_v<TResponseContent, void>)
				{
					if (!Response->GetContent().IsEmpty())
					{
						Context->ResponseContent = Serialisation::Json::DeserialiseShared<TResponseContent>(Response->GetContent());
					}
				}
			}

			ResponsePromise2.SetValue(Context);
			GetOnResponseReceived2().Broadcast(Context);
		}

	private:
		TPromise<TSharedPtr<FResponseContext>> ResponsePromise2;
		FLactoseRestResponseReceived2Delegate ResponseReceived2;
	};

	using FRequest = TRequest<>;
}
