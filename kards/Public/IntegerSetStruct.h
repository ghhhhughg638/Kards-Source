#pragma once

#include "CoreMinimal.h"
#include "IntegerSetStruct.generated.h"

/** Native editor-compatible form of the game's integerSetStruct. */
USTRUCT(BlueprintType)
struct FintegerSetStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSet<int32> ints_4_45225A814E5C13611C5A2C83E4E77015;
};
