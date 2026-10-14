#if WITH_DEV_AUTOMATION_TESTS

#include "BuffLocalizationSource.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBuffLocalizationSourceTest,
    "Kards.Localization.BuffLocalizationSource",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBuffLocalizationSourceTest::RunTest(const FString& Parameters)
{
    static_cast<void>(Parameters);
    const FString SourcePath = TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kardsC/Content/Structs/Localization/T_buffTextLocalize.json");

    TArray<FBuffLocalizationRowSource> Rows;
    FString Error;
    TestTrue(TEXT("The official source parses"), FBuffLocalizationSource::LoadAndValidate(SourcePath, 117, Rows, Error));
    if (!Error.IsEmpty())
    {
        AddError(Error);
    }
    TestEqual(TEXT("The official source contains 117 rows"), Rows.Num(), 117);

    const FBuffLocalizationRowSource* SupplyShortage = Rows.FindByPredicate(
        [](const FBuffLocalizationRowSource& Row)
        {
            return Row.RowName == TEXT("custom_card_event_supply_shortage");
        });
    TestNotNull(TEXT("The supply-shortage row is retained"), SupplyShortage);
    if (SupplyShortage != nullptr)
    {
        TestEqual(
            TEXT("The official supply-shortage FText key is unchanged"),
            SupplyShortage->BuffText.Key,
            FString(TEXT("F65A3A9E430A50392DD417849631FD4B")));
        TestEqual(
            TEXT("The official supply-shortage source is unchanged"),
            SupplyShortage->BuffText.SourceString,
            FString(TEXT("Receives 1 damage at the start of owner's turn.")));
    }

    TArray<FBuffLocalizationRowSource> RejectedRows;
    FString RejectionError;
    TestFalse(
        TEXT("An unexpected official row count is rejected"),
        FBuffLocalizationSource::LoadAndValidate(SourcePath, 116, RejectedRows, RejectionError));
    TestTrue(TEXT("Row-count rejection explains the problem"), RejectionError.Contains(TEXT("Expected 116")));
    TestEqual(TEXT("Rejected parsing exposes no partial rows"), RejectedRows.Num(), 0);
    return true;
}

#endif
