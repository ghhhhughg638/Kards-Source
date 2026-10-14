#pragma once

#include "CoreMinimal.h"
#include "KardsCoreNativeSchema.generated.h"

// Reflection-only compatibility declarations for native /Script/KardsCore
// structures referenced by the current cooked Blueprint exports.

DECLARE_DYNAMIC_DELEGATE(FDialogDelegate);

USTRUCT(BlueprintType)
struct KARDSCORE_API FDialogModalSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FText title;

	UPROPERTY(BlueprintReadOnly)
	FText Content;

	UPROPERTY(BlueprintReadOnly)
	FText YesButton;

	UPROPERTY(BlueprintReadOnly)
	FText NoButton;

	UPROPERTY(BlueprintReadOnly)
	FText MiddleButton;

	UPROPERTY(BlueprintReadOnly)
	bool bShowTitle = false;

	UPROPERTY(BlueprintReadOnly)
	bool bShowContent = false;

	UPROPERTY(BlueprintReadOnly)
	bool bOnlyCloseButton = false;

	UPROPERTY(BlueprintReadOnly)
	bool bShowMiddleButton = false;

	UPROPERTY(BlueprintReadOnly)
	bool bAllowClickOutsideToClose = false;

	UPROPERTY(BlueprintReadOnly)
	FDialogDelegate OnConfirm;

	UPROPERTY(BlueprintReadOnly)
	FDialogDelegate OnCancel;

	UPROPERTY(BlueprintReadOnly)
	FDialogDelegate OnAltAction;
};

#if !WITH_EDITOR
// The JMAP size describes the game's Shipping ABI. The editor uses a distinct
// FText/delegate representation, so only enforce this assertion for game builds.
static_assert(sizeof(FDialogModalSettings) == 136, "DialogModalSettings current JMAP layout changed.");
#endif
