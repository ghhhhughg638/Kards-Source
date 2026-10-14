// Local editor compatibility declaration generated from the official UHT header dump.
// Reflection only: the shipped game supplies the canonical native implementation.
// Do not package or distribute this project's kards module DLL.
#pragma once
#include "CoreMinimal.h"
#include "EUGCTextFilterPolicy.generated.h"

UENUM(BlueprintType)
enum class EUGCTextFilterPolicy : uint8 {
    Mask,
    Reject,
};


