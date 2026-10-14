// Local editor compatibility declaration generated from the official UHT header dump.
// Reflection only: the shipped game supplies the canonical native implementation.
// Do not package or distribute this project's kards module DLL.
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
//CROSS-MODULE INCLUDE V2: -ModuleName=Engine -ObjectName=GameInstanceSubsystem -FallbackName=GameInstanceSubsystem
#include "WebSocketSubsystem.generated.h"

UCLASS(Blueprintable)
class KARDSSUBSYSTEMS_API UWebSocketSubsystem : public UGameInstanceSubsystem {
    GENERATED_BODY()
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWebSocketFailure, const FString&, Error);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketOpen);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnWebSocketMessageReceived, const FString&, Channel, const FString&, Sender, const FString&, Message, const FString&, Context);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketClosed);
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString WebSocketUrl;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString WebSocketToken;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FString BearerToken;
    
    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FWebSocketFailure OnWebSocketFailure;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool bIsEnabled;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    bool bDebugDisconnected;
    
    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FOnWebSocketOpen OnWebSocketOpen;
    
    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FOnWebSocketClosed OnWebSocketClosed;
    
    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess=true))
    FOnWebSocketMessageReceived OnWebSocketMessageReceived;
    
    UWebSocketSubsystem();

    UFUNCTION(BlueprintCallable)
    void SendWebSocketMessage(const FString& Channel, const FString& Receiver, const FString& Message, const FString& Context);
    
    UFUNCTION(BlueprintCallable)
    void RetryOpenWebSocket(const FString& LastError);
    
    UFUNCTION(BlueprintCallable)
    void PingWebSocket();
    
    UFUNCTION(BlueprintCallable)
    void OpenWebSocket(const FString& URL, const FString& Token);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsConnected() const;
    
    UFUNCTION(BlueprintCallable)
    void DoRetryOpenWebSocket();
    
    UFUNCTION(BlueprintCallable)
    void DebugWebSocketFailureOnOff(const bool bActive);
    
    UFUNCTION(BlueprintCallable)
    void CloseWebSocket();
    
};


