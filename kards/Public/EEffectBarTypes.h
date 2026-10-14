#pragma once

#include "CoreMinimal.h"
#include "EEffectBarTypes.generated.h"

UENUM(BlueprintType)
enum class EEffectBarTypes : uint8
{
	NotAvailable = 0,
	Ambush = 1,
	Guarded = 2,
	Guarding = 3,
	Pinned = 4,
	Smokescreen = 5,
	HeavyArmor1 = 6,
	HeavyArmor2 = 7,
	HeavyArmor3 = 8,
	Passive = 9,
	Fury = 10,
	UnderEnemyControl = 11,
	Destruction = 12,
	Trigger = 13,
	Mobilize = 14,
	Immune = 15,
	Pincer = 16,
	Intel = 17,
	Veteran = 18,
	Countdown = 19,
	GuardedLeft = 20,
	GuardedRight = 21,
	Suppressed = 22,
	Salvage = 23,
	Salvaged = 24,
	Covert = 25,
	Shock = 26,
	More = 27,
};
