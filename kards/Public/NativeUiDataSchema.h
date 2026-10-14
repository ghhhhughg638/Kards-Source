#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "ECardSetEnum.h"
#include "EFactionEnum.h"
#include "EnumCampaignStrategy.h"
#include "ESideEnum.h"
#include "BlueprintJsonObject.h"
#include "NativeUiDataSchema.generated.h"

// Local native-schema compatibility types required to deserialize the
// corresponding cooked /Script/kards properties in the editor project.
// These declarations are based on the current FModel USMAP, not guessed
// Blueprint shells. They are editor-project shims and must not be shipped as
// a replacement for the game's canonical kards module.

UENUM(BlueprintType)
enum class EDailyMissionTriggersEnum : uint8
{
	NotAvailable = 0,
	OnPlayedFromHand = 1,
	OnMoveToFrontline = 2,
	OnDealDamage = 3,
	OnCardDestruction = 4,
	AtEndOfGame = 5,
	OnDailyMissionCompleted = 6,
	AtEndOfTurn = 7,
	OnDraftDeckCreated = 8,
	OnDraftEnded = 9,
	OnMissionFinished = 10,
	EDailyMissionTriggersEnum_MAX = 11 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EUGCTextFilterOutcome : uint8
{
	Clean = 0,
	Modified = 1,
	Rejected = 2,
	Unavailable = 3,
	Failed = 4,
	EUGCTextFilterOutcome_MAX = 5 UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct KARDS_API FUGCTextFilterResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EUGCTextFilterOutcome Outcome = EUGCTextFilterOutcome::Clean;

	UPROPERTY(BlueprintReadOnly)
	FString OutputText;

	UPROPERTY(BlueprintReadOnly)
	bool bOperationSucceeded = false;

	UPROPERTY(BlueprintReadOnly)
	bool bWasModified = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FWidgetBackStackEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UUserWidget> BackHandlerWidget;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UWidget> PreviouslyFocusedWidget;

	UPROPERTY(BlueprintReadWrite)
	bool bIsHighPriority = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsBeingProcessed = false;
};

DECLARE_DYNAMIC_DELEGATE(FWidgetInputActionDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FWidgetInputActionAxisDelegate, float, AxisValue);

USTRUCT(BlueprintType)
struct KARDS_API FWidgetInputAction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UUserWidget> Widget;

	UPROPERTY(BlueprintReadWrite)
	FWidgetInputActionDelegate ActionCallback;

	UPROPERTY(BlueprintReadWrite)
	FWidgetInputActionAxisDelegate AxisCallback;
};

USTRUCT(BlueprintType)
struct KARDS_API FWidgetInputActionList
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FWidgetInputAction> Widgets;
};

#if !WITH_EDITOR
// The JMAP size describes the game's Shipping ABI. The editor uses a distinct
// weak-pointer/delegate representation, so only enforce this assertion for game builds.
static_assert(sizeof(FWidgetInputAction) == 40, "WidgetInputAction JMAP layout changed.");
#endif
static_assert(sizeof(FWidgetInputActionList) == 16, "WidgetInputActionList JMAP layout changed.");

USTRUCT(BlueprintType)
struct KARDS_API FActionValue2
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text;
};

USTRUCT(BlueprintType)
struct KARDS_API FSubAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FActionValue2> Values;
};

USTRUCT(BlueprintType)
struct KARDS_API FAction2
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 action_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString action_type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FActionValue2> action_data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSubAction> sub_actions;
};

USTRUCT(BlueprintType)
struct KARDS_API FSubActionBatch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSubAction> subActions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Delay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAction2 owningAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isFirstBatchInAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isLastBatchInAction = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FSubActionWithMappedValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> integerMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> stringMap;
};

USTRUCT(BlueprintType)
struct KARDS_API FVisualAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSubActionWithMappedValues SubAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float delayBefore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float delayAfter = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool finishManually = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FCardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 kredits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 attack = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 defense = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 range = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName faction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName soundEnterPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName soundMoveToFrontline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName soundAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString flavorText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName rarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 operationCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool is_gold = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 card_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName side;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 location_number = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 movement_left = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 attack_left = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 enter_play_on_turn = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool has_ever_attacked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 pinnedTurns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasBlitz = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasAmbush = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 heavyArmor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasSmokescreen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasFury = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasGuard = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasMobilize = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasAlpine = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasBeenAttackedThisTurn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasAttackedThisTurn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isBeingGuarded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 maxAttack = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 maxDefense = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 gotchaActivated = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName customName1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName customName2;
};

USTRUCT(BlueprintType)
struct KARDS_API FCampaignCardStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 attack = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 defense = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 kredits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 operationCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 heavyArmor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasAmbush = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasBlitz = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasGuard = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasFury = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasSmokescreen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasMobilize = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FCampaignCardUpgrades
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> upgrades;
};

USTRUCT(BlueprintType)
struct KARDS_API FCampaignScenarioStars
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 scenario = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> stars;
};

USTRUCT(BlueprintType)
struct KARDS_API FCampaign
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString campaign_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 scenario = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EnumCampaignStrategy strategy = EnumCampaignStrategy::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCampaignCardUpgrades> card_upgrades;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString last_card_upgraded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> pending_upgrades;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 high_score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCampaignScenarioStars> scenario_stars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EnumCampaignStrategy> strategies_done;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EnumCampaignStrategy first_strategy = EnumCampaignStrategy::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool purchased = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> rewards_given;
};

USTRUCT(BlueprintType)
struct KARDS_API FDraftCard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 count = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FDraftCards
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDraftCard> Cards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> factions;
};

USTRUCT(BlueprintType)
struct KARDS_API FDraftCardStatus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 card_count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 total_cards = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FDraftStatus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 losses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDraftCardStatus Cards;
};

USTRUCT(BlueprintType)
struct KARDS_API FDraftMakePickResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString faction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 card_count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 total_cards = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> next_picks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDraftStatus draft_status;
};

USTRUCT(BlueprintType)
struct KARDS_API FDraftRewardInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Packs = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 gold_min = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 gold_max = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Random = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FMiniSitNGo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime start_date;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime end_date;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBlueprintJsonObject Rules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasWon = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FLaunchMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString URL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString url_text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString close_text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString download_info;
};

USTRUCT(BlueprintType)
struct KARDS_API FNewPlayerLoginReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 day = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Reset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Seconds = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FKardsPlayersInfoMisc
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMobileReviewDone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreateDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOptedOutOfProfile = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> featuredAchievements;
};

USTRUCT(BlueprintType)
struct KARDS_API FKardsPlayersInfo2
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString heartbeat_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString player_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_tag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Diamonds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Dust = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 draft_admissions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Packs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString URL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString library_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString decks_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString packs_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString dailymissions_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString achievements_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> tutorials_finished;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString season_end;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 stars = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool is_officer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool has_been_officer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Email;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool email_reward_received = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool email_verified = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLaunchMessage> launch_messages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> new_cards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FNewPlayerLoginReward new_player_login_reward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 usa_xp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 germany_xp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 soviet_xp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 japan_xp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 britain_xp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 usa_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 germany_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 soviet_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 japan_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 britain_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 usa_level_claimed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 germany_level_claimed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 soviet_level_claimed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 japan_level_claimed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 britain_level_claimed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime last_logon_date;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime double_xp_end_date;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime last_crate_claimed_date;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 claimable_crate_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKardsPlayersInfoMisc misc;
};

USTRUCT(BlueprintType)
struct KARDS_API FPacksReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECardSetEnum cardSet = ECardSetEnum::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactionEnum faction = EFactionEnum::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ECardSetEnum> cardSets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isOfficerPacks = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FDailyMissionsStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString mission_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText infoText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDailyMissionTriggersEnum Triggers = EDailyMissionTriggersEnum::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 counter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Reward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 gold_reward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 resource_reward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 draft_reward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPacksReward> packs_reward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> cards_reward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> item_rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> alt_icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> BackgroundImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 min_games_won = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 max_times_given = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 forced_order = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Enabled = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FGenericRewardStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> gold_cards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Cards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Items;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPacksReward> Packs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Resources = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 drafts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Diamonds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Reason;
};

USTRUCT(BlueprintType)
struct KARDS_API FPlayerEquippedItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString item_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactionEnum faction = EFactionEnum::NotAvailable;
};

USTRUCT(BlueprintType)
struct KARDS_API FPlayerItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString details;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString item_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 cnt = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FPlayerMatchData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Tag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactionEnum faction = EFactionEnum::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESideEnum side = ESideEnum::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool is_officer = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FPlayerFriend
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool is_online = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString player_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_tag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString busy_status;
};

USTRUCT(BlueprintType)
struct KARDS_API FPlayerPack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_set;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime date_opened;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString details;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime unlock_date;
};

USTRUCT(BlueprintType)
struct KARDS_API FPlayerPackCard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool recycled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Dust = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool is_gold_card = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool new_card = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FCardback2
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EFactionEnum> factions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<UTexture2D>> Textures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BaseColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor OutlineColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	// The current USMAP exposes the fields above but the package's native
	// element stride is 200 bytes. Preserve the opaque native tail without
	// assigning semantics to fields absent from the schema.
	uint8 OpaqueNativeStorage[56] = {};
};

USTRUCT(BlueprintType)
struct KARDS_API FPlayerStat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString stat_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 stat_int = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString stat_str;
};

USTRUCT(BlueprintType)
struct KARDS_API FStoreProductCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString currency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString amount;
};

USTRUCT(BlueprintType)
struct KARDS_API FStoreProduct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString desc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString end_date;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString reset_date;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString image_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString banner_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStoreProductCost> pricing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool can_purchase = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FKardsResponseInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool successful = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Origin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 http_code = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString verb;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString URL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString error_text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool is_fatal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString request_started;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString response_received;
};

USTRUCT(BlueprintType)
struct KARDS_API FPlayerNotification
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString notification;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 int_1 = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 int_2 = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString str_1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString str_2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 from_player_id = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FSinglePlayerExtraData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsArray = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FEndOfMatchDetails
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString faction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 old_xp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 new_xp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 old_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 new_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 old_national_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 gold_first_win_of_the_day = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGenericRewardStruct> extra_rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 new_gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 old_rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 new_rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 old_stars = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 new_stars = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ranked_up = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool is_double_xp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool winner = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hit_officer_club = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 new_club_elo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 old_club_elo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 old_club_rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 new_club_rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 season_wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 current_session_wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 dust_for_level = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FMulliganResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCardData> deck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCardData> replacement_cards;
};

USTRUCT(BlueprintType)
struct KARDS_API FMulliganData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString side;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMulliganResponse mulligan;
};

USTRUCT(BlueprintType)
struct KARDS_API FStringKeyIntValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FAchievement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 counter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 last_claim_level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsDirty = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FDMission
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString mission_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 counter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Slot = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsDirty = false;
};

USTRUCT(BlueprintType)
struct KARDS_API FLibraryStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsOwned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsBasic = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsAllegiance = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsBreakthrough = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsLegions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsBrothersInArms = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsWinterWar = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsCovertOps = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsBloodAndIron = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsNavalWarfare = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsHomefront = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatCardsOceaniaStorm = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FCardsBlacklist
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime end_date;
};

USTRUCT(BlueprintType)
struct KARDS_API FCardsReservePoolChanges
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime activation_date;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> to_be_reserved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> coming_back;
};

USTRUCT(BlueprintType)
struct KARDS_API FForceClientClosedMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText messageHeader;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText messageBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString buttonBehaviour;
};

USTRUCT(BlueprintType)
struct KARDS_API FDebugGameCheckpoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 match_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 action_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString myBoardItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString enemyBoardItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString battlefield;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 actionsToExecute = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FStartingMatchData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCardData> deck_left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCardData> deck_right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCardData location_card_left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCardData location_card_right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCardData> starting_hand_left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCardData> starting_hand_right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_level_left = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_level_right = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_stars_left = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player_stars_right = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString match_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString actions_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString left_player_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString right_player_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 left_player_tag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 right_player_tag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool left_player_officer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool right_player_officer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_back_left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_back_right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 right_officer_rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 left_officer_rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ally_faction_left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ally_faction_right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> equipment_left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> equipment_right;
};

USTRUCT(BlueprintType)
struct KARDS_API FTourneyRoundMatchup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player1_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player2_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString player1_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString player2_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player1_score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 player2_score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString last_match_end;
};

USTRUCT(BlueprintType)
struct KARDS_API FAchievementStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDailyMissionTriggersEnum Triggers = EDailyMissionTriggersEnum::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 counter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> BackgroundImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> ribbonImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 position = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool is_hidden = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Enabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Group;
};

USTRUCT(BlueprintType)
struct KARDS_API FLibraryCard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 deck_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 gold_card_count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 wanted_card_count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 recently_crafted_count = 0;
};

USTRUCT(BlueprintType)
struct KARDS_API FDeckHeader
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString main_faction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ally_faction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString card_back;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool deck_invalid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool skirmish_invalid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool knockout_invalid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString deck_invalid_reason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString skirmish_invalid_reason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString knockout_invalid_reason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime last_played;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool favorite = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool contains_reserved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString deck_code;
};

static_assert(sizeof(FLibraryCard) == 40, "LibraryCard USMAP layout changed.");
static_assert(sizeof(FDeckHeader) == 160, "DeckHeader USMAP layout changed.");
static_assert(sizeof(FAchievementStruct) == 104, "AchievementStruct USMAP layout changed.");
static_assert(sizeof(FLaunchMessage) == 112, "LaunchMessage USMAP layout changed.");
static_assert(sizeof(FNewPlayerLoginReward) == 32, "NewPlayerLoginReward USMAP layout changed.");
static_assert(sizeof(FKardsPlayersInfoMisc) == 40, "KardsPlayersInfoMisc USMAP layout changed.");
static_assert(sizeof(FKardsPlayersInfo2) == 440, "KardsPlayersInfo2 USMAP layout changed.");
static_assert(sizeof(FPacksReward) == 32, "PacksReward USMAP layout changed.");
static_assert(sizeof(FDebugGameCheckpoint) == 64, "DebugGameCheckpoint export/UE4SS layout changed.");
static_assert(sizeof(FUGCTextFilterResult) == 32, "UGCTextFilterResult export layout changed.");
static_assert(sizeof(FWidgetBackStackEntry) == 20, "WidgetBackStackEntry export layout changed.");
static_assert(sizeof(FDailyMissionsStruct) == 224, "DailyMissionsStruct USMAP layout changed.");
static_assert(sizeof(FGenericRewardStruct) == 96, "GenericRewardStruct USMAP layout changed.");
static_assert(sizeof(FTourneyRoundMatchup) == 64, "TourneyRoundMatchup USMAP layout changed.");
static_assert(sizeof(FDraftRewardInfo) == 20, "DraftRewardInfo USMAP layout changed.");
static_assert(sizeof(FMiniSitNGo) == 64, "MiniSitNGo USMAP layout changed.");
static_assert(sizeof(FActionValue2) == 40, "ActionValue2 USMAP layout changed.");
static_assert(sizeof(FSubAction) == 32, "SubAction USMAP layout changed.");
static_assert(sizeof(FAction2) == 64, "Action2 USMAP layout changed.");
static_assert(sizeof(FSubActionBatch) == 96, "SubActionBatch USMAP layout changed.");
static_assert(sizeof(FSubActionWithMappedValues) == 176, "SubActionWithMappedValues USMAP layout changed.");
static_assert(sizeof(FVisualAction) == 192, "VisualAction USMAP layout changed.");
static_assert(sizeof(FCardData) == 280, "CardData USMAP layout changed.");
static_assert(sizeof(FCampaignCardStats) == 48, "CampaignCardStats USMAP layout changed.");
static_assert(sizeof(FCampaignCardUpgrades) == 32, "CampaignCardUpgrades USMAP layout changed.");
static_assert(sizeof(FCampaignScenarioStars) == 24, "CampaignScenarioStars USMAP layout changed.");
static_assert(sizeof(FCampaign) == 136, "Campaign USMAP layout changed.");
static_assert(sizeof(FDraftCard) == 24, "DraftCard USMAP layout changed.");
static_assert(sizeof(FDraftCards) == 32, "DraftCards USMAP layout changed.");
static_assert(sizeof(FDraftCardStatus) == 8, "DraftCardStatus USMAP layout changed.");
static_assert(sizeof(FDraftStatus) == 32, "DraftStatus USMAP layout changed.");
static_assert(sizeof(FDraftMakePickResponse) == 88, "DraftMakePickResponse USMAP layout changed.");
static_assert(sizeof(FPlayerEquippedItem) == 40, "PlayerEquippedItem USMAP layout changed.");
static_assert(sizeof(FPlayerItem) == 40, "PlayerItem USMAP layout changed.");
static_assert(sizeof(FPlayerMatchData) == 64, "PlayerMatchData USMAP layout changed.");
static_assert(sizeof(FPlayerFriend) == 64, "PlayerFriend USMAP layout changed.");
static_assert(sizeof(FPlayerPack) == 56, "PlayerPack USMAP layout changed.");
static_assert(sizeof(FPlayerPackCard) == 32, "PlayerPackCard USMAP layout changed.");
static_assert(sizeof(FCardback2) == 200, "Cardback2 USMAP layout changed.");
static_assert(sizeof(FPlayerStat) == 40, "PlayerStat USMAP layout changed.");
static_assert(sizeof(FStoreProductCost) == 32, "StoreProductCost USMAP layout changed.");
static_assert(sizeof(FStoreProduct) == 144, "StoreProduct USMAP layout changed.");
static_assert(sizeof(FPlayerNotification) == 72, "PlayerNotification USMAP layout changed.");
static_assert(sizeof(FKardsResponseInfo) == 152, "KardsResponseInfo JMap layout changed.");
static_assert(sizeof(FSinglePlayerExtraData) == 40, "SinglePlayerExtraData USMAP layout changed.");
static_assert(sizeof(FEndOfMatchDetails) == 112, "EndOfMatchDetails USMAP layout changed.");
static_assert(sizeof(FMulliganResponse) == 32, "MulliganResponse USMAP layout changed.");
static_assert(sizeof(FMulliganData) == 48, "MulliganData USMAP layout changed.");
static_assert(sizeof(FStringKeyIntValue) == 24, "StringKeyIntValue USMAP layout changed.");
static_assert(sizeof(FAchievement) == 40, "Achievement USMAP layout changed.");
static_assert(sizeof(FDMission) == 40, "DMission USMAP layout changed.");
static_assert(sizeof(FLibraryStats) == 48, "LibraryStats USMAP layout changed.");
static_assert(sizeof(FCardsBlacklist) == 24, "CardsBlacklist USMAP layout changed.");
static_assert(sizeof(FCardsReservePoolChanges) == 168, "CardsReservePoolChanges USMAP layout changed.");
static_assert(sizeof(FForceClientClosedMessage) == 64, "ForceClientClosedMessage USMAP layout changed.");
static_assert(sizeof(FStartingMatchData) == 824, "StartingMatchData USMAP layout changed.");
