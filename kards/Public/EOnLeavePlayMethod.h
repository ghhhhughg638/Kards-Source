#pragma once
#include "CoreMinimal.h"
#include "EOnLeavePlayMethod.generated.h"

UENUM(BlueprintType)
enum class EOnLeavePlayMethod : uint8 {
    NotAvailable,
    OnDestroyed,
    OnRemoved,
    OnMovedToHand,
    OnMovedToDeck,
    OnChangedOwnership,
    OnConverted,
};

