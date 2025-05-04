#pragma once

#include "Catalyst.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

/**
 * Rules defining when a callback is executed.
 */
enum class ECallbackOn
{
	// The callback will be executed regardless of success or error.
	Always,
	// The callback will only be executed on successful responses.
	Success,
	// The callback will only be executed on error responses.
	Error
};

/**
 * Represents a pending or completed Catalyst Client Service operation.
 * It can be used as a kind of Future, Cancellation Token or Debug Context.
 *
 * This is an abstract class and intended to be specialised using TCatalystOperation.
 */
class CATALYST_API FCatalystOperation : public TSharedFromThis<FCatalystOperation>
{
public:
	explicit FCatalystOperation(const TSharedRef<IHttpRequest>& InHttpRequest);
	virtual ~FCatalystOperation();
	
	void Init();

	/**
	 * The operation has succeeded, and any valid response is ready to be used.
	 */
	virtual bool IsSuccess() const = 0;

	/**
	 * The operation is still pending and is not ready to be used.
	 */
	bool IsPending() const;

	/**
	 * The operation has failed, and an error response is ready to be used.
	 */
	bool IsError() const;

	TSharedPtr<IHttpResponse> GetError() const;

	/**
	 * The local time the operation was created.
	 */
	FDateTime GetCreateTime() const;

	/**
	 * String-representation of the operation.
	 */
	FString ToString() const;

	/**
	 * Cancels the pending operation.
	 */
	bool Cancel();

protected:
	virtual void OnHttpResponse(
		FHttpRequestPtr HttpRequest,
		FHttpResponsePtr HttpResponse,
		bool bProcessedSuccessfully);

protected:
	FDateTime CreateTime;
	TSharedRef<IHttpRequest> HttpRequest;
	ECallbackOn CallbackCondition = ECallbackOn::Always;
};

/**
 * Represents a pending or completed Catalyst Client Service operation.
 * It can be used as a kind of Future, Cancellation Token or Debug Context.
 */
template<typename TResponse>
class TCatalystOperation : public FCatalystOperation
{
public:
	using FCallback = TDelegate<void(TSharedRef<TCatalystOperation>)>;
	using FCallbackFunc = TFunction<void(TSharedRef<TCatalystOperation>)>;

	explicit TCatalystOperation(const TSharedRef<IHttpRequest>& InHttpRequest)
		: FCatalystOperation(InHttpRequest)
	{}

	~TCatalystOperation() override = default;


	bool IsSuccess() const override
	{
		return Response.IsValid();
	}

	/**
	 * Gets the valid response if it is available.
	 * Returns null when not available due to pending or failure.
	 */
	TSharedPtr<TResponse> GetResponse()
	{
		return Response;
	}

	/**
	 * Binds a callback to be executed when a valid response or error has been received.
	 */
	void Then(FCallback&& Callback)
	{
		checkf(!ResponseCallback.IsBound(), TEXT("Response Callback has already been bound"));
		ResponseCallback = MoveTemp(Callback);
	}

	void Then(const ECallbackOn Condition, FCallback&& Callback)
	{
		CallbackCondition = Condition;
		Then(MoveTemp(Callback));
	}

	void Then(FCallbackFunc&& CallbackFunc)
	{
		Then(FCallback::CreateLambda(MoveTemp(CallbackFunc)));
	}
	
	void Then(const ECallbackOn Condition, FCallbackFunc&& CallbackFunc)
	{
		Then(Condition, FCallback::CreateLambda(MoveTemp(CallbackFunc)));
	}

	template<typename TObject>
	void Then(TObject* Object, typename FCallback::template TMethodPtr<TObject> InFunc)
	{
		Then(FCallback::CreateUObject(Object, InFunc));
	}

	template<typename TObject>
	void Then(const ECallbackOn Condition, TObject* Object, typename FCallback::template TMethodPtr<TObject> InFunc)
	{
		Then(Condition, FCallback::CreateUObject(Object, InFunc));
	}

protected:
	void OnHttpResponse(
		FHttpRequestPtr HttpRequest,
		FHttpResponsePtr HttpResponse,
		bool bProcessedSuccessfully) override
	{
		if (!HttpResponse.IsValid())
		{
			UE_LOG(LogCatalyst, Error, TEXT("Received Unknown Error for Operation %s"), *ToString());
			
			if (CallbackCondition == ECallbackOn::Always || CallbackCondition == ECallbackOn::Error)
				ResponseCallback.ExecuteIfBound(SharedThis(this));
		}
		else if (HttpResponse->GetResponseCode() != 200)
		{
			UE_LOG(LogCatalyst, Warning, TEXT("Received Error for Operation %s: Code: %d - Content: %s"),
				*ToString(),
				HttpResponse->GetResponseCode(),
				*HttpResponse->GetContentAsString());

			if (CallbackCondition == ECallbackOn::Always || CallbackCondition == ECallbackOn::Error)
				ResponseCallback.ExecuteIfBound(SharedThis(this));
		}
		else
		{
			// Convert content from JSON
			const TArray<uint8>& Bytes = HttpResponse->GetContent();
			TOptional<TResponse> ResponseOptional = TResponse::FromBytes(Bytes);
			if (!ResponseOptional.IsSet())
			{
				UE_LOG(LogCatalyst, Error, TEXT("Received Error for Operation %s: Failed to deserialise response"), *ToString());
				
				if (CallbackCondition == ECallbackOn::Always || CallbackCondition == ECallbackOn::Error)
					ResponseCallback.ExecuteIfBound(SharedThis(this));
			}
			else
			{
				UE_LOG(LogCatalyst, Verbose, TEXT("Received Response for Operation %s"), *ToString());

				Response = MakeShared<TResponse>(MoveTemp(ResponseOptional.GetValue()));
				
				if (CallbackCondition == ECallbackOn::Always || CallbackCondition == ECallbackOn::Success)
					ResponseCallback.ExecuteIfBound(SharedThis(this));
			}
		}

		FCatalystOperation::OnHttpResponse(HttpRequest, HttpResponse, bProcessedSuccessfully);
	}

protected:
	TSharedPtr<TResponse> Response;
	FCallback ResponseCallback;
};
