#pragma once
#include "CoreMinimal.h"
#include "ChooseOneCardStruct.generated.h"

USTRUCT(BlueprintType)
struct FChooseOneCardStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FText cardText;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool selectTargetOnPlayedFromHand;
    
    KARDS_API FChooseOneCardStruct();
};

