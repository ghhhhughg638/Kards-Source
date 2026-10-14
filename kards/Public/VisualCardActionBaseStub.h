#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//CROSS-MODULE INCLUDE V2: -ModuleName=Engine -ObjectName=Actor -FallbackName=Actor
#include "VisualCardActionBaseStub.generated.h"

UCLASS(Blueprintable)
class AVisualCardActionBaseStub : public AActor {
    GENERATED_BODY()
public:
    AVisualCardActionBaseStub(const FObjectInitializer& ObjectInitializer);

};


