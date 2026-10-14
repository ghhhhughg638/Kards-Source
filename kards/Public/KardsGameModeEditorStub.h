#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ESideEnum.h"

#if WITH_EDITORONLY_DATA
#include "KardsGameModeEditorStub.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraShake);

// Editor-only compatibility parent for the official BP_Logic asset.
// The game supplies the native class at this exact /Script/kards path.
UCLASS(BlueprintType, Blueprintable)
class KARDS_API AkardsGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	ESideEnum activeSide = ESideEnum::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	ESideEnum mySide = ESideEnum::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	ESideEnum enemySide = ESideEnum::NotAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kards|Editor Compatibility")
	FName CurrentLevel;

	UPROPERTY(EditAnywhere, BlueprintAssignable, BlueprintCallable, Category="Kards|Editor Compatibility")
	FOnCameraShake OnCameraShake;
};
#endif
