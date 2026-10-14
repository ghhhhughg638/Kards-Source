#pragma once
#include "CoreMinimal.h"
#include "EFactionEnum.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
//CROSS-MODULE INCLUDE V2: -ModuleName=Engine -ObjectName=Actor -FallbackName=Actor
//CROSS-MODULE INCLUDE V2: -ModuleName=GameplayTags -ObjectName=GameplayTag -FallbackName=GameplayTag
//CROSS-MODULE INCLUDE V2: -ModuleName=KardsCore -ObjectName=EFactionEnum -FallbackName=EFactionEnum
#include "ECampaignStarStatus.h"
#include "ECardBuffTypes.h"
#include "ECardLocationEnum.h"
#include "EChangeType.h"
#include "ECombatKeyword.h"
#include "EDurationPolicy.h"
#include "EGameplayRestrictions.h"
#include "ESideEnum.h"
#include "EStatsTextColor.h"
#include "EnumCampaignStrategy.h"
#include "KardsGameModeEditorStub.h"
#include "CardFunctionsStub.generated.h"

class UBaseCardObject;

UCLASS(Blueprintable)
class ACardFunctionsStub : public AActor {
    GENERATED_BODY()
public:
    ACardFunctionsStub(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void WhichStrategy(EnumCampaignStrategy& Branches);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void WasRightMostCardWhenPlayedFromHand(UBaseCardObject* card, bool& WasLeftMost);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void WasLeftMostCardWhenPlayedFromHand(UBaseCardObject* card, bool& WasLeftMost);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void UpdateCampaignStarStatus(int32 star, const FText& Message, ECampaignStarStatus starStatus);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void TriggerDestruction(UBaseCardObject* card, int32 instigatorID, bool StealSide, bool RemoveDestruction, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void TriggerDeployment(UBaseCardObject* card, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void TakeControlOfEnemyUnit(UBaseCardObject* card, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SuppressUnit(int32 CardID, int32 instigatorID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SuppressMultipleUnits(const TArray<int32>& cardsToSuppress, int32 instigatorID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void StealCardFromBoardToDeck(int32 CardID, int32 instigatorID, ESideEnum deckSide, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SpawnNextToCard(FName card_name, int32 spawnerID, ESideEnum side, int32& spawnedCardID, bool copySpawnerAttack, bool copySpawnerDefense, EFactionEnum salvageFaction, bool NewMakeVeteran);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SpawnMultipleCardsOnBattlefield(ESideEnum side, bool Frontline, const TArray<FName>& cardNames, int32 spawnerID, bool NewGiveBlitz, TArray<int32>& spawnedCardIDs, bool NewMakeVeteran);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SpawnCardOnTopOfDeck(FName card_name, ESideEnum side, int32 spawnerID, int32 numberOfCards, UPARAM(Ref) FText& campaignName, EFactionEnum salvageFaction, TArray<int32>& spawnedCardIDs);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SpawnCardOnBattlefield(ESideEnum side, bool Frontline, FName card_name, int32 spawnerID, UPARAM(Ref) FText& campaignName, bool NewGiveBlitz, int32 locationNumber, EFactionEnum salvageFaction, bool NewMakeVeteran, bool forceGoldCard, int32& spawnedCardID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SpawnCardInHandBySide(ESideEnum side, FName card_name, int32 spawnerID, bool cardSeen, bool SkipDrawAnimation, bool fromOppositeSide, const FString& newCardText, UPARAM(Ref) FText& campaignName, EFactionEnum salvageFaction, int32& spawnedCardID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SpawnCardInFrontline(FName card_name, ESideEnum side, int32 spawnerID, UPARAM(Ref) FText& campaignName, bool NewGiveBlitz, int32& spawnedCardID, EFactionEnum salvageFaction, int32 locationNumber, bool NewMakeVeteran);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SpawnCardInDeckBySide(ESideEnum side, FName card_name, int32 spawnerID, int32 numberOfCards, EFactionEnum salvageFaction, bool HideFromOpponent, bool Bottom, bool Shuffle, bool SkipDrawAnimation, bool RandomWithoutShuffle, TArray<int32>& spawnedCardIDs);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ShuffleDeckBySide(ESideEnum sideToShuffle, bool skipSubAction, int32 instigatorID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ShowTutorialMessage(UPARAM(Ref) FText& Message, const FString& abilityString, int32 CardID, float Delay, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ShowNotification(ESideEnum sideToShow, UPARAM(Ref) FText& Text);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ShowCampaignMessage(const FText& Message, bool showOnce, bool showMulliganWhenFinished, float Delay, bool startOfGamea);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SetObjectiveCounter(int32 Number);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SetDeckBySide(ESideEnum deckSide, const TArray<int32>& deckCardIDs, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SetCountdown(int32 CardID, int32 Value, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SetCardsSeenByCipher(int32 numberOfCardsSeen, int32 instigatorID, ESideEnum side, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SetCardSeen(int32 cardID_Seen, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void selectTargetFromHand(int32 cardSelectingHandTarget, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void selectCardToDraw(int32 cardSelectingCardToDraw, bool selectFromTopOfDeck, bool isEffect, int32& drawnCardID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void SalvageMultipleUnits(const TArray<int32>& cardsToSalvage, int32 instigatorID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RevealCard(int32 CardID, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ResetUnitOperations(int32 CardID, int32 giverID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ReportTampering(const FString& reportReason);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveSmokescreen(int32 CardID, int32 giverID, bool RemoveAllGivers, bool skipAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveShock(int32 CardID, int32 instigatorID, bool RemoveAllGivers, bool skipAction, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveSalvage(int32 CardID, int32 instigatorID, int32& qqq, bool skipAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemovePin(UBaseCardObject* card, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveMultipleCardsFromBoard(const TArray<int32>& CardIDs, int32 discarderID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveMobilize(int32 CardID, int32 instigatorID, bool skipAction, bool suppress, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveImmune(int32 CardID, int32 giverID, bool& qqq, bool skipAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveGuard(int32 CardID, int32 instigatorID, bool skipAction, bool RemoveAllGivers);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveGameplaySideEffect(ESideEnum SideToRemove, FGameplayTag EffectTag, UBaseCardObject* InstigatorCard, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveGameplayRestriction(ESideEnum side, EGameplayRestrictions Type, int32 CardID, bool RemoveAll, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveFury(int32 CardID, int32 giverID, bool skipAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveCustomGameplayTag(FGameplayTag TagToRemove, int32 cardToRemove, int32 instigatorID, bool RemoveAll, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveCardFromBoard(int32 CardID, int32 discarderID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveBond(int32 CardID, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveBlitz(int32 CardID, bool& qqq, int32 giverID, bool skipAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveAmbush(int32 CardID, int32 giverID, bool skipAction, bool RemoveAllGivers);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveAlpine(int32 CardID, int32& qqq, bool skipAction, int32 giverID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ReleaseControlOfEnemyUnit(UBaseCardObject* card, int32 instigatorID, ECardLocationEnum originalLocation, ESideEnum originalSide);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void RandomIntFromRangeWithStream(int32 Minimum, int32 Maximum, int32& randomResult);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void PlayCardDirectlyFromHand(UBaseCardObject* card, bool toFrontline, int32 instigatorID, int32& qqq, int32 locationNumber);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void PinUnit(int32 CardID, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void PersistCustomFields(int32 CardID, bool refreshEffectBar);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void NotifySideEffectTrigger(ESideEnum SideToNotify, FGameplayTag EffectTag, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void MoveUnitFromSupportToFrontLine(UBaseCardObject* card, int32 instigatorID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void MoveUnitFromBoardToOwnersHand(UBaseCardObject* card, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void MoveMultipleCardsToTopOfOwnersDeck(const TArray<int32>& CardIDs, int32 instigatorID, int32 positionFromTop, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void MoveCardToTopOfOwnersDeck(int32 CardID, int32 instigatorID, int32 positionFromTop, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void MoveCardInHandToLeftMost(UBaseCardObject* card, bool& sucess);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void MakeVeteran(UBaseCardObject* card, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void MakeCountAsTank(int32 CardID, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void MakeCardsFight(UBaseCardObject* unitThisSide, UBaseCardObject* unitOppositeSide, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void MakeCardRetreat(const TArray<UBaseCardObject*>& Cards, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void LoseKreditSlot(ESideEnum side);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_SetStringArray(UBaseCardObject* card, const FString& VariableName, const TArray<FString>& Values, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_SetString(UBaseCardObject* card, const FString& VariableName, const FString& Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_SetIntArray(UBaseCardObject* card, const FString& VariableName, const TArray<int32>& Values, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_SetInt(UBaseCardObject* card, const FString& VariableName, int32 Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_SetBoolArray(UBaseCardObject* card, const FString& VariableName, const TArray<bool>& Values, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_SetBool(UBaseCardObject* card, const FString& VariableName, bool Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_RemoveFromStringArray(UBaseCardObject* card, const FString& VariableName, const FString& Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_RemoveFromIntArray(UBaseCardObject* card, const FString& VariableName, int32 Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_RemoveFromBoolArray(UBaseCardObject* card, const FString& VariableName, bool Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_GetStringArray(UBaseCardObject* card, const FString& VariableName, TArray<FString>& Values, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_GetString(UBaseCardObject* card, const FString& VariableName, FString& Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_GetIntArray(UBaseCardObject* card, const FString& VariableName, TArray<int32>& Values, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_GetInt(UBaseCardObject* card, const FString& VariableName, int32& Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_GetBoolArray(UBaseCardObject* card, const FString& VariableName, TArray<bool>& Values, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_GetBool(UBaseCardObject* card, const FString& VariableName, bool& Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_Clear(UBaseCardObject* card, const FString& VariableName, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_AddToStringArray(UBaseCardObject* card, const FString& VariableName, const FString& Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_AddToIntArray(UBaseCardObject* card, const FString& VariableName, int32 Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void JSON_AddToBoolArray(UBaseCardObject* card, const FString& VariableName, bool Value, bool& Found);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void IsUsingStrategy(EnumCampaignStrategy strategy, bool& usingIt);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void IsTopDeckNavy(ESideEnum deckSide, bool& isNavy);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void IsLocationFull(ECardLocationEnum Location, bool& isFull);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void IsCardReserved(FName InCardName, bool& isReserved);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void IncrementObjectiveCounter();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GotchaTriggered(UBaseCardObject* card, int32 instigatorID, bool stopFurtherCardActions, bool skipDiscardingOrder, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveStarForCampaign(int32 Number);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveSmokescreen(int32 CardID, int32 giverID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveShock(int32 CardID, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveSalvage(int32 CardID, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveRandomCombatKeyword(int32 CardID, int32 instigatorID, ECombatKeyword& keywordGiven, bool& success);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveMobilize(int32 CardID, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveKreditsBySide(ESideEnum side, int32 kredits, int32 instigatorID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveImmune(int32 CardID, int32 giverID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveGuard(int32 CardID, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveFury(int32 CardID, int32 giverID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveBond(int32 CardID, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveBlitz(int32 CardID, int32 giverID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveAmbush(int32 CardID, int32 giverID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GiveAlpine(int32 CardID, int32& qqq, int32 giverID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetUnitTypeCountOnBoard(ESideEnum SideToGet, int32& TypeCount);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetUnitsToTheRight(UBaseCardObject* card, TArray<UBaseCardObject*>& units);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetUnitsToTheLeft(UBaseCardObject* card, TArray<UBaseCardObject*>& units);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetUnitDestroyedThisTurn(bool& UnitDestroyed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetTurnNumber(int32& TurnNumber);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetTotalKreditsLostThisBattle(ESideEnum side, int32& kreditsLost);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetTargetedCard(UBaseCardObject* callerCard, bool& hasTarget, UBaseCardObject*& card);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetSupportLineLocationBySide(ESideEnum side, ECardLocationEnum& Location);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetStartingSide(ESideEnum& side);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetRightMostCardInHand(UBaseCardObject* card, bool& WasFound, UBaseCardObject*& RightMostCard);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetRandomCard(UPARAM(Ref) TArray<UBaseCardObject*>& Cards, bool skipCustomAlways, UBaseCardObject*& randomCard);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetPlayingSide(ESideEnum& OutPlayingSide);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetOpponentSide(ESideEnum& OutOpponentSide);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetOperationKreditsSpentThisTurn(int32& kreditsSpentThisTurn);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetMaxKreditsBySide(ESideEnum side, int32& maxkredits);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetMainNationForSide(ESideEnum side, EFactionEnum& mainNation);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetLocationCardBySide(UBaseCardObject*& card, int32& locationCardID, ESideEnum side);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetLeftMostCardInHand(UBaseCardObject* card, bool& WasFound, UBaseCardObject*& LeftMostCard);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetKreditsBySide(ESideEnum side, int32& kredits);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetHQ_DamagedAmountThisTurnBySide(ESideEnum SideToGet, int32& damagedAmount);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetHandLocationBySide(ESideEnum side, ECardLocationEnum& handLocation);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void getFrontlineLimit(bool& isFrontlineLimited);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetDestroyedCardsIDsThisBattle(TArray<int32>& destroyedCardsIDs);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetDestroyedCardsIDsByTurn(int32 turnID, TArray<int32>& destroyedCardsIDs);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetDestroyedCardsCountBySide(ESideEnum SideToGet, int32& count);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetDeckLocationBySide(ESideEnum side, ECardLocationEnum& deckLocation);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetDeckByside(ESideEnum side, TArray<int32>& deckCardIDs);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetClientSide(ESideEnum& OutClientSide);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetCardsPlayedThisTurn(TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetCardsPlayedFromHandThisTurn(TArray<int32>& CardIDsPlayedThisTurn);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetCardsPlayedFromHandLastTurn(TArray<int32>& CardIDsPlayedLastTurn);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetCardsPlayedFromHandByTurnNumber(int32 TurnNumber, TArray<int32>& CardIDsPlayed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetCardsOnBoardBySide(ESideEnum side, bool unitsOnly, bool includeCovertCards, TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetCardsInSupportLineBySide(ESideEnum side, bool unitsOnly, bool includeCovertCards, TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetCardsInHandBySide(ESideEnum side, TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetCardsInFrontlineBySide(ESideEnum side, bool includeCovertCards, TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetCardFromID(int32 CardID, UBaseCardObject*& card);

    // The stock BP_CardFunctions exposes the notifier through this pure
    // Blueprint event.  Card blueprints call it through the CardFunctionsStub
    // base pointer, so the signature must also exist on this editor shim.
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void GetCardFunctionsNotifier(UObject*& OutNotifier);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetAllyNationForSide(ESideEnum side, EFactionEnum& allyNation);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetAllUnitsOnBoard(bool includeCovertCards, TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetAllForecastCards(bool includeNotAttainable, bool includeReserved, TArray<UBaseCardObject*>& forecastCards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetAllCardsPlayedThisBattle(TArray<int32>& playedCardsIDs);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetActiveGotchasOrdered(TArray<int32>& cardIDs);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetAllCardsOnBoard(bool includeCovertCards, TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetAllCardsInFrontline(bool includeCovertCards, TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetAllCards(TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetAllActiveStaticCards(bool includeNotAttainable, bool includeReserved, TArray<UBaseCardObject*>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GetAdjacentCards(UBaseCardObject* card, bool includeCovertCards, TArray<UBaseCardObject*>& adjacentCards);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void Get_X_AndMoreAttackCardsOnBoard(ESideEnum side, TArray<int32>& cardsIDs, int32 attack, bool includeCovert);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void GainKreditSlot(UBaseCardObject* cardGivingKreditSlot, ESideEnum side);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void FullyHealCard(UBaseCardObject* card, int32 healerCardID, int32& HealedAmount);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void Forecast(UBaseCardObject* cardTriggeringForecast, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ForceEndTurn();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ResolveTriggerQueue();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ForceCardChangeLocation(int32 CardID, int32 instigatorID, ECardLocationEnum NewLocation, int32 newLocationNumber, bool& moved, ECardLocationEnum& OldLocation, int32& oldLocationNumber);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void EndMatch(ESideEnum winnerSide, float Delay);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DrawSpecificCardFromDeckBySide(int32 instigatorID, int32 CardID, ESideEnum side, bool cardSeen);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DrawCardsFromDeckBySide(int32 instigatorID, ESideEnum side, int32 NumCards, bool cardSeen, bool OpponentDraw, TArray<int32>& cardsIDs, float drawDelay);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void DoesSideEffectTagExist(ESideEnum SideToCheck, FGameplayTag EffectTag, bool& Exist);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void DoesSideControlTheFrontline(ESideEnum side, bool& doesHe);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DiscardRandomCardFromHand(ESideEnum side, int32 discarderID, int32& discardedCardID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DiscardCardFromHand(int32 CardID, int32 discarderID, bool skipSubAction, bool ForceDiscard, bool& success);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DiscardCardFromDeck(int32 CardID, int32 discarderID, bool skipTriggers, bool skipVisuals, bool& success);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DestroyMultipleCards(const TArray<int32>& cardsToDestroy, int32 destroyerCardID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DestroyCard(UBaseCardObject* card, UBaseCardObject* destroyerCard);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DecrementCountdown(int32 CardID, bool& countdownFinished, bool& countdownFound);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DamageMultipleCards(const TArray<int32>& receiverIDs, int32 amount, int32 damagerCardID, TArray<int32>& cardsDestroyed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void DamageCard(UBaseCardObject* card, int32 amount, int32 damagerCardID, bool isRedirected, bool fromFight, bool isFightDefenderDamage, bool& targetDestroyed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void CustomAbilityRemove(const FString& ability, int32 CardID, int32 giverID, bool RemoveAllGivers, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void CustomAbilityAdd(const FString& ability, int32 CardID, int32 giverID, bool skipBuffTexts, bool skipSubActions, bool skipAddToBattlelog, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ConvertCard(const TArray<int32>& CardIDs, int32 instigatorID, FName convertToCardName, int32 convertIntoCardID, bool skipTrigger, TArray<int32>& newCardIDs);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ChangeOperationCost(UBaseCardObject* card, int32 instigatorID, int32 amount, EChangeType ChangeType, bool isBuff, bool skipAction, bool skipAddToBattlelog);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ChangeKreditCost(UBaseCardObject* card, int32 instigatorID, int32 amount, EChangeType ChangeType, bool skipCovertCheck, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ChangeHeavyArmor(UBaseCardObject* card, int32 instigatorID, int32 amount, EChangeType ChangeType, bool skipAction, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ChangeFrontlineLimiter(int32 limiter, bool Remove, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ChangedPinnedTurns(int32 CardID, int32 instigatorID, int32 turnsToChange, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ChangeDefense(UBaseCardObject* card, int32 instigatorID, int32 amount, EChangeType ChangeType, bool skipAction, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ChangeAttack(UBaseCardObject* card, int32 instigatorID, int32 amount, EChangeType ChangeType, bool skipAction, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void ApplyGameplaySideEffect(ESideEnum SideToApply, FGameplayTag EffectTag, UBaseCardObject* InstigatorCard, EDurationPolicy DurationPolicy, int32 turnsToLast, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
    void AppendText(UPARAM(Ref) FText& firstText, UPARAM(Ref) FText& secondText, FText& Text);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void AdjustCardPositionInDeck(ESideEnum deckSide, int32 cardToAdjust, int32 positionFromTop, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void AddToBattleLog(int32 CardID, int32 instigatorID, bool cardHidden, bool showAnimation);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void AddNumberToText(int32 CardID, UPARAM(Ref) FText& numberText, EStatsTextColor Color, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void AddKreditsTax(UBaseCardObject* card, int32 costToAdd, int32 instigatorID, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void AddIntelToCard(int32 CardID, int32 instigatorID, int32 amount, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void AddGameplayRestriction(ESideEnum side, EGameplayRestrictions Type, int32 CardID, int32 turnsToLast);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void AddDefenseToMultipleCards(const TArray<int32>& receiverIDs, int32 amount, int32 giverCardID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void AddCustomGameplayTag(FGameplayTag TagToAdd, int32 cardToAdd, int32 instigatorID, int32& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void AddAttackUntilEndOfTurn(UBaseCardObject* card, int32 instigatorID, int32 attackToAdd);
    
};


