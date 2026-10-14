#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "EFactionEnum.h"
#include "ERarityEnum.h"
#include "BoardItem2EditorShim.generated.h"

// Local-project compatibility declarations used to restore and compile game
// assets. Do not package or distribute this project's kards module DLL: the
// game supplies the canonical runtime /Script/kards types.
UENUM(BlueprintType)
enum class EItemSource : uint8
{
    NotAvailable = 0,
    StarterItem = 1,
    PurchasableItem = 2,
    RewardedItem = 3,
    SpecialItem = 4,
    EItemSource_MAX = 5 UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FBoardChildItem2
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> MobileMaterial;

    // Compatibility assumption: the source graph consumes this as a soft
    // class reference, and its referenced base Blueprint derives from AActor.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftClassPtr<AActor> boardPropActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform myTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform enemyTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform myTransformTabletView;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform enemyTransformTabletView;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool hasTabletView = false;
};

USTRUCT(BlueprintType)
struct FBoardItem2 : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EFactionEnum> factions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FBoardChildItem2> Items;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName item_name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> Thumbnail;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> Image;

    // Compatibility assumption: see FBoardChildItem2::boardPropActor.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftClassPtr<AActor> boardPropActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemSource ItemSource = EItemSource::NotAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsInteractable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERarityEnum rarity = ERarityEnum::Common;
};

static_assert(sizeof(FBoardChildItem2) == 592, "BoardChildItem2 compatibility layout changed.");
static_assert(sizeof(FBoardItem2) == 240, "BoardItem2 compatibility layout changed.");
