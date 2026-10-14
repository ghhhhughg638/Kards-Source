#pragma once

#include "CoreMinimal.h"
#include "KardsSubsystemBase.h"
#include "StoreSubsystem.generated.h"

/**
 * Reflection-only compatibility shell for the native KARDS store subsystem.
 * Only the signatures that restored Blueprints call are declared here.
 */
UCLASS(Blueprintable)
class KARDSSUBSYSTEMS_API UStoreSubsystem : public UKardsSubsystemBase
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static FString GetBattlePassName(int32 addMonths);
};
