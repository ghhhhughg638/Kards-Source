#pragma once

#include "CoreMinimal.h"
#include "BlueprintJsonValue.h"
#include "JsonObjectWrapper.h"
#include "Subsystems/EngineSubsystem.h"
#include "ConfigSubsystem.generated.h"

/**
 * Reflection-only compatibility shell for the native KARDS configuration
 * subsystem.  Card blueprints only need these signatures while restoring;
 * the shipped game provides the runtime implementation.
 */
UCLASS(Blueprintable)
class KARDSSUBSYSTEMS_API UConfigSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfigReady_BP);

    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, EditAnywhere,
        meta=(AllowPrivateAccess=true))
    FOnConfigReady_BP OnConfigReady_BP;

    UConfigSubsystem();

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool GetString(const FString& FieldName, FString& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    FString GetServerConfigRaw() const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    FString GetLocalConfigRaw() const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool GetJsonObject(const FString& FieldName, FJsonObjectWrapper& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetJSONArray(const FString& Key, bool& KeyExists,
        TArray<FBlueprintJsonValue>& Value) const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool GetInt(const FString& FieldName, int32& Value);

    UFUNCTION(BlueprintPure)
    bool GetFloat(const FString& FieldName, double& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool GetDateTime(const FString& FieldName, FDateTime& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool GetBool(const FString& FieldName, bool& Value);
};
