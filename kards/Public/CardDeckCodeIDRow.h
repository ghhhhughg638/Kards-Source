#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CardDeckCodeIDRow.generated.h"

USTRUCT(BlueprintType)
struct FCardDeckCodeIDRow : public FTableRowBase {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString card;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString deck_code_id;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 ID;

    KARDS_API FCardDeckCodeIDRow();
};