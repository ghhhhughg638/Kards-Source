#pragma once

#include "Commandlets/Commandlet.h"
#include "KardsRealAssetReconstructorCommandlet.generated.h"

UCLASS()
class KARDSLOCALIZATIONTOOLS_API UKardsRealAssetReconstructorCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UKardsRealAssetReconstructorCommandlet();
	virtual int32 Main(const FString& Params) override;
};
