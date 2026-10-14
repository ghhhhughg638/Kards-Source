#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ECardSetEnum.h"
#include "ECardLocationEnum.h"
#include "EFactionEnum.h"
#include "ESideEnum.h"
#include "ETypeEnum.h"
#include "EnumAreTheyEqual.h"
#include "ECardEffectTypes.h"
#include "EEffectBarTypes.h"
#include "FunctionLibrary.generated.h"

class UBaseCardObject;
class UUserWidget;

// Local compatibility enum for /Script/kards.PlatformType. The usmap exposes
// Desktop=0 and Mobile=1; the compiled game's native module remains canonical.
// Do not distribute this project's kards module DLL; the game supplies these
// native runtime types and implementations.
UENUM(BlueprintType)
enum class PlatformType : uint8
{
    Desktop = 0,
    Mobile = 1,
    PlatformType_MAX = 2 UMETA(Hidden)
};

/**
 * Small compatibility surface for the native KARDS FunctionLibrary class.
 *
 * The shipped game exposes these helpers from /Script/kards.  The mod project
 * only needs their reflected signatures for restored card graphs, but the
 * comparisons are implemented as well so restored cards behave correctly when
 * executed in the editor/runtime shim.
 */
UCLASS(Blueprintable)
class KARDS_API UFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static EEffectBarTypes CardEffectToEffectBar(ECardEffectTypes Type);

    /**
     * Returns the static/default card object registered under CardName.
     *
     * The shipped KARDS runtime provides this lookup from its card registry.
     * The editor shim resolves loaded card classes by their static/default
     * object. The game/editor card loader must have loaded the card class.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="KARDS|Cards")
    static UBaseCardObject* GetStaticCard(FName CardName);

    // These accessors are native host interfaces used by the official
    // BP_CardFunctions graph.  Their exact names, parameter directions, and
    // Blueprint flags are present in the official kards UHT header dump.
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static void GetStaticCampaignName(FName cardName, FText& campaignName);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static void GetStaticCardSet(FName cardName, ECardSetEnum& cardSet);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static void GetStaticKredits(FName cardName, int32& kredits);

    // Used by BP_BaseCard.SetWeightFromAttackAndDefense (EX_CallMath).
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static void GetStaticAttack(FName cardName, int32& attack);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static void GetStaticDefense(FName cardName, int32& defense);

    // Used by BP_BoardCard's EventGraph.
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static void GetStaticOperationCost(FName cardName, int32& operationCost);

    // Used by BP_Board's EventGraph (official header dump: FunctionLibrary.h).
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static UUserWidget* GetOrCreateWidget(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static void GetStaticText(FName cardName, FText& Text);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static bool isConsole();

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static bool IsSimulatingInEditor();

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static void SortCardsByName(TArray<UBaseCardObject*>& cards);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static TArray<FString> SortStringArray(const TArray<FString>& StringArray, const bool Reversed);

    UFUNCTION(BlueprintCallable)
    static void SortIntegerArrayDirectly(UPARAM(Ref) TArray<int32>& IntegerArray, const bool Reversed);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static TArray<int32> SortIntegerArray(const TArray<int32>& IntegerArray, const bool Reversed);

    UFUNCTION(BlueprintCallable)
    static void SortCardsForStore(UPARAM(Ref) TArray<UBaseCardObject*>& cardArray);

    UFUNCTION(BlueprintCallable)
    static void SetStaticCampaignName(FName cardName, FText campaignName);

    // BP_Table bytecode shows a static pure call with a PlatformType out pin
    // named Branches and no input pins. Exact native UFunction flags are not
    // present in the usmap; this declaration is an editor-compatibility guess.
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="KARDS|Platform")
    static void BranchOnPlatformType(PlatformType& Branches);

    // BP_Table bytecode shows a static pure zero-input Boolean result.
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="KARDS|Platform")
    static bool isMobile();

    UFUNCTION(BlueprintCallable)
    static void EnumCompareType(ETypeEnum type1, ETypeEnum type2, EnumAreTheyEqual& Branches);

    UFUNCTION(BlueprintCallable)
    static void EnumCompareSide(ESideEnum side1, ESideEnum side2, EnumAreTheyEqual& Branches);

    UFUNCTION(BlueprintCallable)
    static void EnumCompareFaction(EFactionEnum faction1, EFactionEnum faction2, EnumAreTheyEqual& Branches);

    UFUNCTION(BlueprintCallable)
    static void EnumCompareCardLocation(ECardLocationEnum location1, ECardLocationEnum location2, EnumAreTheyEqual& Branches);
};
