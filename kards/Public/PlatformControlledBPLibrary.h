#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PlatformControlledBPLibrary.generated.h"

/** Reflection-only platform helper declarations used by restored cards. */
UCLASS(Blueprintable)
class KARDS_API UPlatformControlledBPLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UPlatformControlledBPLibrary();

};
