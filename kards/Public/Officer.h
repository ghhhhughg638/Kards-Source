// Local editor compatibility declaration generated from the official UHT header dump.
// Reflection only; the shipped game owns the canonical native type.
#pragma once
#include "CoreMinimal.h"
#include "EFactionEnum.h"
//CROSS-MODULE INCLUDE V2: -ModuleName=KardsCore -ObjectName=EFactionEnum -FallbackName=EFactionEnum
#include "Officer.generated.h"

USTRUCT(BlueprintType)
struct FOfficer {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 player_id;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString player_name;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 officer_elo;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<EFactionEnum> field_marshals;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    EFactionEnum faction_most_played;
};


