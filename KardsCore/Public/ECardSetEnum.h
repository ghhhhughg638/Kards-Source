#pragma once
#include "CoreMinimal.h"
#include "ECardSetEnum.generated.h"

UENUM(BlueprintType)
enum class ECardSetEnum : uint8 {
    NotAvailable,
    Basic,
    Special,
    OnlySpawnable,
    Candidate,
    Placeholder,
    Expansion1,
    Expansion2,
    Expansion2019,
    Campaign2020,
    Breakthrough,
    Wildcards,
    Legions,
    WorldAtWar,
    Core,
    BrothersInArms,
    WinterWar,
    CovertOps,
    BloodAndIron,
    NavalWarfare,
    Homefront,
    OceaniaStorm,
    Reserved,
    CurrentExpansion = CovertOps,
};

