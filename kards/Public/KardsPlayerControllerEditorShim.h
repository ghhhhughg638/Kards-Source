#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "NativeUiDataSchema.h"
#include "KardsPlayerControllerEditorShim.generated.h"

// Editor-only compatibility shell for loading BP_PlayerController and the
// current UI input graphs. The shipping game's native class owns the real
// implementations; do not package this project's kards module DLL.
UCLASS(BlueprintType, Blueprintable, NotPlaceable, Config=Game)
class KARDS_API AKardsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	FWidgetBackStackEntry AddBackhandler(UUserWidget* BackHandlerWidget, UWidget* PreviouslyFocusedWidget, bool bIsHighPriority);

	UFUNCTION(BlueprintCallable)
	void AddInputAxisBinding(UInputAction* InputAction, UUserWidget* UserWidget, FWidgetInputActionAxisDelegate Callback);

	UFUNCTION(BlueprintCallable)
	void AddInputBinding(UInputAction* InputAction, UUserWidget* UserWidget, FWidgetInputActionDelegate Callback);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetActiveBackhandler(FWidgetBackStackEntry& OutBackhandlerEntry);

	UFUNCTION(BlueprintCallable)
	bool RemoveBackhandler(UUserWidget* BackHandlerWidget);

	UFUNCTION(BlueprintCallable)
	void RemoveInputBindings(UInputAction* InputAction, UUserWidget* UserWidget);

	UFUNCTION(BlueprintCallable)
	void TriggerInputAxisBindings(UInputAction* InputAction, float AxisValue);

	UFUNCTION(BlueprintCallable)
	void TriggerInputBindings(UInputAction* InputAction);

protected:
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TMap<UInputAction*, FWidgetInputActionList> Bindings;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TArray<FWidgetBackStackEntry> WidgetBackHandlerStack;
};
