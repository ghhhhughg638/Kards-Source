#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveGameFunctionLibrary.generated.h"

// Local editor compatibility subset of the native KARDS SaveGameFunctionLibrary
// (official header dump: Public/SaveGameFunctionLibrary.h). Only the accessors
// consumed by restored graphs are declared; the game supplies the real class.
UCLASS(Blueprintable)
class KARDS_API USaveGameFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    static void LoadOtherSettings(bool& IsStreamerMode, bool& SaveFileValid);
};
