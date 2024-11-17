// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <array>

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "Rest/LactoseRestRequest.h"
#include "LactoseRestSendDebugTab.generated.h"

/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseRestSendDebugTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseRestSendDebugTab();

	// Begin override UDebugAppTab
	void Render() override;
	// End override UDebugAppTab

	void OnResponseReceived(TSharedRef<Lactose::Rest::IRequest::FResponseContext> Context);

	std::array<char, 512> UrlBuffer;
	std::array<char, 2048> PayloadBuffer;
	std::array<const char*, 7> Verbs = { "GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS", "HEAD" };
	int32 SelectedVerbIndex = 0;

	TSharedPtr<Lactose::Rest::IRequest::FResponseContext> LastResponseContext;
	TFuture<TSharedPtr<Lactose::Rest::IRequest::FResponseContext>> FutureResponseContext;
};
