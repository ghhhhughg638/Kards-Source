#pragma once
#include "CoreMinimal.h"
#include "CardsGivingAbility.generated.h"

USTRUCT(BlueprintType)
struct FCardsGivingAbility {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<int32> CardsGivingAbility;
    
    KARDS_API FCardsGivingAbility();
};

