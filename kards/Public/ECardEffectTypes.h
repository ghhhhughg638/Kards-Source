#pragma once
#include "CoreMinimal.h"
#include "ECardEffectTypes.generated.h"

UENUM(BlueprintType)
enum class ECardEffectTypes : uint8 {
    NotAvailable,
    Passive = 9,
    Trigger = 13,
};

