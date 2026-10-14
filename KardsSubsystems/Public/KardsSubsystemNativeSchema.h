#pragma once

#include "CoreMinimal.h"
#include "KardsSubsystemNativeSchema.generated.h"

// Local reflection compatibility declarations for cooked native types whose
// script package is /Script/KardsSubsystems. These are schema shims only; the
// original module implementation remains owned by the game.

UENUM(BlueprintType)
enum class EStoreOfferItemType : uint8
{
	NotAvailable = 0,
	Pack = 1,
	Card = 2,
	RandomCard = 3,
	Deck = 4,
	Cardback = 5,
	AltArt = 6,
	Emote = 7,
	Campaign = 8,
	Medkit = 9,
	Gold = 10,
	Diamonds = 11,
	Draft = 12,
	Prop = 13,
	BattlePass = 14,
	Avatar = 15,
	Token = 16,
	LootBox = 17,
	EStoreOfferItemType_MAX = 18 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EFrontPageItemType : uint8
{
	Carousel = 0,
	SideBarButton = 1,
	Popup = 2,
	EFrontPageItemType_MAX = 3 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EFrontPageSidebarSlot : uint8
{
	Top = 0,
	Middle = 1,
	Bottom = 2,
	EFrontPageSidebarSlot_MAX = 3 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EShopAdContextFlags : uint8
{
	NotSet = 0,
	EndOfMatch = 1,
	SessionStart = 2,
	OutsideMatch = 254,
	Any = 255
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FStoreOfferItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStoreOfferItemType Type = EStoreOfferItemType::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;

	// FModel reports a 24-byte native stride, while USMAP exposes only the two
	// fields above. Preserve the remaining opaque storage without inventing a
	// reflected field name or meaning.
	uint8 OpaqueNativeStorage[16] = {};
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FFrontPageText
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FontSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FFrontPageIcon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString URL;
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FFrontPageElementContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFrontPageItemType Type = EFrontPageItemType::Carousel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFrontPageSidebarSlot Slot = EFrontPageSidebarSlot::Top;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Link;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ImageURL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFrontPageText Heading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFrontPageText SubHeading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFrontPageIcon icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCountdown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShopAdContextFlags AdContext = EShopAdContextFlags::NotSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AdFrequency = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AdLimit = 0;
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FFrontPageElementSchema
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ElementID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFrontPageElementContent Content;
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FFriend
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnline = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerTag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BusyStatus;
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FIAPFulfilledReceipt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReceiptID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OfferName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStoreOfferItem> Items;
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FIAPLegacyPacks
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString cardSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CardCountPerPack = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FIAPLegacyReceivedProducts
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> equipment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIAPLegacyPacks> Packs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DraftAdmissions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Diamonds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Dust = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> campaigns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Cards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Decks;
};

USTRUCT(BlueprintType)
struct KARDSSUBSYSTEMS_API FRewardVariables
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStoreOfferItem Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ReasonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NationalLevelOrSeasonRank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Variant = 0;
};

static_assert(sizeof(FStoreOfferItem) == 24, "StoreOfferItem USMAP/export stride changed.");
static_assert(sizeof(FFrontPageText) == 24, "FrontPageText USMAP layout changed.");
static_assert(sizeof(FFrontPageIcon) == 16, "FrontPageIcon USMAP layout changed.");
static_assert(sizeof(FFrontPageElementContent) == 120, "FrontPageElementContent USMAP layout changed.");
static_assert(sizeof(FFrontPageElementSchema) == 144, "FrontPageElementSchema USMAP/export size changed.");
static_assert(sizeof(FFriend) == 64, "Friend USMAP layout changed.");
static_assert(sizeof(FIAPFulfilledReceipt) == 40, "IAPFulfilledReceipt USMAP layout changed.");
static_assert(sizeof(FIAPLegacyPacks) == 24, "IAPLegacyPacks USMAP layout changed.");
static_assert(sizeof(FIAPLegacyReceivedProducts) == 96, "IAPLegacyReceivedProducts USMAP layout changed.");
static_assert(sizeof(FRewardVariables) == 48, "RewardVariables USMAP layout changed.");
