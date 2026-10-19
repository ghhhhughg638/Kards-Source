#include "KardsSessionEditorShim.h"

void AKardsSession::LogPlayerEvent(FString EventType, FString Data)
{
	(void)EventType;
	(void)Data;
}

void AKardsSession::PollPlayerNotifications()
{
}

// These no-op bodies only let the local editor reflect and compile dependent
// Blueprints. They are not KARDS game implementations.
bool AKardsSession::HasRole(FString role_name)
{
	(void)role_name;
	return false;
}

void AKardsSession::GetEnemyPlayerBoardItem(int32 enemy_player_id)
{
	(void)enemy_player_id;
}

void AKardsSession::Connect(const ECredentialTypeEnum credential_type, FString Username, FString Password, const bool bRememberMe, const bool bAllowAccountCreation)
{
	(void)credential_type;
	(void)Username;
	(void)Password;
	(void)bRememberMe;
	(void)bAllowAccountCreation;
	UE_LOG(LogTemp, Warning, TEXT("AKardsSession::Connect is a stub and does nothing"));
}

void AKardsSession::GetUnrealEditorMultipleUsersUsername(FString users)
{
	(void)users;
}

void AKardsSession::JsonToPlayerNotification(const FString Json, FPlayerNotification& Object)
{
	(void)Json;
	Object = FPlayerNotification{};
}

FString AKardsSession::NotificationContextToJson(FString notification, int32 int_1, int32 int_2, FString str_1, FString str_2)
{
	(void)notification;
	(void)int_1;
	(void)int_2;
	(void)str_1;
	(void)str_2;
	return FString{};
}

void AKardsSession::ResetMatchVariable()
{
}

void AKardsSession::FatalHeartbeat()
{
}

void AKardsSession::GetPlayerItems()
{
}


void AKardsSession::EquipItem(EFactionEnum faction, const FString& Slot, const FString& ItemName)
{
	(void)faction;
	(void)Slot;
	(void)ItemName;
}

