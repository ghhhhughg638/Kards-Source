#pragma once

#include "Commandlets/Commandlet.h"
#include "RebuildTextNotificationsCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API URebuildTextNotificationsCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    URebuildTextNotificationsCommandlet();
    virtual int32 Main(const FString& Params) override;
};
