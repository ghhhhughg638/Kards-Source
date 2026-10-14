#include "RestoreBPTableSchemaAssetsCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/FileManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Dom/JsonObject.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/DataTable.h"
#include "Engine/EngineTypes.h"
#include "Engine/UserDefinedEnum.h"
#include "Factories/DataTableFactory.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "Kismet2/EnumEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "StructUtils/UserDefinedStruct.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/StructOnScope.h"
#include "UObject/UnrealType.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogBPTableSchemaAssets, Log, All);

namespace BPTableSchemaAssets
{
	constexpr int32 ExpectedBoardTableRows = 28;
	constexpr TCHAR BoardMatField[] = TEXT("boardMat_3_EF923F9C42CFB14AA53F6B8B42864CE6");
	constexpr TCHAR ComponentField[] = TEXT("Component_2_D7E02C8D47B46BEDCE085E92EC25B8BE");
	constexpr TCHAR SimplifiedMaterialField[] = TEXT("SimplifiedMaterial_12_BD0F733D417A1F675D228789132AAD10");
	constexpr TCHAR MaterialField[] = TEXT("Material_11_946C9BAB41F4597B67FC1D9CCFE188A1");
	constexpr TCHAR MaterialSlotField[] = TEXT("MaterialSlot_10_035D12E34D2745B7BF0FCBAF9C89C28D");

	struct FSourceExport
	{
		FString Type;
		FString Name;
		FString Package;
		TSharedPtr<FJsonObject> Object;
	};

	struct FSourceFile
	{
		FString RelativePath;
		FString FullPath;
		TArray<FSourceExport> Exports;
		TMap<FString, int32> ExportIndexByName;
	};

	struct FEnumEntry
	{
		FString InternalName;
		int64 Value = INDEX_NONE;
		FText DisplayName;
	};

	struct FEnumDefinition
	{
		FString AssetName;
		FString PackageName;
		FString SourcePath;
		int64 MaxValue = INDEX_NONE;
		int32 UniqueNameIndex = 0;
		TArray<FEnumEntry> Entries;
	};

	struct FStructField
	{
		FString InternalName;
		FString FriendlyName;
		FGuid Guid;
		FEdGraphPinType PinType;
		EPropertyFlags PropertyFlags = CPF_None;
	};

	struct FStructDefinition
	{
		FString AssetName;
		FString PackageName;
		FGuid Guid;
		TArray<FStructField> Fields;
	};

	struct FBoardRow
	{
		FName Name;
		FSoftObjectPath MaterialPath;
	};

	struct FBoardTableDefinition
	{
		FString AssetName;
		FString PackageName;
		// Omitted DataTable import-option properties use UDataTable's native false defaults.
		bool bIgnoreExtraFields = false;
		bool bIgnoreMissingFields = false;
		TArray<FBoardRow> Rows;
	};

	struct FBlueprintSignatureDefinition
	{
		FString AssetName;
		FString PackageName;
		UClass* ParentClass = nullptr;
	};

	struct FPreparedSources
	{
		FSourceFile MaterialSwitcherFile;
		FSourceFile LevelManagerFile;
		FEnumDefinition PlatformEnum;
		FEnumDefinition BackgroundEnum;
		FStructDefinition BoardMatStruct;
		FStructDefinition MaterialSwapStruct;
		FBoardTableDefinition DesktopBoardTable;
		FBoardTableDefinition MobileBoardTable;
		FBlueprintSignatureDefinition MaterialSwitcher;
		FBlueprintSignatureDefinition LevelManager;
	};

	struct FBuiltAssets
	{
		UUserDefinedEnum* PlatformEnum = nullptr;
		UUserDefinedEnum* BackgroundEnum = nullptr;
		UUserDefinedStruct* BoardMatStruct = nullptr;
		UUserDefinedStruct* MaterialSwapStruct = nullptr;
		UDataTable* DesktopBoardTable = nullptr;
		UDataTable* MobileBoardTable = nullptr;
		UBlueprint* MaterialSwitcher = nullptr;
		UBlueprint* LevelManager = nullptr;
		TArray<UObject*> SaveOrder;
	};

	bool TryGetRequiredString(
		const TSharedPtr<FJsonObject>& Object,
		const TCHAR* FieldName,
		FString& OutValue,
		FString& OutError)
	{
		if (!Object.IsValid() || !Object->TryGetStringField(FieldName, OutValue) || OutValue.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Missing or invalid JSON string field '%s'."), FieldName);
			return false;
		}
		return true;
	}

	TSharedPtr<FJsonObject> GetObjectField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		const TSharedPtr<FJsonObject>* Value = nullptr;
		return Object.IsValid() && Object->TryGetObjectField(FieldName, Value) && Value != nullptr
			? *Value
			: nullptr;
	}

	bool HasPropertyFlag(const TSharedPtr<FJsonObject>& Property, const TCHAR* FlagName)
	{
		FString Flags;
		return Property.IsValid()
			&& Property->TryGetStringField(TEXT("PropertyFlags"), Flags)
			&& Flags.Contains(FlagName, ESearchCase::CaseSensitive);
	}

	bool TryParseInteger(const TSharedPtr<FJsonValue>& Value, int64& OutValue)
	{
		if (!Value.IsValid() || Value->Type != EJson::Number)
		{
			return false;
		}
		const double Number = Value->AsNumber();
		if (!FMath::IsFinite(Number)
			|| FMath::FloorToDouble(Number) != Number
			|| Number < static_cast<double>(MIN_int64)
			|| Number > static_cast<double>(MAX_int64))
		{
			return false;
		}
		OutValue = static_cast<int64>(Number);
		return true;
	}

	bool ParseGuidDigits(const FString& GuidText, FGuid& OutGuid)
	{
		FString Digits = GuidText;
		Digits.ReplaceInline(TEXT("-"), TEXT(""));
		return Digits.Len() == 32 && FGuid::ParseExact(Digits, EGuidFormats::Digits, OutGuid);
	}

	bool BuildDisplayText(const TSharedPtr<FJsonObject>& Value, FText& OutText, FString& OutError)
	{
		if (!Value.IsValid())
		{
			OutError = TEXT("Enum display-name entry is not an object.");
			return false;
		}

		FString CultureInvariantString;
		if (Value->TryGetStringField(TEXT("CultureInvariantString"), CultureInvariantString))
		{
			OutText = FText::AsCultureInvariant(CultureInvariantString);
			return true;
		}

		FString Namespace;
		FString Key;
		FString SourceString;
		if (Value->TryGetStringField(TEXT("Namespace"), Namespace)
			&& Value->TryGetStringField(TEXT("Key"), Key)
			&& Value->TryGetStringField(TEXT("SourceString"), SourceString))
		{
			OutText = FText::ChangeKey(Namespace, Key, FText::FromString(SourceString));
			return true;
		}

		FString LocalizedString;
		if (Value->TryGetStringField(TEXT("LocalizedString"), LocalizedString))
		{
			OutText = FText::FromString(LocalizedString);
			return true;
		}

		OutError = TEXT("Enum display-name entry has no recognized FText representation.");
		return false;
	}

	bool LoadSourceFile(
		const FString& SourceRoot,
		const FString& RelativePath,
		const int32 ExpectedExportCount,
		FSourceFile& OutFile,
		FString& OutError)
	{
		OutFile = FSourceFile();
		OutFile.RelativePath = RelativePath;
		OutFile.FullPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(SourceRoot, RelativePath));

		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *OutFile.FullPath))
		{
			OutError = FString::Printf(TEXT("Cannot read required source JSON: %s."), *OutFile.FullPath);
			return false;
		}

		TArray<TSharedPtr<FJsonValue>> RootValues;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, RootValues)
			|| RootValues.Num() != ExpectedExportCount)
		{
			OutError = FString::Printf(
				TEXT("%s must be a JSON export array with exactly %d entries."),
				*OutFile.FullPath,
				ExpectedExportCount);
			return false;
		}

		TSet<FString> SeenNames;
		OutFile.Exports.Reserve(RootValues.Num());
		for (const TSharedPtr<FJsonValue>& RootValue : RootValues)
		{
			if (!RootValue.IsValid() || RootValue->Type != EJson::Object)
			{
				OutError = FString::Printf(TEXT("%s contains a non-object export."), *OutFile.FullPath);
				return false;
			}

			FSourceExport Export;
			Export.Object = RootValue->AsObject();
			if (!TryGetRequiredString(Export.Object, TEXT("Type"), Export.Type, OutError)
				|| !TryGetRequiredString(Export.Object, TEXT("Name"), Export.Name, OutError))
			{
				OutError = FString::Printf(TEXT("%s: %s"), *OutFile.FullPath, *OutError);
				return false;
			}
		// FModel includes Package on the class/type root export, but often
		// omits it from child Function, SCS, and component exports. Package
		// identity is validated only for the selected primary export below.
		Export.Object->TryGetStringField(TEXT("Package"), Export.Package);
			if (SeenNames.Contains(Export.Name))
			{
				OutError = FString::Printf(TEXT("%s contains duplicate export name '%s'."), *OutFile.FullPath, *Export.Name);
				return false;
			}
			SeenNames.Add(Export.Name);
			const FString ExportName = Export.Name;
			const int32 ExportIndex = OutFile.Exports.Add(MoveTemp(Export));
			OutFile.ExportIndexByName.Add(ExportName, ExportIndex);
		}
		return true;
	}

	const FSourceExport* FindExport(const FSourceFile& File, const TCHAR* Name)
	{
		const int32* Index = File.ExportIndexByName.Find(Name);
		return Index != nullptr && File.Exports.IsValidIndex(*Index) ? &File.Exports[*Index] : nullptr;
	}

	bool ValidateExportIdentity(
		const FSourceExport* Export,
		const TCHAR* ExpectedType,
		const TCHAR* ExpectedName,
		const TCHAR* ExpectedPackage,
		FString& OutError)
	{
		if (Export == nullptr
			|| Export->Type != ExpectedType
			|| Export->Name != ExpectedName
			|| Export->Package != ExpectedPackage)
		{
			OutError = FString::Printf(
				TEXT("Export identity mismatch; expected %s %s in %s."),
				ExpectedType,
				ExpectedName,
				ExpectedPackage);
			return false;
		}
		return true;
	}

	bool LoadUserDefinedEnum(
		const FSourceFile& File,
		const TCHAR* AssetName,
		const TCHAR* PackageName,
		const int32 ExpectedEntryCount,
		FEnumDefinition& OutDefinition,
		FString& OutError)
	{
		const FSourceExport* Export = FindExport(File, AssetName);
		const FString ExpectedPackage = FString::Printf(TEXT("kards/Content/Structs/%s"), AssetName);
		if (!ValidateExportIdentity(Export, TEXT("UserDefinedEnum"), AssetName, *ExpectedPackage, OutError))
		{
			return false;
		}

		const TSharedPtr<FJsonObject> Names = GetObjectField(Export->Object, TEXT("Names"));
		const TSharedPtr<FJsonObject> Properties = GetObjectField(Export->Object, TEXT("Properties"));
		const TArray<TSharedPtr<FJsonValue>>* DisplayNameMap = nullptr;
		if (!Names.IsValid()
			|| !Properties.IsValid()
			|| !Properties->TryGetArrayField(TEXT("DisplayNameMap"), DisplayNameMap)
			|| DisplayNameMap == nullptr
			|| DisplayNameMap->Num() != ExpectedEntryCount)
		{
			OutError = FString::Printf(TEXT("%s enum is missing its exact Names/DisplayNameMap arrays."), AssetName);
			return false;
		}

		const FString Prefix = FString(AssetName) + TEXT("::");
		TMap<FString, int64> ValuesByName;
		bool bSawMax = false;
		int64 MaxValue = INDEX_NONE;
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Names->Values)
		{
			if (!Pair.Key.StartsWith(Prefix) || !Pair.Value.IsValid())
			{
				OutError = FString::Printf(TEXT("%s contains an unexpected enum key '%s'."), AssetName, *Pair.Key);
				return false;
			}
			int64 Value = INDEX_NONE;
			if (!TryParseInteger(Pair.Value, Value))
			{
				OutError = FString::Printf(TEXT("%s contains a non-integral enum value for '%s'."), AssetName, *Pair.Key);
				return false;
			}
			const FString InternalName = Pair.Key.RightChop(Prefix.Len());
			if (InternalName.Contains(TEXT("MAX")))
			{
				if (bSawMax)
				{
					OutError = FString::Printf(TEXT("%s has multiple MAX enum entries."), AssetName);
					return false;
				}
				bSawMax = true;
				MaxValue = Value;
				continue;
			}
			ValuesByName.Add(InternalName, Value);
		}

		if (!bSawMax || ValuesByName.Num() != ExpectedEntryCount || MaxValue != ExpectedEntryCount)
		{
			OutError = FString::Printf(
				TEXT("%s must have %d named entries and a MAX sentinel equal to %d."),
				AssetName,
				ExpectedEntryCount,
				ExpectedEntryCount);
			return false;
		}

		TMap<FString, FText> DisplayByName;
		for (const TSharedPtr<FJsonValue>& DisplayValue : *DisplayNameMap)
		{
			const TSharedPtr<FJsonObject> DisplayEntry = DisplayValue.IsValid() ? DisplayValue->AsObject() : nullptr;
			FString Key;
			const TSharedPtr<FJsonObject> TextObject = GetObjectField(DisplayEntry, TEXT("Value"));
			FText DisplayText;
			if (!DisplayEntry.IsValid()
				|| !TryGetRequiredString(DisplayEntry, TEXT("Key"), Key, OutError)
				|| !ValuesByName.Contains(Key)
				|| !BuildDisplayText(TextObject, DisplayText, OutError)
				|| DisplayByName.Contains(Key))
			{
				if (OutError.IsEmpty())
				{
					OutError = FString::Printf(TEXT("%s has an inconsistent display-name map entry '%s'."), AssetName, *Key);
				}
				return false;
			}
			DisplayByName.Add(Key, DisplayText);
		}
		if (DisplayByName.Num() != ExpectedEntryCount)
		{
			OutError = FString::Printf(TEXT("%s display-name map does not cover every enum value."), AssetName);
			return false;
		}

		OutDefinition = FEnumDefinition();
		OutDefinition.AssetName = AssetName;
		OutDefinition.PackageName = FString::Printf(TEXT("/Game/Structs/%s"), AssetName);
		OutDefinition.SourcePath = File.FullPath;
		OutDefinition.MaxValue = MaxValue;
		for (const TPair<FString, int64>& Pair : ValuesByName)
		{
			if (Pair.Value < 0 || Pair.Value >= ExpectedEntryCount)
			{
				OutError = FString::Printf(TEXT("%s has out-of-range enum value %lld."), AssetName, Pair.Value);
				return false;
			}
			FEnumEntry& Entry = OutDefinition.Entries.AddDefaulted_GetRef();
			Entry.InternalName = Pair.Key;
			Entry.Value = Pair.Value;
			Entry.DisplayName = DisplayByName.FindRef(Pair.Key);
		}
		OutDefinition.Entries.Sort([](const FEnumEntry& A, const FEnumEntry& B)
		{
			return A.Value < B.Value;
		});
		for (int32 Index = 0; Index < OutDefinition.Entries.Num(); ++Index)
		{
			if (OutDefinition.Entries[Index].Value != Index)
			{
				OutError = FString::Printf(TEXT("%s enum values are not a contiguous 0..%d map."), AssetName, ExpectedEntryCount - 1);
				return false;
			}
		}

		int32 LargestGeneratedName = INDEX_NONE;
		for (const FEnumEntry& Entry : OutDefinition.Entries)
		{
			if (Entry.InternalName.StartsWith(TEXT("NewEnumerator")))
			{
				const FString Suffix = Entry.InternalName.RightChop(FCString::Strlen(TEXT("NewEnumerator")));
				if (Suffix.IsNumeric())					LargestGeneratedName = FMath::Max(LargestGeneratedName, FCString::Atoi(*Suffix));
			}
		}
		OutDefinition.UniqueNameIndex = LargestGeneratedName + 1;
		return true;
	}

	bool ResolveStructFieldType(
		const TSharedPtr<FJsonObject>& Property,
		FEdGraphPinType& OutPinType,
		EPropertyFlags& OutFlags,
		FString& OutError)
	{
		FString TypeName;
		if (!TryGetRequiredString(Property, TEXT("Type"), TypeName, OutError))
		{
			return false;
		}
		OutPinType = FEdGraphPinType();
		if (TypeName == TEXT("SoftObjectProperty"))
		{
			const TSharedPtr<FJsonObject> ClassRef = GetObjectField(Property, TEXT("PropertyClass"));
			FString ObjectName;
			FString ObjectPath;
			if (!TryGetRequiredString(ClassRef, TEXT("ObjectName"), ObjectName, OutError)
				|| !TryGetRequiredString(ClassRef, TEXT("ObjectPath"), ObjectPath, OutError)
				|| ObjectName != TEXT("Class'MaterialInstance'")
				|| ObjectPath != TEXT("/Script/Engine"))
			{
				OutError = TEXT("boardMat soft reference is not exactly MaterialInstance from /Script/Engine.");
				return false;
			}
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_SoftObject;
			OutPinType.PinSubCategoryObject = UMaterialInstance::StaticClass();
		}
		else if (TypeName == TEXT("ObjectProperty"))
		{
			const TSharedPtr<FJsonObject> ClassRef = GetObjectField(Property, TEXT("PropertyClass"));
			FString ObjectName;
			FString ObjectPath;
			if (!TryGetRequiredString(ClassRef, TEXT("ObjectName"), ObjectName, OutError)
				|| !TryGetRequiredString(ClassRef, TEXT("ObjectPath"), ObjectPath, OutError)
				|| ObjectPath != TEXT("/Script/Engine"))
			{
				OutError = TEXT("S_MaterialSwapItem object field has an invalid native class reference.");
				return false;
			}
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
			if (ObjectName == TEXT("Class'PrimitiveComponent'"))
			{
				OutPinType.PinSubCategoryObject = UPrimitiveComponent::StaticClass();
			}
			else if (ObjectName == TEXT("Class'MaterialInterface'"))
			{
				OutPinType.PinSubCategoryObject = UMaterialInterface::StaticClass();
			}
			else
			{
				OutError = FString::Printf(TEXT("Unsupported exact native object field class '%s'."), *ObjectName);
				return false;
			}
		}
		else if (TypeName == TEXT("IntProperty"))
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Int;
		}
		else
		{
			OutError = FString::Printf(TEXT("Unsupported structure field type '%s'."), *TypeName);
			return false;
		}

		OutFlags = CPF_None;
		if (HasPropertyFlag(Property, TEXT("Edit"))) OutFlags |= CPF_Edit;
		if (HasPropertyFlag(Property, TEXT("BlueprintVisible"))) OutFlags |= CPF_BlueprintVisible;
		if (HasPropertyFlag(Property, TEXT("BlueprintReadOnly"))) OutFlags |= CPF_BlueprintReadOnly;
		if (HasPropertyFlag(Property, TEXT("InstancedReference"))) OutFlags |= CPF_InstancedReference;
		if (!EnumHasAnyFlags(OutFlags, CPF_BlueprintVisible))
		{
			OutError = TEXT("All BP_Table schema struct fields must remain BlueprintVisible.");
			return false;
		}
		return true;
	}

	bool ParseStructField(
		const TSharedPtr<FJsonObject>& Property,
		const TCHAR* ExpectedInternalName,
		const TCHAR* ExpectedType,
		FStructField& OutField,
		FString& OutError)
	{
		FString TypeName;
		if (!TryGetRequiredString(Property, TEXT("Name"), OutField.InternalName, OutError)
			|| !TryGetRequiredString(Property, TEXT("Type"), TypeName, OutError))
		{
			return false;
		}
		if (OutField.InternalName != ExpectedInternalName || TypeName != ExpectedType)
		{
			OutError = FString::Printf(
				TEXT("Unexpected struct field; expected %s of type %s."),
				ExpectedInternalName,
				ExpectedType);
			return false;
		}

		int32 GuidSeparator = INDEX_NONE;
		if (!OutField.InternalName.FindLastChar(TEXT('_'), GuidSeparator) || GuidSeparator == INDEX_NONE)
		{
			OutError = FString::Printf(TEXT("Struct field %s does not carry its serialized variable GUID."), *OutField.InternalName);
			return false;
		}
		const FString GuidDigits = OutField.InternalName.Mid(GuidSeparator + 1);
		if (!ParseGuidDigits(GuidDigits, OutField.Guid))
		{
			OutError = FString::Printf(TEXT("Struct field %s has an invalid serialized variable GUID."), *OutField.InternalName);
			return false;
		}

		const int32 IndexSeparator = OutField.InternalName.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, GuidSeparator - 1);
		if (IndexSeparator == INDEX_NONE)
		{
			OutError = FString::Printf(TEXT("Struct field %s has no serialized field index."), *OutField.InternalName);
			return false;
		}
		const FString Friendly = OutField.InternalName.Left(IndexSeparator);
		const FString IndexString = OutField.InternalName.Mid(IndexSeparator + 1, GuidSeparator - IndexSeparator - 1);
		if (Friendly.IsEmpty() || !IndexString.IsNumeric())
		{
			OutError = FString::Printf(TEXT("Struct field %s has an invalid name/index prefix."), *OutField.InternalName);
			return false;
		}
		OutField.FriendlyName = Friendly;
		return ResolveStructFieldType(Property, OutField.PinType, OutField.PropertyFlags, OutError);
	}

	bool LoadStructDefinition(
		const FSourceFile& File,
		const TCHAR* AssetName,
		const TCHAR* PackageName,
		const TArray<TPair<FString, FString>>& ExpectedFields,
		FStructDefinition& OutDefinition,
		FString& OutError)
	{
		const FSourceExport* Export = FindExport(File, AssetName);
		const FString ExpectedSourcePackage = FString::Printf(TEXT("kards/Content/Structs/%s"), AssetName);
		if (!ValidateExportIdentity(Export, TEXT("UserDefinedStruct"), AssetName, *ExpectedSourcePackage, OutError))
		{
			return false;
		}

		const TSharedPtr<FJsonObject> Properties = GetObjectField(Export->Object, TEXT("Properties"));
		const TArray<TSharedPtr<FJsonValue>>* ChildProperties = nullptr;
		FString GuidText;
		if (!Properties.IsValid()
			|| !Properties->TryGetStringField(TEXT("Guid"), GuidText)
			|| !Export->Object->TryGetArrayField(TEXT("ChildProperties"), ChildProperties)
			|| ChildProperties == nullptr
			|| ChildProperties->Num() != ExpectedFields.Num()
			|| !ParseGuidDigits(GuidText, OutDefinition.Guid))
		{
			OutError = FString::Printf(TEXT("%s struct identity/field list/GUID does not match its expected schema."), AssetName);
			return false;
		}

		OutDefinition.AssetName = AssetName;
		OutDefinition.PackageName = PackageName;
		OutDefinition.Fields.Reset();
		for (int32 Index = 0; Index < ExpectedFields.Num(); ++Index)
		{
			const TSharedPtr<FJsonObject> Property = (*ChildProperties)[Index].IsValid()
				? (*ChildProperties)[Index]->AsObject()
				: nullptr;
			FStructField Field;
			if (!Property.IsValid()
				|| !ParseStructField(
					Property,
					*ExpectedFields[Index].Key,
					*ExpectedFields[Index].Value,
					Field,
					OutError))
			{
				OutError = FString::Printf(TEXT("%s: %s"), AssetName, *OutError);
				return false;
			}
			OutDefinition.Fields.Add(MoveTemp(Field));
		}
		return true;
	}

	bool LoadBoardTableDefinition(
		const FSourceFile& File,
		const TCHAR* AssetName,
		const TCHAR* ExpectedRowStructPath,
		FBoardTableDefinition& OutDefinition,
		FString& OutError)
	{
		const FSourceExport* Export = FindExport(File, AssetName);
		const FString ExpectedPackage = FString::Printf(TEXT("kards/Content/Structs/%s"), AssetName);
		if (!ValidateExportIdentity(Export, TEXT("DataTable"), AssetName, *ExpectedPackage, OutError))
		{
			return false;
		}
		const TSharedPtr<FJsonObject> Properties = GetObjectField(Export->Object, TEXT("Properties"));
		const TSharedPtr<FJsonObject> RowStruct = GetObjectField(Properties, TEXT("RowStruct"));
		FString RowStructPath;
		bool bIgnoreExtraFields = false;
		bool bIgnoreMissingFields = false;
		const TSharedPtr<FJsonObject> Rows = GetObjectField(Export->Object, TEXT("Rows"));
		if (!Properties.IsValid())
		{
			OutError = FString::Printf(TEXT("%s DataTable is missing its Properties object."), AssetName);
			return false;
		}
		const TSharedPtr<FJsonValue>* IgnoreExtraValue = Properties->Values.Find(TEXT("bIgnoreExtraFields"));
		const TSharedPtr<FJsonValue>* IgnoreMissingValue = Properties->Values.Find(TEXT("bIgnoreMissingFields"));
		if ((IgnoreExtraValue != nullptr && !(*IgnoreExtraValue)->TryGetBool(bIgnoreExtraFields))
			|| (IgnoreMissingValue != nullptr && !(*IgnoreMissingValue)->TryGetBool(bIgnoreMissingFields)))
		{
			OutError = FString::Printf(TEXT("%s DataTable import options must be Boolean when present."), AssetName);
			return false;
		}
		if (!RowStruct.IsValid()
			|| !TryGetRequiredString(RowStruct, TEXT("ObjectPath"), RowStructPath, OutError)
			|| RowStructPath != ExpectedRowStructPath
			|| !Rows.IsValid()
			|| Rows->Values.Num() != ExpectedBoardTableRows)
		{
			OutError = FString::Printf(TEXT("%s DataTable has an unexpected RowStruct or row count."), AssetName);
			return false;
		}

		OutDefinition = FBoardTableDefinition();
		OutDefinition.AssetName = AssetName;
		OutDefinition.PackageName = FString::Printf(TEXT("/Game/Structs/%s"), AssetName);
		OutDefinition.bIgnoreExtraFields = bIgnoreExtraFields;
		OutDefinition.bIgnoreMissingFields = bIgnoreMissingFields;
		TArray<FString> RowNames;
		Rows->Values.GetKeys(RowNames);
		RowNames.Sort();
		TSet<FString> SeenRows;
		for (const FString& RowName : RowNames)
		{
			if (SeenRows.Contains(RowName))
			{
				OutError = FString::Printf(TEXT("%s contains duplicate row name '%s'."), AssetName, *RowName);
				return false;
			}
			SeenRows.Add(RowName);
			const TSharedPtr<FJsonValue>* RowValue = Rows->Values.Find(RowName);
			const TSharedPtr<FJsonObject> RowObject = RowValue != nullptr && RowValue->IsValid()
				? (*RowValue)->AsObject()
				: nullptr;
			if (!RowObject.IsValid() || RowObject->Values.Num() != 1)
			{
				OutError = FString::Printf(TEXT("%s row '%s' must contain exactly one boardMat soft-object field."), AssetName, *RowName);
				return false;
			}
			const TSharedPtr<FJsonValue>* FieldValue = RowObject->Values.Find(BoardMatField);
			const TSharedPtr<FJsonObject> SoftObject = FieldValue != nullptr && FieldValue->IsValid()
				? (*FieldValue)->AsObject()
				: nullptr;
			FString AssetPathName;
			FString SubPathString;
			if (!SoftObject.IsValid()
				|| !TryGetRequiredString(SoftObject, TEXT("AssetPathName"), AssetPathName, OutError)
				|| AssetPathName.Left(6) != TEXT("/Game/")
				|| !SoftObject->TryGetStringField(TEXT("SubPathString"), SubPathString)
				|| !SubPathString.IsEmpty())
			{
				OutError = FString::Printf(TEXT("%s row '%s' has an invalid boardMat soft-object path."), AssetName, *RowName);
				return false;
			}
			FBoardRow& Row = OutDefinition.Rows.AddDefaulted_GetRef();
			Row.Name = FName(*RowName);
			Row.MaterialPath = FSoftObjectPath(AssetPathName);
		}
		return OutDefinition.Rows.Num() == ExpectedBoardTableRows;
	}

	const TSharedPtr<FJsonObject>* FindChildProperty(
		const FSourceExport* ClassExport,
		const TCHAR* PropertyName)
	{
		if (ClassExport == nullptr)
		{
			return nullptr;
		}
		const TArray<TSharedPtr<FJsonValue>>* ChildProperties = nullptr;
		if (!ClassExport->Object->TryGetArrayField(TEXT("ChildProperties"), ChildProperties)
			|| ChildProperties == nullptr)
		{
			return nullptr;
		}
		for (const TSharedPtr<FJsonValue>& Value : *ChildProperties)
		{
			const TSharedPtr<FJsonObject> Property = Value.IsValid() ? Value->AsObject() : nullptr;
			FString Name;
			if (Property.IsValid()
				&& Property->TryGetStringField(TEXT("Name"), Name)
				&& Name == PropertyName)
			{
				static TSharedPtr<FJsonObject> Result;
				Result = Property;
				return &Result;
			}
		}
		return nullptr;
	}

	TSharedPtr<FJsonObject> CopyChildProperty(
		const FSourceExport* ClassExport,
		const TCHAR* PropertyName)
	{
		const TSharedPtr<FJsonObject>* Property = FindChildProperty(ClassExport, PropertyName);
		return Property != nullptr ? *Property : nullptr;
	}

	int32 CountParameterProperties(const FSourceExport* FunctionExport)
	{
		if (FunctionExport == nullptr)
		{
			return INDEX_NONE;
		}
		const TArray<TSharedPtr<FJsonValue>>* ChildProperties = nullptr;
		if (!FunctionExport->Object->TryGetArrayField(TEXT("ChildProperties"), ChildProperties)
			|| ChildProperties == nullptr)
		{
			return 0;
		}
		int32 Count = 0;
		for (const TSharedPtr<FJsonValue>& Value : *ChildProperties)
		{
			const TSharedPtr<FJsonObject> Property = Value.IsValid() ? Value->AsObject() : nullptr;
			if (HasPropertyFlag(Property, TEXT("Parm")))
			{
				++Count;
			}
		}
		return Count;
	}

	bool ValidateBlueprintParent(
		const FSourceExport* ClassExport,
		const TCHAR* ExpectedName,
		const TCHAR* ExpectedPath,
		FString& OutError)
	{
		const TSharedPtr<FJsonObject> Parent = GetObjectField(ClassExport != nullptr ? ClassExport->Object : nullptr, TEXT("SuperStruct"));
		FString ObjectName;
		FString ObjectPath;
		if (!Parent.IsValid()
			|| !TryGetRequiredString(Parent, TEXT("ObjectName"), ObjectName, OutError)
			|| !TryGetRequiredString(Parent, TEXT("ObjectPath"), ObjectPath, OutError)
			|| ObjectName != ExpectedName
			|| ObjectPath != ExpectedPath)
		{
			OutError = FString::Printf(TEXT("Blueprint parent mismatch; expected %s at %s."), ExpectedName, ExpectedPath);
			return false;
		}
		return true;
	}

	bool ValidateMulticastDelegateProperty(
		const FSourceExport* OwnerClass,
		const TCHAR* PropertyName,
		const TCHAR* SignatureFunctionName,
		const FSourceFile& File,
		const TCHAR* ExpectedPackage,
		const TArray<TPair<FString, FString>>& ExpectedParameters,
		FString& OutError)
	{
		const TSharedPtr<FJsonObject> Property = CopyChildProperty(OwnerClass, PropertyName);
		FString Type;
		const TSharedPtr<FJsonObject> SignatureReference = GetObjectField(Property, TEXT("SignatureFunction"));
		FString ObjectName;
		FString ObjectPath;
		const FSourceExport* SignatureExport = FindExport(File, SignatureFunctionName);
		if (!Property.IsValid()
			|| !Property->TryGetStringField(TEXT("Type"), Type)
			|| Type != TEXT("MulticastInlineDelegateProperty")
			|| !SignatureReference.IsValid()
			|| !TryGetRequiredString(SignatureReference, TEXT("ObjectName"), ObjectName, OutError)
			|| !TryGetRequiredString(SignatureReference, TEXT("ObjectPath"), ObjectPath, OutError)
			|| !ObjectName.Contains(SignatureFunctionName)
			|| !ObjectPath.StartsWith(ExpectedPackage)
			|| SignatureExport == nullptr
			|| SignatureExport->Type != TEXT("Function")
			|| CountParameterProperties(SignatureExport) != ExpectedParameters.Num())
		{
			OutError = FString::Printf(TEXT("Delegate %s does not match its exact signature export."), PropertyName);
			return false;
		}

		for (const TPair<FString, FString>& Expected : ExpectedParameters)
		{
			bool bFound = false;
			const TArray<TSharedPtr<FJsonValue>>* ChildProperties = nullptr;
			SignatureExport->Object->TryGetArrayField(TEXT("ChildProperties"), ChildProperties);
			if (ChildProperties != nullptr)
			{
				for (const TSharedPtr<FJsonValue>& Value : *ChildProperties)
				{
					const TSharedPtr<FJsonObject> Parameter = Value.IsValid() ? Value->AsObject() : nullptr;
					FString Name;
					FString ParameterType;
					if (Parameter.IsValid()
						&& Parameter->TryGetStringField(TEXT("Name"), Name)
						&& Parameter->TryGetStringField(TEXT("Type"), ParameterType)
						&& Name == Expected.Key
						&& ParameterType == Expected.Value
						&& HasPropertyFlag(Parameter, TEXT("Parm")))
					{
						bFound = true;
						break;
					}
				}
			}
			if (!bFound)
			{
				OutError = FString::Printf(TEXT("Delegate %s is missing parameter %s of type %s."), PropertyName, *Expected.Key, *Expected.Value);
				return false;
			}
		}
		return true;
	}

	bool ValidateBlueprintSources(
		const FSourceFile& TableFile,
		const FSourceFile& MaterialSwitcherFile,
		const FSourceFile& LevelManagerFile,
		FBlueprintSignatureDefinition& OutMaterialSwitcher,
		FBlueprintSignatureDefinition& OutLevelManager,
		FString& OutError)
	{
		const FSourceExport* TableClass = FindExport(TableFile, TEXT("BP_Table_C"));
		const FSourceExport* MaterialSwitcherClass = FindExport(MaterialSwitcherFile, TEXT("BPC_MaterialSwitcher_C"));
		const FSourceExport* LevelManagerClass = FindExport(LevelManagerFile, TEXT("BP_LevelManager_C"));
		if (!ValidateExportIdentity(
				TableClass,
				TEXT("BlueprintGeneratedClass"),
				TEXT("BP_Table_C"),
				TEXT("kards/Content/Blueprints/BP_Table"),
				OutError)
			|| !ValidateExportIdentity(
				MaterialSwitcherClass,
				TEXT("BlueprintGeneratedClass"),
				TEXT("BPC_MaterialSwitcher_C"),
				TEXT("kards/Content/Blueprints/Components/BPC_MaterialSwitcher"),
				OutError)
			|| !ValidateExportIdentity(
				LevelManagerClass,
				TEXT("BlueprintGeneratedClass"),
				TEXT("BP_LevelManager_C"),
				TEXT("kards/Content/Blueprints/Logic/BP_LevelManager"),
				OutError)
			|| !ValidateBlueprintParent(TableClass, TEXT("Class'Actor'"), TEXT("/Script/Engine"), OutError)
			|| !ValidateBlueprintParent(MaterialSwitcherClass, TEXT("Class'ActorComponent'"), TEXT("/Script/Engine"), OutError)
			|| !ValidateBlueprintParent(LevelManagerClass, TEXT("Class'Actor'"), TEXT("/Script/Engine"), OutError))
		{
			return false;
		}

		const TSharedPtr<FJsonObject> ComponentProperty = CopyChildProperty(TableClass, TEXT("BPC_MaterialSwitcher"));
		const TSharedPtr<FJsonObject> ComponentClass = GetObjectField(ComponentProperty, TEXT("PropertyClass"));
		FString ComponentType;
		FString ComponentPath;
		if (!ComponentProperty.IsValid()
			|| !ComponentProperty->TryGetStringField(TEXT("Type"), ComponentType)
			|| ComponentType != TEXT("ObjectProperty")
			|| !ComponentClass.IsValid()
			|| !TryGetRequiredString(ComponentClass, TEXT("ObjectPath"), ComponentPath, OutError)
			|| ComponentPath != TEXT("kards/Content/Blueprints/Components/BPC_MaterialSwitcher.0"))
		{
			OutError = TEXT("BP_Table does not reference the expected BPC_MaterialSwitcher class property.");
			return false;
		}

		const TSharedPtr<FJsonObject> MaterialArray = CopyChildProperty(MaterialSwitcherClass, TEXT("MaterialsToSwap"));
		const TSharedPtr<FJsonObject> ArrayInner = GetObjectField(MaterialArray, TEXT("Inner"));
		const TSharedPtr<FJsonObject> InnerStruct = GetObjectField(ArrayInner, TEXT("Struct"));
		FString ArrayType;
		FString InnerType;
		FString InnerStructName;
		FString InnerStructPath;
		if (!MaterialArray.IsValid()
			|| !MaterialArray->TryGetStringField(TEXT("Type"), ArrayType)
			|| ArrayType != TEXT("ArrayProperty")
			|| !ArrayInner.IsValid()
			|| !ArrayInner->TryGetStringField(TEXT("Type"), InnerType)
			|| InnerType != TEXT("StructProperty")
			|| !InnerStruct.IsValid()
			|| !TryGetRequiredString(InnerStruct, TEXT("ObjectName"), InnerStructName, OutError)
			|| !TryGetRequiredString(InnerStruct, TEXT("ObjectPath"), InnerStructPath, OutError)
			|| InnerStructName != TEXT("UserDefinedStruct'S_MaterialSwapItem'")
			|| InnerStructPath != TEXT("kards/Content/Structs/S_MaterialSwapItem.0"))
		{
			OutError = TEXT("BPC_MaterialSwitcher.MaterialsToSwap is not the expected S_MaterialSwapItem array.");
			return false;
		}

		const TSharedPtr<FJsonObject> SimpleProperty = CopyChildProperty(MaterialSwitcherClass, TEXT("bSimple"));
		FString SimpleType;
		if (!SimpleProperty.IsValid()
			|| !SimpleProperty->TryGetStringField(TEXT("Type"), SimpleType)
			|| SimpleType != TEXT("BoolProperty"))
		{
			OutError = TEXT("BPC_MaterialSwitcher.bSimple is not a Boolean property.");
			return false;
		}

		if (!ValidateMulticastDelegateProperty(
				MaterialSwitcherClass,
				TEXT("OnMaterialUpdate"),
				TEXT("OnMaterialUpdate__DelegateSignature"),
				MaterialSwitcherFile,
				TEXT("kards/Content/Blueprints/Components/BPC_MaterialSwitcher"),
				{{TEXT("bSimple"), TEXT("BoolProperty")}},
				OutError)
			|| !ValidateMulticastDelegateProperty(
				LevelManagerClass,
				TEXT("OnLevelLoaded"),
				TEXT("OnLevelLoaded__DelegateSignature"),
				LevelManagerFile,
				TEXT("kards/Content/Blueprints/Logic/BP_LevelManager"),
				{},
				OutError))
		{
			return false;
		}

		const FSourceExport* UpdateFunction = FindExport(MaterialSwitcherFile, TEXT("Update"));
		if (UpdateFunction == nullptr
			|| UpdateFunction->Type != TEXT("Function")
			|| CountParameterProperties(UpdateFunction) != 0)
		{
			OutError = TEXT("BPC_MaterialSwitcher Update must exist as a zero-parameter function in the official JSON.");
			return false;
		}

		OutMaterialSwitcher.AssetName = TEXT("BPC_MaterialSwitcher");
		OutMaterialSwitcher.PackageName = TEXT("/Game/Blueprints/Components/BPC_MaterialSwitcher");
		OutMaterialSwitcher.ParentClass = UActorComponent::StaticClass();
		OutLevelManager.AssetName = TEXT("BP_LevelManager");
		OutLevelManager.PackageName = TEXT("/Game/Blueprints/Logic/BP_LevelManager");
		OutLevelManager.ParentClass = AActor::StaticClass();
		return true;
	}

	bool PrepareSources(const FString& SourceRoot, FPreparedSources& OutSources, FString& OutError)
	{
		FSourceFile PlatformEnumFile;
		FSourceFile BackgroundEnumFile;
		FSourceFile BoardMatFile;
		FSourceFile MaterialSwapFile;
		FSourceFile DesktopTableFile;
		FSourceFile MobileTableFile;
		FSourceFile MaterialSwitcherFile;
		FSourceFile LevelManagerFile;
		FSourceFile BPTableFile;

		if (!LoadSourceFile(SourceRoot, TEXT("Structs/E_Platform.json"), 1, PlatformEnumFile, OutError)
			|| !LoadSourceFile(SourceRoot, TEXT("Structs/E_BackgroundType.json"), 1, BackgroundEnumFile, OutError)
			|| !LoadSourceFile(SourceRoot, TEXT("Structs/boardMat.json"), 1, BoardMatFile, OutError)
			|| !LoadSourceFile(SourceRoot, TEXT("Structs/S_MaterialSwapItem.json"), 1, MaterialSwapFile, OutError)
			|| !LoadSourceFile(SourceRoot, TEXT("Structs/boardMats.json"), 1, DesktopTableFile, OutError)
			|| !LoadSourceFile(SourceRoot, TEXT("Structs/boardMats_Mobile.json"), 1, MobileTableFile, OutError)
			|| !LoadSourceFile(SourceRoot, TEXT("Blueprints/Components/BPC_MaterialSwitcher.json"), 7, MaterialSwitcherFile, OutError)
			|| !LoadSourceFile(SourceRoot, TEXT("Blueprints/Logic/BP_LevelManager.json"), 22, LevelManagerFile, OutError)
			|| !LoadSourceFile(SourceRoot, TEXT("Blueprints/BP_Table.json"), 20, BPTableFile, OutError))
		{
			return false;
		}

		if (!LoadUserDefinedEnum(
				PlatformEnumFile,
				TEXT("E_Platform"),
				TEXT("/Game/Structs/E_Platform"),
				2,
				OutSources.PlatformEnum,
				OutError)
			|| !LoadUserDefinedEnum(
				BackgroundEnumFile,
				TEXT("E_BackgroundType"),
				TEXT("/Game/Structs/E_BackgroundType"),
				26,
				OutSources.BackgroundEnum,
				OutError))
		{
			return false;
		}

		if (OutSources.PlatformEnum.Entries[0].DisplayName.ToString() != TEXT("PC")
			|| OutSources.PlatformEnum.Entries[1].DisplayName.ToString() != TEXT("Mobile")
			|| OutSources.PlatformEnum.Entries[0].Value != 0
			|| OutSources.PlatformEnum.Entries[1].Value != 1)
		{
			OutError = TEXT("E_Platform must preserve PC=0 and Mobile=1 from the official export.");
			return false;
		}

		TArray<TPair<FString, FString>> BoardMatFields;
		BoardMatFields.Emplace(BoardMatField, TEXT("SoftObjectProperty"));
		TArray<TPair<FString, FString>> SwapFields;
		SwapFields.Emplace(ComponentField, TEXT("ObjectProperty"));
		SwapFields.Emplace(SimplifiedMaterialField, TEXT("ObjectProperty"));
		SwapFields.Emplace(MaterialField, TEXT("ObjectProperty"));
		SwapFields.Emplace(MaterialSlotField, TEXT("IntProperty"));
		if (!LoadStructDefinition(
				BoardMatFile,
				TEXT("boardMat"),
				TEXT("/Game/Structs/boardMat"),
				BoardMatFields,
				OutSources.BoardMatStruct,
				OutError)
			|| !LoadStructDefinition(
				MaterialSwapFile,
				TEXT("S_MaterialSwapItem"),
				TEXT("/Game/Structs/S_MaterialSwapItem"),
				SwapFields,
				OutSources.MaterialSwapStruct,
				OutError))
		{
			return false;
		}

		if (!LoadBoardTableDefinition(
				DesktopTableFile,
				TEXT("boardMats"),
				TEXT("kards/Content/Structs/boardMat.0"),
				OutSources.DesktopBoardTable,
				OutError)
			|| !LoadBoardTableDefinition(
				MobileTableFile,
				TEXT("boardMats_Mobile"),
				TEXT("kards/Content/Structs/boardMat.0"),
				OutSources.MobileBoardTable,
				OutError))
		{
			return false;
		}

		static const TCHAR* ExpectedRows[] = {
			TEXT("default"), TEXT("fabric"), TEXT("metal"), TEXT("blueprint"), TEXT("poland"),
			TEXT("london"), TEXT("moscow"), TEXT("desert"), TEXT("desert2"), TEXT("tropical"),
			TEXT("tropical2"), TEXT("metal2"), TEXT("washington"), TEXT("bryansk"), TEXT("ardennes"),
			TEXT("colonial"), TEXT("reset"), TEXT("fabricDark"), TEXT("tank_us"), TEXT("tank_britain"),
			TEXT("tank_japan"), TEXT("tank_germany"), TEXT("tank_soviet"), TEXT("submarine"),
			TEXT("supply_train"), TEXT("concrete_bunker"), TEXT("fortress_darwin"), TEXT("iwo_jima")};
		TSet<FString> ExpectedRowSet;
		for (const TCHAR* RowName : ExpectedRows)
		{
			ExpectedRowSet.Add(RowName);
		}
		for (const FBoardTableDefinition* Table : {&OutSources.DesktopBoardTable, &OutSources.MobileBoardTable})
		{
			TSet<FString> ActualRows;
			for (const FBoardRow& Row : Table->Rows)
			{
				ActualRows.Add(Row.Name.ToString());
			}
		bool bRowsMatch = ActualRows.Num() == ExpectedRowSet.Num();
		for (const FString& ExpectedRow : ExpectedRowSet)
		{
			bRowsMatch = bRowsMatch && ActualRows.Contains(ExpectedRow);
		}
		if (!bRowsMatch)
			{
				OutError = FString::Printf(TEXT("%s does not contain the exact official 28 row identities."), *Table->AssetName);
				return false;
			}
		}

		if (!ValidateBlueprintSources(
				BPTableFile,
				MaterialSwitcherFile,
				LevelManagerFile,
				OutSources.MaterialSwitcher,
				OutSources.LevelManager,
				OutError))
		{
			return false;
		}
		OutSources.MaterialSwitcherFile = MoveTemp(MaterialSwitcherFile);
		OutSources.LevelManagerFile = MoveTemp(LevelManagerFile);

		UE_LOG(
			LogBPTableSchemaAssets,
			Display,
			TEXT("Validated current exports/kards sources: E_Platform, E_BackgroundType, boardMat, S_MaterialSwapItem, both 28-row board tables, BPC_MaterialSwitcher signature, BP_LevelManager delegate, BP_Table reference."));
		return true;
	}

	bool PreflightTargetPackagesAreAbsent(FString& OutError)
	{
		static const TCHAR* Packages[] = {
			TEXT("/Game/Structs/E_Platform"),
			TEXT("/Game/Structs/E_BackgroundType"),
			TEXT("/Game/Structs/boardMat"),
			TEXT("/Game/Structs/S_MaterialSwapItem"),
			TEXT("/Game/Structs/boardMats"),
			TEXT("/Game/Structs/boardMats_Mobile"),
			TEXT("/Game/Blueprints/Components/BPC_MaterialSwitcher"),
			TEXT("/Game/Blueprints/Logic/BP_LevelManager")};

		for (const TCHAR* PackageName : Packages)
		{
			FString ExistingFilename;
			if (FindPackage(nullptr, PackageName) != nullptr
				|| FPackageName::DoesPackageExist(PackageName))
			{
				OutError = FString::Printf(
					TEXT("Target package already exists; refusing to overwrite: %s%s"),
					PackageName,
					ExistingFilename.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" (%s)"), *ExistingFilename));
				return false;
			}

			const FString UAssetFilename = FPackageName::LongPackageNameToFilename(
				PackageName,
				FPackageName::GetAssetPackageExtension());
			FString BaseFilename = UAssetFilename;
			BaseFilename.RemoveFromEnd(FPackageName::GetAssetPackageExtension());
			for (const TCHAR* SidecarExtension : {TEXT("uexp"), TEXT("ubulk"), TEXT("uptnl")})
			{
				const FString SidecarPath = BaseFilename + TEXT(".") + SidecarExtension;
				if (IFileManager::Get().FileExists(*SidecarPath))
				{
					OutError = FString::Printf(TEXT("Orphaned package sidecar exists; refusing to overwrite: %s"), *SidecarPath);
					return false;
				}
			}
		}
		return true;
	}

	UUserDefinedEnum* BuildEnumAsset(const FEnumDefinition& Definition, FString& OutError)
	{
		UPackage* Package = CreatePackage(*Definition.PackageName);
		UUserDefinedEnum* Enum = Package == nullptr
			? nullptr
			: Cast<UUserDefinedEnum>(FEnumEditorUtils::CreateUserDefinedEnum(
				Package,
				FName(*Definition.AssetName),
				RF_Public | RF_Standalone | RF_Transactional));
		if (Enum == nullptr)
		{
			OutError = FString::Printf(TEXT("Could not create user enum %s."), *Definition.PackageName);
			return nullptr;
		}

		TArray<TPair<FName, int64>> EnumNames;
		EnumNames.Reserve(Definition.Entries.Num());
		for (const FEnumEntry& Entry : Definition.Entries)
		{
			EnumNames.Emplace(FName(*Enum->GenerateFullEnumName(*Entry.InternalName)), Entry.Value);
		}
		if (!Enum->SetEnums(EnumNames, UEnum::ECppForm::Namespaced))
		{
			OutError = FString::Printf(TEXT("Could not assign exact enumerators to %s."), *Definition.PackageName);
			return nullptr;
		}

		Enum->UniqueNameIndex = Definition.UniqueNameIndex;
		Enum->DisplayNameMap.Reset();
		for (int32 Index = 0; Index < Definition.Entries.Num(); ++Index)
		{
			Enum->DisplayNameMap.Add(
				FName(*Enum->GetNameStringByIndex(Index)),
				Definition.Entries[Index].DisplayName);
		}
		Enum->SetMetaData(TEXT("BlueprintType"), TEXT("true"));
		Enum->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(Enum);

		if (Enum->NumEnums() != Definition.Entries.Num() + 1
			|| Enum->GetValueByIndex(Definition.Entries.Num()) != Definition.MaxValue)
		{
			OutError = FString::Printf(TEXT("Built enum %s has an invalid MAX sentinel."), *Definition.PackageName);
			return nullptr;
		}
		for (int32 Index = 0; Index < Definition.Entries.Num(); ++Index)
		{
			const FEnumEntry& Expected = Definition.Entries[Index];
			if (Enum->GetNameStringByIndex(Index) != Expected.InternalName
				|| Enum->GetValueByIndex(Index) != Expected.Value
				|| Enum->GetDisplayNameTextByIndex(Index).ToString() != Expected.DisplayName.ToString())
			{
				OutError = FString::Printf(
					TEXT("Built enum %s differs from source at enumerator index %d."),
					*Definition.PackageName,
					Index);
				return nullptr;
			}
		}
		return Enum;
	}

	bool VerifyStructField(UUserDefinedStruct* Struct, const FStructField& Field, FString& OutError)
	{
		FProperty* Property = Struct != nullptr
			? FindFProperty<FProperty>(Struct, FName(*Field.InternalName))
			: nullptr;
		if (Property == nullptr || !Property->HasAllPropertyFlags(Field.PropertyFlags))
		{
			OutError = FString::Printf(TEXT("Compiled struct field %s.%s is absent or has incorrect flags."), *GetNameSafe(Struct), *Field.InternalName);
			return false;
		}

		if (Field.PinType.PinCategory == UEdGraphSchema_K2::PC_SoftObject)
		{
			const FSoftObjectProperty* SoftProperty = CastField<FSoftObjectProperty>(Property);
			if (SoftProperty == nullptr
				|| SoftProperty->PropertyClass != Field.PinType.PinSubCategoryObject.Get())
			{
				OutError = FString::Printf(TEXT("Compiled soft-object field %s.%s has the wrong class."), *GetNameSafe(Struct), *Field.InternalName);
				return false;
			}
		}
		else if (Field.PinType.PinCategory == UEdGraphSchema_K2::PC_Object)
		{
			const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property);
			if (ObjectProperty == nullptr
				|| ObjectProperty->PropertyClass != Field.PinType.PinSubCategoryObject.Get())
			{
				OutError = FString::Printf(TEXT("Compiled object field %s.%s has the wrong class."), *GetNameSafe(Struct), *Field.InternalName);
				return false;
			}
		}
		else if (Field.PinType.PinCategory == UEdGraphSchema_K2::PC_Int
			&& CastField<FIntProperty>(Property) == nullptr)
		{
			OutError = FString::Printf(TEXT("Compiled integer field %s.%s is not int32."), *GetNameSafe(Struct), *Field.InternalName);
			return false;
		}
		return true;
	}

	UUserDefinedStruct* BuildStructAsset(const FStructDefinition& Definition, FString& OutError)
	{
		UPackage* Package = CreatePackage(*Definition.PackageName);
		UUserDefinedStruct* Struct = Package == nullptr
			? nullptr
			: FStructureEditorUtils::CreateUserDefinedStruct(
				Package,
				FName(*Definition.AssetName),
				RF_Public | RF_Standalone | RF_Transactional);
		if (Struct == nullptr)
		{
			OutError = FString::Printf(TEXT("Could not create user struct %s."), *Definition.PackageName);
			return nullptr;
		}
		Struct->Guid = Definition.Guid;
		FAssetRegistryModule::AssetCreated(Struct);

		for (const FStructField& Field : Definition.Fields)
		{
			if (!FStructureEditorUtils::AddVariable(Struct, Field.PinType))
			{
				OutError = FString::Printf(TEXT("Could not add field %s to %s."), *Field.InternalName, *Definition.PackageName);
				return nullptr;
			}
			TArray<FStructVariableDescription>& Variables = FStructureEditorUtils::GetVarDesc(Struct);
			if (Variables.IsEmpty())
			{
				OutError = FString::Printf(TEXT("Structure editor lost field %s in %s."), *Field.InternalName, *Definition.PackageName);
				return nullptr;
			}
			FStructVariableDescription& Variable = Variables.Last();
			Variable.VarName = FName(*Field.InternalName);
			Variable.FriendlyName = Field.FriendlyName;
			Variable.VarGuid = Field.Guid;
		}

		FStructureEditorUtils::CompileStructure(Struct);
		if (Struct->Status != EUserDefinedStructureStatus::UDSS_UpToDate)
		{
			OutError = FString::Printf(TEXT("User struct %s did not compile: %s"), *Definition.PackageName, *Struct->ErrorMessage);
			return nullptr;
		}
		if (Struct->Guid != Definition.Guid)
		{
			OutError = FString::Printf(TEXT("Compiled struct %s lost its source GUID."), *Definition.PackageName);
			return nullptr;
		}
		for (const FStructField& Field : Definition.Fields)
		{
			if (!VerifyStructField(Struct, Field, OutError))
			{
				return nullptr;
			}
		}
		Struct->MarkPackageDirty();
		return Struct;
	}

	UDataTable* BuildBoardTable(
		const FBoardTableDefinition& Definition,
		UUserDefinedStruct* BoardMatStruct,
		FString& OutError)
	{
		FSoftObjectProperty* MaterialProperty = BoardMatStruct != nullptr
			? CastField<FSoftObjectProperty>(FindFProperty<FProperty>(BoardMatStruct, BoardMatField))
			: nullptr;
		if (MaterialProperty == nullptr || MaterialProperty->PropertyClass != UMaterialInstance::StaticClass())
		{
			OutError = TEXT("boardMat row struct does not expose the exact MaterialInstance soft-object field.");
			return nullptr;
		}

		UPackage* Package = CreatePackage(*Definition.PackageName);
		UDataTableFactory* Factory = NewObject<UDataTableFactory>();
		Factory->Struct = BoardMatStruct;
		UDataTable* DataTable = Package == nullptr
			? nullptr
			: Cast<UDataTable>(Factory->FactoryCreateNew(
				UDataTable::StaticClass(),
				Package,
				FName(*Definition.AssetName),
				RF_Public | RF_Standalone | RF_Transactional,
				nullptr,
				GWarn));
		if (DataTable == nullptr)
		{
			OutError = FString::Printf(TEXT("Could not create data table %s."), *Definition.PackageName);
			return nullptr;
		}
		DataTable->bIgnoreExtraFields = Definition.bIgnoreExtraFields;
		DataTable->bIgnoreMissingFields = Definition.bIgnoreMissingFields;
		FAssetRegistryModule::AssetCreated(DataTable);
		if (DataTable->GetRowStruct() != BoardMatStruct
			|| DataTable->bIgnoreExtraFields != Definition.bIgnoreExtraFields
			|| DataTable->bIgnoreMissingFields != Definition.bIgnoreMissingFields)
		{
			OutError = FString::Printf(TEXT("Data table %s did not retain the exact boardMat RowStruct and source/default import-policy flags."), *Definition.PackageName);
			return nullptr;
		}

		for (const FBoardRow& Row : Definition.Rows)
		{
			FStructOnScope RowMemory(BoardMatStruct);
			void* ValueAddress = MaterialProperty->ContainerPtrToValuePtr<void>(RowMemory.GetStructMemory());
			MaterialProperty->SetPropertyValue(ValueAddress, FSoftObjectPtr(Row.MaterialPath));
			DataTable->AddRow(Row.Name, RowMemory.GetStructMemory(), BoardMatStruct);
		}
		DataTable->MarkPackageDirty();

		if (DataTable->GetRowMap().Num() != ExpectedBoardTableRows
			|| DataTable->bIgnoreExtraFields != Definition.bIgnoreExtraFields
			|| DataTable->bIgnoreMissingFields != Definition.bIgnoreMissingFields)
		{
			OutError = FString::Printf(TEXT("Data table %s has an unexpected row count or source/default import-policy flags after construction."), *Definition.PackageName);
			return nullptr;
		}
		for (const FBoardRow& ExpectedRow : Definition.Rows)
		{
			const uint8* RowData = DataTable->FindRowUnchecked(ExpectedRow.Name);
			if (RowData == nullptr)
			{
				OutError = FString::Printf(TEXT("Data table %s is missing row %s."), *Definition.PackageName, *ExpectedRow.Name.ToString());
				return nullptr;
			}
			const FSoftObjectPtr Actual = MaterialProperty->GetPropertyValue_InContainer(RowData);
			if (Actual.ToSoftObjectPath() != ExpectedRow.MaterialPath)
			{
				OutError = FString::Printf(TEXT("Data table %s row %s changed its soft path."), *Definition.PackageName, *ExpectedRow.Name.ToString());
				return nullptr;
			}
		}
		return DataTable;
	}

	EPropertyFlags BlueprintPropertyFlags(const TSharedPtr<FJsonObject>& Property)
	{
		EPropertyFlags Flags = CPF_None;
		if (HasPropertyFlag(Property, TEXT("Edit"))) Flags |= CPF_Edit;
		if (HasPropertyFlag(Property, TEXT("BlueprintVisible"))) Flags |= CPF_BlueprintVisible;
		if (HasPropertyFlag(Property, TEXT("BlueprintReadOnly"))) Flags |= CPF_BlueprintReadOnly;
		if (HasPropertyFlag(Property, TEXT("DisableEditOnInstance"))) Flags |= CPF_DisableEditOnInstance;
		if (HasPropertyFlag(Property, TEXT("BlueprintAssignable"))) Flags |= CPF_BlueprintAssignable;
		if (HasPropertyFlag(Property, TEXT("BlueprintCallable"))) Flags |= CPF_BlueprintCallable;
		if (HasPropertyFlag(Property, TEXT("ExposeOnSpawn"))) Flags |= CPF_ExposeOnSpawn;
		if (HasPropertyFlag(Property, TEXT("InstancedReference"))) Flags |= CPF_InstancedReference;
		if (HasPropertyFlag(Property, TEXT("ContainsInstancedReference"))) Flags |= CPF_ContainsInstancedReference;
		return Flags;
	}

	bool AddBlueprintVariable(
		UBlueprint* Blueprint,
		const TSharedPtr<FJsonObject>& SourceProperty,
		const FName Name,
		const FEdGraphPinType& PinType,
		FString& OutError)
	{
		if (Blueprint == nullptr || !FBlueprintEditorUtils::AddMemberVariable(Blueprint, Name, PinType))
		{
			OutError = FString::Printf(TEXT("Could not add signature property %s to dependency Blueprint."), *Name.ToString());
			return false;
		}
		for (FBPVariableDescription& Variable : Blueprint->NewVariables)
		{
			if (Variable.VarName == Name)
			{
				Variable.PropertyFlags |= BlueprintPropertyFlags(SourceProperty);
				return true;
			}
		}
		OutError = FString::Printf(TEXT("New Blueprint signature property %s was not registered."), *Name.ToString());
		return false;
	}

	bool AddDelegateSignatureGraph(
		UBlueprint* Blueprint,
		const FName DelegateName,
		const FSourceExport* SignatureExport,
		const TArray<TPair<FString, FString>>& Parameters,
		FString& OutError)
	{
		FEdGraphPinType DelegateType;
		DelegateType.PinCategory = UEdGraphSchema_K2::PC_MCDelegate;
		if (!FBlueprintEditorUtils::AddMemberVariable(Blueprint, DelegateName, DelegateType))
		{
			OutError = FString::Printf(TEXT("Could not add event dispatcher %s."), *DelegateName.ToString());
			return false;
		}

		for (FBPVariableDescription& Variable : Blueprint->NewVariables)
		{
			if (Variable.VarName == DelegateName)
			{
				Variable.PropertyFlags |= CPF_Edit | CPF_BlueprintVisible | CPF_BlueprintAssignable
					| CPF_BlueprintCallable | CPF_DisableEditOnInstance;
				break;
			}
		}

		UEdGraph* Graph = FBlueprintEditorUtils::CreateNewGraph(
			Blueprint,
			DelegateName,
			UEdGraph::StaticClass(),
			UEdGraphSchema_K2::StaticClass());
		if (Graph == nullptr)
		{
			OutError = FString::Printf(TEXT("Could not create delegate signature graph %s."), *DelegateName.ToString());
			return false;
		}
		Graph->bEditable = false;
		const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
		Schema->CreateDefaultNodesForGraph(*Graph);
		Schema->CreateFunctionGraphTerminators(*Graph, static_cast<UClass*>(nullptr));
		Schema->AddExtraFunctionFlags(Graph, FUNC_BlueprintCallable | FUNC_BlueprintEvent | FUNC_Public);
		Schema->MarkFunctionEntryAsEditable(Graph, true);

		TArray<UK2Node_FunctionEntry*> Entries;
		Graph->GetNodesOfClass(Entries);
		if (Entries.Num() != 1 || CountParameterProperties(SignatureExport) != Parameters.Num())
		{
			OutError = FString::Printf(TEXT("Delegate %s does not have exactly one generated function entry."), *DelegateName.ToString());
			return false;
		}

		for (const TPair<FString, FString>& Parameter : Parameters)
		{
			FEdGraphPinType PinType;
			if (Parameter.Value == TEXT("BoolProperty"))
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
			}
			else
			{
				OutError = FString::Printf(TEXT("Unsupported signature pin type %s for %s.%s."), *Parameter.Value, *DelegateName.ToString(), *Parameter.Key);
				return false;
			}
			if (Entries[0]->CreateUserDefinedPin(FName(*Parameter.Key), PinType, EGPD_Output, false) == nullptr)
			{
				OutError = FString::Printf(TEXT("Could not create delegate parameter %s.%s."), *DelegateName.ToString(), *Parameter.Key);
				return false;
			}
		}

		Blueprint->DelegateSignatureGraphs.Add(Graph);
		return true;
	}

	bool AddZeroParameterFunctionGraph(UBlueprint* Blueprint, const FName FunctionName, FString& OutError)
	{
		UEdGraph* Graph = FBlueprintEditorUtils::CreateNewGraph(
			Blueprint,
			FunctionName,
			UEdGraph::StaticClass(),
			UEdGraphSchema_K2::StaticClass());
		if (Graph == nullptr)
		{
			OutError = FString::Printf(TEXT("Could not create function signature graph %s."), *FunctionName.ToString());
			return false;
		}
		FBlueprintEditorUtils::AddFunctionGraph<UFunction>(
			Blueprint,
			Graph,
			true,
			static_cast<UFunction*>(nullptr));
		if (!Blueprint->FunctionGraphs.Contains(Graph))
		{
			OutError = FString::Printf(TEXT("Could not register Blueprint function %s."), *FunctionName.ToString());
			return false;
		}
		const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
		Schema->AddExtraFunctionFlags(Graph, FUNC_Public | FUNC_BlueprintCallable);
		Schema->MarkFunctionEntryAsEditable(Graph, true);
		TArray<UK2Node_FunctionEntry*> Entries;
		Graph->GetNodesOfClass(Entries);
		if (Entries.Num() != 1)
		{
			OutError = FString::Printf(TEXT("Function %s did not produce one entry node."), *FunctionName.ToString());
			return false;
		}
		return true;
	}

	UBlueprint* BuildBlueprintShell(
		const FBlueprintSignatureDefinition& Definition,
		FString& OutError);

	bool VerifyLevelManagerShell(UBlueprint* Blueprint, FString& OutError);

	bool VerifyMaterialSwitcherShell(UBlueprint* Blueprint, UUserDefinedStruct* MaterialSwapStruct, FString& OutError)
	{
		if (Blueprint == nullptr
			|| Blueprint->GeneratedClass == nullptr
			|| Blueprint->ParentClass != UActorComponent::StaticClass())
		{
			OutError = TEXT("BPC_MaterialSwitcher shell has the wrong generated class or parent.");
			return false;
		}
		FArrayProperty* Materials = FindFProperty<FArrayProperty>(Blueprint->GeneratedClass, TEXT("MaterialsToSwap"));
		FStructProperty* InnerStruct = Materials != nullptr ? CastField<FStructProperty>(Materials->Inner) : nullptr;
		FBoolProperty* Simple = FindFProperty<FBoolProperty>(Blueprint->GeneratedClass, TEXT("bSimple"));
		FMulticastDelegateProperty* Dispatcher = FindFProperty<FMulticastDelegateProperty>(Blueprint->GeneratedClass, TEXT("OnMaterialUpdate"));
		UFunction* Update = Blueprint->GeneratedClass->FindFunctionByName(TEXT("Update"));
		if (Materials == nullptr
			|| InnerStruct == nullptr
			|| InnerStruct->Struct != MaterialSwapStruct
			|| Simple == nullptr
			|| Dispatcher == nullptr
			|| Dispatcher->SignatureFunction == nullptr
			|| Dispatcher->SignatureFunction->GetName() != TEXT("OnMaterialUpdate__DelegateSignature")
			|| FindFProperty<FBoolProperty>(Dispatcher->SignatureFunction, TEXT("bSimple")) == nullptr
			|| Update == nullptr
			|| Update->NumParms != 0)
		{
			OutError = TEXT("BPC_MaterialSwitcher shell failed its exact property/function signature verification.");
			return false;
		}
		return true;
	}

	bool BuildLevelManagerShell(
		const FSourceFile& SourceFile,
		const FBlueprintSignatureDefinition& Definition,
		UBlueprint*& OutBlueprint,
		FString& OutError)
	{
		const FSourceExport* ClassExport = FindExport(SourceFile, TEXT("BP_LevelManager_C"));
		const TSharedPtr<FJsonObject> DispatcherProperty = CopyChildProperty(ClassExport, TEXT("OnLevelLoaded"));
		const FSourceExport* DispatcherSignature = FindExport(SourceFile, TEXT("OnLevelLoaded__DelegateSignature"));
		if (!DispatcherProperty.IsValid() || DispatcherSignature == nullptr)
		{
			OutError = TEXT("BP_LevelManager OnLevelLoaded signature is missing from source JSON.");
			return false;
		}

		OutBlueprint = BuildBlueprintShell(Definition, OutError);
		if (OutBlueprint == nullptr)
		{
			return false;
		}
		if (!AddDelegateSignatureGraph(
				OutBlueprint,
				FName(TEXT("OnLevelLoaded")),
				DispatcherSignature,
				{},
				OutError))
		{
			return false;
		}
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(OutBlueprint);
		FKismetEditorUtilities::CompileBlueprint(OutBlueprint, EBlueprintCompileOptions::SkipSave);
		if (OutBlueprint->Status == BS_Error)
		{
			OutError = TEXT("The BP_LevelManager delegate signature shell did not compile.");
			return false;
		}
		OutBlueprint->MarkPackageDirty();
		return VerifyLevelManagerShell(OutBlueprint, OutError);
	}

	bool VerifyLevelManagerShell(UBlueprint* Blueprint, FString& OutError)
	{
		if (Blueprint == nullptr
			|| Blueprint->GeneratedClass == nullptr
			|| Blueprint->ParentClass != AActor::StaticClass())
		{
			OutError = TEXT("BP_LevelManager shell has the wrong generated class or parent.");
			return false;
		}
		FMulticastDelegateProperty* Dispatcher = FindFProperty<FMulticastDelegateProperty>(Blueprint->GeneratedClass, TEXT("OnLevelLoaded"));
		if (Dispatcher == nullptr
			|| Dispatcher->SignatureFunction == nullptr
			|| Dispatcher->SignatureFunction->GetName() != TEXT("OnLevelLoaded__DelegateSignature")
			|| Dispatcher->SignatureFunction->NumParms != 0)
		{
			OutError = TEXT("BP_LevelManager OnLevelLoaded dispatcher signature is not zero-parameter.");
			return false;
		}
		return true;
	}

	bool BuildMaterialSwitcherShell(
		const FSourceFile& SourceFile,
		const FBlueprintSignatureDefinition& Definition,
		UUserDefinedStruct* MaterialSwapStruct,
		UBlueprint*& OutBlueprint,
		FString& OutError);

	bool BuildAllAssets(const FPreparedSources& Sources, FBuiltAssets& OutAssets, FString& OutError)
	{
		OutAssets = FBuiltAssets();
		OutAssets.PlatformEnum = BuildEnumAsset(Sources.PlatformEnum, OutError);
		if (OutAssets.PlatformEnum == nullptr) return false;
		OutAssets.SaveOrder.Add(OutAssets.PlatformEnum);

		OutAssets.BackgroundEnum = BuildEnumAsset(Sources.BackgroundEnum, OutError);
		if (OutAssets.BackgroundEnum == nullptr) return false;
		OutAssets.SaveOrder.Add(OutAssets.BackgroundEnum);

		OutAssets.BoardMatStruct = BuildStructAsset(Sources.BoardMatStruct, OutError);
		if (OutAssets.BoardMatStruct == nullptr) return false;
		OutAssets.SaveOrder.Add(OutAssets.BoardMatStruct);

		OutAssets.MaterialSwapStruct = BuildStructAsset(Sources.MaterialSwapStruct, OutError);
		if (OutAssets.MaterialSwapStruct == nullptr) return false;
		OutAssets.SaveOrder.Add(OutAssets.MaterialSwapStruct);

		OutAssets.DesktopBoardTable = BuildBoardTable(Sources.DesktopBoardTable, OutAssets.BoardMatStruct, OutError);
		if (OutAssets.DesktopBoardTable == nullptr) return false;
		OutAssets.SaveOrder.Add(OutAssets.DesktopBoardTable);

		OutAssets.MobileBoardTable = BuildBoardTable(Sources.MobileBoardTable, OutAssets.BoardMatStruct, OutError);
		if (OutAssets.MobileBoardTable == nullptr) return false;
		OutAssets.SaveOrder.Add(OutAssets.MobileBoardTable);

		if (!BuildMaterialSwitcherShell(
				Sources.MaterialSwitcherFile,
				Sources.MaterialSwitcher,
				OutAssets.MaterialSwapStruct,
				OutAssets.MaterialSwitcher,
				OutError))
		{
			return false;
		}
		OutAssets.SaveOrder.Add(OutAssets.MaterialSwitcher);

		if (!BuildLevelManagerShell(
				Sources.LevelManagerFile,
				Sources.LevelManager,
				OutAssets.LevelManager,
				OutError))
		{
			return false;
		}
		OutAssets.SaveOrder.Add(OutAssets.LevelManager);
		return true;
	}

	bool VerifyAllBuiltAssets(const FBuiltAssets& Assets, FString& OutError)
	{
		if (Assets.SaveOrder.Num() != 8
			|| Assets.PlatformEnum == nullptr
			|| Assets.BackgroundEnum == nullptr
			|| Assets.BoardMatStruct == nullptr
			|| Assets.MaterialSwapStruct == nullptr
			|| Assets.DesktopBoardTable == nullptr
			|| Assets.MobileBoardTable == nullptr
			|| Assets.MaterialSwitcher == nullptr
			|| Assets.LevelManager == nullptr)
		{
			OutError = TEXT("The in-memory schema build did not produce all eight expected dependency assets.");
			return false;
		}

		if (Assets.PlatformEnum->GetPathName() != TEXT("/Game/Structs/E_Platform.E_Platform")
			|| Assets.BackgroundEnum->GetPathName() != TEXT("/Game/Structs/E_BackgroundType.E_BackgroundType")
			|| Assets.BoardMatStruct->GetPathName() != TEXT("/Game/Structs/boardMat.boardMat")
			|| Assets.MaterialSwapStruct->GetPathName() != TEXT("/Game/Structs/S_MaterialSwapItem.S_MaterialSwapItem")
			|| Assets.DesktopBoardTable->GetPathName() != TEXT("/Game/Structs/boardMats.boardMats")
			|| Assets.MobileBoardTable->GetPathName() != TEXT("/Game/Structs/boardMats_Mobile.boardMats_Mobile")
			|| Assets.MaterialSwitcher->GetPathName() != TEXT("/Game/Blueprints/Components/BPC_MaterialSwitcher.BPC_MaterialSwitcher")
			|| Assets.LevelManager->GetPathName() != TEXT("/Game/Blueprints/Logic/BP_LevelManager.BP_LevelManager"))
		{
			OutError = TEXT("One or more in-memory dependency objects have an unexpected package/object path.");
			return false;
		}
		if (!VerifyMaterialSwitcherShell(Assets.MaterialSwitcher, Assets.MaterialSwapStruct, OutError)
			|| !VerifyLevelManagerShell(Assets.LevelManager, OutError))
		{
			return false;
		}
		return true;
	}

	FString GetAssetFilename(const FString& PackageName)
	{
		return FPackageName::LongPackageNameToFilename(
			PackageName,
			FPackageName::GetAssetPackageExtension());
	}

	void DeletePackageFiles(const FString& PackageName)
	{
		FString Base = GetAssetFilename(PackageName);
		Base.RemoveFromEnd(FPackageName::GetAssetPackageExtension());
		for (const TCHAR* Extension : {TEXT("uasset"), TEXT("uexp"), TEXT("ubulk"), TEXT("uptnl")})
		{
			const FString Filename = Base + TEXT(".") + Extension;
			if (IFileManager::Get().FileExists(*Filename))
			{
				IFileManager::Get().Delete(*Filename, false, true, true);
			}
		}
	}

	bool SaveAllAssets(const FBuiltAssets& Assets, FString& OutError)
	{
		for (UObject* Asset : Assets.SaveOrder)
		{
			if (Asset == nullptr || Asset->GetOutermost() == nullptr)
			{
				OutError = TEXT("A dependency asset has no package during Write preflight.");
				return false;
			}
			const FString PackageName = Asset->GetOutermost()->GetName();
			FString ExistingFilename;
			if (FPackageName::DoesPackageExist(PackageName, &ExistingFilename))
			{
				OutError = FString::Printf(TEXT("Package appeared after initial preflight; refusing to overwrite %s."), *PackageName);
				return false;
			}
		}

		TArray<FString> SavedPackages;
		for (int32 Index = 0; Index < Assets.SaveOrder.Num(); ++Index)
		{
			UObject* Asset = Assets.SaveOrder[Index];
			UPackage* Package = Asset->GetOutermost();
			const FString Filename = GetAssetFilename(Package->GetName());
			IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);

			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.SaveFlags = SAVE_NoError;
			SaveArgs.bSlowTask = false;
			if (!UPackage::SavePackage(Package, Asset, *Filename, SaveArgs))
			{
				OutError = FString::Printf(TEXT("Package save failed for %s; rolling back files created by this batch."), *Package->GetName());
				for (const FString& SavedPackage : SavedPackages)
				{
					DeletePackageFiles(SavedPackage);
				}
				DeletePackageFiles(Package->GetName());
				return false;
			}
			SavedPackages.Add(Package->GetName());
		}
		return true;
	}

}

URestoreBPTableSchemaAssetsCommandlet::URestoreBPTableSchemaAssetsCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 URestoreBPTableSchemaAssetsCommandlet::Main(const FString& Params)
{
	using namespace BPTableSchemaAssets;

	const bool bNoSave = FParse::Param(*Params, TEXT("NoSave"));
	const bool bWrite = FParse::Param(*Params, TEXT("Write"));
	if (bNoSave == bWrite)
	{
		UE_LOG(LogBPTableSchemaAssets, Error, TEXT("Specify exactly one of -NoSave or -Write."));
		return 1;
	}

	FString SourceRoot;
	if (!FParse::Value(*Params, TEXT("SourceRoot="), SourceRoot) || SourceRoot.IsEmpty())
	{
		UE_LOG(LogBPTableSchemaAssets, Error, TEXT("Specify -SourceRoot=<Exports/kards/Content>."));
		return 2;
	}
	SourceRoot = FPaths::ConvertRelativePathToFull(SourceRoot);
	FString NormalizedSourceRoot = SourceRoot;
	NormalizedSourceRoot.ReplaceInline(TEXT("\\"), TEXT("/"));
	while (NormalizedSourceRoot.EndsWith(TEXT("/")))
	{
		NormalizedSourceRoot.LeftChopInline(1);
	}
	if (!FPaths::DirectoryExists(SourceRoot)
		|| !NormalizedSourceRoot.EndsWith(TEXT("/Exports/kards/Content"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogBPTableSchemaAssets, Error, TEXT("SourceRoot must end in the current Exports/kards/Content path (not kardsC/kardsD): %s"), *SourceRoot);
		return 2;
	}
	FString Error;
	FPreparedSources Sources;
	if (!PrepareSources(SourceRoot, Sources, Error))
	{
		UE_LOG(LogBPTableSchemaAssets, Error, TEXT("Source/schema validation failed: %s"), *Error);
		return 3;
	}
	if (!PreflightTargetPackagesAreAbsent(Error))
	{
		UE_LOG(LogBPTableSchemaAssets, Error, TEXT("Target package preflight failed: %s"), *Error);
		return 4;
	}

	FBuiltAssets Assets;
	if (!BuildAllAssets(Sources, Assets, Error))
	{
		UE_LOG(LogBPTableSchemaAssets, Error, TEXT("In-memory schema construction failed: %s"), *Error);
		return 5;
	}
	if (!VerifyAllBuiltAssets(Assets, Error))
	{
		UE_LOG(LogBPTableSchemaAssets, Error, TEXT("In-memory dependency verification failed: %s"), *Error);
		return 6;
	}

	if (bNoSave)
	{
		UE_LOG(
			LogBPTableSchemaAssets,
			Display,
			TEXT("NOSAVE VERIFIED: 8 exact BP_Table dependency schemas compiled in memory; board-table import flags preserve source values (omitted JSON fields use native false defaults); no package was saved and BP_Table body was not loaded or modified."));
		return 0;
	}

	if (!SaveAllAssets(Assets, Error))
	{
		UE_LOG(LogBPTableSchemaAssets, Error, TEXT("Write failed: %s"), *Error);
		return 7;
	}
	UE_LOG(
		LogBPTableSchemaAssets,
		Display,
		TEXT("WRITE VERIFIED: saved only the 8 dependency schema assets with board-table import flags preserved from source/defaults; BP_Table body was not loaded, restored, or saved."));
	return 0;
}

namespace BPTableSchemaAssets
{
	UBlueprint* BuildBlueprintShell(
		const FBlueprintSignatureDefinition& Definition,
		FString& OutError)
	{
		if (Definition.ParentClass == nullptr)
		{
			OutError = FString::Printf(TEXT("Blueprint shell %s has no exact parent class."), *Definition.PackageName);
			return nullptr;
		}
		UPackage* Package = CreatePackage(*Definition.PackageName);
		UBlueprint* Blueprint = Package == nullptr
			? nullptr
			: FKismetEditorUtilities::CreateBlueprint(
				Definition.ParentClass,
				Package,
				FName(*Definition.AssetName),
				BPTYPE_Normal,
				UBlueprint::StaticClass(),
				UBlueprintGeneratedClass::StaticClass(),
				FName(TEXT("RestoreBPTableSchemaAssets")));
		if (Blueprint == nullptr)
		{
			OutError = FString::Printf(TEXT("Could not create Blueprint signature shell %s."), *Definition.PackageName);
			return nullptr;
		}
		FAssetRegistryModule::AssetCreated(Blueprint);
		return Blueprint;
	}

	bool BuildMaterialSwitcherShell(
		const FSourceFile& SourceFile,
		const FBlueprintSignatureDefinition& Definition,
		UUserDefinedStruct* MaterialSwapStruct,
		UBlueprint*& OutBlueprint,
		FString& OutError)
	{
		const FSourceExport* ClassExport = FindExport(SourceFile, TEXT("BPC_MaterialSwitcher_C"));
		const TSharedPtr<FJsonObject> MaterialsProperty = CopyChildProperty(ClassExport, TEXT("MaterialsToSwap"));
		const TSharedPtr<FJsonObject> SimpleProperty = CopyChildProperty(ClassExport, TEXT("bSimple"));
		const TSharedPtr<FJsonObject> DispatcherProperty = CopyChildProperty(ClassExport, TEXT("OnMaterialUpdate"));
		const FSourceExport* DispatcherSignature = FindExport(SourceFile, TEXT("OnMaterialUpdate__DelegateSignature"));
		if (!MaterialsProperty.IsValid() || !SimpleProperty.IsValid() || !DispatcherProperty.IsValid() || DispatcherSignature == nullptr)
		{
			OutError = TEXT("BPC_MaterialSwitcher required member signatures are missing from source JSON.");
			return false;
		}

		OutBlueprint = BuildBlueprintShell(Definition, OutError);
		if (OutBlueprint == nullptr)
		{
			return false;
		}

		FEdGraphPinType MaterialsType;
		MaterialsType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		MaterialsType.PinSubCategoryObject = MaterialSwapStruct;
		MaterialsType.ContainerType = EPinContainerType::Array;
		if (!AddBlueprintVariable(OutBlueprint, MaterialsProperty, FName(TEXT("MaterialsToSwap")), MaterialsType, OutError))
		{
			return false;
		}
		FEdGraphPinType SimpleType;
		SimpleType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
		if (!AddBlueprintVariable(OutBlueprint, SimpleProperty, FName(TEXT("bSimple")), SimpleType, OutError))
		{
			return false;
		}
		if (!AddDelegateSignatureGraph(
				OutBlueprint,
				FName(TEXT("OnMaterialUpdate")),
				DispatcherSignature,
				{{TEXT("bSimple"), TEXT("BoolProperty")}},
				OutError)
			|| !AddZeroParameterFunctionGraph(OutBlueprint, FName(TEXT("Update")), OutError))
		{
			return false;
		}

		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(OutBlueprint);
		FKismetEditorUtilities::CompileBlueprint(OutBlueprint, EBlueprintCompileOptions::SkipSave);
		if (OutBlueprint->Status == BS_Error)
		{
			OutError = TEXT("The BPC_MaterialSwitcher signature shell did not compile.");
			return false;
		}
		OutBlueprint->MarkPackageDirty();
		return VerifyMaterialSwitcherShell(OutBlueprint, MaterialSwapStruct, OutError);
	}
}
