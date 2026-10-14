#pragma once

#include "Commandlets/Commandlet.h"
#include "CreateAudioPlaceholdersCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API UCreateAudioPlaceholdersCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UCreateAudioPlaceholdersCommandlet();
    virtual int32 Main(const FString& Params) override;
};
