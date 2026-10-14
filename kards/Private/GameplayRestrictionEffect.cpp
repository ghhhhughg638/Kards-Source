#include "GameplayRestrictionEffect.h"

FGameplayRestrictionEffect::FGameplayRestrictionEffect() {
    this->AffectedSide = ESideEnum::NotAvailable;
    this->RestrictionType = EGameplayRestrictions::cannotDrawCardAtTurnStart;
    this->CardID = 0;
    this->TurnsRemaining = 0;
}

