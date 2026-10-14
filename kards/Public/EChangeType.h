#pragma once
#include "CoreMinimal.h"
#include "EChangeType.generated.h"

UENUM(BlueprintType)
enum class EChangeType : uint8 {
    tempBuffGive,
    permBuff,
    SetValue,
    Suppress,
    tempBuffRemove,
    veteranSet,
    customAdd,
    customRemove,
    combatModify,
    notUsed,
};

