#pragma once
#include "CoreMinimal.h"
#include "ECampaignStarStatus.generated.h"

UENUM(BlueprintType)
enum class ECampaignStarStatus : uint8 {
    StarConditionMet,
    StarConditionNotMet,
    StarFailed,
    StarCompleted,
};

