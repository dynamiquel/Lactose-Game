#include "Services/LactoseServiceSubsystem.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Services/LactoseServicesLog.h"

void ULactoseServiceSubsystem::GetServiceInfo()
{
	FHttpModule& HttpModule = FHttpModule::Get();
	auto HttpRequest = HttpModule.CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(TEXT("https://lactose.mookrata.ovh/identity/info"));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &ULactoseServiceSubsystem::OnGetServiceInfoResponse);
	HttpRequest->ProcessRequest();
}

void ULactoseServiceSubsystem::GetServiceInfo2()
{
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = Lactose::Rest::FRequest::Create(*RestSubsystem);
	RestRequest->SetUrl(TEXT("https://lactose.mookrata.ovh/identity/info"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ULactoseServiceSubsystem::OnGetServiceInfoResponse2);
	RestRequest->Send();
}

void ULactoseServiceSubsystem::GetServiceInfo3()
{
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetServiceInfoRequest::Create(*RestSubsystem);
	RestRequest->SetUrl(TEXT("https://lactose.mookrata.ovh/identity/info"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ULactoseServiceSubsystem::OnGetServiceInfoResponse3);
	RestRequest->Send();
}

void ULactoseServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void ULactoseServiceSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void ULactoseServiceSubsystem::OnGetServiceInfoResponse(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	if (bConnectedSuccessfully)
	{
		UE_LOG(LogLactoseServices, Log, TEXT("Responsed: %s"), *Response->GetContentAsString())
	}
	else if (Response.IsValid())
	{
		UE_LOG(LogLactoseServices, Log, TEXT("Failed: %d"), Response->GetResponseCode());
	}
	else
	{
		UE_LOG(LogLactoseServices, Log, TEXT("Failed"));
	}
}

void ULactoseServiceSubsystem::OnGetServiceInfoResponse2(TSharedRef<Lactose::Rest::FRequest::FResponseContext> Context)
{
	if (Context->IsSuccessful())
	{
		UE_LOG(LogLactoseServices, Log, TEXT("Responsed: %s"), *Context->HttpResponse->GetContentAsString())
	}
	else
	{
		UE_LOG(LogLactoseServices, Log, TEXT("Failed: %d"), Context->HttpResponse->GetResponseCode());
	}
}

void ULactoseServiceSubsystem::OnGetServiceInfoResponse3(TSharedRef<FGetServiceInfoRequest::FResponseContext> Context)
{
	if (Context->ResponseContent)
	{
		UE_LOG(LogLactoseServices, Log, TEXT("Responsed: %s"), *Context->HttpResponse->GetContentAsString())
	}
	else
	{
		UE_LOG(LogLactoseServices, Log, TEXT("Failed: %d"), Context->HttpResponse->GetResponseCode());
	}
}
