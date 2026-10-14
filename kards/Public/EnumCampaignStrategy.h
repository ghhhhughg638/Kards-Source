#pragma once
#include "CoreMinimal.h"
#include "EnumCampaignStrategy.generated.h"

UENUM(BlueprintType)
enum class EnumCampaignStrategy : uint8 {
    NotAvailable,
    Strategy_0,
    Strategy_1,
    Strategy_2,
    EnumCampaignStrategy_MAX = 4 UMETA(Hidden),
};

