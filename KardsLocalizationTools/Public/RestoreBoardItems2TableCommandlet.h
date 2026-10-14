#pragma once

#include "Commandlets/Commandlet.h"
#include "RestoreBoardItems2TableCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API URestoreBoardItems2TableCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	URestoreBoardItems2TableCommandlet();
	virtual int32 Main(const FString& Params) override;
};
