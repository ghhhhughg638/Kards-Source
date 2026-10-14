#pragma once

#include "CoreMinimal.h"
#include "BlueprintJsonValue.generated.h"

/** Opaque JSON value used by the game's BlueprintJsonLibrary nodes. */
USTRUCT(BlueprintType)
struct FBlueprintJsonValue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString SerializedJson;
};

/** Value-kind enum exposed by BlueprintJsonLibrary::JsonType. */
UENUM(BlueprintType)
enum class EJsonType : uint8
{
    None = 0,
    Null = 1,
    String = 2,
    Number = 3,
    Boolean = 4,
    Array = 5,
    Object = 6,
};
