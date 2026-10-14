#pragma once
#include "CoreMinimal.h"
#include "ECardLocationEnum.generated.h"

UENUM(BlueprintType)
enum class ECardLocationEnum : uint8 {
    NotAvailable,
    Deck_Left,
    Deck_Right,
    Hand_Left,
    Hand_Right,
    Board_HQLeft,
    Board_HQRight,
    Board_Frontline,
    Discard,
    Deck,
};

