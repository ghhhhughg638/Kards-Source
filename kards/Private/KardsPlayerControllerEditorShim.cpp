#include "KardsPlayerControllerEditorShim.h"

// These no-op bodies only let the local editor reflect and compile dependent
// Blueprints. They intentionally do not emulate the game's runtime controller.
FWidgetBackStackEntry AKardsPlayerController::AddBackhandler(UUserWidget* BackHandlerWidget, UWidget* PreviouslyFocusedWidget, bool bIsHighPriority)
{
	return FWidgetBackStackEntry{};
}

void AKardsPlayerController::AddInputAxisBinding(UInputAction* InputAction, UUserWidget* UserWidget, FWidgetInputActionAxisDelegate Callback)
{
}

void AKardsPlayerController::AddInputBinding(UInputAction* InputAction, UUserWidget* UserWidget, FWidgetInputActionDelegate Callback)
{
}

bool AKardsPlayerController::GetActiveBackhandler(FWidgetBackStackEntry& OutBackhandlerEntry)
{
	OutBackhandlerEntry = FWidgetBackStackEntry{};
	return false;
}

bool AKardsPlayerController::RemoveBackhandler(UUserWidget* BackHandlerWidget)
{
	return false;
}

void AKardsPlayerController::RemoveInputBindings(UInputAction* InputAction, UUserWidget* UserWidget)
{
}

void AKardsPlayerController::TriggerInputAxisBindings(UInputAction* InputAction, float AxisValue)
{
}

void AKardsPlayerController::TriggerInputBindings(UInputAction* InputAction)
{
}
