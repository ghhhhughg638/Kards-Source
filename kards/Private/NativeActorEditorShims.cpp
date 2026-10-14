#include "NativeActorEditorShims.h"
#include "CardBackAsset.h"

bool ACardLibrary::TryGetCardTypeFromDeckCode(FString deckCode, FString& OutCardFileName)
{
    OutCardFileName.Reset(); return false;
}
bool ACardLibrary::TryGetDeckCodeFromCardType(FString CardFileName, FString& OutDeckCode)
{
    OutDeckCode.Reset(); return false;
}

void ADecks::SetSession(UObject* Session)
{
    (void)Session;
}

void APacks::SetSession(UObject* Session)
{
    (void)Session;
}

UKardsAssetManager* UKardsAssetManager::GetKardsAssetManager()
{
    return nullptr;
}

void UKardsAssetManager::GetKardMap(TMap<FName, TSoftClassPtr<UBaseCardObject>>& map)
{
    map.Reset();
}

void UKardsAssetManager::CaptureAndSendPerformanceAnalyticsEvent(
    APlayerController* playerController,
    const FString& eventName)
{
    (void)playerController;
    (void)eventName;
}

UCardBackAsset* UKardsAssetManager::GetCardbackAsset(FName Cardback)
{
    // Signature-only editor shim.
    (void)Cardback;
    return nullptr;
}


