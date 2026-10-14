#pragma once
#include "CoreMinimal.h"
#include "EVisualCardWeight.generated.h"

UENUM(BlueprintType)
enum class EVisualCardWeight : uint8 {
    None,
    Light,
    Medium,
    Heavy,
};

