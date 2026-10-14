#include "BaseCardObject.h"
#include "Templates/SubclassOf.h"

UBaseCardObject::UBaseCardObject() {
    this->Type = ETypeEnum::NotAvailable;
    this->attack = 0;
    this->attackBuff = 0;
    this->defense = 0;
    this->range = 0;
    this->faction = EFactionEnum::NotAvailable;
    this->kredits = 0;
    this->kreditsBuff = 0;
    this->KreditsTax_AsEnemyTarget = 0;
    this->operationCost = 0;
    this->operationCostBuff = 0;
    this->effectDamage = 0;
    this->rarity = ERarityEnum::NotAvailable;
    this->selectTargetOnPlayedFromHand = false;
    this->waitWithOnOtherCardPlayedUntilHandTargetSelected = false;
    this->cardSet = ECardSetEnum::NotAvailable;
    this->returnSet = ECardSetEnum::NotAvailable;
    this->isReserved = false;
    this->isInPermanentPool = false;
    this->showBoostIcon = false;
    this->EffectType = ECardEffectTypes::NotAvailable;
    this->campaignUpgradable = false;
    this->draftBaseWeight = 1.00f;
    this->draftAllyWeight = 1.00f;
    this->hasDeployment = false;
    this->hasDestruction = false;
    this->hasBlitz = false;
    this->hasAmbush = false;
    this->heavyArmor = 0;
    this->heavyArmorBuff = 0;
    this->hasSmokescreen = false;
    this->hasMobilize = false;
    this->hasAlpine = false;
    this->hasFury = false;
    this->hasGuard = false;
    this->hasShock = false;
    this->hasCovert = false;
    this->hasScrying = false;
    this->hasPincer = false;
    this->cipher = 0;
    this->cardSeen = false;
    this->exileNation = EFactionEnum::NotAvailable;
    this->hasSalvage = false;
    this->CardID = 0;
    this->movementLeft = 0;
    this->attackLeft = 0;
    this->enterPlayOnTurn = 0;
    this->hasEverAttacked = false;
    this->Location = ECardLocationEnum::NotAvailable;
    this->side = ESideEnum::NotAvailable;
    this->originalSide = ESideEnum::NotAvailable;
    this->locationNumber = 0;
    this->pinnedTurns = 0;
    this->hasBeenAttackedThisTurn = false;
    this->hasAttackedThisTurn = false;
    this->attackCountThisTurn = 0;
    this->isBeingGuarded = false;
    this->isImmune = false;
    this->underEnemyControl = false;
    this->maxAttack = 0;
    this->maxDefense = 0;
    this->CurrentTarget = NULL;
    this->gotchaActivated = 0;
    this->isGoldCard = false;
    this->handCardLookDirty = false;
    this->chooseOneIndex = 0;
    this->isSalvaged = false;
    this->salvageFaction = EFactionEnum::NotAvailable;
    this->isSuppressed = false;
    this->isRevealed = false;
    this->cardHighlightBits = 0;
    this->soundEnterPlay = NULL;
    this->soundMoveToFrontline = NULL;
    this->soundAttack = NULL;
    this->soundBlowUp = NULL;
    this->soundProjectileImpact = NULL;
    this->soundProjectileFiring = NULL;
    this->ImageAnchor = EImageAnchor::Middle;
    this->Delay = 0.00f;
    this->blowUpExplosion = NULL;
    this->blowUpExplosionNiagara = NULL;
    this->blowUpCameraShake = NULL;
    this->OnPlayedFromHandVisualsNiagara = NULL;
    this->planeShadow = NULL;
    this->visualCardWeight = EVisualCardWeight::None;
    this->isDraw = false;
    this->isDiscard = false;
    this->isRemoval = false;
    this->isDirectDamage = false;
    this->isHQDamage = false;
    this->isHQRepair = false;
    this->isRepair = false;
    this->isKreditsBuff = false;
    this->isAttackBuff = false;
    this->isDefenseBuff = false;
    this->threat_level = 0.00f;
    this->targetOverride = NULL;
    this->cardFunction = NULL;
    this->skipCardMarkers = false;
}

void UBaseCardObject::WhichChooseOne(EnumChooseOneCardBeingPlayed& Branches) {
}

void UBaseCardObject::UpdateExtraPlayTriggers(bool& triggerNext) {
}

void UBaseCardObject::ShouldHighlightInHand_Implementation(bool& shouldHighlight) {
}

void UBaseCardObject::ShouldGotchaTrigger(UBaseCardObject* triggerCard, bool& shouldIt) {
}

void UBaseCardObject::SetImageDataTableRow(FCardImageInfo NewRow) {
}

void UBaseCardObject::SetExtraPlayTriggers(int32 extraTriggers) {
}

void UBaseCardObject::setEncryptionKey(int32 NewKey) {
}

void UBaseCardObject::setAndEncryptKreditBuff(int32 newKreditBuff, int32 key1, int32 key2, int32 FrameCount) {
}

void UBaseCardObject::setAndEncryptKredit(int32 newKredit, int32 key1, int32 key2, int32 FrameCount) {
}

void UBaseCardObject::setAndEncryptDefense(int32 newDefense, int32 key1, int32 key2, int32 FrameCount) {
}

void UBaseCardObject::setAndEncryptCardType() {
}

void UBaseCardObject::setAndEncryptAttackBuff(int32 newAttackBuff, int32 key1, int32 key2, int32 FrameCount) {
}

void UBaseCardObject::setAndEncryptAttack(int32 newAttack, int32 key1, int32 key2, int32 FrameCount) {
}

void UBaseCardObject::ResetCardAttributes() {
}

void UBaseCardObject::RemoveGameplayEffectByInstigator(FGameplayTag EffectTag, UBaseCardObject* InstigatorCard) {
}


void UBaseCardObject::OnSuppressed_Implementation() {
}




































void UBaseCardObject::OnOtherCardDealDamageAddDamageAfterCalc_Implementation(UBaseCardObject* cardDealingDamage, UBaseCardObject* toCard, int32 Damage, bool fromAttack, bool isRedirected, int32& damageToAdd, bool& stopAdding) {
}

void UBaseCardObject::OnOtherCardDealDamageAddDamage_Implementation(UBaseCardObject* cardDealingDamage, UBaseCardObject* toCard, int32 Damage, bool fromAttack, bool isDefenderDamage, int32& damageToAdd, bool& reRunAtEnd) {
}






void UBaseCardObject::OnOtherCardAttackSwitchTarget_Implementation(UBaseCardObject* cardAttacking, UBaseCardObject* oldDefender, UBaseCardObject*& newDefender) {
}

void UBaseCardObject::OnOtherCardAttacks_Implementation(UBaseCardObject* cardAttacking, UBaseCardObject* defenderCard, bool& stopAttack, bool& AttackedAndStopped) {
}


















void UBaseCardObject::OnDealDamageAddDamageAfterCalc_Implementation(UBaseCardObject* toCard, int32 Damage, bool fromAttack, bool isAttacker, bool isRedirected, int32& damageToAdd) {
}











void UBaseCardObject::OnCardDealDamage_ModifyDamageDealt_Implementation(UBaseCardObject* toCard, int32 Damage, bool fromAttack, bool fromFight, int32& newDamage) {
}





void UBaseCardObject::OnBeforeOtherCardLoseMobilize_Implementation(UBaseCardObject* card, bool& dontLoseMobilize) {
}

void UBaseCardObject::OnBeforeOtherCardGainDefenseAfterAdd_Implementation(UBaseCardObject* cardGainingDefense, int32 defenseGaining, bool& stopAction) {
}

void UBaseCardObject::OnBeforeOtherCardGainDefense_Implementation(UBaseCardObject* cardGainingDefense, int32 defenseToAdd, int32& newDefenseToAdd, bool& stopAction) {
}


void UBaseCardObject::OnBeforeOtherCardDeploymentTrigger_Implementation(UBaseCardObject* cardDeploying, bool& cancelDeploymentEffect) {
}




























void UBaseCardObject::IsWeatherCard(bool& NewIsWeatherCard) {
}

void UBaseCardObject::IsForecastCard(bool& isIt) {
}

void UBaseCardObject::IsVeteran(bool ignoreSuppress, bool& isIt) {
}

void UBaseCardObject::IsValidHandTarget_Implementation(UBaseCardObject* targetedCard, bool& isIt, FString& Reason) {
}

void UBaseCardObject::IsUnrevealedCovertCard(bool& isIt) {
}

void UBaseCardObject::IsUnit(bool& isIt) {
}

void UBaseCardObject::IsTankDestroyer(bool& isIt) {
}

void UBaseCardObject::IsTank(bool& isIt) {
}

void UBaseCardObject::IsSideActive(ESideEnum SideToCheck, bool& Active) {
}

void UBaseCardObject::IsSameSideUnit(ESideEnum SideToCheck, bool& isIt) {
}

void UBaseCardObject::IsPinned(bool& NewIsPinned) {
}

void UBaseCardObject::IsOwnedByClientSide(bool& IsOwned) {
}

void UBaseCardObject::IsOrder(bool& isIt) {
}

void UBaseCardObject::IsLocation(bool& isIt) {
}

void UBaseCardObject::IsLocatedOnBoard(bool& isIt) {
}

void UBaseCardObject::IsLocatedInHand(bool& isIt) {
}

void UBaseCardObject::IsLocatedInDeck(bool& isIt) {
}

void UBaseCardObject::IsInfantry(bool& isIt) {
}

void UBaseCardObject::IsGunUnit(bool& isIt) {
}

void UBaseCardObject::IsGroundUnit(bool& isIt) {
}

void UBaseCardObject::IsGotcha(bool& isIt) {
}

void UBaseCardObject::IsFighter(bool& isIt) {
}

void UBaseCardObject::IsExile(bool& isIt) {
}

void UBaseCardObject::IsDamaged(bool& isIt) {
}

void UBaseCardObject::IsChooseOneCard(bool& isIt) {
}

void UBaseCardObject::isBuffedByCard(int32 instigatorID, bool& isBuffed) {
}

void UBaseCardObject::IsBomber(bool& isIt) {
}

void UBaseCardObject::IsArtillery(bool& isIt) {
}

void UBaseCardObject::IsArmorUnit(bool& isIt) {
}

void UBaseCardObject::IsAntiTank(bool& isIt) {
}

void UBaseCardObject::IsAntiAir(bool& isIt) {
}

void UBaseCardObject::IsAirUnit(bool& isIt) {
}

void UBaseCardObject::InitStaticGameplayTags() {
}

void UBaseCardObject::InitializeEncryption() {
}

void UBaseCardObject::HasMovementLeft(bool& doesIt) {
}

void UBaseCardObject::HasIntel(bool& doesIt) {
}

void UBaseCardObject::HasCustomAbilityFromCard(const FString& ability, int32 giverID, bool& doesIt) {
}

void UBaseCardObject::HasCustomAbility(const FString& ability, bool& doesIt) {
}

void UBaseCardObject::HasCantBeAttackedBy(const FString& unitType, bool& hasIt) {
}

void UBaseCardObject::HasCantAttackType(const FString& cardType, bool& hasIt) {
}

void UBaseCardObject::HasCantAttack(bool& hasIt) {
}

void UBaseCardObject::HasCampaignUpgrade(int32 campaignUpgrade, bool& doesIt) {
}

void UBaseCardObject::HasBond(bool& hasIt) {
}

void UBaseCardObject::HasAttackLeft(bool& doesIt) {
}

void UBaseCardObject::hasActivePincerEffect(bool& doesIt) {
}

void UBaseCardObject::hasActiveCountdownEffect(bool& doesIt) {
}

void UBaseCardObject::getTotalOperationCost(int32& totalOperationCost) {
}

void UBaseCardObject::getTotalKreditCost(int32& totalKreditCost) {
}

void UBaseCardObject::getTotalHeavyArmor(int32& totalHeavyArmor) {
}

void UBaseCardObject::getTotalDefense(int32& totalDefense) {
}

void UBaseCardObject::getTotalAttack(int32& totalAttack) {
}

void UBaseCardObject::getStaticVeteranUpgrade(UBaseCardObject*& staticVeteranCard) {
}

FGameplayTagContainer UBaseCardObject::GetRuntimeEffectTags() const {
    return FGameplayTagContainer{};
}

void UBaseCardObject::GetPlayFromHandDamage_Implementation(UBaseCardObject* targetCard, int32& Damage) {
}

void UBaseCardObject::GetPassiveDefenseBuff_Implementation(int32 incomingDamage, int32& amount) {
}

void UBaseCardObject::GetOppositeSide(ESideEnum& oppositeSide) {
}

TSubclassOf<AVisualCardActionBaseStub> UBaseCardObject::GetOnPlayedFromHandVisuals() const {
    return NULL;
}

TSubclassOf<AVisualCardActionBaseStub> UBaseCardObject::GetOnChooseOneCardVisuals() const {
    return NULL;
}

TSubclassOf<AVisualCardActionBaseStub> UBaseCardObject::GetOnAttackVisuals() const {
    return NULL;
}

void UBaseCardObject::getKreditTempBuffAmount(int32 instigatorID, int32& tempAmount) {
}

void UBaseCardObject::getIntel(int32& intel) {
}

void UBaseCardObject::GetImageDataTableRow(FCardImageInfo& OutRow) {
}

void UBaseCardObject::getHasVeteranUpgrade(bool& doesIt) {
}

void UBaseCardObject::getHasSmokescreen(bool& doesIt) {
}

void UBaseCardObject::getHasShock(bool& doesIt) {
}

void UBaseCardObject::getHasImmune(bool& doesIt) {
}

void UBaseCardObject::getHasGuard(bool& doesIt) {
}

void UBaseCardObject::getHasGameplayTag(FGameplayTag inputTag, bool& HasTag) const {
}

void UBaseCardObject::getHasFury(bool& doesIt) {
}

void UBaseCardObject::getHasBlitz(bool& doesIt) {
}

void UBaseCardObject::getHasAmbush(bool& doesIt) {
}

void UBaseCardObject::getHasAlpine(bool& doesIt) {
}

void UBaseCardObject::getExileNation(EFactionEnum& exileFaction) {
}

void UBaseCardObject::GetCustomName2Attributes(TArray<FString>& Attributes) {
}

void UBaseCardObject::getCountdownValue(int32& countdownValue) {
}

void UBaseCardObject::GetCombatKeywords(TArray<ECombatKeyword>& Keywords, int32& numberOfKeywords) {
}

void UBaseCardObject::GetChooseSpawnCards_Implementation(TArray<UBaseCardObject*>& Cards, bool& MarkAsSeen, bool& keepOrder) {
}

void UBaseCardObject::getCardsBuffedByThisCard(TArray<int32>& Cards) {
}

void UBaseCardObject::getbuffsFromCardsAsJsonString(FString& buffsFromCardsJsonString) {
}

void UBaseCardObject::GetBeforeAttackDefenseBuff_Implementation(bool asAttacker, int32& amount) {
}

void UBaseCardObject::GetBeforeAttackDamage_Implementation(bool asAttacker, int32& amount) {
}

void UBaseCardObject::GetBeforeAttackAttackBuff_Implementation(bool asAttacker, int32& amount) {
}

void UBaseCardObject::getAttackTempBuffAmount(int32 instigatorID, int32& tempAmount) {
}

void UBaseCardObject::getAndDecryptKreditBuff(int32& decryptedKreditBuff) {
}

void UBaseCardObject::getAndDecryptKredit(int32& decryptedKredit) {
}

void UBaseCardObject::getAndDecryptDefense(int32& decryptedDefense) {
}

void UBaseCardObject::getAndDecryptAttackBuff(int32& decryptedAttackBuff) {
}

void UBaseCardObject::getAndDecryptAttack(int32& decryptedAttack) {
}

void UBaseCardObject::GetAllChildrenGameplayTagsAsStrings(FGameplayTag ParentTag, bool TagContainerOnly, TArray<FString>& ChildrenTags, bool& hasChildrenTags) const {
}

void UBaseCardObject::FilterCardsToScry_Implementation(UBaseCardObject* card, bool& IsValid) {
}

bool UBaseCardObject::DoesEffectTagExist(FGameplayTag EffectTag) const {
    return false;
}

void UBaseCardObject::CustomName2Remove(const FString& Attribute) {
}

void UBaseCardObject::CustomName2HasAttribute(const FString& Attribute, bool& doesIt) {
}

void UBaseCardObject::CustomName2Add(const FString& Attribute) {
}

void UBaseCardObject::CustomName1Remove(const FString& Attribute) {
}

void UBaseCardObject::CustomName1HasAttribute(const FString& Attribute, bool& doesIt) {
}

void UBaseCardObject::CustomName1Add(const FString& Attribute) {
}

UBaseCardObject* UBaseCardObject::CreateCopy() {
    return NULL;
}

void UBaseCardObject::CopyData(UBaseCardObject* toCard) {
}

void UBaseCardObject::CardSimulateTampering() {
}

void UBaseCardObject::CanPlayFromHand_Implementation(bool& canIt, FString& Reason, FString& reasonParam1, FString& reasonParam2, UBaseCardObject*& targetedCard) {
}

void UBaseCardObject::CanOtherCardBeTargetted_Implementation(bool& canIt, FString& Reason, FString& reasonParam1, FString& reasonParam2, UBaseCardObject* targettingCard, UBaseCardObject* targetCard, bool byPlayFromHand) {
}

void UBaseCardObject::CanMoveAndAttackInTheSameTurn(bool& canIt) {
}

void UBaseCardObject::CanBeTargetted_Implementation(bool& canIt, FString& Reason, FString& reasonParam1, FString& reasonParam2, UBaseCardObject* targettingCard, bool byPlayFromHand) {
}

void UBaseCardObject::CampaignSetTitle(FText& TextValue) {
}

void UBaseCardObject::CampaignSetText(FText& TextValue, bool secondUpgrade) {
}

void UBaseCardObject::CampaignSetOpCost(int32 Value, bool secondUpgrade) {
}

void UBaseCardObject::CampaignSetKreditCost(int32 Value, bool secondUpgrade) {
}

void UBaseCardObject::CampaignSetHeavyArmor(int32 Value, bool secondUpgrade) {
}

void UBaseCardObject::CampaignSetDefense(int32 Value, bool secondUpgrade) {
}

void UBaseCardObject::CampaignSetAttack(int32 Value, bool secondUpgrade) {
}

void UBaseCardObject::CampaignRemoveSmokescreen(bool secondUpgrade) {
}

void UBaseCardObject::CampaignRemoveMobilize(bool secondUpgrade) {
}

void UBaseCardObject::CampaignRemoveGuard(bool secondUpgrade) {
}

void UBaseCardObject::CampaignRemoveFury(bool secondUpgrade) {
}

void UBaseCardObject::CampaignRemoveBlitz(bool secondUpgrade) {
}

void UBaseCardObject::CampaignRemoveAmbush(bool secondUpgrade) {
}


void UBaseCardObject::CampaignIncreaseHeavyArmor(int32 increaseValue, bool secondUpgrade) {
}

void UBaseCardObject::CampaignAddSmokescreen(bool secondUpgrade) {
}

void UBaseCardObject::CampaignAddOpCost(int32 Value, bool secondUpgrade) {
}

void UBaseCardObject::CampaignAddMobilize(bool secondUpgrade) {
}

void UBaseCardObject::CampaignAddKreditCost(int32 Value, bool secondUpgrade, bool Clear) {
}

void UBaseCardObject::CampaignAddGuard(bool secondUpgrade) {
}

void UBaseCardObject::CampaignAddFury(bool secondUpgrade) {
}

void UBaseCardObject::CampaignAddDefense(int32 Value, bool secondUpgrade) {
}

void UBaseCardObject::CampaignAddBlitz(bool secondUpgrade) {
}

void UBaseCardObject::CampaignAddAttack(int32 Value, bool secondUpgrade) {
}

void UBaseCardObject::CampaignAddAmbush(bool secondUpgrade) {
}

void UBaseCardObject::BlockCardFromBeingPlayedFromHand_Implementation(bool& bLocked, FString& Reason, FString& reasonParam1, FString& reasonParam2) {
}

void UBaseCardObject::ApplyGameplayEffect(FGameplayEffect EffectToApply) {
}


