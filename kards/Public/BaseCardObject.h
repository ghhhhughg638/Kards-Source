#pragma once
#include "CoreMinimal.h"
#include "BlueprintJsonObject.h"
#include "ECardSetEnum.h"
#include "EFactionEnum.h"
#include "ERarityEnum.h"
#include "ETypeEnum.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
//CROSS-MODULE INCLUDE V2: -ModuleName=BlueprintJson -ObjectName=BlueprintJsonObject -FallbackName=BlueprintJsonObject
//CROSS-MODULE INCLUDE V2: -ModuleName=CoreUObject -ObjectName=Vector2D -FallbackName=Vector2D
//CROSS-MODULE INCLUDE V2: -ModuleName=Engine -ObjectName=PrimaryDataAsset -FallbackName=PrimaryDataAsset
//CROSS-MODULE INCLUDE V2: -ModuleName=GameplayTags -ObjectName=GameplayTag -FallbackName=GameplayTag
//CROSS-MODULE INCLUDE V2: -ModuleName=GameplayTags -ObjectName=GameplayTagContainer -FallbackName=GameplayTagContainer
//CROSS-MODULE INCLUDE V2: -ModuleName=KardsCore -ObjectName=ECardSetEnum -FallbackName=ECardSetEnum
//CROSS-MODULE INCLUDE V2: -ModuleName=KardsCore -ObjectName=EFactionEnum -FallbackName=EFactionEnum
//CROSS-MODULE INCLUDE V2: -ModuleName=KardsCore -ObjectName=ERarityEnum -FallbackName=ERarityEnum
//CROSS-MODULE INCLUDE V2: -ModuleName=KardsCore -ObjectName=ETypeEnum -FallbackName=ETypeEnum
#include "CardBuffData.h"
#include "CardImageInfo.h"
#include "CardsGivingAbility.h"
#include "ChooseOneCardStruct.h"
#include "ECardEffectTypes.h"
#include "ECardLocationEnum.h"
#include "ECombatKeyword.h"
#include "EHelpBubbleTypes.h"
#include "EImageAnchor.h"
#include "EOnEnterPlayMethod.h"
#include "EOnLeavePlayMethod.h"
#include "ERegisteredCardFunction.h"
#include "ESideEnum.h"
#include "EVisualCardWeight.h"
#include "EnumChooseOneCardBeingPlayed.h"
#include "GameplayEffect.h"
#include "Templates/SubclassOf.h"
#include "BaseCardObject.generated.h"

class ACardFunctionsStub;
class AVisualCardActionBaseStub;
class UBaseCardObject;
class UCameraShakeBase;
class UNiagaraSystem;
class UParticleSystem;
class USoundBase;
class UTexture;
class UTexture2D;

// The editor shim exposes this metadata so K2Node_GetSubsystem keeps its
// WorldContext pin when card blueprints are compiled.  The shipped game uses
// the official BaseCardObject class; this is only an editor/baking aid.
UCLASS(Blueprintable, meta=(ShowWorldContextPin))
class KARDS_API UBaseCardObject : public UPrimaryDataAsset {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FGameplayTagContainer GameplayTags;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FName Name;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FText title;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ETypeEnum Type;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 attack;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 attackBuff;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 defense;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 range;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    EFactionEnum faction;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 kredits;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 kreditsBuff;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 KreditsTax_AsEnemyTarget;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FText Text;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FText flavorText;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 operationCost;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 operationCostBuff;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TMap<int32, FCardBuffData> buffsFromCards;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<int32> cardsBuffedByThisCard;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 effectDamage;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ERarityEnum rarity;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool selectTargetOnPlayedFromHand;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool waitWithOnOtherCardPlayedUntilHandTargetSelected;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FText selectHandCardNotifyText;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ECardSetEnum cardSet;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ECardSetEnum returnSet;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isReserved;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isInPermanentPool;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool showBoostIcon;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ECardEffectTypes EffectType;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<FChooseOneCardStruct> chooseOneCards;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<ERegisteredCardFunction> usedTriggers;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSet<EHelpBubbleTypes> extraHelpTextBubbleTypes;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool campaignUpgradable;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FText campaignName;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    float draftBaseWeight;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    float draftAllyWeight;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<ERegisteredCardFunction> suppressionExceptionTriggers;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasDeployment;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasDestruction;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasBlitz;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasAmbush;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 heavyArmor;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 heavyArmorBuff;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasSmokescreen;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasMobilize;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasAlpine;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasFury;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasGuard;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasShock;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasCovert;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString spawnCardName;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasScrying;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TMap<FString, FCardsGivingAbility> receivedAbilitiesFromCards;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasPincer;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 cipher;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool cardSeen;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    EFactionEnum exileNation;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasSalvage;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 CardID;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 movementLeft;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 attackLeft;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 enterPlayOnTurn;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasEverAttacked;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ECardLocationEnum Location;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ESideEnum side;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ESideEnum originalSide;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 locationNumber;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 pinnedTurns;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasBeenAttackedThisTurn;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool hasAttackedThisTurn;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 attackCountThisTurn;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isBeingGuarded;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isImmune;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<int32> cardsGivingImmunity;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool underEnemyControl;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 maxAttack;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 maxDefense;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    UBaseCardObject* CurrentTarget;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSet<EHelpBubbleTypes> helpTextBubbleTypes;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 gotchaActivated;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isGoldCard;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool handCardLookDirty;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 chooseOneIndex;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString spawnCardChosen;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TArray<int32> activeUpgrades;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isSalvaged;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    EFactionEnum salvageFaction;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isSuppressed;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isRevealed;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    int32 cardHighlightBits;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    USoundBase* soundEnterPlay;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    USoundBase* soundMoveToFrontline;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    USoundBase* soundAttack;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    USoundBase* soundBlowUp;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    USoundBase* soundProjectileImpact;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    USoundBase* soundProjectileFiring;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSoftObjectPtr<UTexture> Image;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSoftObjectPtr<UTexture> imagePointer;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    EImageAnchor ImageAnchor;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    float Delay;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString artist;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSoftObjectPtr<UTexture2D> historyPhoto;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FVector2D historyPhotoSize;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString historyPhotoSource;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString historyPhotoIdentificationCode;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString historyPhotoAuthor;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FText historyPhotoDescription;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FText historyTypeDescription;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    UParticleSystem* blowUpExplosion;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    UNiagaraSystem* blowUpExplosionNiagara;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSubclassOf<UCameraShakeBase> blowUpCameraShake;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSoftClassPtr<AVisualCardActionBaseStub> OnAttackVisualsSoft;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSoftClassPtr<AVisualCardActionBaseStub> OnPlayedFromHandVisualsSoft;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    TSoftClassPtr<AVisualCardActionBaseStub> OnChooseOneCardVisualsSoft;
    
    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    UNiagaraSystem* OnPlayedFromHandVisualsNiagara;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    UTexture* planeShadow;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    EVisualCardWeight visualCardWeight;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FName customName1;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FName customName2;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FBlueprintJsonObject customJson;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString Notes;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isDraw;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isDiscard;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isRemoval;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isDirectDamage;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isHQDamage;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isHQRepair;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isRepair;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isKreditsBuff;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isAttackBuff;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool isDefenseBuff;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    float threat_level;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    UBaseCardObject* targetOverride;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    ACardFunctionsStub* cardFunction;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool skipCardMarkers;
    
    UBaseCardObject();

    UFUNCTION(BlueprintCallable)
    void WhichChooseOne(EnumChooseOneCardBeingPlayed& Branches);
    
    UFUNCTION(BlueprintCallable)
    void UpdateExtraPlayTriggers(bool& triggerNext);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ShouldHighlightInHand(bool& shouldHighlight);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void ShouldGotchaTrigger(UBaseCardObject* triggerCard, bool& shouldIt);
    
    UFUNCTION(BlueprintCallable)
    void SetImageDataTableRow(FCardImageInfo NewRow);
    
    UFUNCTION(BlueprintCallable)
    void SetExtraPlayTriggers(int32 extraTriggers);
    
    UFUNCTION(BlueprintCallable)
    void setEncryptionKey(int32 NewKey);
    
    UFUNCTION(BlueprintCallable)
    void setAndEncryptKreditBuff(int32 newKreditBuff, int32 key1, int32 key2, int32 FrameCount);
    
    UFUNCTION(BlueprintCallable)
    void setAndEncryptKredit(int32 newKredit, int32 key1, int32 key2, int32 FrameCount);
    
    UFUNCTION(BlueprintCallable)
    void setAndEncryptDefense(int32 newDefense, int32 key1, int32 key2, int32 FrameCount);
    
    UFUNCTION(BlueprintCallable)
    void setAndEncryptCardType();
    
    UFUNCTION(BlueprintCallable)
    void setAndEncryptAttackBuff(int32 newAttackBuff, int32 key1, int32 key2, int32 FrameCount);
    
    UFUNCTION(BlueprintCallable)
    void setAndEncryptAttack(int32 newAttack, int32 key1, int32 key2, int32 FrameCount);
    
    UFUNCTION(BlueprintCallable)
    void ResetCardAttributes();
    
    UFUNCTION(BlueprintCallable)
    void RemoveGameplayEffectByInstigator(FGameplayTag EffectTag, UBaseCardObject* InstigatorCard);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnSurvivedCombat(UBaseCardObject* cardCombated);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnSuppressed();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnSuccesfulDiscard(int32 discarderID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnStartOfTurn(int32 TurnNumber);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnStartOfGame();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnReceiveDamage(UBaseCardObject* fromCard, int32 Damage);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnPlayedFromHand(UBaseCardObject* targetCard);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnPincerEffectRemoved(UBaseCardObject* card);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnPincerEffectReceived(UBaseCardObject* PincerGiver);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnPincerEffectApplied(UBaseCardObject* card);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherUnitUnpinned(UBaseCardObject* card);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherUnitPinned(UBaseCardObject* cardBeingPinned);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCovertCardSpawned(UBaseCardObject* cardSpawned);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCovertCardPlayedFromHand(UBaseCardObject* cardPlayed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardSurvivedCombat(UBaseCardObject* survivor, UBaseCardObject* cardCombated);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardSuppressed(UBaseCardObject* card);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardSpawnedInHand(int32 spawnedCardID, ESideEnum spawnedSide);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardSalvaged(int32 cardSalvagedID, int32 newCardSalvagedID, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardRevealed(UBaseCardObject* cardBeingRevealed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardRetreat(UBaseCardObject* cardRetreated, bool& stopAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardReset(UBaseCardObject* cardReset, int32 resetCardID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardReceiveDamage(UBaseCardObject* fromCard, UBaseCardObject* toCard, bool fromAttack, int32 Damage);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardPlayedFromHand(UBaseCardObject* cardPlayed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardMoveToFrontline(UBaseCardObject* cardMoved, bool ForceMove, int32 moveCost);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardMoveFromFrontline(UBaseCardObject* cardMoved);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardMovedToLocationInHand();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardLoseSmokescreen(UBaseCardObject* card);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardLocationMoved(UBaseCardObject* cardMoved, ECardLocationEnum OldLocation, ECardLocationEnum NewLocation, bool ChangeOwner, const FString& MoveReason);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardLeaveBoardOrOwner(UBaseCardObject* cardLeaving, ECardLocationEnum goingToLocation, EOnLeavePlayMethod Method);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardKreditCostChanged(int32 cardChangingCost);

    // These two events are present on the shipped BaseCardObject even though
    // older UHT dumps omitted them from the generated header.  BP_CardFunctions
    // dispatches them when an operation cost is spent; exposing the exact
    // parent signatures lets restored card blueprints override them normally.
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOperationKreditsSpent(int32 kreditsSpent);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardOperationKreditsSpent(UBaseCardObject* cardOperated, int32 kreditsSpent);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardFullyRepaired(UBaseCardObject* cardRepaired, int32 amount);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardForecasted(UBaseCardObject* cardDeveloped, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardEnterPlay(UBaseCardObject* cardPlayed, EOnEnterPlayMethod Method);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardDrawnFromDeck(int32 drawnCardID, bool StartOfTurnDraw, ESideEnum drawnSide);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardDiscarded(UBaseCardObject* discardedCard, int32 discarderID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardDeveloped(UBaseCardObject* cardDeveloped, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardDestroyed(UBaseCardObject* cardDestroyed, UBaseCardObject* killer, bool TriggerNotDestroyed, ECardLocationEnum destroyedLocation, bool selfIsAlsoGettingDestroyed, bool destroyedInCombat);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnOtherCardDealDamageAddDamageAfterCalc(UBaseCardObject* cardDealingDamage, UBaseCardObject* toCard, int32 Damage, bool fromAttack, bool isRedirected, int32& damageToAdd, bool& stopAdding);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnOtherCardDealDamageAddDamage(UBaseCardObject* cardDealingDamage, UBaseCardObject* toCard, int32 Damage, bool fromAttack, bool isDefenderDamage, int32& damageToAdd, bool& reRunAtEnd);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardDealDamage(UBaseCardObject* cardDealingDamage, UBaseCardObject* toCard, int32 Damage, bool isCombatDamage, bool CounterDamage, bool isRedirected);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardCreatedAlterCard(UBaseCardObject* cardPlayed, EOnEnterPlayMethod Method);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardConverted(const TArray<int32>& oldCardIDs, const TArray<int32>& newCardIDs, FName newCardName, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardBlitzChanged(UBaseCardObject* cardChanged);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardBecomingVeteran(UBaseCardObject* cardBecomingVeteran);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnOtherCardAttackSwitchTarget(UBaseCardObject* cardAttacking, UBaseCardObject* oldDefender, UBaseCardObject*& newDefender);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnOtherCardAttacks(UBaseCardObject* cardAttacking, UBaseCardObject* defenderCard, bool& stopAttack, bool& AttackedAndStopped);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnOtherCardAbilitiesChanged(UBaseCardObject* cardChanging);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnMoveToFrontline(bool ForceMove, int32 moveCost);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnMoveFromFrontline();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnLoseMobilize();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnLeaveBoardOrOwner(ECardLocationEnum goingToLocation, EOnLeavePlayMethod leavePlayMethod);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnIntelTriggered(UBaseCardObject* intelCard, int32 intelValue);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnHandTargetSelected(int32 handTargetCardID, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnGainMobilize();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnFullyRepaired(int32 amount);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnFrontlineOwnershipChange(int32 Instigator, ESideEnum oldSide, ESideEnum NewSide);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnFatigueDamage(ESideEnum deckSide, int32 Damage, bool fromBond);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnEnterPlay(EOnEnterPlayMethod Method);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnEndOfTurn(int32 TurnNumber);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnDestructionEffectTriggered(UBaseCardObject* cardTriggered, int32 instigatorID, bool SelfAlsoDestroyed, int32& TriggerMultiple);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnDestroyed(UBaseCardObject* killer, bool TriggerNotDestroyed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnDeploymentEffectTriggered(UBaseCardObject* cardTriggered, int32 instigatorID, int32& TriggerMultiple);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnDeckShuffled(ESideEnum deckSide, UBaseCardObject* InstigatorCard);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnDealDamageAddDamageAfterCalc(UBaseCardObject* toCard, int32 Damage, bool fromAttack, bool isAttacker, bool isRedirected, int32& damageToAdd);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCustomFieldsReceiveChanges();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCreateCardApplyCampaignUpgrades();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCreateCard();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCovertCardPlayedFromHand();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCounterMeasureTriggered(UBaseCardObject* countermeasureTriggering, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCardSpawnedInHand(ESideEnum spawnedSide);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCardRevealed();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCardReset();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCardLocationMoved(ECardLocationEnum OldLocation, ECardLocationEnum NewLocation, bool ChangeOwner, const FString& MoveReason);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCardDrawnFromDeck(bool StartOfTurnDraw, ESideEnum drawnSide);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnCardDealDamage_ModifyDamageDealt(UBaseCardObject* toCard, int32 Damage, bool fromAttack, bool fromFight, int32& newDamage);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnCardDealDamage(UBaseCardObject* toCard, int32 Damage, bool isCombatDamage, bool CounterDamage, bool isRedirected, bool& qqq);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBeforeStartOfTurn();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBeforeRetreat(bool& stopAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBeforeOtherCardPlayedFromHand(UBaseCardObject* cardPlayed);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnBeforeOtherCardLoseMobilize(UBaseCardObject* card, bool& dontLoseMobilize);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnBeforeOtherCardGainDefenseAfterAdd(UBaseCardObject* cardGainingDefense, int32 defenseGaining, bool& stopAction);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnBeforeOtherCardGainDefense(UBaseCardObject* cardGainingDefense, int32 defenseToAdd, int32& newDefenseToAdd, bool& stopAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBeforeOtherCardDestroyed(UBaseCardObject* cardDestroyed, UBaseCardObject* killer, bool TriggerNotDestroyed, bool destroyedInCombat);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnBeforeOtherCardDeploymentTrigger(UBaseCardObject* cardDeploying, bool& cancelDeploymentEffect);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBeforeOtherCardAttacks(UBaseCardObject* cardAttacking, UBaseCardObject* defenderCard);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBeforeGainDefense(bool& stopAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBeforeFullyRepaired(UBaseCardObject* cardRepaired, bool& stopAction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBeforeDestroyed(UBaseCardObject* killer, bool TriggerNotDestroyed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBeforeAttack(UBaseCardObject* defenderCard);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnBecomingVeteran();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAttemptedDiscard(bool& cancelDiscard, int32 discarderID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAttackStopped();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOtherCardSuppressed(UBaseCardObject* card);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOtherCardOperactionCostChanged(int32 cardToChange, int32 Instigator);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOtherCardOperactionCostBuffsReset(int32 cardToChange, int32 Instigator);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOtherCardLeaveBoardOrOwner(UBaseCardObject* cardLeaving, ECardLocationEnum OldLocation);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOtherCardGainDefense(UBaseCardObject* cardGainingDefense, int32 defenseGained);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOtherCardDefenseIsSet(UBaseCardObject* cardDefenseSet);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOtherCardChangeAttack(UBaseCardObject* cardChanged, int32 changedAmount, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOtherCardAttacks(UBaseCardObject* defenderCard, UBaseCardObject* attackerCard, int32 damageToDefender, int32 attackCost);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOperationCostChanged(int32 Instigator);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterLeaveBoard(ECardLocationEnum goingToLocation);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterKreditCostChanged(int32 Instigator);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterGainDefense(int32 defenseGained);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterExtraKreditSlotGain(UBaseCardObject* cardGivingKredit, ESideEnum sideGaining, bool isNegativeGain);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterDestroyed(UBaseCardObject* killer, bool TriggerNotDestroyed);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterDefenseIsSet();
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterDeckChanged(ESideEnum deckSide);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterChangeAttack(int32 changedAmount, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterCardMovedToDeckFromHand();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterOtherCardMovedFromHandToDeck(UBaseCardObject* cardMoved, int32 instigatorID);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void OnAfterAttack(UBaseCardObject* defenderCard, bool wasShockAttack, int32 attackCost);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsWeatherCard(bool& NewIsWeatherCard);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsForecastCard(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsVeteran(bool ignoreSuppress, bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void IsValidHandTarget(UBaseCardObject* targetedCard, bool& isIt, FString& Reason);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsUnrevealedCovertCard(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsUnit(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsTankDestroyer(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsTank(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsSideActive(ESideEnum SideToCheck, bool& Active);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsSameSideUnit(ESideEnum SideToCheck, bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsPinned(bool& NewIsPinned);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsOwnedByClientSide(bool& IsOwned);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsOrder(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsLocation(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsLocatedOnBoard(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsLocatedInHand(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsLocatedInDeck(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsInfantry(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsGunUnit(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsGroundUnit(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsGotcha(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsFighter(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsExile(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsDamaged(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsChooseOneCard(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void isBuffedByCard(int32 instigatorID, bool& isBuffed);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsBomber(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsArtillery(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsArmorUnit(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsAntiTank(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsAntiAir(bool& isIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsAirUnit(bool& isIt);
    
    UFUNCTION(BlueprintCallable)
    void InitStaticGameplayTags();
    
    UFUNCTION(BlueprintCallable)
    void InitializeEncryption();
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasMovementLeft(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasIntel(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasCustomAbilityFromCard(const FString& ability, int32 giverID, bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasCustomAbility(const FString& ability, bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasCantBeAttackedBy(const FString& unitType, bool& hasIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasCantAttackType(const FString& cardType, bool& hasIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasCantAttack(bool& hasIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasCampaignUpgrade(int32 campaignUpgrade, bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasBond(bool& hasIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void HasAttackLeft(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void hasActivePincerEffect(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void hasActiveCountdownEffect(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getTotalOperationCost(int32& totalOperationCost);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getTotalKreditCost(int32& totalKreditCost);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getTotalHeavyArmor(int32& totalHeavyArmor);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getTotalDefense(int32& totalDefense);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getTotalAttack(int32& totalAttack);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getStaticVeteranUpgrade(UBaseCardObject*& staticVeteranCard);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    FGameplayTagContainer GetRuntimeEffectTags() const;
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure)
    void GetPlayFromHandDamage(UBaseCardObject* targetCard, int32& Damage);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure)
    void GetPassiveDefenseBuff(int32 incomingDamage, int32& amount);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetOppositeSide(ESideEnum& oppositeSide);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    TSubclassOf<AVisualCardActionBaseStub> GetOnPlayedFromHandVisuals() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    TSubclassOf<AVisualCardActionBaseStub> GetOnChooseOneCardVisuals() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    TSubclassOf<AVisualCardActionBaseStub> GetOnAttackVisuals() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getKreditTempBuffAmount(int32 instigatorID, int32& tempAmount);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getIntel(int32& intel);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetImageDataTableRow(FCardImageInfo& OutRow);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasVeteranUpgrade(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasSmokescreen(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasShock(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasImmune(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasGuard(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasGameplayTag(FGameplayTag inputTag, bool& HasTag) const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasFury(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasBlitz(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasAmbush(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getHasAlpine(bool& doesIt);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getExileNation(EFactionEnum& exileFaction);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetCustomName2Attributes(TArray<FString>& Attributes);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getCountdownValue(int32& countdownValue);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetCombatKeywords(TArray<ECombatKeyword>& Keywords, int32& numberOfKeywords);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void GetChooseSpawnCards(TArray<UBaseCardObject*>& Cards, bool& MarkAsSeen, bool& keepOrder);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getCardsBuffedByThisCard(TArray<int32>& Cards);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getbuffsFromCardsAsJsonString(FString& buffsFromCardsJsonString);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure)
    void GetBeforeAttackDefenseBuff(bool asAttacker, int32& amount);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure)
    void GetBeforeAttackDamage(bool asAttacker, int32& amount);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure)
    void GetBeforeAttackAttackBuff(bool asAttacker, int32& amount);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getAttackTempBuffAmount(int32 instigatorID, int32& tempAmount);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getAndDecryptKreditBuff(int32& decryptedKreditBuff);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getAndDecryptKredit(int32& decryptedKredit);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getAndDecryptDefense(int32& decryptedDefense);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getAndDecryptAttackBuff(int32& decryptedAttackBuff);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void getAndDecryptAttack(int32& decryptedAttack);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetAllChildrenGameplayTagsAsStrings(FGameplayTag ParentTag, bool TagContainerOnly, TArray<FString>& ChildrenTags, bool& hasChildrenTags) const;
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void FilterCardsToScry(UBaseCardObject* card, bool& IsValid);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool DoesEffectTagExist(FGameplayTag EffectTag) const;
    
    UFUNCTION(BlueprintCallable)
    void CustomName2Remove(const FString& Attribute);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void CustomName2HasAttribute(const FString& Attribute, bool& doesIt);
    
    UFUNCTION(BlueprintCallable)
    void CustomName2Add(const FString& Attribute);
    
    UFUNCTION(BlueprintCallable)
    void CustomName1Remove(const FString& Attribute);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void CustomName1HasAttribute(const FString& Attribute, bool& doesIt);
    
    UFUNCTION(BlueprintCallable)
    void CustomName1Add(const FString& Attribute);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    UBaseCardObject* CreateCopy();
    
    UFUNCTION(BlueprintCallable)
    void CopyData(UBaseCardObject* toCard);
    
    UFUNCTION(BlueprintCallable)
    void CardSimulateTampering();
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void CanPlayFromHand(bool& canIt, FString& Reason, FString& reasonParam1, FString& reasonParam2, UBaseCardObject*& targetedCard);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void CanOtherCardBeTargetted(bool& canIt, FString& Reason, FString& reasonParam1, FString& reasonParam2, UBaseCardObject* targettingCard, UBaseCardObject* targetCard, bool byPlayFromHand);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void CanMoveAndAttackInTheSameTurn(bool& canIt);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void CanBeTargetted(bool& canIt, FString& Reason, FString& reasonParam1, FString& reasonParam2, UBaseCardObject* targettingCard, bool byPlayFromHand);
    
    UFUNCTION(BlueprintCallable)
    void CampaignSetTitle(UPARAM(Ref) FText& TextValue);
    
    UFUNCTION(BlueprintCallable)
    void CampaignSetText(UPARAM(Ref) FText& TextValue, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignSetOpCost(int32 Value, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignSetKreditCost(int32 Value, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignSetHeavyArmor(int32 Value, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignSetDefense(int32 Value, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignSetAttack(int32 Value, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignRemoveSmokescreen(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignRemoveMobilize(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignRemoveGuard(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignRemoveFury(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignRemoveBlitz(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignRemoveAmbush(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void CampaignOnEndOfGame();
    
    UFUNCTION(BlueprintCallable)
    void CampaignIncreaseHeavyArmor(int32 increaseValue, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddSmokescreen(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddOpCost(int32 Value, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddMobilize(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddKreditCost(int32 Value, bool secondUpgrade, bool Clear);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddGuard(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddFury(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddDefense(int32 Value, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddBlitz(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddAttack(int32 Value, bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable)
    void CampaignAddAmbush(bool secondUpgrade);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void BlockCardFromBeingPlayedFromHand(bool& bLocked, FString& Reason, FString& reasonParam1, FString& reasonParam2);
    
    UFUNCTION(BlueprintCallable)
    void ApplyGameplayEffect(FGameplayEffect EffectToApply);
    
};

