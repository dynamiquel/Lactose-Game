#include "Rest/LactoseRestSubsystem.h"

#include "Rest/LactoseRestLog.h"


bool ULactoseRestSubsystem::SendRequest(const Sr<Lactose::Rest::IRequest>& Request)
{
	if (Request->GetInternal()->ProcessRequest())
	{
		FScopeLock Lock(&PendingRequestsLock);
		PendingRequests.Add(Request);

		Log::Verbose(LogLactoseRest
			, TEXT("Sent Request to '%s'. Content:\n%s"),
			*Request->GetInternal()->GetURL(),
			*Request->GetContentString());
		
		return true;
	}

	return false;
}

void ULactoseRestSubsystem::RemoveRequest(const Sr<Lactose::Rest::IRequest>& Request)
{
	FScopeLock Lock(&PendingRequestsLock);
	PendingRequests.Remove(Request);
}

void ULactoseRestSubsystem::AddAuthorization(const FString& AccessToken)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	
	HttpModule.AddDefaultHeader(
		TEXT("Authorization"),
		FString::Printf(TEXT("Bearer %s"), *AccessToken));
}

void ULactoseRestSubsystem::RemoveAuthorization()
{
	FHttpModule& HttpModule = FHttpModule::Get();
	
	HttpModule.AddDefaultHeader(
		TEXT("Authorization"),
		FString());
}
