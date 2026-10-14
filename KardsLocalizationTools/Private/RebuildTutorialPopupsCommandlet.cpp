#include "RebuildTutorialPopupsCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "EdGraphSchema_K2.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Factories/DataTableFactory.h"
#include "Internationalization/Text.h"
#include "Kismet2/StructureEditorUtils.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "StructUtils/UserDefinedStruct.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/StructOnScope.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"

DEFINE_LOG_CATEGORY_STATIC(LogTutorialPopupsRebuilder, Log, All);

namespace TutorialPopupsRebuilder
{
    constexpr int32 ExpectedRowCount = 67;

    const TCHAR* DefaultSourceRoot =
        TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kards/Content");

    const TCHAR* StructName = TEXT("tutorialPopup");
    const TCHAR* TableName = TEXT("tutorialPopups");
    const TCHAR* StructPackageName = TEXT("/Game/Structs/tutorialPopup");
    const TCHAR* StructObjectPath = TEXT("/Game/Structs/tutorialPopup.tutorialPopup");
    const TCHAR* TablePackageName = TEXT("/Game/Structs/tutorialPopups");
    const TCHAR* TableObjectPath = TEXT("/Game/Structs/tutorialPopups.tutorialPopups");
    const TCHAR* StructGuidText = TEXT("7E2F3866-48C15731-8003BE80-953FAF30");

    const TCHAR* TriggerPropertyName = TEXT("trigger_2_7E4EA08648FCEB9A8A60CEAB9E197E4C");
    const TCHAR* TypePropertyName = TEXT("type_4_214493904F32A2B4A44953A76CE929D7");
    const TCHAR* MessagePropertyName = TEXT("message_18_017DCDDB4B0DEC3872D640B50AF82A81");
    const TCHAR* ExtraMessagePropertyName = TEXT("extraMsg_22_CAAC968C4EA2D0DCF7911B838477594E");
    const TCHAR* CommentPropertyName = TEXT("comment_13_16D7007B470BD201DD764FBB560F5818");
    const TCHAR* TitlePropertyName = TEXT("title_9_67ACCE704D693D94113F6693A9288AAC");
    const TCHAR* IconPropertyName = TEXT("icon_12_6528B0CA499C712ED514BCB999AD8ABE");
    const TCHAR* HideForISBNPropertyName = TEXT("hideForISBN_25_F6CBE2AF427280598050369E65EFF03D");

    const FGuid StructGuid(0x7E2F3866, 0x48C15731, 0x8003BE80, 0x953FAF30);

    struct FPropertySpec
    {
        const TCHAR* Name;
        const TCHAR* FriendlyName;
        const TCHAR* Type;
        FGuid Guid;
    };

    const FPropertySpec PropertySpecs[] =
    {
        { TriggerPropertyName, TEXT("trigger"), TEXT("StrProperty"), FGuid(0x7E4EA086, 0x48FCEB9A, 0x8A60CEAB, 0x9E197E4C) },
        { TypePropertyName, TEXT("type"), TEXT("StrProperty"), FGuid(0x21449390, 0x4F32A2B4, 0xA44953A7, 0x6CE929D7) },
        { MessagePropertyName, TEXT("message"), TEXT("TextProperty"), FGuid(0x017DCDDB, 0x4B0DEC38, 0x72D640B5, 0x0AF82A81) },
        { ExtraMessagePropertyName, TEXT("extraMsg"), TEXT("TextProperty"), FGuid(0xCAAC968C, 0x4EA2D0DC, 0xF7911B83, 0x8477594E) },
        { CommentPropertyName, TEXT("comment"), TEXT("TextProperty"), FGuid(0x16D7007B, 0x470BD201, 0xDD764FBB, 0x560F5818) },
        { TitlePropertyName, TEXT("title"), TEXT("TextProperty"), FGuid(0x67ACCE70, 0x4D693D94, 0x113F6693, 0xA9288AAC) },
        { IconPropertyName, TEXT("icon"), TEXT("ObjectProperty"), FGuid(0x6528B0CA, 0x499C712E, 0xD514BCB9, 0x99AD8ABE) },
        { HideForISBNPropertyName, TEXT("hideForISBN"), TEXT("BoolProperty"), FGuid(0xF6CBE2AF, 0x42728059, 0x8050369E, 0x65EFF03D) }
    };

    struct FTextSource
    {
        bool bEmpty = true;
        FString Namespace;
        FString Key;
        FString SourceString;
    };

    struct FRow
    {
        FName RowName;
        FString Trigger;
        FString Type;
        FTextSource Message;
        FTextSource ExtraMessage;
        FTextSource Comment;
        FTextSource Title;
        FString IconObjectPath;
        bool bHideForISBN = false;
    };

    FString StructSourcePath(const FString& SourceRoot)
    {
        return FPaths::Combine(SourceRoot, TEXT("Structs/tutorialPopup.json"));
    }

    FString TableSourcePath(const FString& SourceRoot)
    {
        return FPaths::Combine(SourceRoot, TEXT("Structs/tutorialPopups.json"));
    }

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

    bool ValidateStructSource(const FString& SourcePath, FString& OutError)
    {
        TSharedPtr<FJsonObject> Root;
        if (!LoadJsonRoot(SourcePath, Root, OutError))
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
            || ChildProperties->Num() != UE_ARRAY_COUNT(PropertySpecs))
        {
            OutError = TEXT("tutorialPopup source identity does not match the official export.");
            return false;
        }

        for (int32 Index = 0; Index < UE_ARRAY_COUNT(PropertySpecs); ++Index)
        {
            const TSharedPtr<FJsonObject> Property = (*ChildProperties)[Index].IsValid()
                ? (*ChildProperties)[Index]->AsObject()
                : nullptr;
            FString PropertyType;
            FString PropertyName;
            if (!Property.IsValid()
                || !Property->TryGetStringField(TEXT("Type"), PropertyType)
                || !Property->TryGetStringField(TEXT("Name"), PropertyName)
                || PropertyType != PropertySpecs[Index].Type
                || PropertyName != PropertySpecs[Index].Name)
            {
                OutError = FString::Printf(
                    TEXT("tutorialPopup property identity mismatch at index %d."), Index);
                return false;
            }
        }
        return true;
    }

    bool ParseTextSource(
        const TSharedPtr<FJsonObject>& Object,
        const FString& Context,
        FTextSource& OutText,
        FString& OutError)
    {
        OutText = FTextSource();
        if (!Object.IsValid())
        {
            OutError = FString::Printf(TEXT("%s is not an object."), *Context);
            return false;
        }

        const TSharedPtr<FJsonValue>* CultureInvariantValue = Object->Values.Find(TEXT("CultureInvariantString"));
        if (CultureInvariantValue != nullptr && (*CultureInvariantValue).IsValid())
        {
            if ((*CultureInvariantValue)->Type == EJson::Null)
            {
                return true;
            }

            FString CultureInvariantString;
            if ((*CultureInvariantValue)->TryGetString(CultureInvariantString))
            {
                OutText.bEmpty = CultureInvariantString.IsEmpty();
                OutText.SourceString = MoveTemp(CultureInvariantString);
                return true;
            }
        }

        if (!Object->TryGetStringField(TEXT("Namespace"), OutText.Namespace)
            || !Object->TryGetStringField(TEXT("Key"), OutText.Key)
            || !Object->TryGetStringField(TEXT("SourceString"), OutText.SourceString))
        {
            OutError = FString::Printf(TEXT("%s is missing an FText identity."), *Context);
            return false;
        }

        OutText.bEmpty = false;
        return true;
    }

    bool ParseIconReference(
        const TSharedPtr<FJsonValue>* Value,
        const FString& Context,
        FString& OutObjectPath,
        FString& OutError)
    {
        OutObjectPath.Reset();
        if (Value == nullptr || !Value->IsValid())
        {
            OutError = FString::Printf(TEXT("%s is missing."), *Context);
            return false;
        }
        if ((*Value)->Type == EJson::Null)
        {
            return true;
        }

        const TSharedPtr<FJsonObject> Reference = (*Value)->AsObject();
        FString ObjectPath;
        FString ObjectName;
        if (!Reference.IsValid()
            || !Reference->TryGetStringField(TEXT("ObjectPath"), ObjectPath)
            || !Reference->TryGetStringField(TEXT("ObjectName"), ObjectName))
        {
            OutError = FString::Printf(TEXT("%s is not a valid Texture2D reference."), *Context);
            return false;
        }

        if (!ObjectPath.StartsWith(TEXT("kards/Content/"), ESearchCase::IgnoreCase))
        {
            OutError = FString::Printf(TEXT("%s points outside the exported kards Content tree."), *Context);
            return false;
        }

        int32 DotIndex = INDEX_NONE;
        if (!ObjectPath.FindLastChar(TEXT('.'), DotIndex))
        {
            OutError = FString::Printf(TEXT("%s has no export suffix."), *Context);
            return false;
        }
        ObjectPath.LeftInline(DotIndex, EAllowShrinking::No);

        int32 QuoteStart = INDEX_NONE;
        int32 QuoteEnd = INDEX_NONE;
        if (!ObjectName.FindChar(TEXT('\''), QuoteStart)
            || !ObjectName.FindLastChar(TEXT('\''), QuoteEnd)
            || QuoteEnd <= QuoteStart)
        {
            OutError = FString::Printf(TEXT("%s has an invalid object name."), *Context);
            return false;
        }
        const FString AssetName = ObjectName.Mid(QuoteStart + 1, QuoteEnd - QuoteStart - 1);

        ObjectPath.RightChopInline(FCString::Strlen(TEXT("kards/Content/")), EAllowShrinking::No);
        ObjectPath.ReplaceInline(TEXT("\\"), TEXT("/"));
        OutObjectPath = FString::Printf(TEXT("/Game/%s.%s"), *ObjectPath, *AssetName);
        return true;
    }

    bool LoadTableSource(
        const FString& SourcePath,
        TArray<FRow>& OutRows,
        FString& OutError)
    {
        OutRows.Reset();

        TSharedPtr<FJsonObject> Root;
        if (!LoadJsonRoot(SourcePath, Root, OutError))
        {
            return false;
        }

        FString Type;
        FString Name;
        const TSharedPtr<FJsonObject>* Properties = nullptr;
        const TSharedPtr<FJsonObject>* RowsObject = nullptr;
        const TSharedPtr<FJsonObject>* RowStruct = nullptr;
        if (!Root->TryGetStringField(TEXT("Type"), Type)
            || !Root->TryGetStringField(TEXT("Name"), Name)
            || Type != TEXT("DataTable")
            || Name != TableName
            || !Root->TryGetObjectField(TEXT("Properties"), Properties)
            || Properties == nullptr
            || !Properties->IsValid()
            || !(*Properties)->TryGetObjectField(TEXT("RowStruct"), RowStruct)
            || RowStruct == nullptr
            || !RowStruct->IsValid()
            || !Root->TryGetObjectField(TEXT("Rows"), RowsObject)
            || RowsObject == nullptr
            || !RowsObject->IsValid()
            || (*RowsObject)->Values.Num() != ExpectedRowCount)
        {
            OutError = FString::Printf(TEXT("%s does not contain the expected %d-row data table."), TableName, ExpectedRowCount);
            return false;
        }

        FString RowStructName;
        FString RowStructPath;
        if (!(*RowStruct)->TryGetStringField(TEXT("ObjectName"), RowStructName)
            || !(*RowStruct)->TryGetStringField(TEXT("ObjectPath"), RowStructPath)
            || RowStructName != TEXT("UserDefinedStruct'tutorialPopup'")
            || !RowStructPath.StartsWith(TEXT("kards/Content/Structs/tutorialPopup"), ESearchCase::IgnoreCase))
        {
            OutError = TEXT("tutorialPopups RowStruct does not reference tutorialPopup.");
            return false;
        }

        OutRows.Reserve(ExpectedRowCount);
        for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*RowsObject)->Values)
        {
            const TSharedPtr<FJsonObject> RowObject = Pair.Value.IsValid() ? Pair.Value->AsObject() : nullptr;
            if (!RowObject.IsValid())
            {
                OutError = FString::Printf(TEXT("Row '%s' is not an object."), *Pair.Key);
                OutRows.Reset();
                return false;
            }

            FRow Row;
            Row.RowName = FName(*Pair.Key);
            if (!RowObject->TryGetStringField(TriggerPropertyName, Row.Trigger)
                || !RowObject->TryGetStringField(TypePropertyName, Row.Type)
                || !RowObject->TryGetBoolField(HideForISBNPropertyName, Row.bHideForISBN))
            {
                OutError = FString::Printf(TEXT("Row '%s' is missing a required scalar field."), *Pair.Key);
                OutRows.Reset();
                return false;
            }

            const TCHAR* TextNames[] =
            {
                MessagePropertyName,
                ExtraMessagePropertyName,
                CommentPropertyName,
                TitlePropertyName
            };
            FTextSource* TextTargets[] =
            {
                &Row.Message,
                &Row.ExtraMessage,
                &Row.Comment,
                &Row.Title
            };
            for (int32 TextIndex = 0; TextIndex < UE_ARRAY_COUNT(TextNames); ++TextIndex)
            {
                const TSharedPtr<FJsonObject>* TextObject = nullptr;
                if (!RowObject->TryGetObjectField(TextNames[TextIndex], TextObject)
                    || TextObject == nullptr
                    || !TextObject->IsValid()
                    || !ParseTextSource(
                        *TextObject,
                        FString::Printf(TEXT("Row '%s' field '%s'"), *Pair.Key, TextNames[TextIndex]),
                        *TextTargets[TextIndex],
                        OutError))
                {
                    OutRows.Reset();
                    return false;
                }
            }

            const TSharedPtr<FJsonValue>* IconValue = RowObject->Values.Find(IconPropertyName);
            if (!ParseIconReference(
                    IconValue,
                    FString::Printf(TEXT("Row '%s' icon"), *Pair.Key),
                    Row.IconObjectPath,
                    OutError))
            {
                OutRows.Reset();
                return false;
            }

            OutRows.Add(MoveTemp(Row));
        }

        OutRows.Sort([](const FRow& Left, const FRow& Right)
        {
            return Left.RowName.LexicalLess(Right.RowName);
        });
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

    FEdGraphPinType MakePinType(const TCHAR* PropertyType)
    {
        FEdGraphPinType PinType;
        if (FCString::Strcmp(PropertyType, TEXT("StrProperty")) == 0)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_String;
        }
        else if (FCString::Strcmp(PropertyType, TEXT("TextProperty")) == 0)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Text;
        }
        else if (FCString::Strcmp(PropertyType, TEXT("ObjectProperty")) == 0)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
            PinType.PinSubCategoryObject = UTexture2D::StaticClass();
        }
        else if (FCString::Strcmp(PropertyType, TEXT("BoolProperty")) == 0)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
        }
        return PinType;
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
        Variables.Reserve(UE_ARRAY_COUNT(PropertySpecs));

        for (const FPropertySpec& Spec : PropertySpecs)
        {
            FStructVariableDescription Variable;
            Variable.VarName = FName(Spec.Name);
            Variable.FriendlyName = Spec.FriendlyName;
            Variable.VarGuid = Spec.Guid;
            Variable.SetPinType(MakePinType(Spec.Type));
            Variables.Add(MoveTemp(Variable));
        }

        FStructureEditorUtils::CompileStructure(Struct);
        if (Struct->Status != EUserDefinedStructureStatus::UDSS_UpToDate)
        {
            OutError = FString::Printf(TEXT("tutorialPopup compilation failed: %s"), *Struct->ErrorMessage);
            return nullptr;
        }

        Struct->MarkPackageDirty();
        return Struct;
    }

    bool ResolveRowProperties(
        UUserDefinedStruct* Struct,
        FStrProperty*& OutTriggerProperty,
        FStrProperty*& OutTypeProperty,
        FTextProperty*& OutMessageProperty,
        FTextProperty*& OutExtraMessageProperty,
        FTextProperty*& OutCommentProperty,
        FTextProperty*& OutTitleProperty,
        FObjectProperty*& OutIconProperty,
        FBoolProperty*& OutHideForISBNProperty,
        FString& OutError)
    {
        OutTriggerProperty = FindFProperty<FStrProperty>(Struct, TriggerPropertyName);
        OutTypeProperty = FindFProperty<FStrProperty>(Struct, TypePropertyName);
        OutMessageProperty = FindFProperty<FTextProperty>(Struct, MessagePropertyName);
        OutExtraMessageProperty = FindFProperty<FTextProperty>(Struct, ExtraMessagePropertyName);
        OutCommentProperty = FindFProperty<FTextProperty>(Struct, CommentPropertyName);
        OutTitleProperty = FindFProperty<FTextProperty>(Struct, TitlePropertyName);
        OutIconProperty = FindFProperty<FObjectProperty>(Struct, IconPropertyName);
        OutHideForISBNProperty = FindFProperty<FBoolProperty>(Struct, HideForISBNPropertyName);

        if (OutTriggerProperty == nullptr
            || OutTypeProperty == nullptr
            || OutMessageProperty == nullptr
            || OutExtraMessageProperty == nullptr
            || OutCommentProperty == nullptr
            || OutTitleProperty == nullptr
            || OutIconProperty == nullptr
            || OutHideForISBNProperty == nullptr
            || OutIconProperty->PropertyClass != UTexture2D::StaticClass())
        {
            OutError = TEXT("The compiled tutorialPopup structure does not contain the expected field types.");
            return false;
        }
        return true;
    }

    FText MakeText(const FTextSource& Source)
    {
        return Source.bEmpty
            ? FText::GetEmpty()
            : FText::ChangeKey(
                Source.Namespace,
                Source.Key,
                FText::FromString(Source.SourceString));
    }

    UTexture2D* LoadIcon(const FRow& Row, int32& InOutMissingIcons)
    {
        if (Row.IconObjectPath.IsEmpty())
        {
            return nullptr;
        }

        UTexture2D* Icon = LoadObject<UTexture2D>(nullptr, *Row.IconObjectPath, nullptr, LOAD_NoWarn);
        if (Icon == nullptr)
        {
            ++InOutMissingIcons;
            UE_LOG(
                LogTutorialPopupsRebuilder,
                Warning,
                TEXT("Missing icon asset for row %s: %s (the row will keep a null icon)."),
                *Row.RowName.ToString(),
                *Row.IconObjectPath);
        }
        return Icon;
    }

    bool AddTableRow(
        UDataTable* DataTable,
        UUserDefinedStruct* Struct,
        FStrProperty* TriggerProperty,
        FStrProperty* TypeProperty,
        FTextProperty* MessageProperty,
        FTextProperty* ExtraMessageProperty,
        FTextProperty* CommentProperty,
        FTextProperty* TitleProperty,
        FObjectProperty* IconProperty,
        FBoolProperty* HideForISBNProperty,
        const FRow& Row,
        int32& InOutMissingIcons,
        FString& OutError)
    {
        FStructOnScope RowData(Struct);
        uint8* Memory = RowData.GetStructMemory();

        TriggerProperty->SetPropertyValue_InContainer(Memory, Row.Trigger);
        TypeProperty->SetPropertyValue_InContainer(Memory, Row.Type);
        MessageProperty->SetPropertyValue_InContainer(Memory, MakeText(Row.Message));
        ExtraMessageProperty->SetPropertyValue_InContainer(Memory, MakeText(Row.ExtraMessage));
        CommentProperty->SetPropertyValue_InContainer(Memory, MakeText(Row.Comment));
        TitleProperty->SetPropertyValue_InContainer(Memory, MakeText(Row.Title));
        IconProperty->SetObjectPropertyValue_InContainer(Memory, LoadIcon(Row, InOutMissingIcons));
        HideForISBNProperty->SetPropertyValue_InContainer(Memory, Row.bHideForISBN);

        DataTable->AddRow(Row.RowName, Memory, Struct);
        return true;
    }

    UDataTable* BuildDataTable(
        UUserDefinedStruct* Struct,
        const TArray<FRow>& Rows,
        int32& OutMissingIcons,
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
                OutError = TEXT("Unable to create /Game/Structs/tutorialPopups.");
                return nullptr;
            }
            FAssetRegistryModule::AssetCreated(DataTable);
        }
        else if (DataTable->GetRowStruct() != Struct)
        {
            OutError = TEXT("Existing tutorialPopups uses a different row struct.");
            return nullptr;
        }

        FStrProperty* TriggerProperty = nullptr;
        FStrProperty* TypeProperty = nullptr;
        FTextProperty* MessageProperty = nullptr;
        FTextProperty* ExtraMessageProperty = nullptr;
        FTextProperty* CommentProperty = nullptr;
        FTextProperty* TitleProperty = nullptr;
        FObjectProperty* IconProperty = nullptr;
        FBoolProperty* HideForISBNProperty = nullptr;
        if (!ResolveRowProperties(
                Struct,
                TriggerProperty,
                TypeProperty,
                MessageProperty,
                ExtraMessageProperty,
                CommentProperty,
                TitleProperty,
                IconProperty,
                HideForISBNProperty,
                OutError))
        {
            return nullptr;
        }

        DataTable->EmptyTable();
        OutMissingIcons = 0;
        for (const FRow& Row : Rows)
        {
            if (!AddTableRow(
                    DataTable,
                    Struct,
                    TriggerProperty,
                    TypeProperty,
                    MessageProperty,
                    ExtraMessageProperty,
                    CommentProperty,
                    TitleProperty,
                    IconProperty,
                    HideForISBNProperty,
                    Row,
                    OutMissingIcons,
                    OutError))
            {
                return nullptr;
            }
        }

        DataTable->MarkPackageDirty();
        return DataTable;
    }

    bool VerifyText(
        const FText& Actual,
        const FTextSource& Expected,
        const FString& Context,
        FString& OutError)
    {
        if (Expected.bEmpty)
        {
            if (!Actual.IsEmpty())
            {
                OutError = FString::Printf(TEXT("%s should be empty."), *Context);
                return false;
            }
            return true;
        }

        const TOptional<FString> Namespace = FTextInspector::GetNamespace(Actual);
        const TOptional<FString> Key = FTextInspector::GetKey(Actual);
        const FString* SourceString = FTextInspector::GetSourceString(Actual);
        if (!Namespace.IsSet()
            || Namespace.GetValue() != Expected.Namespace
            || !Key.IsSet()
            || Key.GetValue() != Expected.Key
            || SourceString == nullptr
            || *SourceString != Expected.SourceString)
        {
            OutError = FString::Printf(
                TEXT("%s FText identity mismatch (found namespace='%s', key='%s', source='%s')."),
                *Context,
                Namespace.IsSet() ? *Namespace.GetValue() : TEXT("<none>"),
                Key.IsSet() ? *Key.GetValue() : TEXT("<none>"),
                SourceString != nullptr ? **SourceString : TEXT("<none>"));
            return false;
        }
        return true;
    }

    bool VerifyAssets(
        UUserDefinedStruct* Struct,
        UDataTable* DataTable,
        const TArray<FRow>& Rows,
        FString& OutError)
    {
        if (Struct == nullptr || DataTable == nullptr)
        {
            OutError = TEXT("Rebuilt tutorialPopup structure or tutorialPopups table is null.");
            return false;
        }
        if (Struct->GetPathName() != StructObjectPath || Struct->Guid != StructGuid)
        {
            OutError = TEXT("tutorialPopup path or struct GUID mismatch.");
            return false;
        }

        const TArray<FStructVariableDescription>& Variables = FStructureEditorUtils::GetVarDesc(Struct);
        if (Variables.Num() != UE_ARRAY_COUNT(PropertySpecs))
        {
            OutError = FString::Printf(
                TEXT("tutorialPopup contains %d variables instead of %d."),
                Variables.Num(),
                UE_ARRAY_COUNT(PropertySpecs));
            return false;
        }

        for (int32 Index = 0; Index < UE_ARRAY_COUNT(PropertySpecs); ++Index)
        {
            const FStructVariableDescription& Variable = Variables[Index];
            if (Variable.VarName != FName(PropertySpecs[Index].Name)
                || Variable.VarGuid != PropertySpecs[Index].Guid
                || Variable.FriendlyName != PropertySpecs[Index].FriendlyName
                || Variable.Category != MakePinType(PropertySpecs[Index].Type).PinCategory)
            {
                OutError = FString::Printf(
                    TEXT("tutorialPopup variable identity mismatch at index %d."), Index);
                return false;
            }

            const FEdGraphPinType ExpectedPinType = MakePinType(PropertySpecs[Index].Type);
            if (PropertySpecs[Index].Type == FString(TEXT("ObjectProperty"))
                && Variable.SubCategoryObject.Get() != ExpectedPinType.PinSubCategoryObject)
            {
                OutError = TEXT("tutorialPopup icon field is not backed by Texture2D.");
                return false;
            }
        }

        int32 PropertyIndex = 0;
        for (TFieldIterator<FProperty> It(Struct, EFieldIterationFlags::None); It; ++It)
        {
            if (PropertyIndex >= UE_ARRAY_COUNT(PropertySpecs)
                || It->GetName() != PropertySpecs[PropertyIndex].Name)
            {
                OutError = FString::Printf(
                    TEXT("tutorialPopup generated property order mismatch at index %d."),
                    PropertyIndex);
                return false;
            }
            ++PropertyIndex;
        }
        if (PropertyIndex != UE_ARRAY_COUNT(PropertySpecs))
        {
            OutError = FString::Printf(
                TEXT("tutorialPopup generated %d properties instead of %d."),
                PropertyIndex,
                UE_ARRAY_COUNT(PropertySpecs));
            return false;
        }

        if (DataTable->GetPathName() != TableObjectPath
            || DataTable->GetRowStruct() != Struct
            || DataTable->GetRowMap().Num() != Rows.Num())
        {
            OutError = FString::Printf(
                TEXT("tutorialPopups path, row struct, or row count mismatch (found %d rows)."),
                DataTable->GetRowMap().Num());
            return false;
        }

        FStrProperty* TriggerProperty = nullptr;
        FStrProperty* TypeProperty = nullptr;
        FTextProperty* MessageProperty = nullptr;
        FTextProperty* ExtraMessageProperty = nullptr;
        FTextProperty* CommentProperty = nullptr;
        FTextProperty* TitleProperty = nullptr;
        FObjectProperty* IconProperty = nullptr;
        FBoolProperty* HideForISBNProperty = nullptr;
        if (!ResolveRowProperties(
                Struct,
                TriggerProperty,
                TypeProperty,
                MessageProperty,
                ExtraMessageProperty,
                CommentProperty,
                TitleProperty,
                IconProperty,
                HideForISBNProperty,
                OutError))
        {
            return false;
        }

        for (const FRow& ExpectedRow : Rows)
        {
            const uint8* RowMemory = DataTable->FindRowUnchecked(ExpectedRow.RowName);
            if (RowMemory == nullptr)
            {
                OutError = FString::Printf(
                    TEXT("tutorialPopups is missing row '%s'."), *ExpectedRow.RowName.ToString());
                return false;
            }

            if (TriggerProperty->GetPropertyValue_InContainer(RowMemory) != ExpectedRow.Trigger
                || TypeProperty->GetPropertyValue_InContainer(RowMemory) != ExpectedRow.Type
                || HideForISBNProperty->GetPropertyValue_InContainer(RowMemory) != ExpectedRow.bHideForISBN)
            {
                OutError = FString::Printf(
                    TEXT("Scalar value mismatch for tutorialPopups row '%s'."),
                    *ExpectedRow.RowName.ToString());
                return false;
            }

            const FString RowContext = ExpectedRow.RowName.ToString();
            if (!VerifyText(
                    MessageProperty->GetPropertyValue_InContainer(RowMemory),
                    ExpectedRow.Message,
                    RowContext + TEXT(".message"),
                    OutError)
                || !VerifyText(
                    ExtraMessageProperty->GetPropertyValue_InContainer(RowMemory),
                    ExpectedRow.ExtraMessage,
                    RowContext + TEXT(".extraMsg"),
                    OutError)
                || !VerifyText(
                    CommentProperty->GetPropertyValue_InContainer(RowMemory),
                    ExpectedRow.Comment,
                    RowContext + TEXT(".comment"),
                    OutError)
                || !VerifyText(
                    TitleProperty->GetPropertyValue_InContainer(RowMemory),
                    ExpectedRow.Title,
                    RowContext + TEXT(".title"),
                    OutError))
            {
                return false;
            }

            UObject* ActualIcon = IconProperty->GetObjectPropertyValue_InContainer(RowMemory);
            if (!ExpectedRow.IconObjectPath.IsEmpty())
            {
                if (UTexture2D* ExpectedIcon = LoadObject<UTexture2D>(
                        nullptr,
                        *ExpectedRow.IconObjectPath,
                        nullptr,
                        LOAD_NoWarn))
                {
                    if (ActualIcon != ExpectedIcon)
                    {
                        OutError = FString::Printf(
                            TEXT("Icon reference mismatch for tutorialPopups row '%s'."),
                            *ExpectedRow.RowName.ToString());
                        return false;
                    }
                }
                else if (ActualIcon != nullptr)
                {
                    OutError = FString::Printf(
                        TEXT("Missing source icon '%s' but row '%s' contains an unexpected object."),
                        *ExpectedRow.IconObjectPath,
                        *ExpectedRow.RowName.ToString());
                    return false;
                }
            }
            else if (ActualIcon != nullptr)
            {
                OutError = FString::Printf(
                    TEXT("tutorialPopups row '%s' should not contain an icon."),
                    *ExpectedRow.RowName.ToString());
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
            OutError = TEXT("Cannot save a null tutorial popup asset package.");
            return false;
        }

        const FString Filename = FPackageName::LongPackageNameToFilename(
            Package->GetName(), FPackageName::GetAssetPackageExtension());
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);

        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
        SaveArgs.SaveFlags = SAVE_NoError;
        SaveArgs.bSlowTask = false;
        SaveArgs.Error = GError;
        if (!UPackage::SavePackage(Package, Asset, *Filename, SaveArgs))
        {
            OutError = FString::Printf(TEXT("Unable to save %s."), *Filename);
            return false;
        }
        return true;
    }
}

URebuildTutorialPopupsCommandlet::URebuildTutorialPopupsCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
    ShowErrorCount = true;
}

int32 URebuildTutorialPopupsCommandlet::Main(const FString& Params)
{
    using namespace TutorialPopupsRebuilder;

    FString SourceRoot;
    if (!FParse::Value(*Params, TEXT("SourceRoot="), SourceRoot))
    {
        SourceRoot = DefaultSourceRoot;
    }
    FPaths::NormalizeDirectoryName(SourceRoot);

    FString Error;
    TArray<FRow> Rows;
    if (!ValidateStructSource(StructSourcePath(SourceRoot), Error)
        || !LoadTableSource(TableSourcePath(SourceRoot), Rows, Error))
    {
        UE_LOG(LogTutorialPopupsRebuilder, Error, TEXT("Source validation failed: %s"), *Error);
        return 1;
    }

    if (FParse::Param(*Params, TEXT("VerifyOnly")))
    {
        UUserDefinedStruct* Struct = LoadObject<UUserDefinedStruct>(nullptr, StructObjectPath, nullptr, LOAD_NoWarn);
        UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TableObjectPath, nullptr, LOAD_NoWarn);
        if (!VerifyAssets(Struct, DataTable, Rows, Error))
        {
            UE_LOG(LogTutorialPopupsRebuilder, Error, TEXT("Reload verification failed: %s"), *Error);
            return 6;
        }

        UE_LOG(
            LogTutorialPopupsRebuilder,
            Display,
            TEXT("RELOAD VERIFIED: %s; %s; rows=%d; official FText identities preserved."),
            StructObjectPath,
            TableObjectPath,
            Rows.Num());
        return 0;
    }

    UUserDefinedStruct* Struct = BuildStruct(Error);
    if (Struct == nullptr)
    {
        UE_LOG(LogTutorialPopupsRebuilder, Error, TEXT("Structure reconstruction failed: %s"), *Error);
        return 2;
    }

    int32 MissingIcons = 0;
    UDataTable* DataTable = BuildDataTable(Struct, Rows, MissingIcons, Error);
    if (DataTable == nullptr)
    {
        UE_LOG(LogTutorialPopupsRebuilder, Error, TEXT("Data-table reconstruction failed: %s"), *Error);
        return 3;
    }

    if (!VerifyAssets(Struct, DataTable, Rows, Error))
    {
        UE_LOG(LogTutorialPopupsRebuilder, Error, TEXT("In-memory verification failed: %s"), *Error);
        return 4;
    }

    if (!SaveAsset(Struct, Error) || !SaveAsset(DataTable, Error))
    {
        UE_LOG(LogTutorialPopupsRebuilder, Error, TEXT("Package save failed: %s"), *Error);
        return 5;
    }

    UE_LOG(
        LogTutorialPopupsRebuilder,
        Display,
        TEXT("VERIFIED: %s; %s; rows=%d; missing icon assets=%d; official FText identities preserved."),
        StructObjectPath,
        TableObjectPath,
        Rows.Num(),
        MissingIcons);
    return 0;
}
