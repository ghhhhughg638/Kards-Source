#include "KardsGameState.h"

AkardsGameState::AkardsGameState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void AkardsGameState::simulateTampering(ESideEnum sideToTamper)
{
}

void AkardsGameState::SetupSidesInBattle(ESideEnum InClientSide, ESideEnum InOpponentSide)
{
}

void AkardsGameState::SetPlayingSide(ESideEnum InPlaingSide)
{
}

void AkardsGameState::setKreditSlotBySide(ESideEnum sideToSet, int32 newKreditSlot,
    int32 key1, int32 key2, int32 FrameCount)
{
}

void AkardsGameState::setKreditBySide(ESideEnum sideToSet, int32 newKredit,
    int32 key1, int32 key2, int32 FrameCount)
{
}

void AkardsGameState::setEncryptionKey(int32 NewKey)
{
}

void AkardsGameState::IsClientPlaying(bool& IsPlaying)
{
    IsPlaying = false;
}

void AkardsGameState::GetPlayingSide(ESideEnum& OutPlayingSide)
{
    OutPlayingSide = ESideEnum::NotAvailable;
}

void AkardsGameState::GetOpponentSide(ESideEnum& OutOpponentSide)
{
    OutOpponentSide = ESideEnum::NotAvailable;
}

void AkardsGameState::getMaxPossibleKredits(int32& OutputMax)
{
    OutputMax = 0;
}

void AkardsGameState::getKreditSlotBySide(ESideEnum SideToGet, int32& outputKreditSlot)
{
    outputKreditSlot = 0;
}

void AkardsGameState::getKreditBySide(ESideEnum SideToGet, int32& outputKredit)
{
    outputKredit = 0;
}

void AkardsGameState::GetEncryptionKey(int32& OutKey)
{
    OutKey = 0;
}

void AkardsGameState::GetClientSide(ESideEnum& OutClientSide)
{
    OutClientSide = ESideEnum::NotAvailable;
}
