// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "LactoseConfigCloudRequests.h"
#include "Services/LactoseServiceSubsystem.h"
#include "LactoseConfigCloudServiceSubsystem.generated.h"

struct FLactoseIdentityGetUserResponse;
class ULactoseIdentityServiceSubsystem;

class LACTOSEGAME_API FLactoseConfigCloudEntry
{
public:
	FLactoseConfigCloudEntry(const FString& InKey, const FString& InValue)
		: Key(InKey)
		, Value(InValue)
	{}
	
	const FString& GetString() const { return Value; }

	template<typename T>
	const T* Get() const
	{
		if (auto* CachedStructPtr = CachedStruct.GetPtr<T>())
			return CachedStructPtr;

		T TempStruct;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(GetString(), OUT &TempStruct))
			CachedStruct.InitializeAs(T::StaticStruct(), reinterpret_cast<uint8*>(&TempStruct));

		return reinterpret_cast<T const*>(CachedStruct.GetMemory());
	}

	template<typename T>
	const T& GetRef() const
	{
		auto* StructPtr = Get<T>();
		check(StructPtr);
		return *StructPtr;
	}

	const UScriptStruct* GetCachedScriptStruct() const { return CachedStruct.GetScriptStruct(); }

	void UpdateValue(const FString& NewValue);

private:
	FString Key;
	FString Value;
	mutable FInstancedStruct CachedStruct;
};

class LACTOSEGAME_API FLactoseConfigCloudConfig
{
public:
	const TMap<FString, Sr<FLactoseConfigCloudEntry>>& GetEntries() const { return Entries; }
	Sp<const FLactoseConfigCloudEntry> FindEntry(const FString& EntryId) const;
	Sp<FLactoseConfigCloudEntry> FindEntry(const FString& EntryId);

	void AddOrUpdateEntry(const FString& EntryId, const FString& EntryValue);
	void DeleteEntry(const FString& EntryId);

private:
	TMap<FString, Sr<FLactoseConfigCloudEntry>> Entries;
};

UENUM()
enum class ELactoseConfigCloudStatus
{
	None,
	Retrieving,
	Loaded
};

/**
 * 
 */
UCLASS()
class LACTOSEGAME_API ULactoseConfigCloudServiceSubsystem : public ULactoseServiceSubsystem
{
	GENERATED_BODY()

	ULactoseConfigCloudServiceSubsystem();

	// Begin override ULactoseServiceSubsystem
	void Initialize(FSubsystemCollectionBase& Collection) override;
	// End override ULactoseServiceSubsystem

public:
	ELactoseConfigCloudStatus GetStatus() const;
	Sp<const FLactoseConfigCloudConfig> GetConfig() const { return Config; }
	void LoadConfig();

protected:
	void OnConfigLoaded(Sr<FGetConfigRequest::FResponseContext> Context);
	
	void OnUserLoggedIn(
		const ULactoseIdentityServiceSubsystem& Sender,
		const Sr<FLactoseIdentityGetUserResponse>& User);

private:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bAutoRetrieveConfig = true;

	TFuture<Sp<FGetConfigRequest::FResponseContext>> GetConfigFuture;

	Sp<FLactoseConfigCloudConfig> Config;
};

namespace Lactose::Config::Events
{
	DECLARE_MULTICAST_DELEGATE_TwoParams(FConfigLoaded,
		const ULactoseConfigCloudServiceSubsystem& /* Sender */,
		Sr<FLactoseConfigCloudConfig> /* Config */);

	inline FConfigLoaded OnConfigLoaded;
}
