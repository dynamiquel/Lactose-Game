#include "Example/CatalystExampleRoles.h"

#include "JsonObjectConverter.h"
#include "Templates/SharedPointer.h"


TArray<uint8> FCatalystExampleQueryRolesRequest::ToBytes(const FCatalystExampleQueryRolesRequest& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleQueryRolesRequest' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleQueryRolesRequest' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleQueryRolesRequest::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleQueryRolesRequest> FCatalystExampleQueryRolesRequest::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleQueryRolesRequest' into a JSON object"));
        return {};
    }
    
    FCatalystExampleQueryRolesRequest DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleQueryRolesRequest>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleQueryRolesRequest' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleQueryRolesResponse::ToBytes(const FCatalystExampleQueryRolesResponse& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleQueryRolesResponse' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleQueryRolesResponse' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleQueryRolesResponse::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleQueryRolesResponse> FCatalystExampleQueryRolesResponse::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleQueryRolesResponse' into a JSON object"));
        return {};
    }
    
    FCatalystExampleQueryRolesResponse DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleQueryRolesResponse>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleQueryRolesResponse' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleGetRolesRequest::ToBytes(const FCatalystExampleGetRolesRequest& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleGetRolesRequest' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleGetRolesRequest' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleGetRolesRequest::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleGetRolesRequest> FCatalystExampleGetRolesRequest::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleGetRolesRequest' into a JSON object"));
        return {};
    }
    
    FCatalystExampleGetRolesRequest DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleGetRolesRequest>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleGetRolesRequest' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleGetRoleResponse::ToBytes(const FCatalystExampleGetRoleResponse& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleGetRoleResponse' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleGetRoleResponse' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleGetRoleResponse::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleGetRoleResponse> FCatalystExampleGetRoleResponse::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleGetRoleResponse' into a JSON object"));
        return {};
    }
    
    FCatalystExampleGetRoleResponse DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleGetRoleResponse>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleGetRoleResponse' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleGetRolesResponse::ToBytes(const FCatalystExampleGetRolesResponse& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleGetRolesResponse' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleGetRolesResponse' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleGetRolesResponse::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleGetRolesResponse> FCatalystExampleGetRolesResponse::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleGetRolesResponse' into a JSON object"));
        return {};
    }
    
    FCatalystExampleGetRolesResponse DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleGetRolesResponse>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleGetRolesResponse' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleCreateRoleRequest::ToBytes(const FCatalystExampleCreateRoleRequest& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleCreateRoleRequest' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleCreateRoleRequest' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleCreateRoleRequest::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleCreateRoleRequest> FCatalystExampleCreateRoleRequest::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleCreateRoleRequest' into a JSON object"));
        return {};
    }
    
    FCatalystExampleCreateRoleRequest DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleCreateRoleRequest>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleCreateRoleRequest' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleRoleEvent::ToBytes(const FCatalystExampleRoleEvent& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleRoleEvent' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleRoleEvent' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleRoleEvent::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleRoleEvent> FCatalystExampleRoleEvent::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleRoleEvent' into a JSON object"));
        return {};
    }
    
    FCatalystExampleRoleEvent DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleRoleEvent>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleRoleEvent' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}
