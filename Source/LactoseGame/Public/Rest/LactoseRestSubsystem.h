#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ILactoseRestRequest.h"
#include "Simp.h"

#include "LactoseRestSubsystem.generated.h"

UCLASS()
class LACTOSEGAME_API ULactoseRestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:	
	template<typename TRequest> requires (std::is_base_of_v<Lactose::Rest::IRequest, TRequest>)
	Sr<TRequest> CreateRequest()
	{
		FHttpModule& HttpModule = FHttpModule::Get();
		
		Sr<IHttpRequest> HttpRequest = HttpModule.CreateRequest();
		HttpRequest->SetVerb(Lactose::Rest::Verbs::GET);
		
		Sr<TRequest> LactoseRequest = MakeShared<TRequest>(this, HttpRequest);
		return LactoseRequest;
	}

	bool SendRequest(const Sr<Lactose::Rest::IRequest>& Request);
	void RemoveRequest(const Sr<Lactose::Rest::IRequest>& Request);

private:
	// Ensures Requests stay alive while they are being processed.
	TSet<Sr<Lactose::Rest::IRequest>> PendingRequests;

	FCriticalSection PendingRequestsLock;
};
