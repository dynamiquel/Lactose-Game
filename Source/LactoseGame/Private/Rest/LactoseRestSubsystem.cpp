#include "Rest/LactoseRestSubsystem.h"

#include "Rest/LactoseRestLog.h"


bool ULactoseRestSubsystem::SendRequest(const TSharedRef<Lactose::Rest::IRequest>& Request)
{
	if (Request->GetInternal()->ProcessRequest())
	{
		FScopeLock Lock(&PendingRequestsLock);
		PendingRequests.Add(Request);

		UE_LOG(LogLactoseRest, Verbose, TEXT("Sent Request to '%s'. Content:\n%s"),
			*Request->GetInternal()->GetURL(),
			*Request->GetContentString());
		
		return true;
	}

	return false;
}

void ULactoseRestSubsystem::RemoveRequest(const TSharedRef<Lactose::Rest::IRequest>& Request)
{
	FScopeLock Lock(&PendingRequestsLock);
	PendingRequests.Remove(Request);
}
