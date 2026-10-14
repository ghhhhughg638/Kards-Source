#pragma once

#include "Commandlets/Commandlet.h"
#include "CreateEffectPlaceholdersCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API UCreateEffectPlaceholdersCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UCreateEffectPlaceholdersCommandlet();
    virtual int32 Main(const FString& Params) override;
};
