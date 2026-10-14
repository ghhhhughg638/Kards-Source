#include "BuffLocalizationSource.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace BuffLocalizationSourceFields
{
    const TCHAR* BuffText = TEXT("buffText_4_CBD3177147DD4DCB7795ED95346B7453");
    const TCHAR* AmountFormat = TEXT("amountFormat_14_F03314384E1C646547358386CBD37212");
    const TCHAR* TextColor = TEXT("textColor_15_A3D49FA246EBA1D5525F63BF76B51BA6");
    const TCHAR* SupplyShortageRow = TEXT("custom_card_event_supply_shortage");
    const TCHAR* SupplyShortageKey = TEXT("F65A3A9E430A50392DD417849631FD4B");
}

bool FBuffLocalizationSource::LoadAndValidate(
    const FString& SourcePath,
    const int32 ExpectedOfficialRowCount,
    TArray<FBuffLocalizationRowSource>& OutRows,
    FString& OutError)
{
    OutRows.Reset();
    OutError.Reset();

    FString JsonText;
    if (!FFileHelper::LoadFileToString(JsonText, *SourcePath))
    {
        OutError = FString::Printf(TEXT("Unable to read buff localization source: %s"), *SourcePath);
        return false;
    }

    TArray<TSharedPtr<FJsonValue>> RootValues;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, RootValues) || RootValues.Num() != 1)
    {
        OutError = TEXT("Buff localization source must contain exactly one root export object.");
        return false;
    }

    const TSharedPtr<FJsonObject> RootObject = RootValues[0].IsValid() ? RootValues[0]->AsObject() : nullptr;
    if (!RootObject.IsValid())
    {
        OutError = TEXT("Buff localization root export is not an object.");
        return false;
    }

    const TSharedPtr<FJsonObject>* RowsObjectPointer = nullptr;
    if (!RootObject->TryGetObjectField(TEXT("Rows"), RowsObjectPointer)
        || RowsObjectPointer == nullptr
        || !RowsObjectPointer->IsValid())
    {
        OutError = TEXT("Buff localization source has no Rows object.");
        return false;
    }

    const TSharedPtr<FJsonObject>& RowsObject = *RowsObjectPointer;
    if (RowsObject->Values.Num() != ExpectedOfficialRowCount)
    {
        OutError = FString::Printf(
            TEXT("Expected %d official buff localization rows but found %d."),
            ExpectedOfficialRowCount,
            RowsObject->Values.Num());
        return false;
    }

    OutRows.Reserve(RowsObject->Values.Num());
    for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : RowsObject->Values)
    {
        const TSharedPtr<FJsonObject> RowObject = Pair.Value.IsValid() ? Pair.Value->AsObject() : nullptr;
        if (!RowObject.IsValid())
        {
            OutError = FString::Printf(TEXT("Row '%s' is not an object."), *Pair.Key);
            OutRows.Reset();
            return false;
        }

        const TSharedPtr<FJsonObject>* BuffTextObjectPointer = nullptr;
        if (!RowObject->TryGetObjectField(BuffLocalizationSourceFields::BuffText, BuffTextObjectPointer)
            || BuffTextObjectPointer == nullptr
            || !BuffTextObjectPointer->IsValid())
        {
            OutError = FString::Printf(TEXT("Row '%s' has no buffText object."), *Pair.Key);
            OutRows.Reset();
            return false;
        }

        FBuffLocalizationRowSource Row;
        Row.RowName = FName(*Pair.Key);
        const TSharedPtr<FJsonObject>& BuffTextObject = *BuffTextObjectPointer;
        if (!BuffTextObject->TryGetStringField(TEXT("Namespace"), Row.BuffText.Namespace)
            || !BuffTextObject->TryGetStringField(TEXT("Key"), Row.BuffText.Key)
            || !BuffTextObject->TryGetStringField(TEXT("SourceString"), Row.BuffText.SourceString)
            || !RowObject->TryGetBoolField(BuffLocalizationSourceFields::AmountFormat, Row.bAmountFormat)
            || !RowObject->TryGetStringField(BuffLocalizationSourceFields::TextColor, Row.TextColor))
        {
            OutError = FString::Printf(TEXT("Row '%s' is missing a required typed field."), *Pair.Key);
            OutRows.Reset();
            return false;
        }

        const bool bKnownColor =
            Row.TextColor == TEXT("E_textColor::NewEnumerator0")
            || Row.TextColor == TEXT("E_textColor::NewEnumerator1")
            || Row.TextColor == TEXT("E_textColor::NewEnumerator2");
        if (!bKnownColor)
        {
            OutError = FString::Printf(TEXT("Row '%s' has unsupported text color '%s'."), *Pair.Key, *Row.TextColor);
            OutRows.Reset();
            return false;
        }

        OutRows.Add(MoveTemp(Row));
    }

    const FBuffLocalizationRowSource* SupplyShortage = OutRows.FindByPredicate(
        [](const FBuffLocalizationRowSource& Row)
        {
            return Row.RowName == FName(BuffLocalizationSourceFields::SupplyShortageRow);
        });
    if (SupplyShortage == nullptr)
    {
        OutError = TEXT("Official source is missing custom_card_event_supply_shortage.");
        OutRows.Reset();
        return false;
    }

    if (SupplyShortage->BuffText.Key != BuffLocalizationSourceFields::SupplyShortageKey)
    {
        OutError = FString::Printf(
            TEXT("Supply-shortage FText key mismatch: expected %s but found %s."),
            BuffLocalizationSourceFields::SupplyShortageKey,
            *SupplyShortage->BuffText.Key);
        OutRows.Reset();
        return false;
    }

    OutRows.Sort(
        [](const FBuffLocalizationRowSource& Left, const FBuffLocalizationRowSource& Right)
        {
            return Left.RowName.LexicalLess(Right.RowName);
        });
    return true;
}
