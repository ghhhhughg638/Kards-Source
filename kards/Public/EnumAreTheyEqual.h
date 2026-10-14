#pragma once

#include "CoreMinimal.h"
#include "EnumAreTheyEqual.generated.h"

/** Branch value returned by the game's enum comparison helpers. */
UENUM(BlueprintType)
enum class EnumAreTheyEqual : uint8
{
    Equal,
    NotEqual,
};
