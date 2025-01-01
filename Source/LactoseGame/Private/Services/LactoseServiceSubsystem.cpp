#include "Services/LactoseServiceSubsystem.h"

#include "Services/LactoseServicesLog.h"

TFuture<Sp<FGetServiceStatusRequest::FResponseContext>> ULactoseServiceSubsystem::GetServiceInfo()
{
	auto& RestSubsystem = Subsystems::GetRef<ULactoseRestSubsystem>(self);
	auto RestRequest = FGetServiceStatusRequest::Create(RestSubsystem);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("info"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ULactoseServiceSubsystem::OnGetServiceInfoResponse);
	
	return RestRequest->Send2();
}

void ULactoseServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void ULactoseServiceSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void ULactoseServiceSubsystem::SetServiceBaseUrl(FString&& InUrl)
{
	ServiceBaseUrl = std::move(InUrl);
}

void ULactoseServiceSubsystem::OnGetServiceInfoResponse(Sr<FGetServiceStatusRequest::FResponseContext> Context)
{
	if (Context->ResponseContent)
	{
		Log::Log(LogLactoseServices, TEXT("Responsed: %s"), *Context->HttpResponse->GetContentAsString());
	}
	else
	{
		Log::Log(LogLactoseServices, TEXT("Failed: %s"), *Context->HttpResponse->GetContentAsString());
	}
}
