#include "RestoreBoardItems2TableCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "BoardItem2EditorShim.h"
#include "Engine/DataTable.h"
#include "Factories/DataTableFactory.h"
#include "HAL/FileManager.h"
#include "Internationalization/Text.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/StructOnScope.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogBoardItems2TableRestore, Log, All);

namespace BoardItems2TableRestore
{
	constexpr int32 ExpectedRowCount = 105;
	constexpr int32 ExpectedChildItemCount = 122;
	constexpr int32 ExpectedTransformCount = 488;
	constexpr TCHAR TableAssetName[] = TEXT("BoardItems2");
	constexpr TCHAR TablePackageName[] = TEXT("/Game/Structs/BoardItems2");
	constexpr TCHAR TableObjectPath[] = TEXT("/Game/Structs/BoardItems2.BoardItems2");

	struct FTextIdentity
	{
		FString Namespace;
		FString Key;
		FString SourceString;
		FText Value;
	};

	struct FTransformIdentity
	{
		FTransform Value = FTransform::Identity;
	};

	struct FChildIdentity
	{
		FBoardChildItem2 Value;
		FTransformIdentity MyTransform;
		FTransformIdentity EnemyTransform;
		FTransformIdentity MyTabletTransform;
		FTransformIdentity EnemyTabletTransform;
	};

	struct FRowIdentity
	{
		FName RowName;
		FString RowNameText;
		FBoardItem2 Value;
		FTextIdentity Title;
	};

	struct FBoardItemsSource
	{
		FString FullPath;
		TArray<FRowIdentity> Rows;
		int32 ChildItemCount = 0;
		int32 TransformCount = 0;
	};

	bool ValidateShimSchema(FString& OutError);
	bool IsEnumPropertyFor(const FProperty* Property, const UEnum* ExpectedEnum);

	TSharedPtr<FJsonObject> GetObjectField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		const TSharedPtr<FJsonObject>* Value = nullptr;
		return Object.IsValid() && Object->TryGetObjectField(FieldName, Value) && Value != nullptr
			? *Value
			: nullptr;
	}

	bool TryParseInteger(const TSharedPtr<FJsonValue>& Value, int64& OutValue)
	{
		if (!Value.IsValid() || Value->Type != EJson::Number)
		{
			return false;
		}
		const double Number = Value->AsNumber();
		// Use an exclusive upper bound because MAX_int64 rounds up to 2^63 as a double.
		if (!FMath::IsFinite(Number)
			|| FMath::FloorToDouble(Number) != Number
			|| Number < -9223372036854775808.0
			|| Number >= 9223372036854775808.0)
		{
			return false;
		}
		OutValue = static_cast<int64>(Number);
		return true;
	}

	bool RequiredString(
		const TSharedPtr<FJsonObject>& Object,
		const TCHAR* FieldName,
		FString& OutValue,
		FString& OutError)
	{
		if (!Object.IsValid() || !Object->TryGetStringField(FieldName, OutValue))
		{
			OutError = FString::Printf(TEXT("Required string '%s' is missing."), FieldName);
			return false;
		}
		return true;
	}

	bool RequiredNumber(
		const TSharedPtr<FJsonObject>& Object,
		const TCHAR* FieldName,
		double& OutValue,
		FString& OutError)
	{
		if (!Object.IsValid() || !Object->TryGetNumberField(FieldName, OutValue) || !FMath::IsFinite(OutValue))
		{
			OutError = FString::Printf(TEXT("Required finite number '%s' is missing."), FieldName);
			return false;
		}
		return true;
	}

	TSharedPtr<FJsonObject> AsObject(const TSharedPtr<FJsonValue>& Value)
	{
		return Value.IsValid() && Value->Type == EJson::Object ? Value->AsObject() : nullptr;
	}

	bool ValidateExactKeys(
		const TSharedPtr<FJsonObject>& Object,
		const TArray<FString>& ExpectedKeys,
		const FString& Context,
		FString& OutError)
	{
		if (!Object.IsValid() || Object->Values.Num() != ExpectedKeys.Num())
		{
			OutError = FString::Printf(TEXT("%s has %d fields; expected exactly %d."), *Context, Object.IsValid() ? Object->Values.Num() : 0, ExpectedKeys.Num());
			return false;
		}
		for (const FString& Key : ExpectedKeys)
		{
			if (!Object->Values.Contains(Key))
			{
				OutError = FString::Printf(TEXT("%s is missing required field '%s'."), *Context, *Key);
				return false;
			}
		}
		return true;
	}

	bool ValidateSoftPath(
		const TSharedPtr<FJsonObject>& Object,
		FSoftObjectPath& OutPath,
		FString& OutError,
		const FString& Context)
	{
		FString AssetPathName;
		FString SubPathString;
		if (!RequiredString(Object, TEXT("AssetPathName"), AssetPathName, OutError)
			|| !RequiredString(Object, TEXT("SubPathString"), SubPathString, OutError))
		{
			OutError = FString::Printf(TEXT("%s: %s"), *Context, *OutError);
			return false;
		}
		if (!SubPathString.IsEmpty())
		{
			OutError = FString::Printf(TEXT("%s has unsupported subobject path '%s'."), *Context, *SubPathString);
			return false;
		}
		if (AssetPathName.IsEmpty())
		{
			OutPath = FSoftObjectPath();
			return true;
		}
		if (!AssetPathName.StartsWith(TEXT("/Game/")) || !AssetPathName.Contains(TEXT(".")))
		{
			OutError = FString::Printf(TEXT("%s soft reference is not an exact /Game/package.object path: %s"), *Context, *AssetPathName);
			return false;
		}
		OutPath = FSoftObjectPath(AssetPathName);
		if (!OutPath.IsValid())
		{
			OutError = FString::Printf(TEXT("%s soft reference is invalid: %s"), *Context, *AssetPathName);
			return false;
		}
		return true;
	}

	bool ResolveEnumValue(const UEnum* Enum, const FString& SerializedValue, int64& OutValue)
	{
		if (Enum == nullptr || SerializedValue.IsEmpty())
		{
			return false;
		}
		OutValue = Enum->GetValueByNameString(SerializedValue, EGetByNameFlags::CheckAuthoredName);
		if (OutValue == INDEX_NONE)
		{
			FString Prefix;
			FString Suffix;
			if (SerializedValue.Split(TEXT("::"), &Prefix, &Suffix, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
			{
				OutValue = Enum->GetValueByNameString(Suffix, EGetByNameFlags::CheckAuthoredName);
			}
		}
		return OutValue != INDEX_NONE && Enum->IsValidEnumValue(OutValue);
	}

	bool ParseEnumValue(
		const UEnum* Enum,
		const TSharedPtr<FJsonValue>& Value,
		int64& OutValue,
		const FString& Context,
		FString& OutError)
	{
		FString Serialized;
		if (!Value.IsValid() || !Value->TryGetString(Serialized)
			|| !ResolveEnumValue(Enum, Serialized, OutValue))
		{
			OutError = FString::Printf(TEXT("%s contains invalid enum value '%s'."), *Context, *Serialized);
			return false;
		}
		return true;
	}

	bool ParseFTextIdentity(
		const TSharedPtr<FJsonValue>& Value,
		FTextIdentity& OutText,
		const FString& Context,
		FString& OutError)
	{
		const TSharedPtr<FJsonObject> Object = AsObject(Value);
		static const TArray<FString> ExpectedKeys = {
			TEXT("Namespace"), TEXT("Key"), TEXT("SourceString"), TEXT("LocalizedString")};
		FString LocalizedString;
		if (!ValidateExactKeys(Object, ExpectedKeys, Context, OutError)
			|| !Object->TryGetStringField(TEXT("Namespace"), OutText.Namespace)
			|| !Object->TryGetStringField(TEXT("Key"), OutText.Key)
			|| !Object->TryGetStringField(TEXT("SourceString"), OutText.SourceString)
			|| !Object->TryGetStringField(TEXT("LocalizedString"), LocalizedString))
		{
			if (OutError.IsEmpty())
			{
				OutError = FString::Printf(TEXT("%s lacks the serialized FText namespace/key/source/localized string."), *Context);
			}
			return false;
		}
		if (OutText.Key.IsEmpty())
		{
			OutError = FString::Printf(TEXT("%s has an empty FText key."), *Context);
			return false;
		}
		OutText.Value = FText::ChangeKey(
			OutText.Namespace,
			OutText.Key,
			FText::FromString(OutText.SourceString));
		return true;
	}

	bool ParseTransform(
		const TSharedPtr<FJsonValue>& Value,
		FTransformIdentity& OutTransform,
		const FString& Context,
		FString& OutError)
	{
		const TSharedPtr<FJsonObject> Object = AsObject(Value);
		const TSharedPtr<FJsonObject> Rotation = Object.IsValid() ? GetObjectField(Object, TEXT("Rotation")) : nullptr;
		const TSharedPtr<FJsonObject> Translation = Object.IsValid() ? GetObjectField(Object, TEXT("Translation")) : nullptr;
		const TSharedPtr<FJsonObject> Scale = Object.IsValid() ? GetObjectField(Object, TEXT("Scale3D")) : nullptr;
		static const TArray<FString> QuaternionKeys = {TEXT("X"), TEXT("Y"), TEXT("Z"), TEXT("W"), TEXT("IsNormalized"), TEXT("Size"), TEXT("SizeSquared")};
		static const TArray<FString> VectorKeys = {TEXT("X"), TEXT("Y"), TEXT("Z")};
		if (!ValidateExactKeys(Rotation, QuaternionKeys, Context + TEXT(".Rotation"), OutError)
			|| !ValidateExactKeys(Translation, VectorKeys, Context + TEXT(".Translation"), OutError)
			|| !ValidateExactKeys(Scale, VectorKeys, Context + TEXT(".Scale3D"), OutError))
		{
			return false;
		}

		double QX = 0.0, QY = 0.0, QZ = 0.0, QW = 0.0;
		double Size = 0.0, SizeSquared = 0.0;
		double TX = 0.0, TY = 0.0, TZ = 0.0;
		double SX = 0.0, SY = 0.0, SZ = 0.0;
		bool bSourceNormalized = false;
		if (!RequiredNumber(Rotation, TEXT("X"), QX, OutError)
			|| !RequiredNumber(Rotation, TEXT("Y"), QY, OutError)
			|| !RequiredNumber(Rotation, TEXT("Z"), QZ, OutError)
			|| !RequiredNumber(Rotation, TEXT("W"), QW, OutError)
			|| !RequiredNumber(Rotation, TEXT("Size"), Size, OutError)
			|| !RequiredNumber(Rotation, TEXT("SizeSquared"), SizeSquared, OutError)
			|| !Rotation->TryGetBoolField(TEXT("IsNormalized"), bSourceNormalized)
			|| !RequiredNumber(Translation, TEXT("X"), TX, OutError)
			|| !RequiredNumber(Translation, TEXT("Y"), TY, OutError)
			|| !RequiredNumber(Translation, TEXT("Z"), TZ, OutError)
			|| !RequiredNumber(Scale, TEXT("X"), SX, OutError)
			|| !RequiredNumber(Scale, TEXT("Y"), SY, OutError)
			|| !RequiredNumber(Scale, TEXT("Z"), SZ, OutError))
		{
			if (OutError.IsEmpty()) OutError = FString::Printf(TEXT("%s has malformed transform metadata."), *Context);
			return false;
		}

		const FQuat Quaternion(QX, QY, QZ, QW);
		const double ComputedSizeSquared = Quaternion.SizeSquared();
		const double ComputedSize = FMath::Sqrt(ComputedSizeSquared);
		if (!FMath::IsNearlyEqual(ComputedSizeSquared, SizeSquared, 0.001)
			|| !FMath::IsNearlyEqual(ComputedSize, Size, 0.001)
			|| (bSourceNormalized && !FMath::IsNearlyEqual(ComputedSizeSquared, 1.0, 0.001)))
		{
			OutError = FString::Printf(TEXT("%s quaternion serialized Size/SizeSquared/IsNormalized fields disagree with X/Y/Z/W."), *Context);
			return false;
		}
		OutTransform.Value = FTransform(Quaternion, FVector(TX, TY, TZ), FVector(SX, SY, SZ));
		return true;
	}

	bool ParseBoardChild(
		const TSharedPtr<FJsonObject>& Object,
		FChildIdentity& OutChild,
		const FString& Context,
		FString& OutError)
	{
		static const TArray<FString> ExpectedKeys = {
			TEXT("StaticMesh"), TEXT("Material"), TEXT("MobileMaterial"), TEXT("boardPropActor"),
			TEXT("myTransform"), TEXT("enemyTransform"), TEXT("myTransformTabletView"),
			TEXT("enemyTransformTabletView"), TEXT("hasTabletView")};
		if (!ValidateExactKeys(Object, ExpectedKeys, Context, OutError)) return false;

		FSoftObjectPath StaticMeshPath, MaterialPath, MobileMaterialPath, PropClassPath;
		if (!ValidateSoftPath(AsObject(Object->TryGetField(TEXT("StaticMesh"))), StaticMeshPath, OutError, Context + TEXT(".StaticMesh"))
			|| !ValidateSoftPath(AsObject(Object->TryGetField(TEXT("Material"))), MaterialPath, OutError, Context + TEXT(".Material"))
			|| !ValidateSoftPath(AsObject(Object->TryGetField(TEXT("MobileMaterial"))), MobileMaterialPath, OutError, Context + TEXT(".MobileMaterial"))
			|| !ValidateSoftPath(AsObject(Object->TryGetField(TEXT("boardPropActor"))), PropClassPath, OutError, Context + TEXT(".boardPropActor")))
		{
			return false;
		}
		OutChild.Value.StaticMesh = TSoftObjectPtr<UStaticMesh>(StaticMeshPath);
		OutChild.Value.Material = TSoftObjectPtr<UMaterialInterface>(MaterialPath);
		OutChild.Value.MobileMaterial = TSoftObjectPtr<UMaterialInterface>(MobileMaterialPath);
		OutChild.Value.boardPropActor = TSoftClassPtr<AActor>(PropClassPath);

		const TSharedPtr<FJsonValue>* MyTransform = Object->Values.Find(TEXT("myTransform"));
		const TSharedPtr<FJsonValue>* EnemyTransform = Object->Values.Find(TEXT("enemyTransform"));
		const TSharedPtr<FJsonValue>* MyTabletTransform = Object->Values.Find(TEXT("myTransformTabletView"));
		const TSharedPtr<FJsonValue>* EnemyTabletTransform = Object->Values.Find(TEXT("enemyTransformTabletView"));
		if (MyTransform == nullptr || !ParseTransform(*MyTransform, OutChild.MyTransform, Context + TEXT(".myTransform"), OutError)
			|| EnemyTransform == nullptr || !ParseTransform(*EnemyTransform, OutChild.EnemyTransform, Context + TEXT(".enemyTransform"), OutError)
			|| MyTabletTransform == nullptr || !ParseTransform(*MyTabletTransform, OutChild.MyTabletTransform, Context + TEXT(".myTransformTabletView"), OutError)
			|| EnemyTabletTransform == nullptr || !ParseTransform(*EnemyTabletTransform, OutChild.EnemyTabletTransform, Context + TEXT(".enemyTransformTabletView"), OutError))
		{
			return false;
		}
		if (!Object->TryGetBoolField(TEXT("hasTabletView"), OutChild.Value.hasTabletView))
		{
			OutError = FString::Printf(TEXT("%s.hasTabletView must be a Boolean."), *Context);
			return false;
		}
		OutChild.Value.myTransform = OutChild.MyTransform.Value;
		OutChild.Value.enemyTransform = OutChild.EnemyTransform.Value;
		OutChild.Value.myTransformTabletView = OutChild.MyTabletTransform.Value;
		OutChild.Value.enemyTransformTabletView = OutChild.EnemyTabletTransform.Value;
		return true;
	}

	bool ParseRow(const FString& RowName, const TSharedPtr<FJsonObject>& Object, FRowIdentity& OutRow, FString& OutError)
	{
		static const TArray<FString> ExpectedKeys = {
			TEXT("title"), TEXT("factions"), TEXT("Items"), TEXT("item_name"), TEXT("Thumbnail"),
			TEXT("Image"), TEXT("boardPropActor"), TEXT("StartDate"), TEXT("EndDate"),
			TEXT("ItemSource"), TEXT("IsInteractable"), TEXT("rarity")};
		if (!ValidateExactKeys(Object, ExpectedKeys, TEXT("BoardItems2 row ") + RowName, OutError)) return false;
		OutRow.RowName = FName(*RowName);
		OutRow.RowNameText = RowName;
		if (OutRow.RowName.IsNone())
		{
			OutError = FString::Printf(TEXT("BoardItems2 row '%s' becomes NAME_None."), *RowName);
			return false;
		}

		const TSharedPtr<FJsonValue>* ItemNameValue = Object->Values.Find(TEXT("item_name"));
		if (ItemNameValue == nullptr || !ItemNameValue->IsValid())
		{
			OutError = FString::Printf(TEXT("BoardItems2 row '%s' has no serialized item_name value."), *RowName);
			return false;
		}
		if ((*ItemNameValue)->Type == EJson::Null)
		{
			OutRow.Value.item_name = NAME_None;
		}
		else
		{
			FString ItemName;
			if (!(*ItemNameValue)->TryGetString(ItemName))
			{
				OutError = FString::Printf(TEXT("BoardItems2 row '%s' item_name must be a string or JSON null."), *RowName);
				return false;
			}
			OutRow.Value.item_name = FName(*ItemName);
		}

		const TSharedPtr<FJsonValue>* TitleValue = Object->Values.Find(TEXT("title"));
		if (!ParseFTextIdentity(TitleValue != nullptr ? *TitleValue : nullptr, OutRow.Title, RowName + TEXT(".title"), OutError)) return false;
		OutRow.Value.title = OutRow.Title.Value;

		const TSharedPtr<FJsonValue>* FactionsValue = Object->Values.Find(TEXT("factions"));
		const TArray<TSharedPtr<FJsonValue>>* Factions = nullptr;
		if (FactionsValue == nullptr || !(*FactionsValue)->TryGetArray(Factions) || Factions == nullptr)
		{
			OutError = FString::Printf(TEXT("BoardItems2 row '%s' factions must be an array."), *RowName);
			return false;
		}
		TSet<int64> SeenFactions;
		for (int32 Index = 0; Index < Factions->Num(); ++Index)
		{
			int64 Value = INDEX_NONE;
			if (!ParseEnumValue(StaticEnum<EFactionEnum>(), (*Factions)[Index], Value, RowName + FString::Printf(TEXT(".factions[%d]"), Index), OutError)
				|| SeenFactions.Contains(Value))
			{
				if (OutError.IsEmpty()) OutError = FString::Printf(TEXT("BoardItems2 row '%s' has a duplicate faction."), *RowName);
				return false;
			}
			SeenFactions.Add(Value);
			OutRow.Value.factions.Add(static_cast<EFactionEnum>(Value));
		}

		const TSharedPtr<FJsonValue>* ItemsValue = Object->Values.Find(TEXT("Items"));
		const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
		if (ItemsValue == nullptr || !(*ItemsValue)->TryGetArray(Items) || Items == nullptr)
		{
			OutError = FString::Printf(TEXT("BoardItems2 row '%s' Items must be an array."), *RowName);
			return false;
		}
		for (int32 Index = 0; Index < Items->Num(); ++Index)
		{
			FChildIdentity Child;
			const TSharedPtr<FJsonObject> ChildObject = AsObject((*Items)[Index]);
			if (!ParseBoardChild(ChildObject, Child, RowName + FString::Printf(TEXT(".Items[%d]"), Index), OutError)) return false;
			OutRow.Value.Items.Add(MoveTemp(Child.Value));
		}

		const TSharedPtr<FJsonValue>* ThumbnailValue = Object->Values.Find(TEXT("Thumbnail"));
		const TSharedPtr<FJsonValue>* ImageValue = Object->Values.Find(TEXT("Image"));
		const TSharedPtr<FJsonValue>* PropValue = Object->Values.Find(TEXT("boardPropActor"));
		FSoftObjectPath ThumbnailPath, ImagePath, PropClassPath;
		if (!ValidateSoftPath(AsObject(ThumbnailValue != nullptr ? *ThumbnailValue : nullptr), ThumbnailPath, OutError, RowName + TEXT(".Thumbnail"))
			|| !ValidateSoftPath(AsObject(ImageValue != nullptr ? *ImageValue : nullptr), ImagePath, OutError, RowName + TEXT(".Image"))
			|| !ValidateSoftPath(AsObject(PropValue != nullptr ? *PropValue : nullptr), PropClassPath, OutError, RowName + TEXT(".boardPropActor"))) return false;
		OutRow.Value.Thumbnail = TSoftObjectPtr<UTexture2D>(ThumbnailPath);
		OutRow.Value.Image = TSoftObjectPtr<UTexture2D>(ImagePath);
		OutRow.Value.boardPropActor = TSoftClassPtr<AActor>(PropClassPath);

		for (const TPair<FString, FDateTime*>& DateField : {
			TPair<FString, FDateTime*>(TEXT("StartDate"), &OutRow.Value.StartDate),
			TPair<FString, FDateTime*>(TEXT("EndDate"), &OutRow.Value.EndDate)})
		{
			const TSharedPtr<FJsonObject> DateObject = AsObject(Object->TryGetField(*DateField.Key));
			int64 Ticks = INDEX_NONE;
			const TSharedPtr<FJsonValue>* TicksValue = DateObject.IsValid() ? DateObject->Values.Find(TEXT("Ticks")) : nullptr;
			if (!DateObject.IsValid() || DateObject->Values.Num() != 1 || TicksValue == nullptr
				|| !TryParseInteger(*TicksValue, Ticks) || Ticks < 0 || Ticks > FDateTime::MaxValue().GetTicks())
			{
				OutError = FString::Printf(TEXT("BoardItems2 row '%s' has invalid %s ticks."), *RowName, *DateField.Key);
				return false;
			}
			*DateField.Value = FDateTime(Ticks);
		}

		const TSharedPtr<FJsonValue>* SourceValue = Object->Values.Find(TEXT("ItemSource"));
		int64 ItemSource = INDEX_NONE;
		if (!ParseEnumValue(StaticEnum<EItemSource>(), SourceValue != nullptr ? *SourceValue : nullptr, ItemSource, RowName + TEXT(".ItemSource"), OutError)) return false;
		OutRow.Value.ItemSource = static_cast<EItemSource>(ItemSource);

		if (!Object->TryGetBoolField(TEXT("IsInteractable"), OutRow.Value.IsInteractable))
		{
			OutError = FString::Printf(TEXT("BoardItems2 row '%s' IsInteractable must be a Boolean."), *RowName);
			return false;
		}
		const TSharedPtr<FJsonValue>* RarityValue = Object->Values.Find(TEXT("rarity"));
		int64 Rarity = INDEX_NONE;
		if (!ParseEnumValue(StaticEnum<ERarityEnum>(), RarityValue != nullptr ? *RarityValue : nullptr, Rarity, RowName + TEXT(".rarity"), OutError)) return false;
		OutRow.Value.rarity = static_cast<ERarityEnum>(Rarity);
		return true;
	}

	bool LoadBoardItemsSource(const FString& SourceRoot, FBoardItemsSource& OutSource, FString& OutError)
	{
		OutSource = FBoardItemsSource();
		OutSource.FullPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(SourceRoot, TEXT("Structs/BoardItems2.json")));
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *OutSource.FullPath))
		{
			OutError = FString::Printf(TEXT("Could not read BoardItems2 JSON: %s"), *OutSource.FullPath);
			return false;
		}
		TArray<TSharedPtr<FJsonValue>> Exports;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, Exports) || Exports.Num() != 1)
		{
			OutError = FString::Printf(TEXT("%s must contain exactly one DataTable export."), *OutSource.FullPath);
			return false;
		}
		const TSharedPtr<FJsonObject> Export = AsObject(Exports[0]);
		FString Type, Name, Package;
		if (!RequiredString(Export, TEXT("Type"), Type, OutError)
			|| !RequiredString(Export, TEXT("Name"), Name, OutError)
			|| !RequiredString(Export, TEXT("Package"), Package, OutError)
			|| Type != TEXT("DataTable") || Name != TEXT("BoardItems2")
			|| Package != TEXT("kards/Content/Structs/BoardItems2"))
		{
			OutError = TEXT("BoardItems2 source identity/type/package mismatch.");
			return false;
		}
		const TSharedPtr<FJsonObject> Properties = AsObject(Export->TryGetField(TEXT("Properties")));
		const TSharedPtr<FJsonObject> RowStruct = Properties.IsValid() ? AsObject(Properties->TryGetField(TEXT("RowStruct"))) : nullptr;
		FString RowStructName, RowStructPath;
		static const TArray<FString> ExpectedPropertyKeys = {
			TEXT("RowStruct"), TEXT("bIgnoreExtraFields"), TEXT("bIgnoreMissingFields")};
		static const TArray<FString> ExpectedRowStructKeys = {TEXT("ObjectName"), TEXT("ObjectPath")};
		bool bIgnoreExtraFields = false;
		bool bIgnoreMissingFields = false;
		if (!ValidateExactKeys(Properties, ExpectedPropertyKeys, TEXT("BoardItems2 Properties"), OutError)) return false;
		if (!Properties->TryGetBoolField(TEXT("bIgnoreExtraFields"), bIgnoreExtraFields)
			|| !Properties->TryGetBoolField(TEXT("bIgnoreMissingFields"), bIgnoreMissingFields))
		{
			OutError = TEXT("BoardItems2 must serialize Boolean bIgnoreExtraFields and bIgnoreMissingFields properties.");
			return false;
		}
		if (!bIgnoreExtraFields || !bIgnoreMissingFields)
		{
			OutError = TEXT("BoardItems2 source must set bIgnoreExtraFields=true and bIgnoreMissingFields=true.");
			return false;
		}
		if (!RowStruct.IsValid()
			|| !ValidateExactKeys(RowStruct, ExpectedRowStructKeys, TEXT("BoardItems2 RowStruct"), OutError)
			|| !RequiredString(RowStruct, TEXT("ObjectName"), RowStructName, OutError)
			|| !RequiredString(RowStruct, TEXT("ObjectPath"), RowStructPath, OutError)
			|| RowStructName != TEXT("ScriptStruct'BoardItem2'")
			|| RowStructPath != TEXT("/Script/kards")
			|| FBoardItem2::StaticStruct()->GetPathName() != TEXT("/Script/kards.BoardItem2")
			|| FBoardChildItem2::StaticStruct()->GetPathName() != TEXT("/Script/kards.BoardChildItem2"))
		{
			OutError = TEXT("BoardItems2 RowStruct must exactly match /Script/kards.BoardItem2 and the current shim.");
			return false;
		}
		if (!ValidateShimSchema(OutError)) return false;

		const TSharedPtr<FJsonObject> Rows = AsObject(Export->TryGetField(TEXT("Rows")));
		if (!Rows.IsValid() || Rows->Values.Num() != ExpectedRowCount)
		{
			OutError = FString::Printf(TEXT("BoardItems2 must contain exactly %d rows."), ExpectedRowCount);
			return false;
		}
		TArray<FString> RowNames;
		Rows->Values.GetKeys(RowNames);
		RowNames.Sort();
		TSet<FName> SeenRowNames;
		for (const FString& RowName : RowNames)
		{
			const FName NameKey(*RowName);
			if (NameKey.IsNone() || SeenRowNames.Contains(NameKey))
			{
				OutError = FString::Printf(TEXT("BoardItems2 row name '%s' is invalid or duplicates another FName."), *RowName);
				return false;
			}
			SeenRowNames.Add(NameKey);
			FRowIdentity Row;
			if (!ParseRow(RowName, AsObject(Rows->TryGetField(RowName)), Row, OutError)) return false;
			OutSource.ChildItemCount += Row.Value.Items.Num();
			OutSource.TransformCount += Row.Value.Items.Num() * 4;
			OutSource.Rows.Add(MoveTemp(Row));
		}
		if (OutSource.Rows.Num() != ExpectedRowCount
			|| OutSource.ChildItemCount != ExpectedChildItemCount
			|| OutSource.TransformCount != ExpectedTransformCount)
		{
			OutError = FString::Printf(
				TEXT("BoardItems2 aggregate mismatch: rows=%d children=%d transforms=%d; expected %d/%d/%d."),
				OutSource.Rows.Num(), OutSource.ChildItemCount, OutSource.TransformCount,
				ExpectedRowCount, ExpectedChildItemCount, ExpectedTransformCount);
			return false;
		}
		return true;
	}

	bool ValidateShimSchema(FString& OutError)
	{
		UScriptStruct* RowStruct = FBoardItem2::StaticStruct();
		UScriptStruct* ChildStruct = FBoardChildItem2::StaticStruct();
		if (RowStruct == nullptr || ChildStruct == nullptr)
		{
			OutError = TEXT("BoardItem2 shim structs are unavailable in reflection.");
			return false;
		}
		auto RequireSoftObjectProperty = [&OutError](UStruct* Struct, const TCHAR* Name, const UClass* RequiredObjectClass) -> bool
		{
			const FSoftObjectProperty* Property = Struct != nullptr
				? CastField<FSoftObjectProperty>(FindFProperty<FProperty>(Struct, Name))
				: nullptr;
			if (Property == nullptr || Property->PropertyClass != RequiredObjectClass)
			{
				OutError = FString::Printf(TEXT("BoardItem2 shim property %s.%s does not match its soft reference class."), *GetNameSafe(Struct), Name);
				return false;
			}
			return true;
		};
		auto RequireSoftClassProperty = [&OutError](UStruct* Struct, const TCHAR* Name, const UClass* RequiredMetaClass) -> bool
		{
			const FSoftClassProperty* Property = Struct != nullptr
				? CastField<FSoftClassProperty>(FindFProperty<FProperty>(Struct, Name))
				: nullptr;
			if (Property == nullptr || Property->MetaClass != RequiredMetaClass)
			{
				OutError = FString::Printf(TEXT("BoardItem2 shim property %s.%s does not match its soft class reference type."), *GetNameSafe(Struct), Name);
				return false;
			}
			return true;
		};
		auto RequireStructProperty = [&OutError](UStruct* Struct, const TCHAR* Name, UScriptStruct* RequiredStruct) -> bool
		{
			const FStructProperty* Property = FindFProperty<FStructProperty>(Struct, Name);
			if (Property == nullptr || Property->Struct != RequiredStruct)
			{
				OutError = FString::Printf(TEXT("BoardItem2 shim property %s.%s has the wrong struct type."), *GetNameSafe(Struct), Name);
				return false;
			}
			return true;
		};
		if (!RequireSoftObjectProperty(RowStruct, TEXT("Thumbnail"), UTexture2D::StaticClass())
			|| !RequireSoftObjectProperty(RowStruct, TEXT("Image"), UTexture2D::StaticClass())
			|| !RequireSoftClassProperty(RowStruct, TEXT("boardPropActor"), AActor::StaticClass())
			|| !RequireSoftObjectProperty(ChildStruct, TEXT("StaticMesh"), UStaticMesh::StaticClass())
			|| !RequireSoftObjectProperty(ChildStruct, TEXT("Material"), UMaterialInterface::StaticClass())
			|| !RequireSoftObjectProperty(ChildStruct, TEXT("MobileMaterial"), UMaterialInterface::StaticClass())
			|| !RequireSoftClassProperty(ChildStruct, TEXT("boardPropActor"), AActor::StaticClass())
			|| !RequireStructProperty(RowStruct, TEXT("StartDate"), TBaseStructure<FDateTime>::Get())
			|| !RequireStructProperty(RowStruct, TEXT("EndDate"), TBaseStructure<FDateTime>::Get())
			|| !RequireStructProperty(ChildStruct, TEXT("myTransform"), TBaseStructure<FTransform>::Get())
			|| !RequireStructProperty(ChildStruct, TEXT("enemyTransform"), TBaseStructure<FTransform>::Get())
			|| !RequireStructProperty(ChildStruct, TEXT("myTransformTabletView"), TBaseStructure<FTransform>::Get())
			|| !RequireStructProperty(ChildStruct, TEXT("enemyTransformTabletView"), TBaseStructure<FTransform>::Get()))
		{
			return false;
		}
		const FArrayProperty* Factions = FindFProperty<FArrayProperty>(RowStruct, TEXT("factions"));
		const FArrayProperty* Items = FindFProperty<FArrayProperty>(RowStruct, TEXT("Items"));
		const FStructProperty* ChildItemProperty = Items != nullptr ? CastField<FStructProperty>(Items->Inner) : nullptr;
		if (Factions == nullptr || !IsEnumPropertyFor(Factions->Inner, StaticEnum<EFactionEnum>())
			|| ChildItemProperty == nullptr || ChildItemProperty->Struct != ChildStruct
			|| !IsEnumPropertyFor(FindFProperty<FProperty>(RowStruct, TEXT("ItemSource")), StaticEnum<EItemSource>())
			|| !IsEnumPropertyFor(FindFProperty<FProperty>(RowStruct, TEXT("rarity")), StaticEnum<ERarityEnum>())
			|| FindFProperty<FTextProperty>(RowStruct, TEXT("title")) == nullptr
			|| FindFProperty<FNameProperty>(RowStruct, TEXT("item_name")) == nullptr
			|| FindFProperty<FBoolProperty>(RowStruct, TEXT("IsInteractable")) == nullptr
			|| FindFProperty<FBoolProperty>(ChildStruct, TEXT("hasTabletView")) == nullptr)
		{
			OutError = TEXT("BoardItem2 shim enum/container/FText/FName/Boolean properties do not match the JSON schema.");
			return false;
		}
		return true;
	}

	bool IsEnumPropertyFor(const FProperty* Property, const UEnum* ExpectedEnum)
	{
		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			return EnumProperty->GetEnum() == ExpectedEnum;
		}
		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
		{
			return ByteProperty->Enum == ExpectedEnum;
		}
		return false;
	}

	bool VerifyText(const FTextIdentity& Expected, const FText& Actual, const FString& Context, FString& OutError)
	{
		const TOptional<FString> Namespace = FTextInspector::GetNamespace(Actual);
		const TOptional<FString> Key = FTextInspector::GetKey(Actual);
		const FString* SourceString = FTextInspector::GetSourceString(Actual);
		if (!Namespace.IsSet() || Namespace.GetValue() != Expected.Namespace
			|| !Key.IsSet() || Key.GetValue() != Expected.Key
			|| SourceString == nullptr || *SourceString != Expected.SourceString)
		{
			OutError = FString::Printf(TEXT("%s FText namespace/key/source identity changed during construction."), *Context);
			return false;
		}
		return true;
	}

	bool VerifyDataTableRow(const FBoardItem2& Actual, const FRowIdentity& Expected, FString& OutError)
	{
		if (Actual.item_name != Expected.Value.item_name
			|| Actual.IsInteractable != Expected.Value.IsInteractable
			|| Actual.ItemSource != Expected.Value.ItemSource
			|| Actual.rarity != Expected.Value.rarity
			|| Actual.StartDate != Expected.Value.StartDate
			|| Actual.EndDate != Expected.Value.EndDate
			|| Actual.factions != Expected.Value.factions
			|| Actual.Items.Num() != Expected.Value.Items.Num()
			|| Actual.Thumbnail.ToSoftObjectPath() != Expected.Value.Thumbnail.ToSoftObjectPath()
			|| Actual.Image.ToSoftObjectPath() != Expected.Value.Image.ToSoftObjectPath()
			|| Actual.boardPropActor.ToSoftObjectPath() != Expected.Value.boardPropActor.ToSoftObjectPath()
			|| !VerifyText(Expected.Title, Actual.title, Expected.RowNameText + TEXT(".title"), OutError))
		{
			if (OutError.IsEmpty()) OutError = FString::Printf(TEXT("BoardItems2 row %s scalar/array/soft-reference values changed."), *Expected.RowNameText);
			return false;
		}
		for (int32 Index = 0; Index < Actual.Items.Num(); ++Index)
		{
			const FBoardChildItem2& Child = Actual.Items[Index];
			const FBoardChildItem2& ExpectedChild = Expected.Value.Items[Index];
			if (Child.StaticMesh.ToSoftObjectPath() != ExpectedChild.StaticMesh.ToSoftObjectPath()
				|| Child.Material.ToSoftObjectPath() != ExpectedChild.Material.ToSoftObjectPath()
				|| Child.MobileMaterial.ToSoftObjectPath() != ExpectedChild.MobileMaterial.ToSoftObjectPath()
				|| Child.boardPropActor.ToSoftObjectPath() != ExpectedChild.boardPropActor.ToSoftObjectPath()
				|| Child.hasTabletView != ExpectedChild.hasTabletView
				|| !Child.myTransform.Equals(ExpectedChild.myTransform, 0.000001)
				|| !Child.enemyTransform.Equals(ExpectedChild.enemyTransform, 0.000001)
				|| !Child.myTransformTabletView.Equals(ExpectedChild.myTransformTabletView, 0.000001)
				|| !Child.enemyTransformTabletView.Equals(ExpectedChild.enemyTransformTabletView, 0.000001))
			{
				OutError = FString::Printf(TEXT("BoardItems2 row %s Items[%d] child/soft-path/transform data changed."), *Expected.RowNameText, Index);
				return false;
			}
		}
		return true;
	}

	bool VerifyBoardItemsTable(UDataTable* Table, const FBoardItemsSource& Source, FString& OutError)
	{
		if (Table == nullptr
			|| Table->GetPathName() != TableObjectPath
			|| Table->GetRowStruct() != FBoardItem2::StaticStruct()
			|| !Table->bIgnoreExtraFields
			|| !Table->bIgnoreMissingFields
			|| Table->GetRowMap().Num() != ExpectedRowCount)
		{
			OutError = TEXT("Constructed BoardItems2 has the wrong path, RowStruct, row count, or JSON field-policy flags.");
			return false;
		}
		for (const FRowIdentity& Expected : Source.Rows)
		{
			const uint8* Memory = Table->FindRowUnchecked(Expected.RowName);
			if (Memory == nullptr)
			{
				OutError = FString::Printf(TEXT("Constructed BoardItems2 is missing row '%s'."), *Expected.RowNameText);
				return false;
			}
			const FBoardItem2* Actual = reinterpret_cast<const FBoardItem2*>(Memory);
			if (!VerifyDataTableRow(*Actual, Expected, OutError)) return false;
		}
		return true;
	}

	UDataTable* BuildBoardItemsTable(const FBoardItemsSource& Source, FString& OutError)
	{
		UPackage* Package = CreatePackage(TablePackageName);
		UDataTableFactory* Factory = NewObject<UDataTableFactory>();
		Factory->Struct = FBoardItem2::StaticStruct();
		UDataTable* Table = Package == nullptr
			? nullptr
			: Cast<UDataTable>(Factory->FactoryCreateNew(
				UDataTable::StaticClass(), Package, FName(TableAssetName),
				RF_Public | RF_Standalone | RF_Transactional, nullptr, GWarn));
		if (Table == nullptr)
		{
			OutError = TEXT("Could not create /Game/Structs/BoardItems2 DataTable.");
			return nullptr;
		}
		Table->bIgnoreExtraFields = true;
		Table->bIgnoreMissingFields = true;
		FAssetRegistryModule::AssetCreated(Table);
		for (const FRowIdentity& Row : Source.Rows)
		{
			Table->AddRow(Row.RowName, Row.Value);
		}
		Table->MarkPackageDirty();
		if (!VerifyBoardItemsTable(Table, Source, OutError)) return nullptr;
		return Table;
	}

	bool TargetPackageAbsent(FString& OutError, bool bRequireNotLoaded)
	{
		if (bRequireNotLoaded && FindPackage(nullptr, TablePackageName) != nullptr)
		{
			OutError = TEXT("/Game/Structs/BoardItems2 is already loaded; refusing to overwrite it.");
			return false;
		}
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			TablePackageName, FPackageName::GetAssetPackageExtension());
		if (IFileManager::Get().FileExists(*PackageFilename))
		{
			OutError = FString::Printf(TEXT("/Game/Structs/BoardItems2 already exists; refusing to overwrite %s."), *PackageFilename);
			return false;
		}
		FString Base = PackageFilename;
		Base.RemoveFromEnd(FPackageName::GetAssetPackageExtension());
		for (const TCHAR* Extension : {TEXT("uexp"), TEXT("ubulk"), TEXT("uptnl")})
		{
			const FString Sidecar = Base + TEXT(".") + Extension;
			if (IFileManager::Get().FileExists(*Sidecar))
			{
				OutError = FString::Printf(TEXT("BoardItems2 orphaned sidecar exists; refusing to overwrite: %s."), *Sidecar);
				return false;
			}
		}
		return true;
	}

	bool WriteTable(UDataTable* Table, FString& OutError)
	{
		if (Table == nullptr || Table->GetOutermost() == nullptr)
		{
			OutError = TEXT("Cannot write a null BoardItems2 package.");
			return false;
		}
		const FString Filename = FPackageName::LongPackageNameToFilename(
			TablePackageName, FPackageName::GetAssetPackageExtension());
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Public | RF_Standalone;
		Args.SaveFlags = SAVE_NoError;
		Args.bSlowTask = false;
		if (!UPackage::SavePackage(Table->GetOutermost(), Table, *Filename, Args))
		{
			OutError = FString::Printf(
				TEXT("SavePackage failed for %s. Any partial package/sidecars were left untouched; inspect or remove them manually before retrying."),
				*Filename);
			return false;
		}
		if (!IFileManager::Get().FileExists(*Filename) || IFileManager::Get().FileSize(*Filename) <= 0)
		{
			OutError = FString::Printf(
				TEXT("SavePackage returned success but the expected package file is missing or empty: %s. Existing artifacts were left untouched."),
				*Filename);
			return false;
		}
		return true;
	}
}

URestoreBoardItems2TableCommandlet::URestoreBoardItems2TableCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 URestoreBoardItems2TableCommandlet::Main(const FString& Params)
{
	using namespace BoardItems2TableRestore;
	const bool bNoSave = FParse::Param(*Params, TEXT("NoSave"));
	const bool bWrite = FParse::Param(*Params, TEXT("Write"));
	const bool bVerify = FParse::Param(*Params, TEXT("Verify"));
	const int32 ModeCount = (bNoSave ? 1 : 0) + (bWrite ? 1 : 0) + (bVerify ? 1 : 0);
	if (ModeCount != 1)
	{
		UE_LOG(LogBoardItems2TableRestore, Error, TEXT("Specify exactly one of -NoSave, -Write, or -Verify."));
		return 1;
	}

	FString SourceRoot;
	if (!FParse::Value(*Params, TEXT("SourceRoot="), SourceRoot) || SourceRoot.IsEmpty())
	{
		UE_LOG(LogBoardItems2TableRestore, Error, TEXT("Specify -SourceRoot=<current Exports/kards/Content>."));
		return 2;
	}
	SourceRoot = FPaths::ConvertRelativePathToFull(SourceRoot);
	FString NormalizedSourceRoot = SourceRoot;
	NormalizedSourceRoot.ReplaceInline(TEXT("\\"), TEXT("/"));
	while (NormalizedSourceRoot.EndsWith(TEXT("/"))) NormalizedSourceRoot.LeftChopInline(1);
	if (!FPaths::DirectoryExists(SourceRoot)
		|| !NormalizedSourceRoot.EndsWith(TEXT("/Exports/kards/Content"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogBoardItems2TableRestore, Error, TEXT("SourceRoot must end in current Exports/kards/Content (not kardsC/kardsD): %s"), *SourceRoot);
		return 2;
	}

	FBoardItemsSource Source;
	FString Error;
	if (!LoadBoardItemsSource(SourceRoot, Source, Error))
	{
		UE_LOG(LogBoardItems2TableRestore, Error, TEXT("Strict BoardItems2 source/schema validation failed: %s"), *Error);
		return 3;
	}
	if (bVerify)
	{
		UDataTable* ExistingTable = LoadObject<UDataTable>(nullptr, TableObjectPath);
		if (ExistingTable == nullptr)
		{
			UE_LOG(LogBoardItems2TableRestore, Error, TEXT("VERIFY failed: could not load existing /Game/Structs/BoardItems2; no asset was created or changed."));
			return 4;
		}
		if (!VerifyBoardItemsTable(ExistingTable, Source, Error))
		{
			UE_LOG(LogBoardItems2TableRestore, Error, TEXT("VERIFY failed: loaded package did not match the current JSON: %s"), *Error);
			return 5;
		}
		UE_LOG(
			LogBoardItems2TableRestore,
			Display,
			TEXT("VERIFY VERIFIED: loaded /Game/Structs/BoardItems2 from the existing package and confirmed rows=%d childItems=%d transforms=%d, field-policy flags, and all serialized row data; no asset was created, dirtied, or saved."),
			Source.Rows.Num(), Source.ChildItemCount, Source.TransformCount);
		return 0;
	}
	if (!TargetPackageAbsent(Error, true))
	{
		UE_LOG(LogBoardItems2TableRestore, Error, TEXT("Target package preflight failed: %s"), *Error);
		return 4;
	}

	UDataTable* Table = BuildBoardItemsTable(Source, Error);
	if (Table == nullptr || !VerifyBoardItemsTable(Table, Source, Error))
	{
		UE_LOG(LogBoardItems2TableRestore, Error, TEXT("In-memory BoardItems2 construction/verification failed: %s"), *Error);
		return 5;
	}
	if (bNoSave)
	{
		UE_LOG(
			LogBoardItems2TableRestore,
			Display,
			TEXT("NOSAVE VERIFIED: BoardItems2 rows=%d childItems=%d transforms=%d; FText identities, enums, dates, soft paths, nested transforms, and both DataTable field-policy flags validated; no package saved."),
			Source.Rows.Num(), Source.ChildItemCount, Source.TransformCount);
		return 0;
	}
	// The in-memory package created above is expected here; recheck only disk state
	// to catch a file that appeared between preflight and SavePackage.
	if (!TargetPackageAbsent(Error, false))
	{
		UE_LOG(LogBoardItems2TableRestore, Error, TEXT("Target changed before Write; refusing to overwrite: %s"), *Error);
		return 6;
	}
	if (!WriteTable(Table, Error))
	{
		UE_LOG(LogBoardItems2TableRestore, Error, TEXT("BoardItems2 Write failed: %s"), *Error);
		return 7;
	}
	UE_LOG(
		LogBoardItems2TableRestore,
		Display,
		TEXT("WRITE SAVED: /Game/Structs/BoardItems2 rows=%d childItems=%d transforms=%d; source, in-memory rows, and field-policy flags were validated before SavePackage, but the on-disk package was not unloaded/reloaded and is not verified. No BoardItem Blueprint body was loaded or saved."),
		Source.Rows.Num(), Source.ChildItemCount, Source.TransformCount);
	return 0;
}
