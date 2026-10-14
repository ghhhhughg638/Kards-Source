#include "BlueprintJsonLibrary.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
TSharedPtr<FJsonObject> ParseObject(const FString& Raw)
{
    TSharedPtr<FJsonObject> Object;
    if (Raw.IsEmpty())
    {
        return Object;
    }

    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
    FJsonSerializer::Deserialize(Reader, Object);
    return Object;
}

TSharedPtr<FJsonValue> ParseValue(const FString& Raw)
{
    TSharedPtr<FJsonValue> Value;
    if (Raw.IsEmpty())
    {
        return Value;
    }

    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
    FJsonSerializer::Deserialize(Reader, Value);
    return Value;
}

FString SerializeObject(const TSharedPtr<FJsonObject>& Object)
{
    if (!Object.IsValid())
    {
        return TEXT("{}");
    }

    FString Result;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
    FJsonSerializer::Serialize(Object.ToSharedRef(), Writer);
    return Result;
}

FString SerializeValue(const TSharedPtr<FJsonValue>& Value)
{
    if (!Value.IsValid())
    {
        return TEXT("null");
    }

    FString Result;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
    FJsonSerializer::Serialize(Value, TEXT(""), Writer);
    return Result;
}

FBlueprintJsonValue WrapJsonValue(const TSharedPtr<FJsonValue>& Value)
{
    FBlueprintJsonValue Result;
    Result.SerializedJson = SerializeValue(Value);
    return Result;
}

EJsonType ToBlueprintJsonType(const TSharedPtr<FJsonValue>& Value)
{
    if (!Value.IsValid())
    {
        return EJsonType::None;
    }

    switch (Value->Type)
    {
    case EJson::Null: return EJsonType::Null;
    case EJson::String: return EJsonType::String;
    case EJson::Number: return EJsonType::Number;
    case EJson::Boolean: return EJsonType::Boolean;
    case EJson::Array: return EJsonType::Array;
    case EJson::Object: return EJsonType::Object;
    default: return EJsonType::None;
    }
}
}

FBlueprintJsonValue UBlueprintJsonLibrary::Conv_JsonObjectToJsonValue(
    const FBlueprintJsonObject& JsonObject,
    const FString& FieldName)
{
    const TSharedPtr<FJsonObject> Object = ParseObject(JsonObject.SerializedJson);
    const TSharedPtr<FJsonValue> Field = Object.IsValid()
        ? Object->TryGetField(FieldName)
        : TSharedPtr<FJsonValue>();
    return WrapJsonValue(Field);
}

FBlueprintJsonObject UBlueprintJsonLibrary::Conv_JsonValueToObject(
    const FBlueprintJsonValue& JsonValue)
{
    FBlueprintJsonObject Result;
    const TSharedPtr<FJsonValue> Value = ParseValue(JsonValue.SerializedJson);
    const TSharedPtr<FJsonObject>* Object = nullptr;
    if (Value.IsValid() && Value->TryGetObject(Object) && Object != nullptr && Object->IsValid())
    {
        Result.SerializedJson = SerializeObject(*Object);
    }
    return Result;
}

TArray<FBlueprintJsonValue> UBlueprintJsonLibrary::Conv_JsonValueToArray(
    const FBlueprintJsonValue& JsonValue)
{
    TArray<FBlueprintJsonValue> Result;
    const TSharedPtr<FJsonValue> Value = ParseValue(JsonValue.SerializedJson);
    const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
    if (Value.IsValid() && Value->TryGetArray(Array) && Array != nullptr)
    {
        Result.Reserve(Array->Num());
        for (const TSharedPtr<FJsonValue>& Entry : *Array)
        {
            Result.Add(WrapJsonValue(Entry));
        }
    }
    return Result;
}

bool UBlueprintJsonLibrary::Conv_JsonValueToBool(const FBlueprintJsonValue& JsonValue)
{
    const TSharedPtr<FJsonValue> Value = ParseValue(JsonValue.SerializedJson);
    bool Result = false;
    return Value.IsValid() && Value->TryGetBool(Result) ? Result : false;
}

float UBlueprintJsonLibrary::Conv_JsonValueToFloat(const FBlueprintJsonValue& JsonValue)
{
    const TSharedPtr<FJsonValue> Value = ParseValue(JsonValue.SerializedJson);
    float Result = 0.0f;
    return Value.IsValid() && Value->TryGetNumber(Result) ? Result : 0.0f;
}

int32 UBlueprintJsonLibrary::Conv_JsonValueToInteger(const FBlueprintJsonValue& JsonValue)
{
    const TSharedPtr<FJsonValue> Value = ParseValue(JsonValue.SerializedJson);
    int32 Result = 0;
    return Value.IsValid() && Value->TryGetNumber(Result) ? Result : 0;
}

FString UBlueprintJsonLibrary::Conv_JsonValueToString(const FBlueprintJsonValue& JsonValue)
{
    const TSharedPtr<FJsonValue> Value = ParseValue(JsonValue.SerializedJson);
    FString Result;
    return Value.IsValid() && Value->TryGetString(Result) ? Result : FString();
}

FBlueprintJsonObject UBlueprintJsonLibrary::Conv_StringToJsonObject(const FString& InJsonRaw)
{
    FBlueprintJsonObject Result;
    if (const TSharedPtr<FJsonObject> Object = ParseObject(InJsonRaw))
    {
        Result.SerializedJson = SerializeObject(Object);
    }
    return Result;
}

FString UBlueprintJsonLibrary::Conv_JsonObjectToString(
    const FBlueprintJsonObject& JsonObject)
{
    if (const TSharedPtr<FJsonObject> Object = ParseObject(JsonObject.SerializedJson))
    {
        return SerializeObject(Object);
    }
    return TEXT("{}");
}

bool UBlueprintJsonLibrary::JsonHasField(
    const FBlueprintJsonObject& JsonObject,
    const FString& FieldName)
{
    const TSharedPtr<FJsonObject> Object = ParseObject(JsonObject.SerializedJson);
    return Object.IsValid() && Object->HasField(FieldName);
}

bool UBlueprintJsonLibrary::JsonHasTypedField(
    const FBlueprintJsonObject& JsonObject,
    const FString& FieldName,
    EJsonType ExpectedType)
{
    const TSharedPtr<FJsonObject> Object = ParseObject(JsonObject.SerializedJson);
    return Object.IsValid()
        && Object->HasField(FieldName)
        && ToBlueprintJsonType(Object->TryGetField(FieldName)) == ExpectedType;
}

bool UBlueprintJsonLibrary::JsonIsNull(const FBlueprintJsonValue& JsonValue)
{
    const TSharedPtr<FJsonValue> Value = ParseValue(JsonValue.SerializedJson);
    return !Value.IsValid() || Value->IsNull();
}

FBlueprintJsonObject UBlueprintJsonLibrary::JsonMake()
{
    return FBlueprintJsonObject();
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeArray(
    const TArray<FBlueprintJsonValue>& Values)
{
    TArray<TSharedPtr<FJsonValue>> Array;
    Array.Reserve(Values.Num());
    for (const FBlueprintJsonValue& Entry : Values)
    {
        if (const TSharedPtr<FJsonValue> Value = ParseValue(Entry.SerializedJson))
        {
            Array.Add(Value);
        }
        else
        {
            Array.Add(MakeShared<FJsonValueNull>());
        }
    }

    const TSharedPtr<FJsonValue> JsonArray = MakeShared<FJsonValueArray>(MoveTemp(Array));
    return WrapJsonValue(JsonArray);
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeBool(bool Value)
{
    const TSharedPtr<FJsonValue> JsonBool = MakeShared<FJsonValueBoolean>(Value);
    return WrapJsonValue(JsonBool);
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeInt(int32 Value)
{
    const TSharedPtr<FJsonValue> JsonNumber = MakeShared<FJsonValueNumber>(static_cast<double>(Value));
    return WrapJsonValue(JsonNumber);
}

FBlueprintJsonValue UBlueprintJsonLibrary::JsonMakeString(const FString& Value)
{
    const TSharedPtr<FJsonValue> JsonString = MakeShared<FJsonValueString>(Value);
    return WrapJsonValue(JsonString);
}

FBlueprintJsonObject UBlueprintJsonLibrary::JsonMakeField(
    const FBlueprintJsonObject& JsonObject,
    const FString& FieldName,
    const FBlueprintJsonValue& Value)
{
    TSharedPtr<FJsonObject> Object = ParseObject(JsonObject.SerializedJson);
    if (!Object.IsValid())
    {
        Object = MakeShared<FJsonObject>();
    }
    Object->SetField(FieldName, ParseValue(Value.SerializedJson));

    FBlueprintJsonObject Result;
    Result.SerializedJson = SerializeObject(Object);
    return Result;
}

FBlueprintJsonObject UBlueprintJsonLibrary::JsonRemoveField(
    const FBlueprintJsonObject& JsonObject,
    const FString& FieldName)
{
    TSharedPtr<FJsonObject> Object = ParseObject(JsonObject.SerializedJson);
    if (!Object.IsValid())
    {
        Object = MakeShared<FJsonObject>();
    }
    Object->RemoveField(FieldName);

    FBlueprintJsonObject Result;
    Result.SerializedJson = SerializeObject(Object);
    return Result;
}

EJsonType UBlueprintJsonLibrary::JsonType(const FBlueprintJsonValue& JsonValue)
{
    return ToBlueprintJsonType(ParseValue(JsonValue.SerializedJson));
}
