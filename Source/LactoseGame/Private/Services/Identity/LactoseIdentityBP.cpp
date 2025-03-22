#include "Services/Identity/LactoseIdentityBP.h"

#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

void ULactoseIdentityLoginFailedDelegateWrapper::OnSubscribed()
{
	NativeDelegateHandle = Lactose::Identity::Events::OnUserLoginFailed.AddUObject(this, &ThisClass::HandleNativeEvent);
}

void ULactoseIdentityLoginFailedDelegateWrapper::OnUnsubscribed()
{
	Lactose::Identity::Events::OnUserLoginFailed.Remove(NativeDelegateHandle);
}

void ULactoseIdentityLoginFailedDelegateWrapper::HandleNativeEvent(const ULactoseIdentityServiceSubsystem& Sender)
{
	OnExecuted();
}
