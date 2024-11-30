#include "Rest/LactoseRestSubsystem.h"

bool ULactoseRestSubsystem::SendRequest(const TSharedRef<Lactose::Rest::IRequest>& Request)
{
	if (Request->GetInternal()->ProcessRequest())
	{
		FScopeLock Lock(&PendingRequestsLock);
		PendingRequests.Add(Request);
		return true;
	}

	return false;
}

void ULactoseRestSubsystem::RemoveRequest(const TSharedRef<Lactose::Rest::IRequest>& Request)
{
	FScopeLock Lock(&PendingRequestsLock);
	PendingRequests.Remove(Request);
}
