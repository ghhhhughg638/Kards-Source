// Local editor compatibility declaration generated from the official UHT header dump.
// Reflection only; the shipped game owns the canonical native type.
#pragma once
#include "CoreMinimal.h"
#include "OfficerRank.generated.h"

USTRUCT(BlueprintType)
struct FOfficerRank {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 rank;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 elo;
};


