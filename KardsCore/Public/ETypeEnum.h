#pragma once
#include "CoreMinimal.h"
#include "ETypeEnum.generated.h"

UENUM(BlueprintType)
enum class ETypeEnum : uint8 {
    NotAvailable,
    location,
    order,
    tank,
    fighter,
    bomber,
    infantry,
    artillery,
    antiair,
    antitank,
    tankdestroyer,
    gotcha,
    wildcard,
};

