#pragma once

#include "Commandlets/Commandlet.h"
#include "RestoreEmojiSchemaShellCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API URestoreEmojiSchemaShellCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	URestoreEmojiSchemaShellCommandlet();
	virtual int32 Main(const FString& Params) override;
};
