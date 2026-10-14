#pragma once
#include "CoreMinimal.h"
#include "EGameplayRestrictions.h"
#include "ESideEnum.h"
#include "GameplayRestrictionEffect.generated.h"

USTRUCT(BlueprintType)
struct FGameplayRestrictionEffect {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ESideEnum AffectedSide;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    EGameplayRestrictions RestrictionType;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 CardID;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 TurnsRemaining;
    
    KARDS_API FGameplayRestrictionEffect();
};

