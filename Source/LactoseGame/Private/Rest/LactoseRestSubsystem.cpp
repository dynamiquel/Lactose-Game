#include "Rest/LactoseRestSubsystem.h"

bool ULactoseRestSubsystem::SendRequest(const TSharedRef<Lactose::Rest::IRequest>& Request)
{
	if (Request->GetInternal()->ProcessRequest())
	{
		PendingRequests.Add(Request);
		return true;
	}

	return false;
}

void ULactoseRestSubsystem::RemoveRequest(const TSharedRef<Lactose::Rest::IRequest>& Request)
{
	PendingRequests.Remove(Request);
}
