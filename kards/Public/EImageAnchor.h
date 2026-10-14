#pragma once
#include "CoreMinimal.h"
#include "EImageAnchor.generated.h"

UENUM(BlueprintType)
enum class EImageAnchor : uint8 {
    Middle,
    Top,
    TopMid,
    BotMid,
    Bot,
};

