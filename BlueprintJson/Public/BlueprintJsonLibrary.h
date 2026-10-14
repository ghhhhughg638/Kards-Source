#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintJsonObject.h"
#include "BlueprintJsonValue.h"
#include "BlueprintJsonLibrary.generated.h"

/**
 * Runtime-compatible declarations for the BlueprintJson functions used by
 * KARDS.  The card project previously contained only the carrier struct, so
 * restored graphs could not resolve any of these native call targets.
 */
UCLASS()
class BLUEPRINTJSON_API UBlueprintJsonLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure)
    static FBlueprintJsonValue Conv_JsonObjectToJsonValue(
        const FBlueprintJsonObject& JsonObject,
        const FString& FieldName);

    UFUNCTION(BlueprintPure)
    static FBlueprintJsonObject Conv_JsonValueToObject(
        const FBlueprintJsonValue& JsonValue);

    UFUNCTION(BlueprintPure)
    static TArray<FBlueprintJsonValue> Conv_JsonValueToArray(
        const FBlueprintJsonValue& JsonValue);

    UFUNCTION(BlueprintPure)
    static bool Conv_JsonValueToBool(const FBlueprintJsonValue& JsonValue);

    UFUNCTION(BlueprintPure)
    static float Conv_JsonValueToFloat(const FBlueprintJsonValue& JsonValue);

    UFUNCTION(BlueprintPure)
    static int32 Conv_JsonValueToInteger(const FBlueprintJsonValue& JsonValue);

    UFUNCTION(BlueprintPure)
    static FString Conv_JsonValueToString(const FBlueprintJsonValue& JsonValue);

    UFUNCTION(BlueprintPure)
    static FBlueprintJsonObject Conv_StringToJsonObject(const FString& InJsonRaw);

    UFUNCTION(BlueprintPure)
    static FString Conv_JsonObjectToString(const FBlueprintJsonObject& JsonObject);

    UFUNCTION(BlueprintPure)
    static bool JsonHasField(
        const FBlueprintJsonObject& JsonObject,
        const FString& FieldName);

    UFUNCTION(BlueprintPure)
    static bool JsonHasTypedField(
        const FBlueprintJsonObject& JsonObject,
        const FString& FieldName,
        EJsonType ExpectedType);

    UFUNCTION(BlueprintPure)
    static bool JsonIsNull(const FBlueprintJsonValue& JsonValue);

    UFUNCTION(BlueprintPure)
    static FBlueprintJsonObject JsonMake();

    UFUNCTION(BlueprintPure)
    static FBlueprintJsonValue JsonMakeArray(
        const TArray<FBlueprintJsonValue>& Values);

    UFUNCTION(BlueprintPure)
    static FBlueprintJsonValue JsonMakeBool(bool Value);

    UFUNCTION(BlueprintPure)
    static FBlueprintJsonValue JsonMakeInt(int32 Value);

    UFUNCTION(BlueprintPure)
    static FBlueprintJsonValue JsonMakeString(const FString& Value);

    UFUNCTION(BlueprintPure)
    static FBlueprintJsonObject JsonMakeField(
        const FBlueprintJsonObject& JsonObject,
        const FString& FieldName,
        const FBlueprintJsonValue& Value);

    UFUNCTION(BlueprintPure)
    static FBlueprintJsonObject JsonRemoveField(
        const FBlueprintJsonObject& JsonObject,
        const FString& FieldName);

    UFUNCTION(BlueprintPure)
    static EJsonType JsonType(const FBlueprintJsonValue& JsonValue);
};
