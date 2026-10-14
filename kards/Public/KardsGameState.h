#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ESideEnum.h"
#include "OnTamperingDetectedDelegate.h"
#include "KardsGameState.generated.h"

/**
 * Editor/runtime compatibility shell for the game's native kardsGameState.
 *
 * The shipped game exposes BP_GameState_Battle with kardsGameState as its
 * native parent.  The mod project does not contain that native class, but a
 * real reflected class is still required when restoring the Blueprint from
 * FModel JSON.  Keeping this shell free of game logic makes it safe for
 * editor restoration while preserving the reflected class identity
 * (/Script/kards.kardsGameState).
 */
UCLASS(BlueprintType, Blueprintable)
class KARDS_API AkardsGameState : public AGameState
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, EditAnywhere,
        meta=(AllowPrivateAccess=true))
    FOnTamperingDetected OnTamperingDetected;

    AkardsGameState(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable)
    void simulateTampering(ESideEnum sideToTamper);

    UFUNCTION(BlueprintCallable)
    void SetupSidesInBattle(ESideEnum InClientSide, ESideEnum InOpponentSide);

    UFUNCTION(BlueprintCallable)
    void SetPlayingSide(ESideEnum InPlaingSide);

    UFUNCTION(BlueprintCallable)
    void setKreditSlotBySide(ESideEnum sideToSet, int32 newKreditSlot,
        int32 key1, int32 key2, int32 FrameCount);

    UFUNCTION(BlueprintCallable)
    void setKreditBySide(ESideEnum sideToSet, int32 newKredit,
        int32 key1, int32 key2, int32 FrameCount);

    UFUNCTION(BlueprintCallable)
    void setEncryptionKey(int32 NewKey);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsClientPlaying(bool& IsPlaying);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetPlayingSide(ESideEnum& OutPlayingSide);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetOpponentSide(ESideEnum& OutOpponentSide);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getMaxPossibleKredits(int32& OutputMax);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getKreditSlotBySide(ESideEnum SideToGet, int32& outputKreditSlot);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getKreditBySide(ESideEnum SideToGet, int32& outputKredit);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void getHQDefenseForSide(ESideEnum side, int32& hqDefense);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetEncryptionKey(int32& OutKey);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetClientSide(ESideEnum& OutClientSide);
};
