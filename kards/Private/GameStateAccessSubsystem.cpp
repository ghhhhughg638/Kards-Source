#include "GameStateAccessSubsystem.h"

UGameStateAccessSubsystem::UGameStateAccessSubsystem() = default;

bool UGameStateAccessSubsystem::IsClientPlaying() const
{
    return false;
}

void UGameStateAccessSubsystem::InitializeGameState(AkardsGameState* InGameState)
{
    // Reflection-only editor shim: the game's native subsystem owns this behavior.
    (void)InGameState;
}

ESideEnum UGameStateAccessSubsystem::GetPlayingSide() const
{
    return ESideEnum::NotAvailable;
}

ESideEnum UGameStateAccessSubsystem::GetOpponentSide() const
{
    return ESideEnum::NotAvailable;
}

ESideEnum UGameStateAccessSubsystem::GetClientSide() const
{
    return ESideEnum::NotAvailable;
}
