// Fill out your copyright notice in the Description page of Project Settings.


#include "CatalystSubsystem.h"

#include "CatalystOperation.h"
#include "HttpModule.h"

bool Catalyst::Verbs::IsValid(const FString& Verb)
{
	return Verb == GET || Verb == POST || Verb == PUT || Verb == HEAD || Verb == TRACE || Verb == DELETE;
}

UCatalystSubsystem& UCatalystSubsystem::Get()
{
	auto* CatalystSubsystem = GEngine->GetEngineSubsystem<ThisClass>();
	check(CatalystSubsystem);
	return *CatalystSubsystem;
}

TSharedRef<IHttpRequest> UCatalystSubsystem::CreateHttpRequest(
	const FString& Url,
	const FString& Method,
	TArray<uint8>&& Content,
	float Timeout)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(Method);
	Request->SetContent(MoveTemp(Content));
	// Would rather separate this out but for now, it's okay.
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(Timeout);

	checkf(Catalyst::Verbs::IsValid(Method), TEXT("Invalid HTTP verb: %s"), *Method);

	return Request;
}

void UCatalystSubsystem::RegisterOperation(TSharedRef<FCatalystOperation> Operation)
{
	FScopeLock Lock(&PendingOperationsLock);
	
	check(!PendingOperations.Contains(Operation));
	PendingOperations.Add(Operation);

	UE_LOG(LogCatalyst, Verbose, TEXT("Registered Operation %s"), *Operation->ToString());
}

void UCatalystSubsystem::UnregisterOperation(TSharedRef<FCatalystOperation> Operation)
{
	FScopeLock Lock(&PendingOperationsLock);
	
	check(PendingOperations.Remove(Operation));

	UE_LOG(LogCatalyst, Verbose, TEXT("Unregistered Operation %s"), *Operation->ToString());
}
