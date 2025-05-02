#include "Example/CatalystExampleUsers.h"

#include "CatalystJson.h"
#include "Templates/SharedPointer.h"


TArray<uint8> FCatalystExampleQueryUsersRequest::ToBytes(const FCatalystExampleQueryUsersRequest& Object)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::StructToJsonObject(Object);
    return JsonObject ? Catalyst::Json::JsonObjectToBytes(JsonObject.ToSharedRef()) : TArray<uint8>();
}

TArray<uint8> FCatalystExampleQueryUsersRequest::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleQueryUsersRequest> FCatalystExampleQueryUsersRequest::FromBytes(const TArray<uint8>& Bytes)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::BytesToJsonObject(Bytes);
    return JsonObject ? Catalyst::Json::JsonObjectToStruct<FCatalystExampleQueryUsersRequest>(JsonObject.ToSharedRef()) : TOptional<FCatalystExampleQueryUsersRequest>();
}

TArray<uint8> FCatalystExampleQueryUsersResponse::ToBytes(const FCatalystExampleQueryUsersResponse& Object)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::StructToJsonObject(Object);
    return JsonObject ? Catalyst::Json::JsonObjectToBytes(JsonObject.ToSharedRef()) : TArray<uint8>();
}

TArray<uint8> FCatalystExampleQueryUsersResponse::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleQueryUsersResponse> FCatalystExampleQueryUsersResponse::FromBytes(const TArray<uint8>& Bytes)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::BytesToJsonObject(Bytes);
    return JsonObject ? Catalyst::Json::JsonObjectToStruct<FCatalystExampleQueryUsersResponse>(JsonObject.ToSharedRef()) : TOptional<FCatalystExampleQueryUsersResponse>();
}

TArray<uint8> FCatalystExampleGetUserRequest::ToBytes(const FCatalystExampleGetUserRequest& Object)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::StructToJsonObject(Object);
    return JsonObject ? Catalyst::Json::JsonObjectToBytes(JsonObject.ToSharedRef()) : TArray<uint8>();
}

TArray<uint8> FCatalystExampleGetUserRequest::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleGetUserRequest> FCatalystExampleGetUserRequest::FromBytes(const TArray<uint8>& Bytes)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::BytesToJsonObject(Bytes);
    return JsonObject ? Catalyst::Json::JsonObjectToStruct<FCatalystExampleGetUserRequest>(JsonObject.ToSharedRef()) : TOptional<FCatalystExampleGetUserRequest>();
}

TArray<uint8> FCatalystExampleGetUserResponse::ToBytes(const FCatalystExampleGetUserResponse& Object)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::StructToJsonObject(Object);
    return JsonObject ? Catalyst::Json::JsonObjectToBytes(JsonObject.ToSharedRef()) : TArray<uint8>();
}

TArray<uint8> FCatalystExampleGetUserResponse::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleGetUserResponse> FCatalystExampleGetUserResponse::FromBytes(const TArray<uint8>& Bytes)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::BytesToJsonObject(Bytes);
    return JsonObject ? Catalyst::Json::JsonObjectToStruct<FCatalystExampleGetUserResponse>(JsonObject.ToSharedRef()) : TOptional<FCatalystExampleGetUserResponse>();
}

TArray<uint8> FCatalystExampleUserEvent::ToBytes(const FCatalystExampleUserEvent& Object)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::StructToJsonObject(Object);
    return JsonObject ? Catalyst::Json::JsonObjectToBytes(JsonObject.ToSharedRef()) : TArray<uint8>();
}

TArray<uint8> FCatalystExampleUserEvent::ToBytes() const
{
    return ToBytes(*this);
}

TOptional<FCatalystExampleUserEvent> FCatalystExampleUserEvent::FromBytes(const TArray<uint8>& Bytes)
{
    TSharedPtr<FJsonObject> JsonObject = Catalyst::Json::BytesToJsonObject(Bytes);
    return JsonObject ? Catalyst::Json::JsonObjectToStruct<FCatalystExampleUserEvent>(JsonObject.ToSharedRef()) : TOptional<FCatalystExampleUserEvent>();
}
