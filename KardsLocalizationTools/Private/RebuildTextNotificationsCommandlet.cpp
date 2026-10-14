#include "RebuildTextNotificationsCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "EdGraphSchema_K2.h"
#include "Engine/DataTable.h"
#include "Factories/DataTableFactory.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
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

DEFINE_LOG_CATEGORY_STATIC(LogTextNotificationsRebuilder, Log, All);

namespace TextNotificationsRebuilder
{
    constexpr int32 ExpectedRowCount = 1;
    constexpr int32 ExpectedEntryCount = 125;

    const TCHAR* StructSourcePath = TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kardsC/Content/Structs/textNotificationsStruct.json");
    const TCHAR* TableSourcePath = TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kardsC/Content/Structs/textNotifications.json");

    const TCHAR* StructPackageName = TEXT("/Game/Structs/textNotificationsStruct");
    const TCHAR* StructObjectPath = TEXT("/Game/Structs/textNotificationsStruct.textNotificationsStruct");
    const TCHAR* TablePackageName = TEXT("/Game/Structs/textNotifications");
    const TCHAR* TableObjectPath = TEXT("/Game/Structs/textNotifications.textNotifications");

    const TCHAR* StructName = TEXT("textNotificationsStruct");
    const TCHAR* TableName = TEXT("textNotifications");
    const TCHAR* RowName = TEXT("MustTarget");
    const TCHAR* TextsPropertyName = TEXT("texts_12_57C9B6784E2CE40B4CC7FAAE3D9145F7");
    const TCHAR* TextsFriendlyName = TEXT("texts");
    const TCHAR* StructGuidText = TEXT("A54F5983-4465263D-CA70F299-84193628");

    const FGuid StructGuid(0xA54F5983, 0x4465263D, 0xCA70F299, 0x84193628);
    const FGuid TextsPropertyGuid(0x57C9B678, 0x4E2CE40B, 0x4CC7FAAE, 0x3D9145F7);

    struct FEntry
    {
        FString MapKey;
        FString Namespace;
        FString TextKey;
        FString SourceString;
    };

    bool LoadJsonRoot(const FString& SourcePath, TSharedPtr<FJsonObject>& OutRoot, FString& OutError)
    {
        FString JsonText;
        if (!FFileHelper::LoadFileToString(JsonText, *SourcePath))
        {
            OutError = FString::Printf(TEXT("Unable to read %s."), *SourcePath);
            return false;
        }

        TArray<TSharedPtr<FJsonValue>> RootValues;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
        if (!FJsonSerializer::Deserialize(Reader, RootValues) || RootValues.Num() != 1)
        {
            OutError = FString::Printf(TEXT("%s must contain exactly one export."), *SourcePath);
            return false;
        }

        OutRoot = RootValues[0].IsValid() ? RootValues[0]->AsObject() : nullptr;
        if (!OutRoot.IsValid())
        {
            OutError = FString::Printf(TEXT("The root export in %s is not an object."), *SourcePath);
            return false;
        }
        return true;
    }

    bool ValidateStructSource(FString& OutError)
    {
        TSharedPtr<FJsonObject> Root;
        if (!LoadJsonRoot(StructSourcePath, Root, OutError))
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
            || Name != StructName
            || !Root->TryGetObjectField(TEXT("Properties"), Properties)
            || Properties == nullptr
            || !Properties->IsValid()
            || !(*Properties)->TryGetStringField(TEXT("Guid"), GuidText)
            || GuidText != StructGuidText
            || !Root->TryGetArrayField(TEXT("ChildProperties"), ChildProperties)
            || ChildProperties == nullptr
            || ChildProperties->Num() != 1)
        {
            OutError = TEXT("textNotificationsStruct source identity does not match the official export.");
            return false;
        }

        const TSharedPtr<FJsonObject> Property = (*ChildProperties)[0].IsValid()
            ? (*ChildProperties)[0]->AsObject()
            : nullptr;
        FString PropertyType;
        FString PropertyName;
        if (!Property.IsValid()
            || !Property->TryGetStringField(TEXT("Type"), PropertyType)
            || !Property->TryGetStringField(TEXT("Name"), PropertyName)
            || PropertyType != TEXT("MapProperty")
            || PropertyName != TextsPropertyName)
        {
            OutError = TEXT("The textNotificationsStruct map field identity does not match the official export.");
            return false;
        }
        return true;
    }

    bool LoadTableSource(TArray<FEntry>& OutEntries, FString& OutError)
    {
        OutEntries.Reset();

        TSharedPtr<FJsonObject> Root;
        if (!LoadJsonRoot(TableSourcePath, Root, OutError))
        {
            return false;
        }

        FString Type;
        FString Name;
        const TSharedPtr<FJsonObject>* Rows = nullptr;
        if (!Root->TryGetStringField(TEXT("Type"), Type)
            || !Root->TryGetStringField(TEXT("Name"), Name)
            || Type != TEXT("DataTable")
            || Name != TableName
            || !Root->TryGetObjectField(TEXT("Rows"), Rows)
            || Rows == nullptr
            || !Rows->IsValid()
            || (*Rows)->Values.Num() != ExpectedRowCount)
        {
            OutError = TEXT("textNotifications source does not contain the expected single data-table row.");
            return false;
        }

        const TSharedPtr<FJsonValue>* RowValue = (*Rows)->Values.Find(RowName);
        if (RowValue == nullptr || !RowValue->IsValid())
        {
            OutError = TEXT("textNotifications source is missing the MustTarget row.");
            return false;
        }

        const TSharedPtr<FJsonObject> RowObject = (*RowValue)->AsObject();
        const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
        if (!RowObject.IsValid()
            || !RowObject->TryGetArrayField(TextsPropertyName, Entries)
            || Entries == nullptr
            || Entries->Num() != ExpectedEntryCount)
        {
            OutError = FString::Printf(TEXT("MustTarget must contain %d map entries."), ExpectedEntryCount);
            return false;
        }

        TSet<FString> SeenMapKeys;
        SeenMapKeys.Reserve(Entries->Num());
        OutEntries.Reserve(Entries->Num());
        for (const TSharedPtr<FJsonValue>& EntryValue : *Entries)
        {
            const TSharedPtr<FJsonObject> EntryObject = EntryValue.IsValid() ? EntryValue->AsObject() : nullptr;
            const TSharedPtr<FJsonObject>* TextObject = nullptr;
            FEntry Entry;
            if (!EntryObject.IsValid()
                || !EntryObject->TryGetStringField(TEXT("Key"), Entry.MapKey)
                || !EntryObject->TryGetObjectField(TEXT("Value"), TextObject)
                || TextObject == nullptr
                || !TextObject->IsValid()
                || !(*TextObject)->TryGetStringField(TEXT("Namespace"), Entry.Namespace)
                || !(*TextObject)->TryGetStringField(TEXT("Key"), Entry.TextKey)
                || !(*TextObject)->TryGetStringField(TEXT("SourceString"), Entry.SourceString)
                || Entry.MapKey.IsEmpty()
                || Entry.TextKey.IsEmpty())
            {
                OutError = TEXT("A textNotifications entry is missing its map key or FText identity.");
                OutEntries.Reset();
                return false;
            }

            if (SeenMapKeys.Contains(Entry.MapKey))
            {
                OutError = FString::Printf(TEXT("Duplicate textNotifications map key: %s."), *Entry.MapKey);
                OutEntries.Reset();
                return false;
            }

            SeenMapKeys.Add(Entry.MapKey);
            OutEntries.Add(MoveTemp(Entry));
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
            OutError = FString::Printf(TEXT("Unable to create %s in %s."), AssetName, PackageName);
            return nullptr;
        }

        FAssetRegistryModule::AssetCreated(Asset);
        return Asset;
    }

    UUserDefinedStruct* BuildStruct(FString& OutError)
    {
        UUserDefinedStruct* Struct = LoadOrCreateAsset<UUserDefinedStruct>(
            StructObjectPath,
            StructPackageName,
            StructName,
            [](UPackage* Package)
            {
                return FStructureEditorUtils::CreateUserDefinedStruct(
                    Package,
                    StructName,
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

        FEdGraphTerminalType ValueType;
        ValueType.TerminalCategory = UEdGraphSchema_K2::PC_Text;
        FEdGraphPinType MapType(
            UEdGraphSchema_K2::PC_String,
            NAME_None,
            nullptr,
            EPinContainerType::Map,
            false,
            ValueType);

        FStructVariableDescription Variable;
        Variable.VarName = FName(TextsPropertyName);
        Variable.FriendlyName = TextsFriendlyName;
        Variable.VarGuid = TextsPropertyGuid;
        Variable.SetPinType(MapType);
        Variables.Add(MoveTemp(Variable));

        FStructureEditorUtils::CompileStructure(Struct);
        if (Struct->Status != EUserDefinedStructureStatus::UDSS_UpToDate)
        {
            OutError = FString::Printf(TEXT("textNotificationsStruct compilation failed: %s"), *Struct->ErrorMessage);
            return nullptr;
        }

        Struct->MarkPackageDirty();
        return Struct;
    }

    bool ResolveMapProperty(
        UUserDefinedStruct* Struct,
        FMapProperty*& OutMapProperty,
        FStrProperty*& OutKeyProperty,
        FTextProperty*& OutValueProperty,
        FString& OutError)
    {
        OutMapProperty = FindFProperty<FMapProperty>(Struct, TextsPropertyName);
        OutKeyProperty = OutMapProperty != nullptr ? CastField<FStrProperty>(OutMapProperty->KeyProp) : nullptr;
        OutValueProperty = OutMapProperty != nullptr ? CastField<FTextProperty>(OutMapProperty->ValueProp) : nullptr;
        if (OutMapProperty == nullptr || OutKeyProperty == nullptr || OutValueProperty == nullptr)
        {
            OutError = TEXT("The compiled structure does not contain a String-to-Text map with the official field name.");
            return false;
        }
        return true;
    }

    bool AddTableRow(
        UDataTable* DataTable,
        UUserDefinedStruct* Struct,
        FMapProperty* MapProperty,
        FStrProperty* KeyProperty,
        FTextProperty* ValueProperty,
        const TArray<FEntry>& Entries,
        FString& OutError)
    {
        FStructOnScope RowData(Struct);
        uint8* Memory = RowData.GetStructMemory();
        void* MapMemory = MapProperty->ContainerPtrToValuePtr<void>(Memory);
        FScriptMapHelper MapHelper(MapProperty, MapMemory);

        for (const FEntry& Entry : Entries)
        {
            const int32 PairIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
            KeyProperty->SetPropertyValue(MapHelper.GetKeyPtr(PairIndex), Entry.MapKey);
            const FText Text = FText::ChangeKey(
                Entry.Namespace,
                Entry.TextKey,
                FText::FromString(Entry.SourceString));
            ValueProperty->SetPropertyValue(MapHelper.GetValuePtr(PairIndex), Text);
        }
        MapHelper.Rehash();
        DataTable->AddRow(FName(RowName), Memory, Struct);
        return true;
    }

    UDataTable* BuildDataTable(
        UUserDefinedStruct* Struct,
        const TArray<FEntry>& Entries,
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
                TableName,
                RF_Public | RF_Standalone | RF_Transactional,
                nullptr,
                GWarn));
            if (DataTable == nullptr)
            {
                OutError = TEXT("Unable to create /Game/Structs/textNotifications.");
                return nullptr;
            }
            FAssetRegistryModule::AssetCreated(DataTable);
        }
        else if (DataTable->GetRowStruct() != Struct)
        {
            OutError = TEXT("Existing textNotifications uses a different row struct.");
            return nullptr;
        }

        FMapProperty* MapProperty = nullptr;
        FStrProperty* KeyProperty = nullptr;
        FTextProperty* ValueProperty = nullptr;
        if (!ResolveMapProperty(Struct, MapProperty, KeyProperty, ValueProperty, OutError))
        {
            return nullptr;
        }

        DataTable->EmptyTable();
        if (!AddTableRow(DataTable, Struct, MapProperty, KeyProperty, ValueProperty, Entries, OutError))
        {
            return nullptr;
        }

        DataTable->MarkPackageDirty();
        return DataTable;
    }

    bool VerifyAssets(
        UUserDefinedStruct* Struct,
        UDataTable* DataTable,
        const TArray<FEntry>& Entries,
        FString& OutError)
    {
        if (Struct == nullptr || DataTable == nullptr)
        {
            OutError = TEXT("Rebuilt structure or data table is null.");
            return false;
        }
        if (Struct->GetPathName() != StructObjectPath || Struct->Guid != StructGuid)
        {
            OutError = TEXT("textNotificationsStruct path or GUID mismatch.");
            return false;
        }

        const TArray<FStructVariableDescription>& Variables = FStructureEditorUtils::GetVarDesc(Struct);
        if (Variables.Num() != 1)
        {
            OutError = TEXT("textNotificationsStruct must contain exactly one field.");
            return false;
        }

        const FStructVariableDescription& Variable = Variables[0];
        if (Variable.VarName != FName(TextsPropertyName)
            || Variable.VarGuid != TextsPropertyGuid
            || Variable.FriendlyName != TextsFriendlyName
            || Variable.Category != UEdGraphSchema_K2::PC_String
            || Variable.ContainerType != EPinContainerType::Map
            || Variable.PinValueType.TerminalCategory != UEdGraphSchema_K2::PC_Text)
        {
            OutError = TEXT("textNotificationsStruct field name, GUID, friendly name, or Map<String, Text> type mismatch.");
            return false;
        }

        if (DataTable->GetPathName() != TableObjectPath
            || DataTable->GetRowStruct() != Struct
            || DataTable->GetRowMap().Num() != ExpectedRowCount)
        {
            OutError = TEXT("textNotifications path, row struct, or row count mismatch.");
            return false;
        }

        FMapProperty* MapProperty = nullptr;
        FStrProperty* KeyProperty = nullptr;
        FTextProperty* ValueProperty = nullptr;
        if (!ResolveMapProperty(Struct, MapProperty, KeyProperty, ValueProperty, OutError))
        {
            return false;
        }

        const uint8* RowMemory = DataTable->FindRowUnchecked(FName(RowName));
        if (RowMemory == nullptr)
        {
            OutError = TEXT("textNotifications is missing the MustTarget row.");
            return false;
        }

        FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(const_cast<uint8*>(RowMemory)));
        if (MapHelper.Num() != Entries.Num())
        {
            OutError = FString::Printf(TEXT("Expected %d map entries after rebuild, found %d."), Entries.Num(), MapHelper.Num());
            return false;
        }

        for (const FEntry& Expected : Entries)
        {
            bool bFound = false;
            for (int32 PairIndex = 0; PairIndex < MapHelper.GetMaxIndex(); ++PairIndex)
            {
                if (!MapHelper.IsValidIndex(PairIndex))
                {
                    continue;
                }

                const FString Key = KeyProperty->GetPropertyValue(MapHelper.GetKeyPtr(PairIndex));
                if (Key != Expected.MapKey)
                {
                    continue;
                }

                const FText Text = ValueProperty->GetPropertyValue(MapHelper.GetValuePtr(PairIndex));
                const TOptional<FString> Namespace = FTextInspector::GetNamespace(Text);
                const TOptional<FString> TextKey = FTextInspector::GetKey(Text);
                const FString* SourceString = FTextInspector::GetSourceString(Text);
                if (!Namespace.IsSet()
                    || Namespace.GetValue() != Expected.Namespace
                    || !TextKey.IsSet()
                    || TextKey.GetValue() != Expected.TextKey
                    || SourceString == nullptr
                    || *SourceString != Expected.SourceString)
                {
                    OutError = FString::Printf(TEXT("FText identity mismatch for map key '%s'."), *Expected.MapKey);
                    return false;
                }

                bFound = true;
                break;
            }

            if (!bFound)
            {
                OutError = FString::Printf(TEXT("Rebuilt table is missing map key '%s'."), *Expected.MapKey);
                return false;
            }
        }
        return true;
    }

    bool SaveAsset(UObject* Asset, FString& OutError)
    {
        UPackage* Package = Asset != nullptr ? Asset->GetOutermost() : nullptr;
        if (Package == nullptr)
        {
            OutError = TEXT("Cannot save a null asset package.");
            return false;
        }

        const FString Filename = FPackageName::LongPackageNameToFilename(
            Package->GetName(),
            FPackageName::GetAssetPackageExtension());
        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
        SaveArgs.SaveFlags = SAVE_NoError;
        SaveArgs.bSlowTask = false;
        if (!UPackage::SavePackage(Package, Asset, *Filename, SaveArgs))
        {
            OutError = FString::Printf(TEXT("Unable to save %s."), *Filename);
            return false;
        }
        return true;
    }
}

URebuildTextNotificationsCommandlet::URebuildTextNotificationsCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
    ShowErrorCount = true;
}

int32 URebuildTextNotificationsCommandlet::Main(const FString& Params)
{
    using namespace TextNotificationsRebuilder;

    FString Error;
    TArray<FEntry> Entries;
    if (!ValidateStructSource(Error) || !LoadTableSource(Entries, Error))
    {
        UE_LOG(LogTextNotificationsRebuilder, Error, TEXT("Source validation failed: %s"), *Error);
        return 1;
    }

    if (FParse::Param(*Params, TEXT("VerifyOnly")))
    {
        UUserDefinedStruct* Struct = LoadObject<UUserDefinedStruct>(nullptr, StructObjectPath);
        UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TableObjectPath);
        if (!VerifyAssets(Struct, DataTable, Entries, Error))
        {
            UE_LOG(LogTextNotificationsRebuilder, Error, TEXT("Reload verification failed: %s"), *Error);
            return 6;
        }

        UE_LOG(
            LogTextNotificationsRebuilder,
            Display,
            TEXT("RELOAD VERIFIED: %s; %s; rows=1; entries=%d; official FText identities preserved."),
            StructObjectPath,
            TableObjectPath,
            Entries.Num());
        return 0;
    }

    UUserDefinedStruct* Struct = BuildStruct(Error);
    if (Struct == nullptr)
    {
        UE_LOG(LogTextNotificationsRebuilder, Error, TEXT("Structure reconstruction failed: %s"), *Error);
        return 2;
    }

    UDataTable* DataTable = BuildDataTable(Struct, Entries, Error);
    if (DataTable == nullptr)
    {
        UE_LOG(LogTextNotificationsRebuilder, Error, TEXT("Data-table reconstruction failed: %s"), *Error);
        return 3;
    }

    if (!VerifyAssets(Struct, DataTable, Entries, Error))
    {
        UE_LOG(LogTextNotificationsRebuilder, Error, TEXT("In-memory verification failed: %s"), *Error);
        return 4;
    }

    if (!SaveAsset(Struct, Error) || !SaveAsset(DataTable, Error))
    {
        UE_LOG(LogTextNotificationsRebuilder, Error, TEXT("Package save failed: %s"), *Error);
        return 5;
    }

    UE_LOG(
        LogTextNotificationsRebuilder,
        Display,
        TEXT("VERIFIED: %s; %s; rows=1; entries=%d; official FText identities preserved."),
        StructObjectPath,
        TableObjectPath,
        Entries.Num());
    return 0;
}
