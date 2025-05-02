#include "CatalystJson.h"

TSharedPtr<FJsonObject> Catalyst::Json::BytesToJsonObject(const TArray<uint8>& JsonData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::Json::BytesToJsonObject);

	FMemoryReader MemoryReader(JsonData);

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
	{
		UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes into a JSON object"));
		return {};
	}

	return JsonObject;
}

TArray<uint8> Catalyst::Json::JsonObjectToBytes(const TSharedRef<FJsonObject>& JsonObject)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::Json::JsonObjectToBytes);

	TArray<uint8> Buffer;
	FMemoryWriter MemoryWriter(Buffer);
	TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
	if (!FJsonSerializer::Serialize(JsonObject, JsonWriter))
	{
		UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object into bytes"));
		return {};
	}

	return Buffer;
}
