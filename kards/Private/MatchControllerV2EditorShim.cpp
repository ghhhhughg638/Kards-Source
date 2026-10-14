#include "MatchControllerV2EditorShim.h"

int32 AMatchControllerV2::GetCurrentActionID() const { return 0; }
void AMatchControllerV2::GetActiveSide(ESideEnum& activeSide) { activeSide = ESideEnum::NotAvailable; }
void AMatchControllerV2::EndMatch(FString side, TArray<FActionValue2> Results) {}
void AMatchControllerV2::AddActionWithSub(FString action_type, TArray<FActionValue2> action_data, TArray<FSubAction> sub_actions) {}
void AMatchControllerV2::AddActionWithSubAndState(FString action_type, TArray<FActionValue2> action_data, TArray<FSubAction> sub_actions,
    FString cardsJsonString, int32 kredits_right, int32 kredits_left, int32 max_kredits_right, int32 max_kredits_left,
    int32 fatigue_damage_right, int32 fatigue_damage_left, int32 campaign_strategy, FString gameplay_restrictions) {}

void AMatchControllerV2::ResyncMatch()
{
}

void AMatchControllerV2::resetLastPingSentTime()
{
}

void AMatchControllerV2::resetLastPingReceivedTime()
{
}

bool AMatchControllerV2::isDraftMatch()
{
    return false;
}

AMatchControllerV2* AMatchControllerV2::GetMatchController(UObject* WorldContextObject)
{
    // This shim only supplies a reflected pin signature for local restoration.
    // It is never intended to replace the game's native implementation.
    (void)WorldContextObject;
    return nullptr;
}

void AMatchControllerV2::IsReconnectMatch(
    bool& isReconnecting,
    bool& clientMulliganDone,
    bool& otherMulliganDone)
{
    // Signature-only editor shim; local restoration does not emulate match state.
    isReconnecting = false;
    clientMulliganDone = false;
    otherMulliganDone = false;
}

void AMatchControllerV2::GetClientSideInfo(FPlayerMatchData& info)
{
    info = FPlayerMatchData{};
}

void AMatchControllerV2::GetOtherSideInfo(FPlayerMatchData& info)
{
    info = FPlayerMatchData{};
}

void AMatchControllerV2::GetNextAction(FAction2& action, bool& success)
{
    action = FAction2{};
    success = false;
}

bool AMatchControllerV2::CanEndTurn()
{
    return false;
}

bool AMatchControllerV2::isAIMatch()
{
    return false;
}

void AMatchControllerV2::SetIsReconecting(bool isReconnecting)
{
    (void)isReconnecting;
}

void AMatchControllerV2::SpectateMatch(int32 playerID, bool isFromSpectateMenu)
{
    (void)playerID;
    (void)isFromSpectateMenu;
}

void AMatchControllerV2::StartEvaluatingActionsForReconnect()
{
}

void AMatchControllerV2::ReconnectDoMulligan()
{
}

void AMatchControllerV2::FinishedEvaluatingActionsForReconnect()
{
}

void AMatchControllerV2::OpponentMulliganTimedOut()
{
}

void AMatchControllerV2::waitingForOpponentToStartTurn()
{
}

void AMatchControllerV2::ResetVariablesForResync()
{
}

void AMatchControllerV2::SendCardsToMulligan(
    const TArray<int32>& cardsToDiscard,
    FString cards)
{
    (void)cardsToDiscard;
    (void)cards;
}
