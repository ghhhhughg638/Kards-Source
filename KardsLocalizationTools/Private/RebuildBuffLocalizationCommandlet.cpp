#include "RebuildBuffLocalizationCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "BuffLocalizationSource.h"
#include "Dom/JsonObject.h"
#include "EdGraphSchema_K2.h"
#include "Engine/DataTable.h"
#include "Engine/UserDefinedEnum.h"
#include "Factories/DataTableFactory.h"
#include "HAL/FileManager.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"
#include "Internationalization/Text.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Kismet2/EnumEditorUtils.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "PackageTools.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "StructUtils/UserDefinedStruct.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/StructOnScope.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "Internationalization/StringTable.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuffLocalizationRebuilder, Log, All);

namespace BuffLocalizationRebuilder
{
    constexpr int32 OfficialRowCount = 117;

    const TCHAR* EnumSourcePath = TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kardsC/Content/E_textColor.json");
    const TCHAR* StructSourcePath = TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kardsC/Content/Structs/Localization/S_buffTextLocalize.json");
    const TCHAR* TableSourcePath = TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kardsC/Content/Structs/Localization/T_buffTextLocalize.json");

    const TCHAR* EnumPackageName = TEXT("/Game/E_textColor");
    const TCHAR* EnumObjectPath = TEXT("/Game/E_textColor.E_textColor");
    const TCHAR* StructPackageName = TEXT("/Game/Structs/Localization/S_buffTextLocalize");
    const TCHAR* StructObjectPath = TEXT("/Game/Structs/Localization/S_buffTextLocalize.S_buffTextLocalize");
    const TCHAR* TablePackageName = TEXT("/Game/Structs/Localization/T_buffTextLocalize");
    const TCHAR* TableObjectPath = TEXT("/Game/Structs/Localization/T_buffTextLocalize.T_buffTextLocalize");
    const TCHAR* StringTableObjectPath = TEXT("/Game/LocalizationData/EN/DIY-EN.DIY-EN");

    const TCHAR* BuffTextPropertyName = TEXT("buffText_4_CBD3177147DD4DCB7795ED95346B7453");
    const TCHAR* AmountFormatPropertyName = TEXT("amountFormat_14_F03314384E1C646547358386CBD37212");
    const TCHAR* TextColorPropertyName = TEXT("textColor_15_A3D49FA246EBA1D5525F63BF76B51BA6");

    const TCHAR* SupplyShortageRowName = TEXT("custom_card_event_supply_shortage");
    const TCHAR* SupplyShortageTextKey = TEXT("F65A3A9E430A50392DD417849631FD4B");
    const TCHAR* CustomRowName = TEXT("custom_card_event_critical_damage_old");
    const TCHAR* CustomStringTableKey = TEXT("card_event_critical_damage_old_buff_text");
    const TCHAR* CustomSourceString = TEXT("When this unit takes damage, pin it.");
    const TCHAR* CustomChineseString = TEXT("此单位受到伤害时，将其压制。");

    const FGuid StructGuid(0xE29CC961, 0x4BCC2BB6, 0x391FEBBA, 0xE7C49022);
    const FGuid BuffTextGuid(0xCBD31771, 0x47DD4DCB, 0x7795ED95, 0x346B7453);
    const FGuid AmountFormatGuid(0xF0331438, 0x4E1C6465, 0x47358386, 0xCBD37212);
    const FGuid TextColorGuid(0xA3D49FA2, 0x46EBA1D5, 0x525F63BF, 0x76B51BA6);

    struct FEnumEntryIdentity
    {
        const TCHAR* InternalName;
        const TCHAR* DisplayName;
        const TCHAR* TextKey;
    };

    const FEnumEntryIdentity EnumEntries[] =
    {
        { TEXT("NewEnumerator0"), TEXT("postive"), TEXT("B746BA684FEA130C383E128AB4CF468A") },
        { TEXT("NewEnumerator1"), TEXT("negative"), TEXT("6374466E41F3C5FF6CF0538EC8DBA6C9") },
        { TEXT("NewEnumerator2"), TEXT("neutral"), TEXT("94E7701442B6EC646FF89481DF220895") }
    };

    bool LoadSingleExportObject(const FString& SourcePath, TSharedPtr<FJsonObject>& OutObject, FString& OutError)
    {
        FString JsonText;
        if (!FFileHelper::LoadFileToString(JsonText, *SourcePath))
        {
            OutError = FString::Printf(TEXT("Unable to read source JSON: %s"), *SourcePath);
            return false;
        }

        TArray<TSharedPtr<FJsonValue>> RootValues;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
        if (!FJsonSerializer::Deserialize(Reader, RootValues) || RootValues.Num() != 1)
        {
            OutError = FString::Printf(TEXT("Source JSON must contain exactly one export: %s"), *SourcePath);
            return false;
        }

        OutObject = RootValues[0].IsValid() ? RootValues[0]->AsObject() : nullptr;
        if (!OutObject.IsValid())
        {
            OutError = FString::Printf(TEXT("Source JSON root export is not an object: %s"), *SourcePath);
            return false;
        }
        return true;
    }

    bool ValidateEnumSource(FString& OutError)
    {
        TSharedPtr<FJsonObject> Root;
        if (!LoadSingleExportObject(EnumSourcePath, Root, OutError))
        {
            return false;
        }

        FString Type;
        FString Name;
        if (!Root->TryGetStringField(TEXT("Type"), Type)
            || !Root->TryGetStringField(TEXT("Name"), Name)
            || Type != TEXT("UserDefinedEnum")
            || Name != TEXT("E_textColor"))
        {
            OutError = TEXT("Enum source is not the expected E_textColor export.");
            return false;
        }

        const TSharedPtr<FJsonObject>* Properties = nullptr;
        const TArray<TSharedPtr<FJsonValue>>* DisplayNames = nullptr;
        if (!Root->TryGetObjectField(TEXT("Properties"), Properties)
            || Properties == nullptr
            || !Properties->IsValid()
            || !(*Properties)->TryGetArrayField(TEXT("DisplayNameMap"), DisplayNames)
            || DisplayNames == nullptr
            || DisplayNames->Num() != UE_ARRAY_COUNT(EnumEntries))
        {
            OutError = TEXT("Enum source has an unexpected DisplayNameMap.");
            return false;
        }

        for (int32 Index = 0; Index < UE_ARRAY_COUNT(EnumEntries); ++Index)
        {
            const TSharedPtr<FJsonObject> Entry = (*DisplayNames)[Index].IsValid() ? (*DisplayNames)[Index]->AsObject() : nullptr;
            const TSharedPtr<FJsonObject>* Value = nullptr;
            FString InternalName;
            FString DisplayName;
            FString TextKey;
            if (!Entry.IsValid()
                || !Entry->TryGetStringField(TEXT("Key"), InternalName)
                || !Entry->TryGetObjectField(TEXT("Value"), Value)
                || Value == nullptr
                || !Value->IsValid()
                || !(*Value)->TryGetStringField(TEXT("SourceString"), DisplayName)
                || !(*Value)->TryGetStringField(TEXT("Key"), TextKey)
                || InternalName != EnumEntries[Index].InternalName
                || DisplayName != EnumEntries[Index].DisplayName
                || TextKey != EnumEntries[Index].TextKey)
            {
                OutError = FString::Printf(TEXT("Enum source identity mismatch at index %d."), Index);
                return false;
            }
        }
        return true;
    }

    bool ValidateStructSource(FString& OutError)
    {
        TSharedPtr<FJsonObject> Root;
        if (!LoadSingleExportObject(StructSourcePath, Root, OutError))
        {
            return false;
        }

        FString Type;
        FString Name;
        const TSharedPtr<FJsonObject>* Properties = nullptr;
        FString GuidText;
        const TArray<TSharedPtr<FJsonValue>>* ChildProperties = nullptr;
        if (!Root->TryGetStringField(TEXT("Type"), Type)
            || !Root->TryGetStringField(TEXT("Name"), Name)
            || Type != TEXT("UserDefinedStruct")
            || Name != TEXT("S_buffTextLocalize")
            || !Root->TryGetObjectField(TEXT("Properties"), Properties)
            || Properties == nullptr
            || !Properties->IsValid()
            || !(*Properties)->TryGetStringField(TEXT("Guid"), GuidText)
            || GuidText != TEXT("E29CC961-4BCC2BB6-391FEBBA-E7C49022")
            || !Root->TryGetArrayField(TEXT("ChildProperties"), ChildProperties)
            || ChildProperties == nullptr
            || ChildProperties->Num() != 3)
        {
            OutError = TEXT("Struct source is not the expected S_buffTextLocalize export.");
            return false;
        }

        const TCHAR* ExpectedTypes[] = { TEXT("TextProperty"), TEXT("BoolProperty"), TEXT("ByteProperty") };
        const TCHAR* ExpectedNames[] = { BuffTextPropertyName, AmountFormatPropertyName, TextColorPropertyName };
        for (int32 Index = 0; Index < 3; ++Index)
        {
            const TSharedPtr<FJsonObject> Property = (*ChildProperties)[Index].IsValid() ? (*ChildProperties)[Index]->AsObject() : nullptr;
            FString PropertyType;
            FString PropertyName;
            if (!Property.IsValid()
                || !Property->TryGetStringField(TEXT("Type"), PropertyType)
                || !Property->TryGetStringField(TEXT("Name"), PropertyName)
                || PropertyType != ExpectedTypes[Index]
                || PropertyName != ExpectedNames[Index])
            {
                OutError = FString::Printf(TEXT("Struct source property identity mismatch at index %d."), Index);
                return false;
            }
        }
        return true;
    }

    template <typename AssetType>
    AssetType* LoadOrCreateAsset(
        const TCHAR* ObjectPath,
        const TCHAR* PackageName,
        const TCHAR* AssetName,
        TFunctionRef<AssetType*(UPackage*)> CreateAsset,
        FString& OutError)
    {
        if (AssetType* Existing = LoadObject<AssetType>(nullptr, ObjectPath, nullptr, LOAD_NoWarn))
        {
            return Existing;
        }

        UPackage* Package = CreatePackage(PackageName);
        if (Package == nullptr)
        {
            OutError = FString::Printf(TEXT("Unable to create package %s."), PackageName);
            return nullptr;
        }

        AssetType* Asset = CreateAsset(Package);
        if (Asset == nullptr)
        {
            OutError = FString::Printf(TEXT("Unable to create asset %s in %s."), AssetName, PackageName);
            return nullptr;
        }

        FAssetRegistryModule::AssetCreated(Asset);
        return Asset;
    }

    UUserDefinedEnum* BuildEnum(FString& OutError)
    {
        UUserDefinedEnum* Enum = LoadOrCreateAsset<UUserDefinedEnum>(
            EnumObjectPath,
            EnumPackageName,
            TEXT("E_textColor"),
            [](UPackage* Package)
            {
                return Cast<UUserDefinedEnum>(FEnumEditorUtils::CreateUserDefinedEnum(
                    Package,
                    TEXT("E_textColor"),
                    RF_Public | RF_Standalone | RF_Transactional));
            },
            OutError);
        if (Enum == nullptr)
        {
            return nullptr;
        }

        TArray<TPair<FName, int64>> Names;
        Names.Reserve(UE_ARRAY_COUNT(EnumEntries));
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(EnumEntries); ++Index)
        {
            Names.Emplace(FName(*Enum->GenerateFullEnumName(EnumEntries[Index].InternalName)), Index);
        }
        if (!Enum->SetEnums(Names, UEnum::ECppForm::Namespaced))
        {
            OutError = TEXT("Unable to assign the exact E_textColor enumerator names.");
            return nullptr;
        }

        Enum->UniqueNameIndex = UE_ARRAY_COUNT(EnumEntries);
        Enum->DisplayNameMap.Reset();
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(EnumEntries); ++Index)
        {
            const FName EntryName(*Enum->GetNameStringByIndex(Index));
            const FText DisplayText = FText::ChangeKey(
                TEXT(""),
                EnumEntries[Index].TextKey,
                FText::FromString(EnumEntries[Index].DisplayName));
            Enum->DisplayNameMap.Add(EntryName, DisplayText);
        }
        Enum->SetMetaData(TEXT("BlueprintType"), TEXT("true"));
        Enum->MarkPackageDirty();
        return Enum;
    }

    void AddStructVariable(
        TArray<FStructVariableDescription>& Variables,
        const TCHAR* InternalName,
        const TCHAR* FriendlyName,
        const FGuid& Guid,
        const FEdGraphPinType& PinType)
    {
        FStructVariableDescription Variable;
        Variable.VarName = FName(InternalName);
        Variable.FriendlyName = FriendlyName;
        Variable.VarGuid = Guid;
        Variable.SetPinType(PinType);
        Variables.Add(MoveTemp(Variable));
    }

    UUserDefinedStruct* BuildStruct(UUserDefinedEnum* Enum, FString& OutError)
    {
        UUserDefinedStruct* Struct = LoadOrCreateAsset<UUserDefinedStruct>(
            StructObjectPath,
            StructPackageName,
            TEXT("S_buffTextLocalize"),
            [](UPackage* Package)
            {
                return FStructureEditorUtils::CreateUserDefinedStruct(
                    Package,
                    TEXT("S_buffTextLocalize"),
                    RF_Public | RF_Standalone | RF_Transactional);
            },
            OutError);
        if (Struct == nullptr)
        {
            return nullptr;
        }

        Struct->Guid = StructGuid;
        TArray<FStructVariableDescription>& Variables = FStructureEditorUtils::GetVarDesc(Struct);
        Variables.Reset();

        AddStructVariable(
            Variables,
            BuffTextPropertyName,
            TEXT("buffText"),
            BuffTextGuid,
            FEdGraphPinType(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
        AddStructVariable(
            Variables,
            AmountFormatPropertyName,
            TEXT("amountFormat"),
            AmountFormatGuid,
            FEdGraphPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
        AddStructVariable(
            Variables,
            TextColorPropertyName,
            TEXT("textColor"),
            TextColorGuid,
            FEdGraphPinType(UEdGraphSchema_K2::PC_Byte, NAME_None, Enum, EPinContainerType::None, false, FEdGraphTerminalType()));

        FStructureEditorUtils::CompileStructure(Struct);
        if (Struct->Status != EUserDefinedStructureStatus::UDSS_UpToDate)
        {
            OutError = FString::Printf(TEXT("S_buffTextLocalize compilation failed: %s"), *Struct->ErrorMessage);
            return nullptr;
        }
        Struct->MarkPackageDirty();
        return Struct;
    }

    bool ResolveRowProperties(
        UUserDefinedStruct* Struct,
        UUserDefinedEnum* Enum,
        FTextProperty*& OutTextProperty,
        FBoolProperty*& OutAmountProperty,
        FByteProperty*& OutColorProperty,
        FString& OutError)
    {
        OutTextProperty = FindFProperty<FTextProperty>(Struct, BuffTextPropertyName);
        OutAmountProperty = FindFProperty<FBoolProperty>(Struct, AmountFormatPropertyName);
        OutColorProperty = FindFProperty<FByteProperty>(Struct, TextColorPropertyName);
        if (OutTextProperty == nullptr || OutAmountProperty == nullptr || OutColorProperty == nullptr)
        {
            OutError = TEXT("Compiled S_buffTextLocalize properties do not match the official field names and types.");
            return false;
        }
        if (OutColorProperty->Enum != Enum)
        {
            OutError = TEXT("Compiled S_buffTextLocalize textColor property is not backed by /Game/E_textColor.");
            return false;
        }
        return true;
    }

    bool AddTableRow(
        UDataTable* DataTable,
        UUserDefinedStruct* Struct,
        FTextProperty* TextProperty,
        FBoolProperty* AmountProperty,
        FByteProperty* ColorProperty,
        const FName RowName,
        const FText& Text,
        const bool bAmountFormat,
        const uint8 Color,
        FString& OutError)
    {
        if (Color > 2)
        {
            OutError = FString::Printf(TEXT("Row '%s' contains an unsupported color index %d."), *RowName.ToString(), Color);
            return false;
        }

        FStructOnScope RowData(Struct);
        uint8* Memory = RowData.GetStructMemory();
        TextProperty->SetPropertyValue_InContainer(Memory, Text);
        AmountProperty->SetPropertyValue_InContainer(Memory, bAmountFormat);
        ColorProperty->SetPropertyValue_InContainer(Memory, Color);
        DataTable->AddRow(RowName, Memory, Struct);
        return true;
    }

    UDataTable* BuildDataTable(
        UUserDefinedStruct* Struct,
        UUserDefinedEnum* Enum,
        UStringTable* StringTable,
        const TArray<FBuffLocalizationRowSource>& OfficialRows,
        FString& OutError)
    {
        UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TableObjectPath, nullptr, LOAD_NoWarn);
        if (DataTable == nullptr)
        {
            UPackage* Package = CreatePackage(TablePackageName);
            UDataTableFactory* Factory = NewObject<UDataTableFactory>();
            Factory->Struct = Struct;
            DataTable = Cast<UDataTable>(Factory->FactoryCreateNew(
                UDataTable::StaticClass(),
                Package,
                TEXT("T_buffTextLocalize"),
                RF_Public | RF_Standalone | RF_Transactional,
                nullptr,
                GWarn));
            if (DataTable == nullptr)
            {
                OutError = TEXT("Unable to create /Game/Structs/Localization/T_buffTextLocalize.");
                return nullptr;
            }
            FAssetRegistryModule::AssetCreated(DataTable);
        }
        else if (DataTable->GetRowStruct() != Struct)
        {
            OutError = TEXT("Existing T_buffTextLocalize uses a different row struct.");
            return nullptr;
        }

        FTextProperty* TextProperty = nullptr;
        FBoolProperty* AmountProperty = nullptr;
        FByteProperty* ColorProperty = nullptr;
        if (!ResolveRowProperties(Struct, Enum, TextProperty, AmountProperty, ColorProperty, OutError))
        {
            return nullptr;
        }

        DataTable->EmptyTable();
        for (const FBuffLocalizationRowSource& SourceRow : OfficialRows)
        {
            uint8 Color = 0;
            if (SourceRow.TextColor == TEXT("E_textColor::NewEnumerator1"))
            {
                Color = 1;
            }
            else if (SourceRow.TextColor == TEXT("E_textColor::NewEnumerator2"))
            {
                Color = 2;
            }

            const FText OfficialText = FText::ChangeKey(
                SourceRow.BuffText.Namespace,
                SourceRow.BuffText.Key,
                FText::FromString(SourceRow.BuffText.SourceString));
            if (!AddTableRow(
                    DataTable,
                    Struct,
                    TextProperty,
                    AmountProperty,
                    ColorProperty,
                    SourceRow.RowName,
                    OfficialText,
                    SourceRow.bAmountFormat,
                    Color,
                    OutError))
            {
                return nullptr;
            }
        }

        const FText CustomText = FText::FromStringTable(StringTable->GetStringTableId(), CustomStringTableKey);
        if (!AddTableRow(
                DataTable,
                Struct,
                TextProperty,
                AmountProperty,
                ColorProperty,
                FName(CustomRowName),
                CustomText,
                false,
                2,
                OutError))
        {
            return nullptr;
        }

        DataTable->MarkPackageDirty();
        return DataTable;
    }

    bool VerifyAssets(
        UUserDefinedEnum* Enum,
        UUserDefinedStruct* Struct,
        UDataTable* DataTable,
        UStringTable* StringTable,
        FString& OutError)
    {
        if (Enum == nullptr || Struct == nullptr || DataTable == nullptr || StringTable == nullptr)
        {
            OutError = TEXT("One or more rebuilt assets could not be loaded for verification.");
            return false;
        }

        if (Enum->GetPathName() != EnumObjectPath || Enum->NumEnums() != 4)
        {
            OutError = TEXT("E_textColor path or enumerator count is incorrect.");
            return false;
        }
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(EnumEntries); ++Index)
        {
            const FText DisplayText = Enum->GetDisplayNameTextByIndex(Index);
            const TOptional<FString> DisplayKey = FTextInspector::GetKey(DisplayText);
            if (Enum->GetNameStringByIndex(Index) != EnumEntries[Index].InternalName
                || DisplayText.ToString() != EnumEntries[Index].DisplayName
                || !DisplayKey.IsSet()
                || DisplayKey.GetValue() != EnumEntries[Index].TextKey)
            {
                OutError = FString::Printf(TEXT("E_textColor identity mismatch at index %d."), Index);
                return false;
            }
        }

        if (Struct->GetPathName() != StructObjectPath || Struct->Guid != StructGuid)
        {
            OutError = TEXT("S_buffTextLocalize path or struct GUID is incorrect.");
            return false;
        }

        const TCHAR* ExpectedPropertyNames[] = { BuffTextPropertyName, AmountFormatPropertyName, TextColorPropertyName };
        int32 PropertyIndex = 0;
        for (TFieldIterator<FProperty> It(Struct, EFieldIterationFlags::None); It; ++It)
        {
            if (PropertyIndex >= 3 || It->GetName() != ExpectedPropertyNames[PropertyIndex])
            {
                OutError = FString::Printf(TEXT("S_buffTextLocalize property order mismatch at index %d."), PropertyIndex);
                return false;
            }
            ++PropertyIndex;
        }
        if (PropertyIndex != 3)
        {
            OutError = FString::Printf(TEXT("S_buffTextLocalize contains %d properties instead of 3."), PropertyIndex);
            return false;
        }

        FTextProperty* TextProperty = nullptr;
        FBoolProperty* AmountProperty = nullptr;
        FByteProperty* ColorProperty = nullptr;
        if (!ResolveRowProperties(Struct, Enum, TextProperty, AmountProperty, ColorProperty, OutError))
        {
            return false;
        }

        if (DataTable->GetPathName() != TableObjectPath
            || DataTable->GetRowStruct() != Struct
            || DataTable->GetRowMap().Num() != OfficialRowCount + 1)
        {
            OutError = FString::Printf(
                TEXT("T_buffTextLocalize path, row struct, or row count is incorrect (found %d rows)."),
                DataTable->GetRowMap().Num());
            return false;
        }

        const uint8* SupplyRow = DataTable->FindRowUnchecked(FName(SupplyShortageRowName));
        if (SupplyRow == nullptr)
        {
            OutError = TEXT("Rebuilt table is missing custom_card_event_supply_shortage.");
            return false;
        }
        const FText SupplyText = TextProperty->GetPropertyValue_InContainer(SupplyRow);
        const TOptional<FString> SupplyKey = FTextInspector::GetKey(SupplyText);
        if (!SupplyKey.IsSet() || SupplyKey.GetValue() != SupplyShortageTextKey)
        {
            OutError = TEXT("Supply-shortage official FText key was not preserved.");
            return false;
        }

        const uint8* CustomRow = DataTable->FindRowUnchecked(FName(CustomRowName));
        if (CustomRow == nullptr)
        {
            OutError = TEXT("Rebuilt table is missing custom_card_event_critical_damage_old.");
            return false;
        }
        const FText CustomText = TextProperty->GetPropertyValue_InContainer(CustomRow);
        FName TableId;
        FString StringTableKey;
        if (!FTextInspector::GetTableIdAndKey(CustomText, TableId, StringTableKey)
            || TableId != StringTable->GetStringTableId()
            || StringTableKey != CustomStringTableKey
            || AmountProperty->GetPropertyValue_InContainer(CustomRow)
            || ColorProperty->GetPropertyValue_InContainer(CustomRow) != 2)
        {
            OutError = FString::Printf(
                TEXT("Custom row reference mismatch: table='%s', expected table='%s', key='%s', amount=%d, color=%d."),
                *TableId.ToString(),
                *StringTable->GetStringTableId().ToString(),
                *StringTableKey,
                AmountProperty->GetPropertyValue_InContainer(CustomRow) ? 1 : 0,
                ColorProperty->GetPropertyValue_InContainer(CustomRow));
            return false;
        }

        FString StoredSource;
        const bool bHasStoredSource = StringTable->GetStringTable()->GetSourceString(CustomStringTableKey, StoredSource);
        if (!bHasStoredSource || StoredSource != CustomSourceString)
        {
            OutError = FString::Printf(
                TEXT("DIY-EN source mismatch after reload: found=%d, stored='%s', expected='%s'."),
                bHasStoredSource ? 1 : 0,
                *StoredSource,
                CustomSourceString);
            return false;
        }
        const FString CurrentCultureName = FInternationalization::Get().GetCurrentCulture()->GetName();
        const FString ExpectedResolvedText = CurrentCultureName.StartsWith(TEXT("zh"))
            ? FString(CustomChineseString)
            : FString(CustomSourceString);
        if (CustomText.ToString() != ExpectedResolvedText)
        {
            OutError = FString::Printf(
                TEXT("Localized FText mismatch for culture '%s': resolved='%s', expected='%s'."),
                *CurrentCultureName,
                *CustomText.ToString(),
                *ExpectedResolvedText);
            return false;
        }
        return true;
    }

    bool SaveAsset(UObject* Asset, FString& OutError)
    {
        UPackage* Package = Asset != nullptr ? Asset->GetOutermost() : nullptr;
        if (Package == nullptr)
        {
            OutError = TEXT("Attempted to save a null asset package.");
            return false;
        }

        const FString Filename = FPackageName::LongPackageNameToFilename(
            Package->GetName(),
            FPackageName::GetAssetPackageExtension());
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);

        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
        SaveArgs.SaveFlags = SAVE_NoError;
        SaveArgs.bSlowTask = false;
        if (!UPackage::SavePackage(Package, Asset, *Filename, SaveArgs))
        {
            OutError = FString::Printf(TEXT("Unable to save package %s to %s."), *Package->GetName(), *Filename);
            return false;
        }
        return true;
    }
}

URebuildBuffLocalizationCommandlet::URebuildBuffLocalizationCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
    ShowErrorCount = true;
}

int32 URebuildBuffLocalizationCommandlet::Main(const FString& Params)
{
    static_cast<void>(Params);
    using namespace BuffLocalizationRebuilder;

    FTextLocalizationManager& LocalizationManager = FTextLocalizationManager::Get();
    LocalizationManager.EnableGameLocalizationPreview(
        FInternationalization::Get().GetCurrentCulture()->GetName());
    LocalizationManager.WaitForAsyncTasks();

    FString Error;
    TArray<FBuffLocalizationRowSource> OfficialRows;
    if (!ValidateEnumSource(Error)
        || !ValidateStructSource(Error)
        || !FBuffLocalizationSource::LoadAndValidate(TableSourcePath, OfficialRowCount, OfficialRows, Error))
    {
        UE_LOG(LogBuffLocalizationRebuilder, Error, TEXT("Source validation failed: %s"), *Error);
        return 1;
    }

    UUserDefinedEnum* Enum = BuildEnum(Error);
    if (Enum == nullptr)
    {
        UE_LOG(LogBuffLocalizationRebuilder, Error, TEXT("Enum reconstruction failed: %s"), *Error);
        return 2;
    }

    UUserDefinedStruct* Struct = BuildStruct(Enum, Error);
    if (Struct == nullptr)
    {
        UE_LOG(LogBuffLocalizationRebuilder, Error, TEXT("Struct reconstruction failed: %s"), *Error);
        return 3;
    }

    UStringTable* StringTable = LoadObject<UStringTable>(nullptr, StringTableObjectPath, nullptr, LOAD_NoWarn);
    if (StringTable == nullptr)
    {
        UE_LOG(LogBuffLocalizationRebuilder, Error, TEXT("Unable to load %s."), StringTableObjectPath);
        return 4;
    }
    StringTable->GetMutableStringTable()->SetSourceString(CustomStringTableKey, CustomSourceString);
    StringTable->MarkPackageDirty();

    UDataTable* DataTable = BuildDataTable(Struct, Enum, StringTable, OfficialRows, Error);
    if (DataTable == nullptr)
    {
        UE_LOG(LogBuffLocalizationRebuilder, Error, TEXT("Data Table reconstruction failed: %s"), *Error);
        return 5;
    }

    if (!VerifyAssets(Enum, Struct, DataTable, StringTable, Error))
    {
        UE_LOG(LogBuffLocalizationRebuilder, Error, TEXT("In-memory verification failed: %s"), *Error);
        return 6;
    }

    UObject* AssetsToSave[] = { StringTable, Enum, Struct, DataTable };
    for (UObject* Asset : AssetsToSave)
    {
        if (!SaveAsset(Asset, Error))
        {
            UE_LOG(LogBuffLocalizationRebuilder, Error, TEXT("Package save failed: %s"), *Error);
            return 7;
        }
    }

    TArray<UPackage*> PackagesToReload;
    PackagesToReload.Reserve(UE_ARRAY_COUNT(AssetsToSave));
    for (UObject* Asset : AssetsToSave)
    {
        PackagesToReload.Add(Asset->GetOutermost());
    }
    for (UPackage* PackageToReload : PackagesToReload)
    {
        if (PackageToReload->GetName() == TablePackageName)
        {
            UStringTable* ReloadedStringTable = LoadObject<UStringTable>(nullptr, StringTableObjectPath, nullptr, LOAD_NoWarn);
            if (ReloadedStringTable == nullptr)
            {
                UE_LOG(LogBuffLocalizationRebuilder, Error, TEXT("String Table disappeared before Data Table reload."));
                return 8;
            }

            FStringTableRegistry& Registry = FStringTableRegistry::Get();
            Registry.UnregisterStringTable(ReloadedStringTable->GetStringTableId());
            Registry.RegisterStringTable(
                ReloadedStringTable->GetStringTableId(),
                ReloadedStringTable->GetMutableStringTable());
        }

        FText ReloadError;
        const TArray<UPackage*> SinglePackage{ PackageToReload };
        if (!UPackageTools::ReloadPackages(
                SinglePackage,
                ReloadError,
                EReloadPackagesInteractionMode::AssumePositive))
        {
            UE_LOG(
                LogBuffLocalizationRebuilder,
                Error,
                TEXT("Package reload failed for %s: %s"),
                *PackageToReload->GetName(),
                *ReloadError.ToString());
            return 8;
        }

    }

    Enum = LoadObject<UUserDefinedEnum>(nullptr, EnumObjectPath, nullptr, LOAD_NoWarn);
    Struct = LoadObject<UUserDefinedStruct>(nullptr, StructObjectPath, nullptr, LOAD_NoWarn);
    DataTable = LoadObject<UDataTable>(nullptr, TableObjectPath, nullptr, LOAD_NoWarn);
    StringTable = LoadObject<UStringTable>(nullptr, StringTableObjectPath, nullptr, LOAD_NoWarn);
    if (!VerifyAssets(Enum, Struct, DataTable, StringTable, Error))
    {
        UE_LOG(LogBuffLocalizationRebuilder, Error, TEXT("Reloaded asset verification failed: %s"), *Error);
        return 9;
    }

    UE_LOG(
        LogBuffLocalizationRebuilder,
        Display,
        TEXT("VERIFIED: culture=%s; E_textColor=3 entries; S_buffTextLocalize GUID=%s; T_buffTextLocalize=%d rows; supply key=%s; custom row=%s -> %s."),
        *FInternationalization::Get().GetCurrentCulture()->GetName(),
        *Struct->Guid.ToString(EGuidFormats::DigitsWithHyphensInBraces),
        DataTable->GetRowMap().Num(),
        SupplyShortageTextKey,
        CustomRowName,
        CustomStringTableKey);
    return 0;
}
