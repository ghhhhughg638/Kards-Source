// Local editor compatibility declaration generated from the official UHT header dump.
// Reflection only; the shipped game owns the canonical native type.
#pragma once
#include "CoreMinimal.h"
#include "PreviousOpponent.generated.h"

USTRUCT(BlueprintType)
struct FPreviousOpponent {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 PlayerID;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString PlayerName;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 PlayerTag;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString deck;
};


