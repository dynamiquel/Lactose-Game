#include "CatalystJson.h"

#include "UObject/PropertyOptional.h"

const Catalyst::Json::CustomExportCallback Catalyst::Json::ExportCallback = Catalyst::Json::CustomExportCallback::CreateLambda(
	[](FProperty* Prop, const void* Data) -> TSharedPtr<FJsonValue>
{
	if (FOptionalProperty* OptionalProperty = CastField<FOptionalProperty>(Prop))
	{
		if (!OptionalProperty->IsSet(Data))
			return MakeShared<FJsonValueNull>();
		
		return FJsonObjectConverter::UPropertyToJsonValue(
			OptionalProperty->GetValueProperty(),
				Data,
				/* CheckFlags */ 0,
				/* SkipFlags */ 0,
				&Catalyst::Json::ExportCallback,
				OptionalProperty
		);
	}

	return nullptr;
});

const Catalyst::Json::CustomImportCallback Catalyst::Json::ImportCallback = Catalyst::Json::CustomImportCallback::CreateLambda(
	[](const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* Value) -> bool
{
	if (FOptionalProperty* OptionalProperty = CastField<FOptionalProperty>(Property))
	{
		if (!JsonValue || JsonValue->IsNull())
		{
			OptionalProperty->ClearValue(Value);
			return true;
		}

		FText Error;
		
		return FJsonObjectConverter::JsonValueToUProperty(
			JsonValue,
			OptionalProperty->GetValueProperty(),
			OptionalProperty->MarkSetAndGetInitializedValuePointerToReplace(Value),
			/* CheckFlags */ 0,
			/* SkipFlags */ 0,
			/* bStrictMode */ false,
			&Error,
			&ImportCallback
		);
	}

	return false;
});

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
