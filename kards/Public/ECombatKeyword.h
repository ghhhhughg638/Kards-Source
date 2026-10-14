#pragma once
#include "CoreMinimal.h"
#include "ECombatKeyword.generated.h"

UENUM(BlueprintType)
enum class ECombatKeyword : uint8 {
    None,
    Ambush,
    Blitz,
    Fury,
    Guard,
    HeavyArmor,
    Shock,
    Smokescreen,
};

