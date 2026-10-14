#pragma once

#include "CoreMinimal.h"
#include "CardIDs.generated.h"

/** Native compatibility copy of the game's CardIDs wrapper struct. */
USTRUCT(BlueprintType)
struct FCardIDs
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<int32> CardIDs;
};
