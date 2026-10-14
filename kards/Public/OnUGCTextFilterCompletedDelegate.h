// Local editor compatibility declaration generated from the official UHT header dump.
// Reflection only: the shipped game supplies the canonical native implementation.
// Do not package or distribute this project's kards module DLL.
#pragma once
#include "CoreMinimal.h"
#include "NativeUiDataSchema.h"
#include "OnUGCTextFilterCompletedDelegate.generated.h"

UDELEGATE(BlueprintCallable) DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUGCTextFilterCompleted, FUGCTextFilterResult, Result);



