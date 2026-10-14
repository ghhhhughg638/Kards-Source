#pragma once
#include "CoreMinimal.h"
#include "ESideEnum.generated.h"

UENUM(BlueprintType)
enum class ESideEnum : uint8 {
    NotAvailable,
    left,
    right,
};

