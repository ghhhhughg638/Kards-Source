// Local editor compatibility declaration generated from the official UHT header dump.
// Reflection only; the shipped game owns the canonical native type.
#pragma once
#include "CoreMinimal.h"
#include "Officer.h"
#include "OfficerArray.h"
#include "OfficerRank.h"
#include "OfficerClubData.generated.h"

USTRUCT(BlueprintType)
struct FOfficerClubData {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<FOfficer> officers;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FOfficerRank rank;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 current_season;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<int32> world_cup_qualifiers;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TMap<int32, FOfficerArray> ranked_leaderboard;
};


