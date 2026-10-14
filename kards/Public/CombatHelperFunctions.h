#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
//CROSS-MODULE INCLUDE V2: -ModuleName=Engine -ObjectName=BlueprintFunctionLibrary -FallbackName=BlueprintFunctionLibrary
#include "ECardLocationEnum.h"
#include "ESideEnum.h"
#include "CombatHelperFunctions.generated.h"

class UObject;

UCLASS(Blueprintable)
class KARDS_API UCombatHelperFunctions : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UCombatHelperFunctions();

    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static bool IsClientPlaying(const UObject* WorldContextObject);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static ESideEnum GetTheOtherSide(ESideEnum SideToGet, const UObject* WorldContextObject);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static ECardLocationEnum GetSupportLineBySide(ESideEnum SideToGet, const UObject* WorldContextObject);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static ESideEnum GetPlayingSide(const UObject* WorldContextObject);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static ECardLocationEnum GetOpponentSideHandLocation(const UObject* WorldContextObject);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static ESideEnum GetOpponentSide(const UObject* WorldContextObject);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static ECardLocationEnum GetHandLocationBySide(ESideEnum SideToGet, const UObject* WorldContextObject);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static ECardLocationEnum GetDeckLocationBySide(ESideEnum SideToGet, const UObject* WorldContextObject);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static ECardLocationEnum GetClientSideHandLocation(const UObject* WorldContextObject);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static ESideEnum GetClientSide(const UObject* WorldContextObject);
    
};

