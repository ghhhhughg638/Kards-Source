#pragma once
#include "CoreMinimal.h"
#include "CardBuffData.generated.h"

USTRUCT(BlueprintType)
struct FCardBuffData {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FText cardName;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TMap<FString, int32> BuffMap;
    
    KARDS_API FCardBuffData();
};

