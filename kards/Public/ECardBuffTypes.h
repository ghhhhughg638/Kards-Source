#pragma once

#include "CoreMinimal.h"
#include "ECardBuffTypes.generated.h"

UENUM(BlueprintType)
enum class ECardBuffTypes : uint8
{
	attack = 0,
	defense = 1,
	kredit = 2,
	operationCost = 3,
	heavyArmor = 4,
	custom = 5,
	combat = 6,
};
