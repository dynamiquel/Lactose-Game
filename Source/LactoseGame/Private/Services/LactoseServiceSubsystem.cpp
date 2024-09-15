#include "Services/LactoseServiceSubsystem.h"

#include "TurboLinkGrpcManager.h"
#include "TurboLinkGrpcUtilities.h"
#include "Services/LactoseServicesLog.h"
#include "SLactoseCommon/HelloService.h"

void ULactoseServiceSubsystem::SayHello()
{
	if (!IsValid(HelloClient))
	{
		UE_LOG(LogLactoseServices, Error, TEXT("Hello Service is unavailable"));
		return;
	}
	
	FGrpcContextHandle CtxHello = HelloClient->InitSayHello();

	FGrpcLactoseCommonHelloRequest HelloRequest;
	HelloRequest.ClientIdentifier = TEXT("Temp");

	const auto NowUtc = FTimespan(FDateTime::Now().GetTicks());
	FGrpcGoogleProtobufTimestamp Now;
	Now.Seconds = static_cast<int64>(NowUtc.GetTotalSeconds());
	Now.Nanos = NowUtc.GetFractionNano();
	HelloRequest.RequestTime = Now;

	HelloClient->SayHello(CtxHello, HelloRequest);
}

void ULactoseServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UTurboLinkGrpcManager* TurboLinkManager = UTurboLinkGrpcUtilities::GetTurboLinkGrpcManager(this);
	if (!ensure(TurboLinkManager))
	{
		return;
	}

	auto* HelloService = TurboLinkManager->MakeService<UHello>();
	if (!ensure(HelloService))
	{
		return;
	}
	
	HelloService->Connect();
	
	HelloClient = HelloService->MakeClient();
	if (!ensure(HelloClient))
	{
		return;
	}
	
	HelloClient->OnSayHelloResponse.AddUniqueDynamic(this, &ThisClass::OnHelloResponse);
}

void ULactoseServiceSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	if (UTurboLinkGrpcManager* TurboLinkManager = UTurboLinkGrpcUtilities::GetTurboLinkGrpcManager(this))
	{
		if (HelloClient)
		{
			HelloClient->Shutdown();

			if (UGrpcService* GrpcService = HelloClient->GetGrpcService())
				TurboLinkManager->ReleaseService(GrpcService);
		}
	}
}

void ULactoseServiceSubsystem::OnHelloResponse(
	FGrpcContextHandle Handle,
	const FGrpcResult& GrpcResult,
	const FGrpcLactoseCommonHelloResponse& Response)
{
	const double RequestTimeMs = FTimespan::FromMicroseconds(Response.RequestTime.Seconds * 1000000. + static_cast<double>(Response.RequestTime.Nanos) / 1000.).GetTotalMilliseconds();
	const double ServerResponseTimeMs = FTimespan::FromMicroseconds(Response.ResponseTime.Seconds * 1000000. + static_cast<double>(Response.ResponseTime.Nanos) / 1000.).GetTotalMilliseconds();
	const double ClientToServerLatencyMs = ServerResponseTimeMs - RequestTimeMs;
	const double NowMs = FTimespan(FDateTime::UtcNow().GetTicks()).GetTotalMilliseconds();
	const double RoundtripMs = NowMs - RequestTimeMs;
	const double ServerToClientMs = NowMs - ServerResponseTimeMs;
	
	UE_LOG(LogLactoseServices, Log, TEXT("%s replied back in %fms (to server: %fms | from server: %fms"),
		*Response.ServiceName,
		RoundtripMs,
		ClientToServerLatencyMs,
		ServerToClientMs);
}
