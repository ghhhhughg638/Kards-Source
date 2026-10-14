#pragma once

#include "CoreMinimal.h"
#include "BaseCardObject.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "NativeActorEditorShims.generated.h"

// Reflection-only compatibility types for Blueprint reconstruction in the
// local editor project. These declarations do not replace the game's module.
class UCardBackAsset;

UCLASS(BlueprintType, Blueprintable)
class KARDS_API ACardLibrary : public AActor
{
    GENERATED_BODY()
public:
    // Canonical JMap signatures; editor reflection only.
    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool TryGetCardTypeFromDeckCode(FString deckCode, FString& OutCardFileName);
    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool TryGetDeckCodeFromCardType(FString CardFileName, FString& OutDeckCode);
};

UCLASS(BlueprintType, Blueprintable)
class KARDS_API ADecks : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void SetSession(UObject* Session);
};

UCLASS(BlueprintType, Blueprintable)
class KARDS_API APacks : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void SetSession(UObject* Session);
};

// Local reflection-only signature for the native asset manager used by
// BP_Logic. The game supplies the canonical class; these methods provide
// editor pins for reconstructing its calls.
UCLASS(BlueprintType, Blueprintable)
class KARDS_API UKardsAssetManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static UKardsAssetManager* GetKardsAssetManager();

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static void GetKardMap(TMap<FName, TSoftClassPtr<UBaseCardObject>>& map);

    UFUNCTION(BlueprintCallable)
    void CaptureAndSendPerformanceAnalyticsEvent(
        APlayerController* playerController,
        const FString& eventName);

    // Used by BP_BoardCard.LoadCardBack (official header dump: KardsAssetManager.h).
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static UCardBackAsset* GetCardbackAsset(FName Cardback);
};

