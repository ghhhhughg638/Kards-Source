#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NativeUiDataSchema.h"
#include "MatchControllerV2EditorShim.generated.h"

// Local editor compatibility declarations based on the UHT header dump and
// checked against the current Blueprint export's 256-byte Match2 stride. Do
// not package or distribute this project's kards module DLL; the game supplies
// the canonical native /Script/kards types.
USTRUCT(BlueprintType)
struct FMatch2
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 current_action_id = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 action_player_id = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString action_side;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString match_url;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString match_type;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 current_turn = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 match_id = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Status;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString start_side;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString actions_url;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 player_id_left = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 player_id_right = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 deck_id_left = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 deck_id_right = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool left_is_online = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool right_is_online = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString player_status_left;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString player_status_right;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FCardData> Cards;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 winner_id = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString winner_side;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FAction2> actions;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FPlayerNotification> notifications;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString modify_date;
};

static_assert(sizeof(FMatch2) == 256, "Match2 UHT/export layout changed.");

// Minimal editor-side reflection for the two mulligan payloads consumed by
// BP_Logic's replay setup graphs. The host game owns the canonical native
// struct; this declaration only supplies the local Blueprint pin schema.
USTRUCT(BlueprintType)
struct KARDS_API FReplayResponse
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FMulliganResponse mulligan_left;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FMulliganResponse mulligan_right;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchInfoFetched);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAiSurrendered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResponseZeroTooOften);

UCLASS(BlueprintType, Blueprintable)
class KARDS_API AMatchControllerV2 : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    bool shouldPollActions = false;
    UPROPERTY(BlueprintAssignable)
    FOnAiSurrendered OnAiSurrendered;
    UPROPERTY(BlueprintAssignable)
    FOnResponseZeroTooOften OnResponseZeroTooOften;

    UPROPERTY(BlueprintReadWrite)
    TArray<FAction2> myReceivedActions;
    UFUNCTION(BlueprintCallable, BlueprintPure)
    int32 GetCurrentActionID() const;
    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetActiveSide(ESideEnum& activeSide);
    UFUNCTION(BlueprintCallable)
    void EndMatch(FString side, TArray<FActionValue2> Results);
    UFUNCTION(BlueprintCallable)
    void AddActionWithSub(FString action_type, TArray<FActionValue2> action_data, TArray<FSubAction> sub_actions);
    UFUNCTION(BlueprintCallable)
    void AddActionWithSubAndState(FString action_type, TArray<FActionValue2> action_data, TArray<FSubAction> sub_actions,
        FString cardsJsonString, int32 kredits_right, int32 kredits_left, int32 max_kredits_right, int32 max_kredits_left,
        int32 fatigue_damage_right, int32 fatigue_damage_left, int32 campaign_strategy, FString gameplay_restrictions);

    UPROPERTY(BlueprintAssignable)
    FOnMatchInfoFetched OnMatchInfoFetched;

    UFUNCTION(BlueprintCallable)
    void ResyncMatch();

    // Canonical property types verified against the game's JMap.
    UPROPERTY(BlueprintReadOnly)
    float TimeSinceLastStartOfTurn = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bUseActionsForMulligan = false;

    // Declared in the UHT header dump next to bUseActionsForMulligan.
    UPROPERTY(BlueprintReadWrite)
    bool bUseTurnSwitchValidation = false;

    UPROPERTY(BlueprintReadWrite)
    bool bUseLocalSubActions = false;

    UPROPERTY(BlueprintReadWrite)
    bool isResyncingMatch = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsStartingDebugGame = false;

    UPROPERTY(BlueprintReadWrite)
    FDebugGameCheckpoint DebugGameCheckpoint;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FMatch2 Match;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FReplayResponse replayResponse;

    // Declared in the UHT header dump (MatchControllerV2.h).
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FStartingMatchData StartingData;

    UFUNCTION(BlueprintCallable)
    void resetLastPingSentTime();

    UFUNCTION(BlueprintCallable)
    void resetLastPingReceivedTime();

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool isDraftMatch();

    // BP_Table's bytecode calls this as a pure/static function, supplies Self
    // as its hidden context, and receives MatchControllerV2*. The exact native
    // parameter name, constness, UFunction flags, and WorldContext metadata
    // require the original header/reflection dump and are inferred here.
    UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
    static AMatchControllerV2* GetMatchController(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void IsReconnectMatch(
        bool& isReconnecting,
        bool& clientMulliganDone,
        bool& otherMulliganDone);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetClientSideInfo(FPlayerMatchData& info);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetOtherSideInfo(FPlayerMatchData& info);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    void GetNextAction(FAction2& action, bool& success);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool CanEndTurn();

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool isAIMatch();

    UFUNCTION(BlueprintCallable)
    void SetIsReconecting(bool isReconnecting);

    UFUNCTION(BlueprintCallable)
    void SpectateMatch(int32 playerID, bool isFromSpectateMenu);

    UFUNCTION(BlueprintCallable)
    void StartEvaluatingActionsForReconnect();

    UFUNCTION(BlueprintCallable)
    void ReconnectDoMulligan();

    UFUNCTION(BlueprintCallable)
    void FinishedEvaluatingActionsForReconnect();

    UFUNCTION(BlueprintCallable)
    void OpponentMulliganTimedOut();

    UFUNCTION(BlueprintCallable)
    void waitingForOpponentToStartTurn();

    UFUNCTION(BlueprintCallable)
    void ResetVariablesForResync();

    UFUNCTION(BlueprintCallable)
    void SendCardsToMulligan(const TArray<int32>& cardsToDiscard, FString cards);
};
