#include "LoggerSubsystem.h"

void ULoggerSubsystem::LogPlayerEvent(const FString& EventType, const FString& Data)
{
    // Intentionally empty. The shipped game supplies the real implementation
    // at runtime; this shim only makes the reflected Blueprint signature
    // available to the editor and cooker.
}

void ULoggerSubsystem::LogError(const FString& Error, bool skipScreen)
{
    // The real game forwards this to its logger/UI.  The compatibility shim
    // deliberately has no side effects while restoring and compiling assets.
}
