#pragma once
#include "CoreMinimal.h"
#include "ERarityEnum.generated.h"

UENUM(BlueprintType)
enum class ERarityEnum : uint8 {
    NotAvailable,
    Common,
    Uncommon,
    Rare,
    Unique,
    Legendary,
};

