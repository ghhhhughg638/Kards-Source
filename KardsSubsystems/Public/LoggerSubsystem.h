#pragma once

#include "CoreMinimal.h"
#include "KardsSubsystemBase.h"
#include "LoggerSubsystem.generated.h"

/**
 * Editor-only reflection shim for the native KARDS class that exists in the
 * shipped game. The final mod contains only cooked Blueprint assets; this
 * class is not a replacement for the game's implementation. The module is
 * Runtime so generated card blueprints are not classified as editor-only
 * function callers.
 */
UCLASS(Blueprintable)
class KARDSSUBSYSTEMS_API ULoggerSubsystem : public UKardsSubsystemBase
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void LogPlayerEvent(const FString& EventType, const FString& Data);

    UFUNCTION(BlueprintCallable)
    void LogError(const FString& Error, bool skipScreen);
};
