#pragma once

#include "JsonObjectConverter.h"

namespace Lactose::Serialisation::Json
{
	template<typename T>
	TOptional<T> Deserialise(const TArray<uint8>& JsonData)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(Lactose::Serialisation::Json::Deserialise);

		FMemoryReader MemoryReader(JsonData);

		Sp<FJsonObject> JsonObject;
		Sr<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
		if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
		{
			UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes into a JSON object"));
			return {};
		}

		T DeserialisedObject;
		FText FailReason;

		bool bConvertedToStruct;
		{
			FGCScopeGuard LockGC;
			bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<T>(
				JsonObject.ToSharedRef(),
				&DeserialisedObject,
				/* CheckFlags */ 0,
				/* SkipFlags */ 0,
				/* bStrictMode */ false,
				OUT &FailReason);
		}

		if (!bConvertedToStruct)
		{
			UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an object. Reason: %s"), *FailReason.ToString());
			return {};
		}

		return DeserialisedObject;
	}

	template<typename T>
	Sp<T> DeserialiseShared(const TArray<uint8>& JsonData)
	{
		TOptional<T> Object = Deserialise<T>(JsonData);
		return Object ? MakeShared<T>(MoveTemp(*Object)).ToSharedPtr() : nullptr;
	}

	template<typename T>
	TArray<uint8> Serialise(const T& Object)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(Lactose::Serialisation::Json::Serialise);

		TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
		if (!JsonObject)
		{
			UE_LOG(LogSerialization, Error, TEXT("Could not serialise object into a JSON object"));
			return {};
		}

		TArray<uint8> Buffer;
		FMemoryWriter MemoryWriter(Buffer);
		TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
		if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
		{
			UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object into bytes"));
			return {};
		}

		return Buffer;
	}

	// TODO: Move
	inline TArray<uint8> StringToBytes(const FString& String)
	{
		const uint64 Utf8Length = FPlatformString::ConvertedLength<UTF8CHAR>(*String, String.Len());
		TArray<uint8> Buffer;
		Buffer.SetNumUninitialized(Utf8Length);
		FPlatformString::Convert(reinterpret_cast<UTF8CHAR*>(Buffer.GetData()), Buffer.Num(), *String, String.Len());
		return Buffer;
	}
}
