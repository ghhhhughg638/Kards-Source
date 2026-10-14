#pragma once
#include "CoreMinimal.h"
#include "EOnEnterPlayMethod.generated.h"

UENUM(BlueprintType)
enum class EOnEnterPlayMethod : uint8 {
    NotAvailable,
    OnPlayedFromHand,
    OnAddedFromHand,
    OnSpawn,
    OnReveal,
    OnConverted,
};

