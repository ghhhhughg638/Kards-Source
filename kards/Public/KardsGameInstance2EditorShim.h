#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BaseCardObject.h"
#include "NativeUiDataSchema.h"
#include "KardsGameInstance2EditorShim.generated.h"

// Empty local editor-compatibility shell for BP_KardsGameInstance's
// /Script/kards.KardsGameInstance2 parent; this is not a game implementation.
UCLASS(BlueprintType, Blueprintable)
class KARDS_API UKardsGameInstance2 : public UGameInstance
{
	GENERATED_BODY()

public:
	// Types verified against the game's JMap; reflection only.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FKardsPlayersInfo2 kardsPlayerInfo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, FCampaignCardStats> campaignOldCardStats;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 matchID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString matchUrl;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 deckToLoadInDeckEditor = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool isSinglePlayerMatch = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FForceClientClosedMessage ForceClientClosedMessage;

	UFUNCTION(BlueprintCallable)
	void AddStaticCard(UBaseCardObject* card);

	UFUNCTION(BlueprintCallable)
	void DisconnectSession();
};
