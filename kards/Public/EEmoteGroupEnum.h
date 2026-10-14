#pragma once
#include "CoreMinimal.h"
#include "EEmoteGroupEnum.generated.h"

UENUM(BlueprintType)
enum class EEmoteGroupEnum : uint8 {
    NotAvailable,
    Start,
    End,
    Good,
    Bad,
    Cheer,
    Taunt,
    Poke,
    Proclaim,
};

