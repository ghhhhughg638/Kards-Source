#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "RestoreAudioAssetsCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API URestoreAudioAssetsCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    URestoreAudioAssetsCommandlet();

    virtual int32 Main(const FString& Params) override;
};
