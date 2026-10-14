#include "RestoreEmojiSchemaShellCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/UserDefinedEnum.h"
#include "Factories/DataTableFactory.h"
#include "HAL/FileManager.h"
#include "Kismet2/EnumEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "StructUtils/UserDefinedStruct.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"

DEFINE_LOG_CATEGORY_STATIC(LogEmojiSchemaShellRestore, Log, All);

namespace EmojiSchemaShellRestore
{
	constexpr int32 OfficialEmojiRowCount = 274;
	const TCHAR* ExpectedSourceSuffix = TEXT("/Exports/kards/Content");
	const TCHAR* EnumAssetName = TEXT("E_EmoteStyle");
	const TCHAR* EnumPackageName = TEXT("/Game/Structs/E_EmoteStyle");
	const TCHAR* EnumObjectPath = TEXT("/Game/Structs/E_EmoteStyle.E_EmoteStyle");
	const TCHAR* StructAssetName = TEXT("emojiDataStructure");
	const TCHAR* StructPackageName = TEXT("/Game/Structs/emojiDataStructure");
	const TCHAR* StructObjectPath = TEXT("/Game/Structs/emojiDataStructure.emojiDataStructure");
	const TCHAR* TableAssetName = TEXT("emojiList");
	const TCHAR* TablePackageName = TEXT("/Game/Structs/emojiList");
	const TCHAR* TableObjectPath = TEXT("/Game/Structs/emojiList.emojiList");
	const TCHAR* StructGuidText = TEXT("D6B30926-431A546D-DEA2EDBC-22827824");

	struct FStyleEntry
	{
		const TCHAR* InternalName;
		const TCHAR* DisplayName;
		const TCHAR* TextKey;
		int64 Value;
	};

	const FStyleEntry StyleEntries[] = {
		{TEXT("NewEnumerator0"), TEXT("Normal"), TEXT("5C5100FC4B1C363F0ABD5CBC39008CBB"), 0},
		{TEXT("NewEnumerator1"), TEXT("SpeechBubble"), TEXT("0BE30F0D4382F0D6B0DDCF85B7757D4F"), 1}};

	enum class EEmojiFieldKind : uint8
	{
		Text,
		FactionArray,
		Group,
		ItemSource,
		Style,
		Texture,
		Color
	};

	struct FEmojiFieldSpec
	{
		const TCHAR* InternalName;
		const TCHAR* TypeName;
		EEmojiFieldKind Kind;
	};

	const FEmojiFieldSpec EmojiFields[] = {
		{TEXT("Title_14_3714C8CA47CDD69AE4BF55A42FE37387"), TEXT("TextProperty"), EEmojiFieldKind::Text},
		{TEXT("factions_18_C47996D048E5E6B540DB048FDDB700BA"), TEXT("ArrayProperty"), EEmojiFieldKind::FactionArray},
		{TEXT("group_20_A1E0286B48412E5B0E77B8B24D087D22"), TEXT("EnumProperty"), EEmojiFieldKind::Group},
		{TEXT("text_25_2F84DC8D4DD308FF2B1445834BF0F46C"), TEXT("TextProperty"), EEmojiFieldKind::Text},
		{TEXT("itemSource_28_4CAD28874EB567D9CA06C485176906E7"), TEXT("EnumProperty"), EEmojiFieldKind::ItemSource},
		{TEXT("emoteStyle_35_2650CABE4855DA54186EE7891338ADF1"), TEXT("ByteProperty"), EEmojiFieldKind::Style},
		{TEXT("emoteTexture_48_91D10C9C4ABA834F0D9E67B8BD1AF422"), TEXT("SoftObjectProperty"), EEmojiFieldKind::Texture},
		{TEXT("emoteTextureVariant_47_115A85F1404C812FA36F0483902B01CA"), TEXT("SoftObjectProperty"), EEmojiFieldKind::Texture},
		{TEXT("secondText_41_1FA8E66543F8974F5028118212D92FD8"), TEXT("TextProperty"), EEmojiFieldKind::Text},
		{TEXT("fontColor_44_A0EF84B84218A22CFDFBED8CBBACCF07"), TEXT("StructProperty"), EEmojiFieldKind::Color}};

	struct FEnumRefs
	{
		UEnum* Faction = nullptr;
		UEnum* Group = nullptr;
		UEnum* ItemSource = nullptr;
		UUserDefinedEnum* Style = nullptr;
	};

	bool TryGetString(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, FString& OutValue)
	{
		return Object.IsValid() && Object->TryGetStringField(Field, OutValue);
	}

	bool LoadSingleExport(
		const FString& SourcePath,
		const TCHAR* ExpectedType,
		const TCHAR* ExpectedName,
		const TCHAR* ExpectedPackage,
		TSharedPtr<FJsonObject>& OutRoot,
		FString& OutError)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *SourcePath))
		{
			OutError = FString::Printf(TEXT("Cannot read source export: %s"), *SourcePath);
			return false;
		}

		TArray<TSharedPtr<FJsonValue>> RootValues;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, RootValues) || RootValues.Num() != 1
			|| !RootValues[0].IsValid() || RootValues[0]->Type != EJson::Object)
		{
			OutError = FString::Printf(TEXT("Expected one JSON export object in %s"), *SourcePath);
			return false;
		}

		OutRoot = RootValues[0]->AsObject();
		FString Type;
		FString Name;
		FString Package;
		if (!TryGetString(OutRoot, TEXT("Type"), Type)
			|| !TryGetString(OutRoot, TEXT("Name"), Name)
			|| !TryGetString(OutRoot, TEXT("Package"), Package)
			|| Type != ExpectedType || Name != ExpectedName || Package != ExpectedPackage)
		{
			OutError = FString::Printf(TEXT("Export identity mismatch for %s"), ExpectedName);
			return false;
		}
		return true;
	}

	bool LoadAndValidateSources(const FString& SourceRoot, FString& OutError)
	{
		FString NormalizedRoot = SourceRoot;
		NormalizedRoot.ReplaceInline(TEXT("\\"), TEXT("/"));
		while (NormalizedRoot.EndsWith(TEXT("/")))
		{
			NormalizedRoot.LeftChopInline(1);
		}
		if (!FPaths::DirectoryExists(SourceRoot)
			|| !NormalizedRoot.EndsWith(ExpectedSourceSuffix, ESearchCase::IgnoreCase))
		{
			OutError = FString::Printf(TEXT("SourceRoot must be the current Exports/kards/Content directory: %s"), *SourceRoot);
			return false;
		}

		TSharedPtr<FJsonObject> EnumRoot;
		TSharedPtr<FJsonObject> StructRoot;
		TSharedPtr<FJsonObject> TableRoot;
		if (!LoadSingleExport(
				FPaths::Combine(SourceRoot, TEXT("Structs/E_EmoteStyle.json")),
				TEXT("UserDefinedEnum"), EnumAssetName, TEXT("kards/Content/Structs/E_EmoteStyle"), EnumRoot, OutError)
			|| !LoadSingleExport(
				FPaths::Combine(SourceRoot, TEXT("Structs/emojiDataStructure.json")),
				TEXT("UserDefinedStruct"), StructAssetName, TEXT("kards/Content/Structs/emojiDataStructure"), StructRoot, OutError)
			|| !LoadSingleExport(
				FPaths::Combine(SourceRoot, TEXT("Structs/emojiList.json")),
				TEXT("DataTable"), TableAssetName, TEXT("kards/Content/Structs/emojiList"), TableRoot, OutError))
		{
			return false;
		}

		const TSharedPtr<FJsonObject> EnumNames = EnumRoot->GetObjectField(TEXT("Names"));
		const TSharedPtr<FJsonObject> EnumProperties = EnumRoot->GetObjectField(TEXT("Properties"));
		const TArray<TSharedPtr<FJsonValue>>* DisplayNameMap = nullptr;
		if (!EnumNames.IsValid() || !EnumProperties.IsValid()
			|| !EnumProperties->TryGetArrayField(TEXT("DisplayNameMap"), DisplayNameMap)
			|| DisplayNameMap == nullptr || DisplayNameMap->Num() != UE_ARRAY_COUNT(StyleEntries))
		{
			OutError = TEXT("E_EmoteStyle export lacks the expected two values/display names.");
			return false;
		}
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(StyleEntries); ++Index)
		{
			const FString ValueKey = FString::Printf(TEXT("E_EmoteStyle::%s"), StyleEntries[Index].InternalName);
			const TSharedPtr<FJsonValue>* Value = EnumNames->Values.Find(ValueKey);
			const TSharedPtr<FJsonObject> DisplayEntry = (*DisplayNameMap)[Index].IsValid()
				? (*DisplayNameMap)[Index]->AsObject() : nullptr;
			const TSharedPtr<FJsonObject> DisplayText = DisplayEntry.IsValid()
				? DisplayEntry->GetObjectField(TEXT("Value")) : nullptr;
			FString Key;
			FString NameSpace;
			FString TextKey;
			FString SourceString;
			if (Value == nullptr || !Value->IsValid() || (*Value)->AsNumber() != StyleEntries[Index].Value
				|| !DisplayEntry.IsValid() || !TryGetString(DisplayEntry, TEXT("Key"), Key)
				|| !DisplayText.IsValid() || !TryGetString(DisplayText, TEXT("Namespace"), NameSpace)
				|| !TryGetString(DisplayText, TEXT("Key"), TextKey)
				|| !TryGetString(DisplayText, TEXT("SourceString"), SourceString)
				|| Key != StyleEntries[Index].InternalName || !NameSpace.IsEmpty()
				|| TextKey != StyleEntries[Index].TextKey || SourceString != StyleEntries[Index].DisplayName)
			{
				OutError = FString::Printf(TEXT("E_EmoteStyle entry mismatch at index %d."), Index);
				return false;
			}
		}
		const TSharedPtr<FJsonValue>* MaxValue = EnumNames->Values.Find(TEXT("E_EmoteStyle::E_MAX"));
		if (MaxValue == nullptr || !MaxValue->IsValid() || (*MaxValue)->AsNumber() != UE_ARRAY_COUNT(StyleEntries))
		{
			OutError = TEXT("E_EmoteStyle export has an unexpected MAX sentinel.");
			return false;
		}

		const TSharedPtr<FJsonObject> StructProperties = StructRoot->GetObjectField(TEXT("Properties"));
		FString StructGuid;
		const TArray<TSharedPtr<FJsonValue>>* ChildProperties = nullptr;
		if (!StructProperties.IsValid()
			|| !TryGetString(StructProperties, TEXT("Guid"), StructGuid)
			|| StructGuid != StructGuidText
			|| !StructRoot->TryGetArrayField(TEXT("ChildProperties"), ChildProperties)
			|| ChildProperties == nullptr || ChildProperties->Num() != UE_ARRAY_COUNT(EmojiFields))
		{
			OutError = TEXT("emojiDataStructure export GUID/field count mismatch.");
			return false;
		}
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(EmojiFields); ++Index)
		{
			const TSharedPtr<FJsonObject> Property = (*ChildProperties)[Index].IsValid()
				? (*ChildProperties)[Index]->AsObject() : nullptr;
			FString FieldName;
			FString FieldType;
			FString FieldFlags;
			if (!Property.IsValid() || !TryGetString(Property, TEXT("Name"), FieldName)
				|| !TryGetString(Property, TEXT("Type"), FieldType)
				|| !TryGetString(Property, TEXT("PropertyFlags"), FieldFlags)
				|| FieldName != EmojiFields[Index].InternalName
				|| FieldType != EmojiFields[Index].TypeName
				|| !FieldFlags.Contains(TEXT("Edit"))
				|| !FieldFlags.Contains(TEXT("BlueprintVisible")))
			{
				OutError = FString::Printf(TEXT("emojiDataStructure source field %d does not match the exact exported schema."), Index);
				return false;
			}

			const auto ValidateRef = [&OutError](
				const TSharedPtr<FJsonObject>& Ref,
				const TCHAR* ObjectName,
				const TCHAR* ObjectPath,
				const FString& Context) -> bool
			{
				FString ActualName;
				FString ActualPath;
				if (Ref.IsValid() && TryGetString(Ref, TEXT("ObjectName"), ActualName)
					&& TryGetString(Ref, TEXT("ObjectPath"), ActualPath)
					&& ActualName == ObjectName && ActualPath == ObjectPath)
				{
					return true;
				}
				OutError = FString::Printf(TEXT("emojiDataStructure dependency reference mismatch: %s"), *Context);
				return false;
			};

			if (EmojiFields[Index].Kind == EEmojiFieldKind::FactionArray)
			{
				const TSharedPtr<FJsonObject> Inner = Property->GetObjectField(TEXT("Inner"));
				const TSharedPtr<FJsonObject> EnumRef = Inner.IsValid() ? Inner->GetObjectField(TEXT("Enum")) : nullptr;
				FString InnerType;
				if (!TryGetString(Inner, TEXT("Type"), InnerType) || InnerType != TEXT("EnumProperty")
					|| !ValidateRef(EnumRef, TEXT("Enum'EFactionEnum'"), TEXT("/Script/KardsCore"), TEXT("factions EFactionEnum")))
				{
					return false;
				}
			}
			else if (EmojiFields[Index].Kind == EEmojiFieldKind::Group
				|| EmojiFields[Index].Kind == EEmojiFieldKind::ItemSource)
			{
				const TSharedPtr<FJsonObject> EnumRef = Property->GetObjectField(TEXT("Enum"));
				const TCHAR* EnumName = EmojiFields[Index].Kind == EEmojiFieldKind::Group
					? TEXT("Enum'EEmoteGroupEnum'") : TEXT("Enum'EItemSource'");
				if (!ValidateRef(EnumRef, EnumName, TEXT("/Script/kards"), FieldName))
				{
					return false;
				}
			}
			else if (EmojiFields[Index].Kind == EEmojiFieldKind::Style)
			{
				const TSharedPtr<FJsonObject> EnumRef = Property->GetObjectField(TEXT("Enum"));
				if (!ValidateRef(EnumRef, TEXT("UserDefinedEnum'E_EmoteStyle'"),
					TEXT("kards/Content/Structs/E_EmoteStyle.0"), FieldName))
				{
					return false;
				}
			}
			else if (EmojiFields[Index].Kind == EEmojiFieldKind::Texture)
			{
				const TSharedPtr<FJsonObject> ClassRef = Property->GetObjectField(TEXT("PropertyClass"));
				if (!ValidateRef(ClassRef, TEXT("Class'Texture2D'"), TEXT("/Script/Engine"), FieldName))
				{
					return false;
				}
			}
			else if (EmojiFields[Index].Kind == EEmojiFieldKind::Color)
			{
				const TSharedPtr<FJsonObject> StructRef = Property->GetObjectField(TEXT("Struct"));
				if (!ValidateRef(StructRef, TEXT("ScriptStruct'Color'"), TEXT("/Script/CoreUObject"), FieldName))
				{
					return false;
				}
			}
		}

		const TSharedPtr<FJsonObject> TableProperties = TableRoot->GetObjectField(TEXT("Properties"));
		const TSharedPtr<FJsonObject> RowStruct = TableProperties.IsValid()
			? TableProperties->GetObjectField(TEXT("RowStruct")) : nullptr;
		const TSharedPtr<FJsonObject> Rows = TableRoot->GetObjectField(TEXT("Rows"));
		FString RowStructName;
		FString RowStructPath;
		if (!TryGetString(RowStruct, TEXT("ObjectName"), RowStructName)
			|| !TryGetString(RowStruct, TEXT("ObjectPath"), RowStructPath)
			|| RowStructName != TEXT("UserDefinedStruct'emojiDataStructure'")
			|| RowStructPath != TEXT("kards/Content/Structs/emojiDataStructure.0")
			|| !Rows.IsValid() || Rows->Values.Num() != OfficialEmojiRowCount)
		{
			OutError = TEXT("emojiList source must have the exact emojiDataStructure RowStruct and all 274 official rows.");
			return false;
		}
		return true;
	}

	bool ParseFieldGuid(const FString& InternalName, FGuid& OutGuid, FString& OutFriendlyName)
	{
		int32 GuidSeparator = INDEX_NONE;
		if (!InternalName.FindLastChar(TEXT('_'), GuidSeparator))
		{
			return false;
		}
		FString GuidDigits = InternalName.Mid(GuidSeparator + 1);
		if (GuidDigits.Len() != 32 || !FGuid::ParseExact(GuidDigits, EGuidFormats::Digits, OutGuid))
		{
			return false;
		}
		const int32 IndexSeparator = InternalName.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, GuidSeparator - 1);
		if (IndexSeparator == INDEX_NONE)
		{
			return false;
		}
		OutFriendlyName = InternalName.Left(IndexSeparator);
		return !OutFriendlyName.IsEmpty();
	}

	bool HasEnumMember(UEnum* Enum, const TCHAR* EnumName, const TCHAR* MemberName, FString& OutError)
	{
		if (Enum == nullptr)
		{
			OutError = FString::Printf(TEXT("Required native enum is unavailable: %s"), EnumName);
			return false;
		}
		const FString AuthoredName = FString::Printf(TEXT("%s::%s"), EnumName, MemberName);
		if (Enum->GetValueByNameString(AuthoredName, EGetByNameFlags::CheckAuthoredName) == INDEX_NONE)
		{
			OutError = FString::Printf(TEXT("Native enum %s lacks required member %s."), EnumName, MemberName);
			return false;
		}
		return true;
	}

	UEnum* LoadNativeEnum(const TCHAR* ObjectPath, const TCHAR* ExpectedPath, const TCHAR* EnumName, FString& OutError)
	{
		UEnum* Enum = LoadObject<UEnum>(nullptr, ObjectPath, nullptr, LOAD_NoWarn);
		if (Enum == nullptr || Enum->GetPathName() != ExpectedPath)
		{
			OutError = FString::Printf(TEXT("Exact native enum dependency is unavailable: %s"), ObjectPath);
			return nullptr;
		}
		return Enum;
	}

	bool ResolveEnumRefs(UUserDefinedEnum* Style, FEnumRefs& OutEnums, FString& OutError)
	{
		OutEnums.Style = Style;
		OutEnums.Faction = LoadNativeEnum(TEXT("/Script/KardsCore.EFactionEnum"),
			TEXT("/Script/KardsCore.EFactionEnum"), TEXT("EFactionEnum"), OutError);
		if (OutEnums.Faction == nullptr) return false;
		OutEnums.Group = LoadNativeEnum(TEXT("/Script/kards.EEmoteGroupEnum"),
			TEXT("/Script/kards.EEmoteGroupEnum"), TEXT("EEmoteGroupEnum"), OutError);
		if (OutEnums.Group == nullptr) return false;
		OutEnums.ItemSource = LoadNativeEnum(TEXT("/Script/kards.EItemSource"),
			TEXT("/Script/kards.EItemSource"), TEXT("EItemSource"), OutError);
		if (OutEnums.ItemSource == nullptr) return false;

		for (const TCHAR* Name : {TEXT("NotAvailable"), TEXT("Anzac"), TEXT("Britain"), TEXT("Germany"), TEXT("Japan"), TEXT("Soviet"), TEXT("USA")})
		{
			if (!HasEnumMember(OutEnums.Faction, TEXT("EFactionEnum"), Name, OutError)) return false;
		}
		for (const TCHAR* Name : {TEXT("NotAvailable"), TEXT("Start"), TEXT("Good"), TEXT("Bad"), TEXT("Taunt"), TEXT("Poke"), TEXT("Cheer"), TEXT("Proclaim"), TEXT("End")})
		{
			if (!HasEnumMember(OutEnums.Group, TEXT("EEmoteGroupEnum"), Name, OutError)) return false;
		}
		for (const TCHAR* Name : {TEXT("NotAvailable"), TEXT("StarterItem"), TEXT("PurchasableItem"), TEXT("RewardedItem")})
		{
			if (!HasEnumMember(OutEnums.ItemSource, TEXT("EItemSource"), Name, OutError)) return false;
		}
		return true;
	}

	UUserDefinedEnum* BuildStyleEnum(FString& OutError)
	{
		UPackage* Package = CreatePackage(EnumPackageName);
		UUserDefinedEnum* Enum = Package == nullptr ? nullptr : Cast<UUserDefinedEnum>(
			FEnumEditorUtils::CreateUserDefinedEnum(Package, EnumAssetName, RF_Public | RF_Standalone | RF_Transactional));
		if (Enum == nullptr)
		{
			OutError = TEXT("Could not create /Game/Structs/E_EmoteStyle.");
			return nullptr;
		}

		TArray<TPair<FName, int64>> Values;
		Values.Reserve(UE_ARRAY_COUNT(StyleEntries));
		for (const FStyleEntry& Entry : StyleEntries)
		{
			Values.Emplace(FName(*Enum->GenerateFullEnumName(Entry.InternalName)), Entry.Value);
		}
		if (!Enum->SetEnums(Values, UEnum::ECppForm::Namespaced))
		{
			OutError = TEXT("Could not set exact E_EmoteStyle enumerator names.");
			return nullptr;
		}
		Enum->UniqueNameIndex = UE_ARRAY_COUNT(StyleEntries);
		Enum->DisplayNameMap.Reset();
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(StyleEntries); ++Index)
		{
			const FName EntryName(*Enum->GetNameStringByIndex(Index));
			Enum->DisplayNameMap.Add(EntryName, FText::ChangeKey(
				TEXT(""), StyleEntries[Index].TextKey, FText::FromString(StyleEntries[Index].DisplayName)));
		}
		Enum->SetMetaData(TEXT("BlueprintType"), TEXT("true"));
		Enum->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(Enum);
		if (Enum->GetPathName() != EnumObjectPath || Enum->NumEnums() != UE_ARRAY_COUNT(StyleEntries) + 1)
		{
			OutError = TEXT("Built E_EmoteStyle path or MAX sentinel is incorrect.");
			return nullptr;
		}
		return Enum;
	}

	FEdGraphPinType MakePinType(const EEmojiFieldKind Kind, const FEnumRefs& Enums)
	{
		FEdGraphPinType Type;
		switch (Kind)
		{
		case EEmojiFieldKind::Text:
			Type.PinCategory = UEdGraphSchema_K2::PC_Text;
			break;
		case EEmojiFieldKind::FactionArray:
			Type.PinCategory = UEdGraphSchema_K2::PC_Byte;
			Type.PinSubCategoryObject = Enums.Faction;
			Type.ContainerType = EPinContainerType::Array;
			break;
		case EEmojiFieldKind::Group:
			Type.PinCategory = UEdGraphSchema_K2::PC_Byte;
			Type.PinSubCategoryObject = Enums.Group;
			break;
		case EEmojiFieldKind::ItemSource:
			Type.PinCategory = UEdGraphSchema_K2::PC_Byte;
			Type.PinSubCategoryObject = Enums.ItemSource;
			break;
		case EEmojiFieldKind::Style:
			Type.PinCategory = UEdGraphSchema_K2::PC_Byte;
			Type.PinSubCategoryObject = Enums.Style;
			break;
		case EEmojiFieldKind::Texture:
			Type.PinCategory = UEdGraphSchema_K2::PC_SoftObject;
			Type.PinSubCategoryObject = UTexture2D::StaticClass();
			break;
		case EEmojiFieldKind::Color:
			Type.PinCategory = UEdGraphSchema_K2::PC_Struct;
			Type.PinSubCategoryObject = TBaseStructure<FColor>::Get();
			break;
		default:
			checkNoEntry();
		}
		return Type;
	}

	UUserDefinedStruct* BuildEmojiStruct(const FEnumRefs& Enums, FString& OutError)
	{
		UPackage* Package = CreatePackage(StructPackageName);
		UUserDefinedStruct* Struct = Package == nullptr ? nullptr : FStructureEditorUtils::CreateUserDefinedStruct(
			Package, StructAssetName, RF_Public | RF_Standalone | RF_Transactional);
		if (Struct == nullptr)
		{
			OutError = TEXT("Could not create /Game/Structs/emojiDataStructure.");
			return nullptr;
		}

		FGuid StructGuid;
		FString StructGuidDigits = StructGuidText;
		StructGuidDigits.ReplaceInline(TEXT("-"), TEXT(""));
		if (!FGuid::ParseExact(StructGuidDigits, EGuidFormats::Digits, StructGuid))
		{
			OutError = TEXT("Configured emojiDataStructure GUID is invalid.");
			return nullptr;
		}
		Struct->Guid = StructGuid;

		for (const FEmojiFieldSpec& Field : EmojiFields)
		{
			if (!FStructureEditorUtils::AddVariable(Struct, MakePinType(Field.Kind, Enums)))
			{
				OutError = FString::Printf(TEXT("Could not add field %s."), Field.InternalName);
				return nullptr;
			}
			TArray<FStructVariableDescription>& Variables = FStructureEditorUtils::GetVarDesc(Struct);
			if (Variables.IsEmpty())
			{
				OutError = FString::Printf(TEXT("UDS editor data lost field %s."), Field.InternalName);
				return nullptr;
			}
			FStructVariableDescription& Variable = Variables.Last();
			Variable.VarName = FName(Field.InternalName);
			Variable.VarGuid = FGuid();
			if (!ParseFieldGuid(Field.InternalName, Variable.VarGuid, Variable.FriendlyName))
			{
				OutError = FString::Printf(TEXT("Could not recover field GUID/name from %s."), Field.InternalName);
				return nullptr;
			}
		}

		FStructureEditorUtils::CompileStructure(Struct);
		if (Struct->Status != EUserDefinedStructureStatus::UDSS_UpToDate)
		{
			OutError = FString::Printf(TEXT("emojiDataStructure compilation failed: %s"), *Struct->ErrorMessage);
			return nullptr;
		}
		Struct->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(Struct);
		return Struct;
	}

	UDataTable* BuildEmptyEmojiList(UUserDefinedStruct* Struct, FString& OutError)
	{
		UPackage* Package = CreatePackage(TablePackageName);
		UDataTableFactory* Factory = NewObject<UDataTableFactory>();
		Factory->Struct = Struct;
		UDataTable* Table = Package == nullptr ? nullptr : Cast<UDataTable>(Factory->FactoryCreateNew(
			UDataTable::StaticClass(), Package, TableAssetName, RF_Public | RF_Standalone | RF_Transactional, nullptr, GWarn));
		if (Table == nullptr || Table->GetRowStruct() != Struct)
		{
			OutError = TEXT("Could not create emojiList with the exact emojiDataStructure RowStruct.");
			return nullptr;
		}
		Table->bIgnoreExtraFields = false;
		Table->bIgnoreMissingFields = false;
		Table->EmptyTable();
		Table->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(Table);
		return Table;
	}

	bool VerifyAssets(UUserDefinedEnum* Style, UUserDefinedStruct* Struct, UDataTable* Table, const FEnumRefs& Enums, FString& OutError)
	{
		FString StructGuidDigits = StructGuidText;
		StructGuidDigits.ReplaceInline(TEXT("-"), TEXT(""));
		FGuid ExpectedStructGuid;
		const bool bExpectedStructGuidValid = FGuid::ParseExact(StructGuidDigits, EGuidFormats::Digits, ExpectedStructGuid);
		if (Style == nullptr || Struct == nullptr || Table == nullptr
			|| Style->GetPathName() != EnumObjectPath
			|| Struct->GetPathName() != StructObjectPath
			|| !bExpectedStructGuidValid || Struct->Guid != ExpectedStructGuid
			|| Struct->Status != EUserDefinedStructureStatus::UDSS_UpToDate
			|| Table->GetPathName() != TableObjectPath || Table->GetRowStruct() != Struct
			|| Table->GetRowMap().Num() != 0 || Table->bIgnoreExtraFields || Table->bIgnoreMissingFields)
		{
			OutError = TEXT("One or more emoji editor schema-shell assets failed identity/empty-table verification.");
			return false;
		}

		if (Style->NumEnums() != UE_ARRAY_COUNT(StyleEntries) + 1)
		{
			OutError = TEXT("E_EmoteStyle has an unexpected enumerator count.");
			return false;
		}
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(StyleEntries); ++Index)
		{
			if (Style->GetNameStringByIndex(Index) != StyleEntries[Index].InternalName
				|| Style->GetValueByIndex(Index) != StyleEntries[Index].Value
				|| Style->GetDisplayNameTextByIndex(Index).ToString() != StyleEntries[Index].DisplayName)
			{
				OutError = FString::Printf(TEXT("E_EmoteStyle value mismatch at index %d."), Index);
				return false;
			}
		}

		int32 FieldIndex = 0;
		for (TFieldIterator<FProperty> It(Struct, EFieldIterationFlags::None); It; ++It, ++FieldIndex)
		{
			if (FieldIndex >= UE_ARRAY_COUNT(EmojiFields) || It->GetName() != EmojiFields[FieldIndex].InternalName
				|| !It->HasAllPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
			{
				OutError = FString::Printf(TEXT("emojiDataStructure property order/name/flags mismatch at index %d."), FieldIndex);
				return false;
			}
			FProperty* Property = *It;
			switch (EmojiFields[FieldIndex].Kind)
			{
			case EEmojiFieldKind::Text:
				if (CastField<FTextProperty>(Property) == nullptr) OutError = TEXT("Text field type mismatch.");
				break;
			case EEmojiFieldKind::FactionArray:
			{
				FArrayProperty* Array = CastField<FArrayProperty>(Property);
				FEnumProperty* Inner = Array != nullptr ? CastField<FEnumProperty>(Array->Inner) : nullptr;
				if (Inner == nullptr || Inner->GetEnum() != Enums.Faction) OutError = TEXT("factions array enum type mismatch.");
				break;
			}
			case EEmojiFieldKind::Group:
		case EEmojiFieldKind::ItemSource:
		{
			FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property);
			UEnum* ExpectedEnum = EmojiFields[FieldIndex].Kind == EEmojiFieldKind::Group ? Enums.Group : Enums.ItemSource;
			if (EnumProperty == nullptr || EnumProperty->GetEnum() != ExpectedEnum) OutError = TEXT("Native enum property type mismatch.");
			break;
		}
			case EEmojiFieldKind::Style:
			{
				FByteProperty* ByteProperty = CastField<FByteProperty>(Property);
				if (ByteProperty == nullptr || ByteProperty->Enum != Style) OutError = TEXT("emoteStyle enum property type mismatch.");
				break;
			}
			case EEmojiFieldKind::Texture:
			{
				FSoftObjectProperty* SoftProperty = CastField<FSoftObjectProperty>(Property);
				if (SoftProperty == nullptr || SoftProperty->PropertyClass != UTexture2D::StaticClass()) OutError = TEXT("Soft Texture2D property type mismatch.");
				break;
			}
			case EEmojiFieldKind::Color:
			{
				FStructProperty* StructProperty = CastField<FStructProperty>(Property);
				if (StructProperty == nullptr || StructProperty->Struct != TBaseStructure<FColor>::Get()) OutError = TEXT("FColor property type mismatch.");
				break;
			}
			}
			if (!OutError.IsEmpty()) return false;
		}
		if (FieldIndex != UE_ARRAY_COUNT(EmojiFields))
		{
			OutError = FString::Printf(TEXT("emojiDataStructure has %d fields, expected %d."), FieldIndex, UE_ARRAY_COUNT(EmojiFields));
			return false;
		}
		return true;
	}

	bool CompileBPEquipmentWithoutSaving(FString& OutError)
	{
		UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_Equipment.BP_Equipment"), nullptr, LOAD_NoWarn);
		if (Blueprint == nullptr)
		{
			OutError = TEXT("Could not load local /Game/Blueprints/BP_Equipment for a no-save compile check.");
			return false;
		}
		FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipSave);
		if (Blueprint->Status == BS_Error)
		{
			OutError = TEXT("BP_Equipment still compiles with errors after restoring the exact emoji schema shells; inspect its asset log for the remaining node.");
			return false;
		}
		return true;
	}

	bool PreflightTargetsAbsent(FString& OutError)
	{
		static const TCHAR* Packages[] = {EnumPackageName, StructPackageName, TablePackageName};
		for (const TCHAR* PackageName : Packages)
		{
			FString ExistingFilename;
			if (FindPackage(nullptr, PackageName) != nullptr || FPackageName::DoesPackageExist(PackageName, &ExistingFilename))
			{
				OutError = FString::Printf(TEXT("Refusing to overwrite existing package %s%s"), PackageName,
					ExistingFilename.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" (%s)"), *ExistingFilename));
				return false;
			}
			FString AssetFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
			FString BaseFilename = AssetFilename;
			BaseFilename.RemoveFromEnd(FPackageName::GetAssetPackageExtension());
			for (const TCHAR* Extension : {TEXT("uexp"), TEXT("ubulk"), TEXT("uptnl")})
			{
				const FString Sidecar = BaseFilename + TEXT(".") + Extension;
				if (IFileManager::Get().FileExists(*Sidecar))
				{
					OutError = FString::Printf(TEXT("Refusing to touch orphaned sidecar: %s"), *Sidecar);
					return false;
				}
			}
		}
		return true;
	}

	FString AssetFilename(const FString& PackageName)
	{
		return FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	}

	void DeletePackageFilesCreatedByThisRun(const FString& PackageName)
	{
		FString Base = AssetFilename(PackageName);
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

	bool TargetFilesStillAbsent(const FString& PackageName, FString& OutError)
	{
		FString ExistingFilename;
		if (FPackageName::DoesPackageExist(PackageName, &ExistingFilename))
		{
			OutError = FString::Printf(TEXT("Target package appeared during shell creation; refusing to overwrite %s (%s)."),
				*PackageName, *ExistingFilename);
			return false;
		}
		FString Base = AssetFilename(PackageName);
		Base.RemoveFromEnd(FPackageName::GetAssetPackageExtension());
		for (const TCHAR* Extension : {TEXT("uexp"), TEXT("ubulk"), TEXT("uptnl")})
		{
			const FString Sidecar = Base + TEXT(".") + Extension;
			if (IFileManager::Get().FileExists(*Sidecar))
			{
				OutError = FString::Printf(TEXT("Target sidecar appeared during shell creation; refusing to overwrite %s."), *Sidecar);
				return false;
			}
		}
		return true;
	}

	bool SaveNewAssets(const TArray<UObject*>& Assets, FString& OutError)
	{
		TArray<FString> SavedPackages;
		for (UObject* Asset : Assets)
		{
			if (Asset == nullptr || Asset->GetOutermost() == nullptr)
			{
				OutError = TEXT("A schema shell has no package during save.");
				break;
			}
			UPackage* Package = Asset->GetOutermost();
			const FString PackageName = Package->GetName();
			if (!TargetFilesStillAbsent(PackageName, OutError))
			{
				break;
			}
			const FString Filename = AssetFilename(PackageName);
			IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
			FSavePackageArgs Args;
			Args.TopLevelFlags = RF_Public | RF_Standalone;
			Args.SaveFlags = SAVE_NoError;
			Args.bSlowTask = false;
			if (!UPackage::SavePackage(Package, Asset, *Filename, Args))
			{
				OutError = FString::Printf(TEXT("Saving %s failed; rolling back only packages created by this commandlet."), *PackageName);
				DeletePackageFilesCreatedByThisRun(PackageName);
				break;
			}
			SavedPackages.Add(PackageName);
		}
		if (!OutError.IsEmpty())
		{
			for (const FString& PackageName : SavedPackages)
			{
				DeletePackageFilesCreatedByThisRun(PackageName);
			}
			return false;
		}
		return true;
	}

}

URestoreEmojiSchemaShellCommandlet::URestoreEmojiSchemaShellCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 URestoreEmojiSchemaShellCommandlet::Main(const FString& Params)
{
	using namespace EmojiSchemaShellRestore;
	const bool bNoSave = FParse::Param(*Params, TEXT("NoSave"));
	const bool bWrite = FParse::Param(*Params, TEXT("Write"));
	const bool bVerifyOnly = FParse::Param(*Params, TEXT("VerifyOnly"));
	if (static_cast<int32>(bNoSave) + static_cast<int32>(bWrite) + static_cast<int32>(bVerifyOnly) != 1)
	{
		UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Specify exactly one of -NoSave, -Write, or -VerifyOnly."));
		return 1;
	}

	FString SourceRoot;
	if (!FParse::Value(*Params, TEXT("SourceRoot="), SourceRoot) || SourceRoot.IsEmpty())
	{
		UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Specify -SourceRoot=<current Exports/kards/Content>."));
		return 2;
	}
	SourceRoot = FPaths::ConvertRelativePathToFull(SourceRoot);
	FString Error;
	if (!LoadAndValidateSources(SourceRoot, Error))
	{
		UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Source validation failed: %s"), *Error);
		return 3;
	}

	UUserDefinedEnum* Style = nullptr;
	UUserDefinedStruct* Struct = nullptr;
	UDataTable* Table = nullptr;
	FEnumRefs Enums;
	if (bVerifyOnly)
	{
		Style = LoadObject<UUserDefinedEnum>(nullptr, EnumObjectPath, nullptr, LOAD_NoWarn);
		Struct = LoadObject<UUserDefinedStruct>(nullptr, StructObjectPath, nullptr, LOAD_NoWarn);
		Table = LoadObject<UDataTable>(nullptr, TableObjectPath, nullptr, LOAD_NoWarn);
		if (!ResolveEnumRefs(Style, Enums, Error))
		{
			UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Native enum verification failed: %s"), *Error);
			return 4;
		}
		if (!VerifyAssets(Style, Struct, Table, Enums, Error))
		{
			UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Asset verification failed: %s"), *Error);
			return 5;
		}
		if (!CompileBPEquipmentWithoutSaving(Error))
		{
			UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("BP_Equipment compile check failed: %s"), *Error);
			return 11;
		}
		UE_LOG(LogEmojiSchemaShellRestore, Display, TEXT("VERIFIED: editor-only E_EmoteStyle, emojiDataStructure and empty emojiList shells; no files were changed."));
		return 0;
	}

	if (!PreflightTargetsAbsent(Error))
	{
		UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Target preflight failed; no assets created: %s"), *Error);
		return 6;
	}
	Style = BuildStyleEnum(Error);
	if (Style == nullptr || !ResolveEnumRefs(Style, Enums, Error))
	{
		UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Exact enum dependency/build failed; no package saved: %s"), *Error);
		return 7;
	}
	Struct = BuildEmojiStruct(Enums, Error);
	if (Struct == nullptr)
	{
		UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Struct shell build failed; no package saved: %s"), *Error);
		return 8;
	}
	Table = BuildEmptyEmojiList(Struct, Error);
	if (Table == nullptr || !VerifyAssets(Style, Struct, Table, Enums, Error))
	{
		UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Empty DataTable shell verification failed; no package saved: %s"), *Error);
		return 9;
	}
	if (!CompileBPEquipmentWithoutSaving(Error))
	{
		UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("BP_Equipment compile check failed; no package saved: %s"), *Error);
		return 11;
	}

	if (bNoSave)
	{
		UE_LOG(LogEmojiSchemaShellRestore, Display, TEXT("NOSAVE VERIFIED: exact enum/UDS signatures plus empty emojiList; no package was written. PAK inclusion remains disabled by design."));
		return 0;
	}

	TArray<UObject*> AssetsToSave;
	AssetsToSave.Add(Style);
	AssetsToSave.Add(Struct);
	AssetsToSave.Add(Table);
	if (!SaveNewAssets(AssetsToSave, Error))
	{
		UE_LOG(LogEmojiSchemaShellRestore, Error, TEXT("Shell save failed; packages from this run were rolled back after the first save error: %s"), *Error);
		return 10;
	}
	UE_LOG(LogEmojiSchemaShellRestore, Display, TEXT("WRITE VERIFIED: saved editor-only schemas at the original /Game/Structs paths with emojiList empty. Exclude all three packages and sidecars from any mod PAK."));
	return 0;
}
