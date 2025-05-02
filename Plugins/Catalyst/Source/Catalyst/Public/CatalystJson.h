#pragma once

#include "JsonObjectConverter.h"
#include "Templates/SharedPointer.h"

namespace Catalyst::Json
{
	CATALYST_API TSharedPtr<FJsonObject> BytesToJsonObject(const TArray<uint8>& JsonData);
	CATALYST_API TArray<uint8> JsonObjectToBytes(const TSharedRef<FJsonObject>& JsonObject);

	template<typename T>
	TSharedPtr<FJsonObject> StructToJsonObject(const T& Struct)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::Json::StructToJsonObject);

		TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Struct);
		if (!JsonObject)
			UE_LOG(LogSerialization, Error, TEXT("Could not serialise object into a JSON object"));

		return JsonObject;
	}

	template<typename T>
	TOptional<T> JsonObjectToStruct(const TSharedRef<FJsonObject>& JsonObject)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::Json::JsonObjectToStruct);

		T DeserialisedStruct;
		FText FailReason;

		bool bConvertedToStruct;
		{
			FGCScopeGuard LockGC;
			bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<T>(
				JsonObject,
				&DeserialisedStruct,
				/* CheckFlags */ 0,
				/* SkipFlags */ 0,
				/* bStrictMode */ false,
				OUT &FailReason);
		}

		if (!bConvertedToStruct)
		{
			UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystTestExUser' object. Reason: %s"),
				*FailReason.ToString());
			
			return {};
		}

		return DeserialisedStruct;
	}
}