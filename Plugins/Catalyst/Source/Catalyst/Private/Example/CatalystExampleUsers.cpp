#include "Example/CatalystExampleUsers.h"

#include "JsonObjectConverter.h"
#include "Templates/SharedPointer.h"


TArray<uint8> FCatalystExampleQueryUsersRequest::ToBytes(const FCatalystExampleQueryUsersRequest& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleQueryUsersRequest' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleQueryUsersRequest' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleQueryUsersRequest::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleQueryUsersRequest> FCatalystExampleQueryUsersRequest::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleQueryUsersRequest' into a JSON object"));
        return {};
    }
    
    FCatalystExampleQueryUsersRequest DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleQueryUsersRequest>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleQueryUsersRequest' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleQueryUsersResponse::ToBytes(const FCatalystExampleQueryUsersResponse& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleQueryUsersResponse' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleQueryUsersResponse' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleQueryUsersResponse::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleQueryUsersResponse> FCatalystExampleQueryUsersResponse::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleQueryUsersResponse' into a JSON object"));
        return {};
    }
    
    FCatalystExampleQueryUsersResponse DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleQueryUsersResponse>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleQueryUsersResponse' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleGetUserRequest::ToBytes(const FCatalystExampleGetUserRequest& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleGetUserRequest' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleGetUserRequest' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleGetUserRequest::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleGetUserRequest> FCatalystExampleGetUserRequest::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleGetUserRequest' into a JSON object"));
        return {};
    }
    
    FCatalystExampleGetUserRequest DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleGetUserRequest>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleGetUserRequest' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleGetUserResponse::ToBytes(const FCatalystExampleGetUserResponse& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleGetUserResponse' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleGetUserResponse' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleGetUserResponse::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleGetUserResponse> FCatalystExampleGetUserResponse::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleGetUserResponse' into a JSON object"));
        return {};
    }
    
    FCatalystExampleGetUserResponse DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleGetUserResponse>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleGetUserResponse' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}

TArray<uint8> FCatalystExampleUserEvent::ToBytes(const FCatalystExampleUserEvent& Object)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::ToJsonBytes)
    
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Object);
    if (!JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise object 'FCatalystExampleUserEvent' into a JSON object"));
        return {};
    }
    
    TArray<uint8> Buffer;
    FMemoryWriter MemoryWriter(Buffer);
    TSharedRef<TJsonWriter<UTF8CHAR>> JsonWriter = TJsonWriterFactory<UTF8CHAR>::Create(&MemoryWriter);
    if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not serialise JSON object 'FCatalystExampleUserEvent' into bytes"));
        return {};
    }
    
    return Buffer;
}

TArray<uint8> FCatalystExampleUserEvent::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleUserEvent> FCatalystExampleUserEvent::FromBytes(const TArray<uint8>& Bytes)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(Catalyst::FromJsonBytes)
    
    FMemoryReader MemoryReader(Bytes);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<UTF8CHAR>> JsonReader = TJsonReaderFactory<UTF8CHAR>::Create(&MemoryReader);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the given bytes for 'FCatalystExampleUserEvent' into a JSON object"));
        return {};
    }
    
    FCatalystExampleUserEvent DeserialisedObject;
    FText FailReason;
    
    bool bConvertedToStruct;
    {
        FGCScopeGuard LockGC;
        bConvertedToStruct = FJsonObjectConverter::JsonObjectToUStruct<FCatalystExampleUserEvent>(
            JsonObject.ToSharedRef(),
            &DeserialisedObject,
            /* CheckFlags */ 0,
            /* SkipFlags */ 0,
            /* bStrictMode */ false,
            OUT &FailReason);
    }
    
    if (!bConvertedToStruct)
    {
        UE_LOG(LogSerialization, Error, TEXT("Could not deserialise the JSON object into an 'FCatalystExampleUserEvent' object. Reason: %s"), *FailReason.ToString());
        return {};
    }
    
    return DeserialisedObject;
}
