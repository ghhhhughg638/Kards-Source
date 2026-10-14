#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BoardItem2EditorShim.h"
#include "ERarityEnum.h"
#include "EFactionEnum.h"
#include "CardBackAsset.generated.h"

class UMaterialInstance;
class UTexture;

// Local editor compatibility declaration generated from the official UHT header
// dump (Public/CardBackAsset.h). Reflection only; the shipped game owns the
// canonical native class.
UCLASS(Blueprintable)
class KARDS_API UCardBackAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FName Name;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FText DisplayName;

    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere)
    EItemSource ItemSource = EItemSource::NotAvailable;

    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere)
    FString DeckCodeID;

    UPROPERTY(AssetRegistrySearchable, BlueprintReadWrite, EditAnywhere)
    ERarityEnum rarity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<EFactionEnum> factions;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr<UTexture> MainTexture;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<TSoftObjectPtr<UTexture>> VariantTextures;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<TSoftObjectPtr<UTexture>> ISBNTextures;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr<UMaterialInstance> UniqueMaterial;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr<UMaterialInstance> UniqueDeckMaterial;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr<UTexture> Icon;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FLinearColor LabelBaseColor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FLinearColor LabelOutlineColor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bIsAvailableForDuration = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FDateTime StartDate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FDateTime EndDate;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsAvailable() const;

    UFUNCTION(BlueprintCallable)
    UMaterialInstance* GetUniqueMaterial(bool bUseDeckMaterial, bool& bHasMaterial);

    UFUNCTION(BlueprintCallable)
    UTexture* GetNextTexture(bool useISBN);

    UFUNCTION(BlueprintCallable)
    UTexture* GetMainTexture(bool useISBN);
};
