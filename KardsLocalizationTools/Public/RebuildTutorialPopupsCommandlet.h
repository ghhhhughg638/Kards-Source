#pragma once

#include "Commandlets/Commandlet.h"
#include "RebuildTutorialPopupsCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API URebuildTutorialPopupsCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    URebuildTutorialPopupsCommandlet();
    virtual int32 Main(const FString& Params) override;
};
