#include "FunctionLibrary.h"

#include "BaseCardObject.h"
#include "UObject/UObjectIterator.h"

EEffectBarTypes UFunctionLibrary::CardEffectToEffectBar(ECardEffectTypes Type)
{
    // Reflection only; the host supplies the mapping.
    return EEffectBarTypes::NotAvailable;
}

namespace
{
template <typename T>
void SetComparison(const T Left, const T Right, EnumAreTheyEqual& Branches)
{
    Branches = Left == Right ? EnumAreTheyEqual::Equal : EnumAreTheyEqual::NotEqual;
}
}

UBaseCardObject* UFunctionLibrary::GetStaticCard(const FName CardName)
{
    if (CardName.IsNone())
    {
        return nullptr;
    }

    // Card blueprints expose their static/default object through the generated
    // class once they have been loaded by the editor/game card registry.
    for (TObjectIterator<UClass> It; It; ++It)
    {
        UClass* CardClass = *It;
        if (CardClass == nullptr
            || !CardClass->IsChildOf(UBaseCardObject::StaticClass())
            || CardClass->HasAnyClassFlags(CLASS_Abstract))
        {
            continue;
        }

        UBaseCardObject* Card = Cast<UBaseCardObject>(CardClass->GetDefaultObject());
        if (Card != nullptr && Card->Name == CardName)
        {
            return Card;
        }
    }

    return nullptr;
}

void UFunctionLibrary::GetStaticCampaignName(const FName cardName, FText& campaignName)
{
    if (const UBaseCardObject* Card = GetStaticCard(cardName))
    {
        campaignName = Card->campaignName;
    }
}

void UFunctionLibrary::GetStaticCardSet(const FName cardName, ECardSetEnum& cardSet)
{
    if (const UBaseCardObject* Card = GetStaticCard(cardName))
    {
        cardSet = Card->cardSet;
    }
}

void UFunctionLibrary::GetStaticKredits(const FName cardName, int32& kredits)
{
    if (const UBaseCardObject* Card = GetStaticCard(cardName))
    {
        kredits = Card->kredits;
    }
}

void UFunctionLibrary::GetStaticOperationCost(const FName cardName, int32& operationCost)
{
    if (const UBaseCardObject* Card = GetStaticCard(cardName))
    {
        operationCost = Card->operationCost;
    }
}

UUserWidget* UFunctionLibrary::GetOrCreateWidget(UObject* WorldContextObject, TSubclassOf<UUserWidget> WidgetClass)
{
    // Signature-only editor shim; widgets are owned by the running game.
    (void)WorldContextObject;
    (void)WidgetClass;
    return nullptr;
}

void UFunctionLibrary::GetStaticAttack(const FName cardName, int32& attack)
{
    if (const UBaseCardObject* Card = GetStaticCard(cardName))
    {
        attack = Card->attack;
    }
}

void UFunctionLibrary::GetStaticDefense(const FName cardName, int32& defense)
{
    if (const UBaseCardObject* Card = GetStaticCard(cardName))
    {
        defense = Card->defense;
    }
}

void UFunctionLibrary::GetStaticText(const FName cardName, FText& Text)
{
    if (const UBaseCardObject* Card = GetStaticCard(cardName))
    {
        Text = Card->Text;
    }
}

bool UFunctionLibrary::isConsole()
{
    return false;
}

bool UFunctionLibrary::IsSimulatingInEditor()
{
    return false;
}

void UFunctionLibrary::SortCardsByName(TArray<UBaseCardObject*>& cards)
{
    (void)cards;
}

void UFunctionLibrary::SetStaticCampaignName(const FName cardName, const FText campaignName)
{
    if (UBaseCardObject* Card = GetStaticCard(cardName))
    {
        Card->campaignName = campaignName;
    }
}

void UFunctionLibrary::BranchOnPlatformType(PlatformType& Branches)
{
#if PLATFORM_ANDROID || PLATFORM_IOS
    Branches = PlatformType::Mobile;
#else
    Branches = PlatformType::Desktop;
#endif
}

bool UFunctionLibrary::isMobile()
{
#if PLATFORM_ANDROID || PLATFORM_IOS
    return true;
#else
    return false;
#endif
}

void UFunctionLibrary::EnumCompareType(
    const ETypeEnum type1,
    const ETypeEnum type2,
    EnumAreTheyEqual& Branches)
{
    SetComparison(type1, type2, Branches);
}

void UFunctionLibrary::EnumCompareSide(
    const ESideEnum side1,
    const ESideEnum side2,
    EnumAreTheyEqual& Branches)
{
    SetComparison(side1, side2, Branches);
}

void UFunctionLibrary::EnumCompareFaction(
    const EFactionEnum faction1,
    const EFactionEnum faction2,
    EnumAreTheyEqual& Branches)
{
    SetComparison(faction1, faction2, Branches);
}

void UFunctionLibrary::EnumCompareCardLocation(
    const ECardLocationEnum location1,
    const ECardLocationEnum location2,
    EnumAreTheyEqual& Branches)
{
    SetComparison(location1, location2, Branches);
}

TArray<FString> UFunctionLibrary::SortStringArray(const TArray<FString>& StringArray, const bool Reversed)
{
    TArray<FString> Result = StringArray;
    Result.Sort([Reversed](const FString& A, const FString& B) { return Reversed ? A > B : A < B; });
    return Result;
}

void UFunctionLibrary::SortIntegerArrayDirectly(TArray<int32>& IntegerArray, const bool Reversed)
{
    IntegerArray.Sort([Reversed](int32 A, int32 B) { return Reversed ? A > B : A < B; });
}

TArray<int32> UFunctionLibrary::SortIntegerArray(const TArray<int32>& IntegerArray, const bool Reversed)
{
    TArray<int32> Result = IntegerArray;
    SortIntegerArrayDirectly(Result, Reversed);
    return Result;
}

void UFunctionLibrary::SortCardsForStore(TArray<UBaseCardObject*>& cardArray)
{
    (void)cardArray;
}
