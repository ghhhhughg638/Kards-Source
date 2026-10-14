#pragma once

#include "CoreMinimal.h"
#include "OnTamperingDetectedDelegate.generated.h"

UDELEGATE(BlueprintCallable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnTamperingDetected,
    const FString&,
    reportReason);
