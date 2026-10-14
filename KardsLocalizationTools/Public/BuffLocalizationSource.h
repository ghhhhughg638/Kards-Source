#pragma once

#include "CoreMinimal.h"

struct FBuffLocalizationTextSource
{
    FString Namespace;
    FString Key;
    FString SourceString;
};

struct FBuffLocalizationRowSource
{
    FName RowName;
    FBuffLocalizationTextSource BuffText;
    bool bAmountFormat = false;
    FString TextColor;
};

class KARDSLOCALIZATIONTOOLS_API FBuffLocalizationSource
{
public:
    static bool LoadAndValidate(
        const FString& SourcePath,
        int32 ExpectedOfficialRowCount,
        TArray<FBuffLocalizationRowSource>& OutRows,
        FString& OutError);
};
