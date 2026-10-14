#pragma once

#include "CoreMinimal.h"
#include "EnumSpawnInDeckLocation.generated.h"

UENUM(BlueprintType)
enum class EnumSpawnInDeckLocation : uint8
{
	NotAvailable = 0,
	Bottom = 1,
	Top = 2,
	Shuffle = 3,
};
