#pragma once

#include "Commandlets/Commandlet.h"
#include "RestoreBPTableSchemaAssetsCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API URestoreBPTableSchemaAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	URestoreBPTableSchemaAssetsCommandlet();
	virtual int32 Main(const FString& Params) override;
};
