#pragma once
#include "CoreMinimal.h"
#include "EFactionEnum.generated.h"

UENUM(BlueprintType)
enum class EFactionEnum : uint8 {
    NotAvailable,
    Germany,
    Britain,
    Japan,
    Soviet,
    USA,
    France,
    Italy,
    Poland,
    Finland,
    Anzac,
    Allies,
    Neutral,
};

