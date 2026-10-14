#pragma once

#include "Commandlets/Commandlet.h"
#include "RebuildBuffLocalizationCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API URebuildBuffLocalizationCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    URebuildBuffLocalizationCommandlet();
    virtual int32 Main(const FString& Params) override;
};
