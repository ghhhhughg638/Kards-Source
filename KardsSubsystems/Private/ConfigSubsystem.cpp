#include "ConfigSubsystem.h"

UConfigSubsystem::UConfigSubsystem()
{
}

bool UConfigSubsystem::GetString(const FString& FieldName, FString& Value)
{
    Value.Reset();
    return false;
}

FString UConfigSubsystem::GetServerConfigRaw() const
{
    return FString();
}

FString UConfigSubsystem::GetLocalConfigRaw() const
{
    return FString();
}

bool UConfigSubsystem::GetJsonObject(const FString& FieldName, FJsonObjectWrapper& Value)
{
    Value.JsonObject.Reset();
    return false;
}

void UConfigSubsystem::GetJSONArray(const FString& Key, bool& KeyExists,
    TArray<FBlueprintJsonValue>& Value) const
{
    KeyExists = false;
    Value.Reset();
}

bool UConfigSubsystem::GetInt(const FString& FieldName, int32& Value)
{
    Value = 0;
    return false;
}

bool UConfigSubsystem::GetFloat(const FString& FieldName, double& Value)
{
    Value = 0.0;
    return false;
}

bool UConfigSubsystem::GetDateTime(const FString& FieldName, FDateTime& Value)
{
    Value = FDateTime();
    return false;
}

bool UConfigSubsystem::GetBool(const FString& FieldName, bool& Value)
{
    Value = false;
    return false;
}
