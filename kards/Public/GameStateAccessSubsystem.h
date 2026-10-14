#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ESideEnum.h"
#include "GameStateAccessSubsystem.generated.h"

class AkardsGameState;

// Local reflection-only compatibility shell for the native KARDS subsystem.
// These bodies do not implement game behavior. Do not package or distribute
// this project's kards module DLL; the shipped game supplies the canonical
// /Script/kards.GameStateAccessSubsystem implementation.
UCLASS(Blueprintable)
class KARDS_API UGameStateAccessSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UGameStateAccessSubsystem();

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsClientPlaying() const;

    UFUNCTION(BlueprintCallable)
    void InitializeGameState(AkardsGameState* InGameState);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    ESideEnum GetPlayingSide() const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    ESideEnum GetOpponentSide() const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    ESideEnum GetClientSide() const;
};
