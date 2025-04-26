// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Catalyst.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/EngineSubsystem.h"
#include "CatalystSubsystem.generated.h"

namespace Catalyst::Verbs
{
	constexpr auto* GET = TEXT("GET");
	constexpr auto* POST = TEXT("POST");
	constexpr auto* PUT = TEXT("PUT");
	constexpr auto* HEAD = TEXT("HEAD");
	constexpr auto* TRACE = TEXT("TRACE");
	constexpr auto* DELETE = TEXT("DELETE");

	bool IsValid(const FString& Verb);
}

class FCatalystOperation;
/**
 * 
 */
UCLASS()
class CATALYST_API UCatalystSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

	friend class FCatalystOperation;

public:
	static UCatalystSubsystem& Get();

	template<typename TOperation> requires (std::is_base_of_v<FCatalystOperation, TOperation>)
	TSharedRef<TOperation> CreateOperation(
		const FString& Url,
		const FString& Method,
		TArray<uint8>&& Content,
		float Timeout)
	{
		TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(
			Url,
			Method,
			MoveTemp(Content),
			Timeout);
		
		TSharedRef<TOperation> Operation = MakeShared<TOperation>(HttpRequest);
		Operation->Init();
		return Operation;
	}

protected:
	TSharedRef<IHttpRequest> CreateHttpRequest(
		const FString& Url,
		const FString& Method,
		TArray<uint8>&& Content,
		float Timeout);
	
	void RegisterOperation(TSharedRef<FCatalystOperation> Operation);
	void UnregisterOperation(TSharedRef<FCatalystOperation> Operation);

protected:
	/**
	 * Keep reference to pending operations to ensure they stay alive while being
	 * processed and for debugging purposes.
	 */
	FCriticalSection PendingOperationsLock;
	TArray<TSharedRef<FCatalystOperation>> PendingOperations;
};
