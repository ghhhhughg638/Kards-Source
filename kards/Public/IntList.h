#pragma once

#include "CoreMinimal.h"
#include "IntList.generated.h"

/** Native compatibility copy of the Blueprint intList struct. */
USTRUCT(BlueprintType)
struct FintList
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<int32> intList_5_2D7F6669409B166D29909DB66C788DC5;
};
