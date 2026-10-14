#pragma once
#include "CoreMinimal.h"
#include "EGameplayRestrictions.generated.h"

UENUM(BlueprintType)
enum class EGameplayRestrictions : uint8 {
    cannotDrawCardAtTurnStart,
    cannotKreditSlotAtTurnStart,
    cannotPlayOrders,
    cannotDeployUnits,
    cannotAttackWithGroundUnits,
    cannotDiscardAnyCardFromHand,
    NotAvailable,
};

