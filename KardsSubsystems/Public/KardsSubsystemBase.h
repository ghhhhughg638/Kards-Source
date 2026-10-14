#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KardsSubsystemBase.generated.h"

/**
 * Editor-only reflection shim matching the native KARDS subsystem base class.
 * The shipped game provides the real class at runtime.
 */
UCLASS(Abstract, Blueprintable)
class KARDSSUBSYSTEMS_API UKardsSubsystemBase : public UGameInstanceSubsystem
{
    GENERATED_BODY()
};

