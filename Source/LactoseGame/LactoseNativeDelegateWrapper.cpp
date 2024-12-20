// Fill out your copyright notice in the Description page of Project Settings.


#include "LactoseNativeDelegateWrapper.h"

void ULactoseNativeDelegateWrapper::Start()
{
	if (!IsAssigned())
		OnSubscribed();
}

void ULactoseNativeDelegateWrapper::Stop()
{
	if (!IsAssigned())
		return;

	OnUnsubscribed();
	NativeDelegateHandle.Reset();
}

void ULactoseNativeDelegateWrapper::Reset()
{
	Stop();
	Delegate.Clear();
}

bool ULactoseNativeDelegateWrapper::IsAssigned() const
{
	return NativeDelegateHandle.IsValid();
}

void ULactoseNativeDelegateWrapper::BeginDestroy()
{
	UObject::BeginDestroy();

	Stop();
}

void ULactoseNativeDelegateWrapper::OnExecuted()
{
	Delegate.Broadcast(this);
}
