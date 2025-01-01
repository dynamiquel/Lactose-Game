// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DebugAppTab.h"
#include "DebugImGuiHelpers.h"
#include "Services/Economy/LactoseEconomyUserItemsRequests.h"
#include "LactoseEconomyUserItemsTab.generated.h"


class ULactoseEconomyServiceSubsystem;
/**
 * 
 */
UCLASS()
class LACTOSEDEBUG_API ULactoseEconomyUserItemsTab : public UDebugAppTab
{
	GENERATED_BODY()

	ULactoseEconomyUserItemsTab();

	// Begin override UDebugAppTab
	void Init() override;
	void Render() override;
	// End override UDebugAppTab
	
	UPROPERTY(Transient)
	TObjectPtr<ULactoseEconomyServiceSubsystem> EconomySubsystem;

	Debug::ImGui::FSearchBox CurrentUserItemsSearchBox { TEXT("###CurrentUserItemsSearchBox") };
	Debug::ImGui::FSearchBox OtherUserItemsSearchBox { TEXT("###OtherUserItemsSearchBox") };
	std::array<char, 128> OtherUserIdBuffer;

	TFuture<Sp<FGetEconomyUserItemsRequest::FResponseContext>> GetOtherUserItemsFuture;
};
