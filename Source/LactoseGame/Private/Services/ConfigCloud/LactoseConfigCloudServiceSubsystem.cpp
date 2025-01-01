// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/ConfigCloud/LactoseConfigCloudServiceSubsystem.h"

#include "Services/LactoseServicesLog.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

void FLactoseConfigCloudEntry::UpdateValue(const FString& NewValue)
{
	if (Value == NewValue)
		return;
		
	Value = NewValue;
	CachedStruct.Reset();
}

Sp<const FLactoseConfigCloudEntry> FLactoseConfigCloudConfig::FindEntry(const FString& EntryId) const
{
	return const_cast<FLactoseConfigCloudConfig*>(this)->FindEntry(EntryId);
}

Sp<FLactoseConfigCloudEntry> FLactoseConfigCloudConfig::FindEntry(const FString& EntryId)
{
	const Sr<FLactoseConfigCloudEntry>* FoundEntry = GetEntries().Find(EntryId);
	return FoundEntry ? FoundEntry->ToSharedPtr() : nullptr;
}

void FLactoseConfigCloudConfig::AddOrUpdateEntry(const FString& EntryId, const FString& EntryValue)
{
	if (Sp<FLactoseConfigCloudEntry> ExistingEntry = FindEntry(EntryId))
		ExistingEntry->UpdateValue(EntryValue);
	else
		Entries.Emplace(EntryId, MakeShared<FLactoseConfigCloudEntry>(EntryId, EntryValue));
}

void FLactoseConfigCloudConfig::DeleteEntry(const FString& EntryId)
{
	Entries.Remove(EntryId);
}

ULactoseConfigCloudServiceSubsystem::ULactoseConfigCloudServiceSubsystem()
{
	SetServiceBaseUrl(TEXT("https://lactose.mookrata.ovh/config"));
}

void ULactoseConfigCloudServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Lactose::Identity::Events::OnUserLoggedIn.AddUObject(this, &ThisClass::OnUserLoggedIn);
}

ELactoseConfigCloudStatus ULactoseConfigCloudServiceSubsystem::GetStatus() const
{
	if (GetConfig())
		return ELactoseConfigCloudStatus::Loaded;

	if (GetConfigFuture.IsValid())
		return ELactoseConfigCloudStatus::Retrieving;

	return ELactoseConfigCloudStatus::None;
}

void ULactoseConfigCloudServiceSubsystem::LoadConfig()
{
	auto RestSubsystem = GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	auto RestRequest = FGetConfigRequest::Create(*RestSubsystem);
	RestRequest->SetVerb(Lactose::Rest::Verbs::POST);
	RestRequest->SetUrl(GetServiceBaseUrl() / TEXT("config/config"));
	RestRequest->GetOnResponseReceived2().AddUObject(this, &ThisClass::OnConfigLoaded);
	
	auto GetConfigRequest = MakeShared<FLactoseConfigCloudGetConfigRequest>();
	GetConfigFuture = RestRequest->SetContentAsJsonAndSendAsync(GetConfigRequest);

	UE_LOG(LogLactoseConfigService, Verbose, TEXT("Sent a Get Config request "));
}

void ULactoseConfigCloudServiceSubsystem::OnConfigLoaded(Sr<FGetConfigRequest::FResponseContext> Context)
{
	if (!Context->ResponseContent)
	{
		UE_LOG(LogLactoseConfigService, Error, TEXT("Did not receive config"));
		return;
	}

	if (!Config)
		Config = MakeShared<FLactoseConfigCloudConfig>();

	TSet<FString> ExistingEntries;
	for (const TTuple<FString, Sr<FLactoseConfigCloudEntry>>& ExistingEntry : GetConfig()->GetEntries())
		ExistingEntries.Add(ExistingEntry.Key);

	for (const TTuple<FString, FString>& Entry : Context->ResponseContent->Entries)
	{
		ExistingEntries.Remove(Entry.Key);
		Config->AddOrUpdateEntry(Entry.Key, Entry.Value);
	}

	// Anything in Existing Entries is an entry that no longer exists.
	for (const FString& LingeringEntry : ExistingEntries)
		Config->DeleteEntry(LingeringEntry);

	UE_LOG(LogLactoseConfigService, Verbose, TEXT("Config loaded with %d entries"), Config->GetEntries().Num());

	Lactose::Config::Events::OnConfigLoaded.Broadcast(*this, Config.ToSharedRef());
}

void ULactoseConfigCloudServiceSubsystem::OnUserLoggedIn(
	const ULactoseIdentityServiceSubsystem& Sender,
	const Sr<FLactoseIdentityGetUserResponse>& User)
{
	if (bAutoRetrieveConfig)
		LoadConfig();
}
