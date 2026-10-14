#include "WebSocketSubsystem.h"

UWebSocketSubsystem::UWebSocketSubsystem()
    : bIsEnabled(false)
    , bDebugDisconnected(false)
{
}

void UWebSocketSubsystem::SendWebSocketMessage(const FString& Channel, const FString& Receiver, const FString& Message, const FString& Context)
{
    (void)Channel;
    (void)Receiver;
    (void)Message;
    (void)Context;
}

void UWebSocketSubsystem::RetryOpenWebSocket(const FString& LastError)
{
    (void)LastError;
}

void UWebSocketSubsystem::PingWebSocket()
{
}

void UWebSocketSubsystem::OpenWebSocket(const FString& URL, const FString& Token)
{
    (void)URL;
    (void)Token;
}

bool UWebSocketSubsystem::IsConnected() const
{
    return false;
}

void UWebSocketSubsystem::DoRetryOpenWebSocket()
{
}

void UWebSocketSubsystem::DebugWebSocketFailureOnOff(const bool bActive)
{
    (void)bActive;
}

void UWebSocketSubsystem::CloseWebSocket()
{
}
