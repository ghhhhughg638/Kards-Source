// Local editor compatibility declaration generated from the official UHT header dump.
// Reflection only: the shipped game supplies the canonical native implementation.
// Do not package or distribute this project's kards module DLL.
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
//CROSS-MODULE INCLUDE V2: -ModuleName=Engine -ObjectName=BlueprintAsyncActionBase -FallbackName=BlueprintAsyncActionBase
#include "EUGCTextFilterPolicy.h"
#include "OnUGCTextFilterCompletedDelegate.h"
#include "FilterUGCTextAsyncAction.generated.h"

class UFilterUGCTextAsyncAction;
class UObject;

UCLASS(Blueprintable)
class KARDS_API UFilterUGCTextAsyncAction : public UBlueprintAsyncActionBase {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FOnUGCTextFilterCompleted Completed;
    
    UFilterUGCTextAsyncAction();

    UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"))
    static UFilterUGCTextAsyncAction* FilterUGCText(UObject* WorldContextObject, const FString& Input, EUGCTextFilterPolicy Policy);
    
};


