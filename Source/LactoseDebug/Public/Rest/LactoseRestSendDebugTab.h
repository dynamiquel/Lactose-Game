// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <array>
#include <string>

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "Rest/LactoseRestRequest.h"
#include "LactoseRestSendDebugTab.generated.h"

USTRUCT()
struct FLactoseRestDebugRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	int32 Verb = 0;
	
	UPROPERTY()
	FString Url;

	UPROPERTY()
	FString Content;
};

USTRUCT()
struct FLactoseRestDebugRequests
{
	GENERATED_BODY()

	/**
	 * Unreal 5.4 has a weird bug with containers in User Config files. They do not save multiple elements properly
	 * and instead try to duplicate the property in the config file.
	 * Wrapping the container in its own struct is a workaround for this bugged behaviour.
	 */

	UPROPERTY()
	TArray<FLactoseRestDebugRequest> Requests;
};

struct FLactoseRestDebugSentRequest
{
	FDateTime TimeSent;
	TFuture<TSharedPtr<Lactose::Rest::IRequest::FResponseContext>> FutureResponseContext;
};

struct FLactoseRestDebugResponse
{
	FDateTime TimeReceived;
	TSharedPtr<Lactose::Rest::IRequest::FResponseContext> ResponseContext;
};

/**
 * 
 */
UCLASS(ProjectUserConfig)
class LACTOSEDEBUG_API ULactoseRestSendDebugTab : public UDebugAppTab
{
	GENERATED_BODY()

private:
	ULactoseRestSendDebugTab();

	// Begin override UDebugAppTab
	void Render() override;
	// End override UDebugAppTab

	void DrawRequestsSection();
	void DrawSelectedRequestSection();
	void DrawResponseSection();

	void CreateNewRequest();
	void ClearRequests();
	void DeleteRequest(int32 SavedRequestIdx);
	void SendRequest(int32 SavedRequestIdx);

	void SaveRequests();
	
	void OnResponseReceived(Sr<Lactose::Rest::IRequest::FResponseContext> Context);
	
private:
	inline static std::array<const char*, 7> Verbs = { "GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS", "HEAD" };

	UPROPERTY(EditAnywhere, Config)
	FLactoseRestDebugRequests SavedRequests;

	UPROPERTY(EditAnywhere, Config)
	int32 SelectedRequestIndex = INDEX_NONE;

	bool bSelectedRequestDirty = false;
	
	FLactoseRestDebugSentRequest SentRequest;
	FLactoseRestDebugResponse RecentResponse;
};
