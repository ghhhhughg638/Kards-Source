#include "GameplayEffect.h"

FGameplayEffect::FGameplayEffect() {
    this->InstigatorCard = NULL;
    this->DurationPolicy = EDurationPolicy::None;
    this->RemainingTurns = 0;
    this->EffectValue = 0;
}

