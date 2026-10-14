#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NativeUiDataSchema.h"
#include "EFactionEnum.h"
#include "KardsSessionEditorShim.generated.h"

// Credential values are copied from the current KARDS JMap. This local enum
// exists only so dependent Blueprints can resolve their reflected pins.
UENUM(BlueprintType)
enum class ECredentialTypeEnum : uint8
{
	CT_Device = 0,
	CT_UserPass = 1,
	CT_GameCenter = 2,
	CT_Steam = 3,
	CT_Kards = 4,
	CT_MSSDK = 5,
	CT_Nintendo = 6,
	CT_MAX = 7 UMETA(Hidden)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDustChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGoldChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisconnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOpponentOffline);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFreeCardGiven, FString, card_type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFetchPlayerItems, const TArray<FPlayerItem>&, Items, const TArray<FPlayerEquippedItem>&, equippedItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFetchEnemyEquippedItems, const TArray<FPlayerEquippedItem>&, equippedItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKardResponse, FKardsResponseInfo, Response, bool, successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLinkingResponse, FString, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnrealEditorMultipleUsersUsername, FString, Username);

// Local editor-compatibility shell for the native /Script/kards.KardsSession
// parent. The bodies intentionally provide no KARDS game behavior.
UCLASS(BlueprintType, Blueprintable, NotPlaceable)
class KARDS_API AKardsSession : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnOpponentOffline OnOpponentOffline;

	UPROPERTY(BlueprintReadWrite)
	int32 player_id = 0;

	UFUNCTION(BlueprintCallable)
	void LogPlayerEvent(FString EventType, FString Data);

	UFUNCTION(BlueprintCallable)
	void PollPlayerNotifications();

	UFUNCTION(BlueprintCallable)
	void GetPlayerItems();

	UFUNCTION(BlueprintCallable)
	void EquipItem(EFactionEnum faction, const FString& Slot, const FString& ItemName);

	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnFetchPlayerItems OnFetchPlayerItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	FMiniSitNGo current_mini_sit_n_go;

	UPROPERTY(BlueprintReadOnly, Category="Kards|Editor Compatibility")
	bool is_connected = false;

	UPROPERTY(BlueprintReadOnly, Category="Kards|Editor Compatibility")
	bool bNotificationsReceived = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	TArray<FCardsReservePoolChanges> cards_reserve_changes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	TArray<FCardsBlacklist> cards_blacklist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	TArray<FPlayerNotification> player_notifications;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	int32 client_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	FString JWT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	FString linker_account;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	TMap<FName, FDateTime> locked_cards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	FString root_url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	bool runningInEditor = false;

	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnDustChanged OnDustChanged;

	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnFetchEnemyEquippedItems OnFetchEnemyEquippedItems;

	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnFreeCardGiven OnFreeCardGiven;

	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnGoldChanged OnGoldChanged;

	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnDisconnected OnDisconnected;

	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnKardResponse OnDriftSessionEstablished;

	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnLinkingResponse OnLinkingResponse;

	UPROPERTY(BlueprintAssignable, Category="Kards|Editor Compatibility")
	FOnUnrealEditorMultipleUsersUsername OnUnrealEditorMultipleUsersUsername;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Kards|Editor Compatibility")
	bool HasRole(FString role_name);

	UFUNCTION(BlueprintCallable, Category="Kards|Editor Compatibility")
	void GetEnemyPlayerBoardItem(int32 enemy_player_id);

	UFUNCTION(BlueprintCallable, Category="Kards|Editor Compatibility")
	void Connect(const ECredentialTypeEnum credential_type, FString Username, FString Password, const bool bRememberMe, const bool bAllowAccountCreation);

	UFUNCTION(BlueprintCallable, Category="Kards|Editor Compatibility")
	void GetUnrealEditorMultipleUsersUsername(FString users);

	UFUNCTION(BlueprintCallable, Category="Kards|Editor Compatibility")
	void JsonToPlayerNotification(const FString Json, FPlayerNotification& Object);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Kards|Editor Compatibility")
	FString NotificationContextToJson(FString notification, int32 int_1, int32 int_2, FString str_1, FString str_2);

	UFUNCTION(BlueprintCallable, Category="Kards|Editor Compatibility")
	void ResetMatchVariable();

	UFUNCTION(BlueprintCallable, Category="Kards|Editor Compatibility")
	void FatalHeartbeat();
};



