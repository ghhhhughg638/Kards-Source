#pragma once
#include "CoreMinimal.h"
#include "EDurationPolicy.generated.h"

UENUM(BlueprintType)
enum class EDurationPolicy : uint8 {
    None,
    Instant,
    Duration,
    Ongoing,
    Static,
};

