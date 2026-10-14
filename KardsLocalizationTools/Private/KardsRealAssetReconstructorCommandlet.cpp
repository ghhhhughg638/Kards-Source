#include "KardsRealAssetReconstructorCommandlet.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Parse.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogKardsRealAssetReconstructor, Log, All);

namespace KardsRealAssetReconstructor
{
	const TCHAR* DefaultSourceRoot =
		TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kards/Content");
	const TCHAR* ReportRelativePath = TEXT("RealAssetReconstruction/asset-reconstruction-report.json");
	const TCHAR* JournalRelativePath = TEXT("RealAssetReconstruction/install-journal.json");
	const TCHAR* BackupRelativeRoot = TEXT("RealAssetReconstruction/Backups");

	const TCHAR* PackageExtensions[] = {
		TEXT(".uasset"), TEXT(".uexp"), TEXT(".ubulk"), TEXT(".uptnl"), TEXT(".m.ubulk")};

	const TCHAR* AudioTypes[] = {
		TEXT("SoundWave"), TEXT("SoundCue"), TEXT("MetaSoundSource"), TEXT("MetaSoundPatch"),
		TEXT("SoundClass"), TEXT("SoundMix"), TEXT("SoundAttenuation")};

	struct FAssetPackageSpec
	{
		FString PackageName;
		FString AssetName;
		FString Type;
		FString SourceBase;
		FString JsonFile;
		TSet<FString> Dependencies;
		TSet<FString> SoftDependencies;
		TArray<FString> MissingDependencies;
		bool bIsRoot = false;
		bool bSourceAvailable = false;
	};

	FString NormalizePackageName(FString PackageName)
	{
		PackageName.TrimStartAndEndInline();
		PackageName.ReplaceInline(TEXT("\\"), TEXT("/"));
		if (PackageName.StartsWith(TEXT("kards/Content/"), ESearchCase::IgnoreCase))
		{
			PackageName.RightChopInline(14, EAllowShrinking::No);
			PackageName = TEXT("/Game/") + PackageName;
		}
		else if (PackageName.StartsWith(TEXT("/Game/"), ESearchCase::IgnoreCase))
		{
			PackageName.RightChopInline(6, EAllowShrinking::No);
			PackageName = TEXT("/Game/") + PackageName;
		}
		else
		{
			return {};
		}

		int32 DotIndex = INDEX_NONE;
		if (PackageName.FindChar(TEXT('.'), DotIndex))
		{
			PackageName.LeftInline(DotIndex, EAllowShrinking::No);
		}
		FPaths::NormalizeFilename(PackageName);
		if (!FPackageName::IsValidLongPackageName(PackageName, false))
		{
			return {};
		}
		return PackageName;
	}

	FString RelativeContentPath(const FString& PackageName)
	{
		return PackageName.StartsWith(TEXT("/Game/"), ESearchCase::IgnoreCase)
			? PackageName.RightChop(6)
			: FString();
	}

	FString PackageBasePath(const FString& Root, const FString& PackageName)
	{
		return FPaths::Combine(Root, *RelativeContentPath(PackageName));
	}

	bool HasPrefix(const FString& Value, const TCHAR* Prefix)
	{
		return Value.StartsWith(Prefix, ESearchCase::IgnoreCase);
	}

	bool IsAudioType(const FString& Type)
	{
		for (const TCHAR* AudioType : AudioTypes)
		{
			if (Type.Equals(AudioType, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	bool IsVisualEffectPackage(const FString& PackageName)
	{
		return PackageName.StartsWith(TEXT("/Game/Effects/"), ESearchCase::IgnoreCase)
			|| PackageName.Contains(TEXT("/CameraShake"), ESearchCase::IgnoreCase)
			|| PackageName.StartsWith(TEXT("/Game/examples/GrenadePack/Blueprints/CameraShake"), ESearchCase::IgnoreCase);
	}

	bool MatchesRootFilter(const FAssetPackageSpec& Spec, const FString& RootFilter)
	{
		if (RootFilter.IsEmpty() || RootFilter.Equals(TEXT("All"), ESearchCase::IgnoreCase))
		{
			return true;
		}
		if (RootFilter.Equals(TEXT("Audio"), ESearchCase::IgnoreCase))
		{
			return IsAudioType(Spec.Type);
		}
		if (RootFilter.Equals(TEXT("Effects"), ESearchCase::IgnoreCase))
		{
			return IsVisualEffectPackage(Spec.PackageName);
		}
		if (RootFilter.Equals(TEXT("Niagara"), ESearchCase::IgnoreCase))
		{
			return Spec.Type.Equals(TEXT("NiagaraSystem"), ESearchCase::IgnoreCase);
		}
		if (RootFilter.Equals(TEXT("Cascade"), ESearchCase::IgnoreCase)
			|| RootFilter.Equals(TEXT("ParticleSystem"), ESearchCase::IgnoreCase))
		{
			return Spec.Type.Equals(TEXT("ParticleSystem"), ESearchCase::IgnoreCase);
		}
		if (RootFilter.Equals(TEXT("VfxAssets"), ESearchCase::IgnoreCase))
		{
			return IsVisualEffectPackage(Spec.PackageName)
				&& !Spec.Type.Equals(TEXT("BlueprintGeneratedClass"), ESearchCase::IgnoreCase);
		}
		if (RootFilter.Equals(TEXT("EffectBlueprints"), ESearchCase::IgnoreCase))
		{
			return IsVisualEffectPackage(Spec.PackageName)
				&& Spec.Type.Equals(TEXT("BlueprintGeneratedClass"), ESearchCase::IgnoreCase);
		}
		if (RootFilter.Equals(TEXT("BlueprintEffects"), ESearchCase::IgnoreCase))
		{
			return Spec.PackageName.StartsWith(TEXT("/Game/Blueprints/Effects/"), ESearchCase::IgnoreCase);
		}
		return false;
	}

	bool IsEffectRoot(const FString& RelativeJsonPath, const bool bIncludeBlueprintEffects)
	{
		return HasPrefix(RelativeJsonPath, TEXT("Effects/"))
			|| (bIncludeBlueprintEffects && HasPrefix(RelativeJsonPath, TEXT("Blueprints/Effects/")))
			|| RelativeJsonPath.Contains(TEXT("/CameraShake"), ESearchCase::IgnoreCase)
			|| HasPrefix(RelativeJsonPath, TEXT("examples/GrenadePack/Blueprints/CameraShake"));
	}

	bool IsUserDiyPackage(const FString& PackageName)
	{
		return PackageName.StartsWith(TEXT("/Game/Blueprints/Cards/DIY/"), ESearchCase::IgnoreCase);
	}

	bool ReadExports(const FString& JsonFile, TArray<TSharedPtr<FJsonValue>>& OutExports)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *JsonFile))
		{
			return false;
		}
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutExports);
	}

	FString PackageFromReference(const FString& Reference)
	{
		return NormalizePackageName(Reference);
	}

	void GatherPackageReferences(
		const TSharedPtr<FJsonValue>& Value,
		TSet<FString>& OutHardReferences,
		TSet<FString>& OutSoftReferences)
	{
		if (!Value.IsValid())
		{
			return;
		}

		if (Value->Type == EJson::Object)
		{
			const TSharedPtr<FJsonObject> Object = Value->AsObject();
			if (!Object.IsValid())
			{
				return;
			}
			for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
			{
				if ((Pair.Key.Equals(TEXT("ObjectPath"), ESearchCase::IgnoreCase)
					|| Pair.Key.Equals(TEXT("AssetPathName"), ESearchCase::IgnoreCase))
					&& Pair.Value.IsValid()
					&& Pair.Value->Type == EJson::String)
				{
					const FString PackageName = PackageFromReference(Pair.Value->AsString());
					if (!PackageName.IsEmpty())
					{
						if (Pair.Key.Equals(TEXT("ObjectPath"), ESearchCase::IgnoreCase))
						{
							OutHardReferences.Add(PackageName);
						}
						else
						{
							OutSoftReferences.Add(PackageName);
						}
					}
				}
				GatherPackageReferences(Pair.Value, OutHardReferences, OutSoftReferences);
			}
		}
		else if (Value->Type == EJson::Array)
		{
			for (const TSharedPtr<FJsonValue>& Element : Value->AsArray())
			{
				GatherPackageReferences(Element, OutHardReferences, OutSoftReferences);
			}
		}
	}

	bool LoadPackageSpec(
		const FString& SourceRoot,
		const FString& PackageName,
		const bool bIsRoot,
		FAssetPackageSpec& OutSpec)
	{
		if (!PackageName.StartsWith(TEXT("/Game/"), ESearchCase::IgnoreCase)
			|| IsUserDiyPackage(PackageName))
		{
			return false;
		}

		OutSpec.PackageName = PackageName;
		OutSpec.AssetName = FPackageName::GetLongPackageAssetName(PackageName);
		OutSpec.SourceBase = PackageBasePath(SourceRoot, PackageName);
		OutSpec.JsonFile = OutSpec.SourceBase + TEXT(".json");
		OutSpec.bIsRoot = bIsRoot;
		OutSpec.bSourceAvailable = IFileManager::Get().FileExists(*(OutSpec.SourceBase + TEXT(".uasset")));

		TArray<TSharedPtr<FJsonValue>> Exports;
		if (IFileManager::Get().FileExists(*OutSpec.JsonFile) && ReadExports(OutSpec.JsonFile, Exports))
		{
			FString FallbackType;
			for (const TSharedPtr<FJsonValue>& ExportValue : Exports)
			{
				const TSharedPtr<FJsonObject> Export = ExportValue.IsValid() ? ExportValue->AsObject() : nullptr;
				if (!Export.IsValid())
				{
					continue;
				}

				FString Type;
				FString Name;
				if (Export->TryGetStringField(TEXT("Type"), Type))
				{
					if (FallbackType.IsEmpty())
					{
						FallbackType = Type;
					}
					if (Export->TryGetStringField(TEXT("Name"), Name) && Name.Equals(OutSpec.AssetName, ESearchCase::IgnoreCase))
					{
						OutSpec.Type = Type;
					}
				}
				GatherPackageReferences(ExportValue, OutSpec.Dependencies, OutSpec.SoftDependencies);
			}
			if (OutSpec.Type.IsEmpty())
			{
				OutSpec.Type = FallbackType;
			}
		}

		OutSpec.Dependencies.Remove(PackageName);
		OutSpec.SoftDependencies.Remove(PackageName);
		return true;
	}

	void DiscoverPackageClosure(
		const FString& SourceRoot,
		const bool bIncludeSoftReferences,
		TMap<FString, FAssetPackageSpec>& OutSpecs,
		TArray<FString>& OutMissingPackages)
	{
		TArray<FString> Queue;
		OutSpecs.GenerateKeyArray(Queue);
		Queue.Sort();

		for (int32 QueueIndex = 0; QueueIndex < Queue.Num(); ++QueueIndex)
		{
			const FString CurrentName = Queue[QueueIndex];
			FAssetPackageSpec* Current = OutSpecs.Find(CurrentName);
			if (Current == nullptr)
			{
				continue;
			}

			TArray<FString> Dependencies = Current->Dependencies.Array();
			if (bIncludeSoftReferences)
			{
				Dependencies.Append(Current->SoftDependencies.Array());
			}
			Dependencies.Sort();
			TSet<FString> UniqueDependencies;
			for (const FString& Dependency : Dependencies)
			{
				UniqueDependencies.Add(Dependency);
			}
			Dependencies = UniqueDependencies.Array();
			Dependencies.Sort();
			for (const FString& Dependency : Dependencies)
			{
				if (OutSpecs.Contains(Dependency) || IsUserDiyPackage(Dependency))
				{
					continue;
				}

				FAssetPackageSpec DependencySpec;
				if (LoadPackageSpec(SourceRoot, Dependency, false, DependencySpec))
				{
					if (DependencySpec.bSourceAvailable)
					{
						Queue.Add(Dependency);
						OutSpecs.Add(Dependency, MoveTemp(DependencySpec));
					}
					else
					{
						const FString TargetFilename = FPackageName::LongPackageNameToFilename(
							Dependency, FPackageName::GetAssetPackageExtension());
						if (!IFileManager::Get().FileExists(*TargetFilename))
						{
						OutMissingPackages.Add(Dependency);
						if (FAssetPackageSpec* CurrentSpec = OutSpecs.Find(CurrentName))
						{
							CurrentSpec->MissingDependencies.Add(Dependency);
						}
						OutSpecs.Add(Dependency, MoveTemp(DependencySpec));
						Queue.Add(Dependency);
					}
					}
				}
				else
				{
					OutMissingPackages.Add(Dependency);
					if (FAssetPackageSpec* CurrentSpec = OutSpecs.Find(CurrentName))
					{
						CurrentSpec->MissingDependencies.Add(Dependency);
					}
				}
			}
		}

		TSet<FString> UniqueMissing;
		for (const FString& Missing : OutMissingPackages)
		{
			UniqueMissing.Add(Missing);
		}
		OutMissingPackages = UniqueMissing.Array();
		OutMissingPackages.Sort();
	}

	bool CollectSeeds(
		const FString& SourceRoot,
		const FString& AssetRegistryFile,
		const bool bIncludeBlueprintEffects,
		TMap<FString, FAssetPackageSpec>& OutSpecs,
		FString& OutError)
	{
		TArray<FString> JsonFiles;
		IFileManager::Get().FindFilesRecursive(JsonFiles, *SourceRoot, TEXT("*.json"), true, false);
		if (JsonFiles.IsEmpty())
		{
			OutError = FString::Printf(TEXT("No FModel JSON exports found below %s"), *SourceRoot);
			return false;
		}

		for (const FString& JsonFile : JsonFiles)
		{
			FString RelativeJsonPath = JsonFile;
			FPaths::MakePathRelativeTo(RelativeJsonPath, *(SourceRoot / TEXT("")));
			FPaths::NormalizeFilename(RelativeJsonPath);
			const bool bMayBeEffect = IsEffectRoot(RelativeJsonPath, bIncludeBlueprintEffects);
			if (!bMayBeEffect)
			{
				continue;
			}

			TArray<TSharedPtr<FJsonValue>> Exports;
			if (!ReadExports(JsonFile, Exports))
			{
				UE_LOG(LogKardsRealAssetReconstructor, Warning, TEXT("Invalid JSON skipped: %s"), *JsonFile);
				continue;
			}

			FString PackageName;
			for (const TSharedPtr<FJsonValue>& ExportValue : Exports)
			{
				const TSharedPtr<FJsonObject> Export = ExportValue.IsValid() ? ExportValue->AsObject() : nullptr;
				FString ExportPackage;
				if (!Export.IsValid() || !Export->TryGetStringField(TEXT("Package"), ExportPackage))
				{
					continue;
				}
				PackageName = NormalizePackageName(ExportPackage);
			}

			if (PackageName.IsEmpty() || IsUserDiyPackage(PackageName))
			{
				continue;
			}

			FAssetPackageSpec Spec;
			if (LoadPackageSpec(SourceRoot, PackageName, true, Spec) && Spec.bSourceAvailable)
			{
				OutSpecs.Add(PackageName, MoveTemp(Spec));
			}
		}

		FString RegistryText;
		if (!FFileHelper::LoadFileToString(RegistryText, *AssetRegistryFile))
		{
			OutError = FString::Printf(TEXT("Could not read current AssetRegistry.json: %s"), *AssetRegistryFile);
			return false;
		}
		const TSharedRef<TJsonReader<>> RegistryReader = TJsonReaderFactory<>::Create(RegistryText);
		TSharedPtr<FJsonObject> RegistryObject;
		if (!FJsonSerializer::Deserialize(RegistryReader, RegistryObject) || !RegistryObject.IsValid())
		{
			OutError = FString::Printf(TEXT("Invalid AssetRegistry.json: %s"), *AssetRegistryFile);
			return false;
		}
		const TArray<TSharedPtr<FJsonValue>>* RegistryAssets = nullptr;
		if (!RegistryObject->TryGetArrayField(TEXT("PreallocatedAssetDataBuffers"), RegistryAssets)
			|| RegistryAssets == nullptr)
		{
			OutError = TEXT("AssetRegistry.json is missing PreallocatedAssetDataBuffers.");
			return false;
		}
		for (const TSharedPtr<FJsonValue>& AssetValue : *RegistryAssets)
		{
			const TSharedPtr<FJsonObject> Asset = AssetValue.IsValid() ? AssetValue->AsObject() : nullptr;
			FString AssetClass;
			FString ExportPackage;
			if (!Asset.IsValid()
				|| !Asset->TryGetStringField(TEXT("AssetClass"), AssetClass)
				|| !IsAudioType(AssetClass)
				|| !Asset->TryGetStringField(TEXT("PackageName"), ExportPackage))
			{
				continue;
			}

			const FString PackageName = NormalizePackageName(ExportPackage);
			if (PackageName.IsEmpty() || IsUserDiyPackage(PackageName) || OutSpecs.Contains(PackageName))
			{
				continue;
			}
			FAssetPackageSpec Spec;
			if (LoadPackageSpec(SourceRoot, PackageName, true, Spec) && Spec.bSourceAvailable)
			{
				Spec.Type = AssetClass;
				OutSpecs.Add(PackageName, MoveTemp(Spec));
			}
		}

		if (OutSpecs.IsEmpty())
		{
			OutError = TEXT("No source packages matched Effects/ or supported audio classes in AssetRegistry.json.");
			return false;
		}
		return true;
	}

	bool IsCVarEnabled(const TCHAR* Name)
	{
		const IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name);
		return Variable != nullptr && Variable->GetInt() == 1;
	}

	FString MakeBackupRoot()
	{
		const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
		return FPaths::Combine(FPaths::ProjectSavedDir(), BackupRelativeRoot, Timestamp, TEXT("Content"));
	}

	FString BackupBasePath(const FString& BackupRoot, const FString& PackageName)
	{
		return PackageBasePath(BackupRoot, PackageName);
	}

	TArray<FString> ExistingPackageExtensions(const FString& BasePath)
	{
		TArray<FString> Existing;
		for (const TCHAR* Extension : PackageExtensions)
		{
			if (IFileManager::Get().FileExists(*(BasePath + Extension)))
			{
				Existing.Add(Extension);
			}
		}
		return Existing;
	}

	bool PackageSidecarsMatchSource(const FAssetPackageSpec& Spec, const FString& TargetBase)
	{
		const TArray<FString> SourceExtensions = ExistingPackageExtensions(Spec.SourceBase);
		const TArray<FString> TargetExtensions = ExistingPackageExtensions(TargetBase);
		if (SourceExtensions.Num() != TargetExtensions.Num())
		{
			return false;
		}

		for (const FString& Extension : SourceExtensions)
		{
			if (!TargetExtensions.Contains(Extension))
			{
				return false;
			}

			const FString SourceFile = Spec.SourceBase + Extension;
			const FString TargetFile = TargetBase + Extension;
			const int64 SourceSize = IFileManager::Get().FileSize(*SourceFile);
			const int64 TargetSize = IFileManager::Get().FileSize(*TargetFile);
			if (SourceSize < 0 || SourceSize != TargetSize)
			{
				return false;
			}

			const FMD5Hash SourceHash = FMD5Hash::HashFile(*SourceFile);
			const FMD5Hash TargetHash = FMD5Hash::HashFile(*TargetFile);
			if (!SourceHash.IsValid() || !TargetHash.IsValid() || SourceHash != TargetHash)
			{
				return false;
			}
		}
		return true;
	}

	bool IsProtectedUserDependency(const FString& PackageName)
	{
		return PackageName.StartsWith(TEXT("/Game/Blueprints/"), ESearchCase::IgnoreCase)
			|| PackageName.StartsWith(TEXT("/Game/Library/"), ESearchCase::IgnoreCase)
			|| PackageName.StartsWith(TEXT("/Game/Structs/"), ESearchCase::IgnoreCase)
			|| PackageName.StartsWith(TEXT("/Game/Maps/"), ESearchCase::IgnoreCase);
	}

	bool ShouldInstallPackage(
		const FAssetPackageSpec& Spec,
		const bool bOverwriteDependencies,
		const bool bOverwriteStaleDependencies,
		bool& bOutPreserveExistingDependency)
	{
		bOutPreserveExistingDependency = false;
		if (Spec.bIsRoot)
		{
			return true;
		}

		const FString TargetBase = PackageBasePath(FPaths::ProjectContentDir(), Spec.PackageName);
		const TArray<FString> TargetExtensions = ExistingPackageExtensions(TargetBase);
		if (TargetExtensions.IsEmpty() || bOverwriteDependencies)
		{
			return true;
		}
		if (IsProtectedUserDependency(Spec.PackageName) && !bOverwriteStaleDependencies)
		{
			bOutPreserveExistingDependency = true;
			return false;
		}

		const TArray<FString> SourceExtensions = ExistingPackageExtensions(Spec.SourceBase);
		bool bTargetIsIncompleteSubset = TargetExtensions.Num() < SourceExtensions.Num();
		for (const FString& Extension : TargetExtensions)
		{
			bTargetIsIncompleteSubset &= SourceExtensions.Contains(Extension);
		}
		if (bTargetIsIncompleteSubset)
		{
			return true;
		}
		if (bOverwriteStaleDependencies && Spec.bSourceAvailable
			&& !PackageSidecarsMatchSource(Spec, TargetBase))
		{
			return true;
		}

		bOutPreserveExistingDependency = true;
		return false;
	}

	bool CopyFile(const FString& Source, const FString& Destination)
	{
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Destination), true);
		return IFileManager::Get().Copy(*Destination, *Source, true, true, false) == COPY_OK;
	}

	bool BackupPackage(
		const FString& BackupRoot,
		const FAssetPackageSpec& Spec,
		TMap<FString, TArray<FString>>& OutBackedUpExtensions)
	{
		const FString TargetBase = PackageBasePath(FPaths::ProjectContentDir(), Spec.PackageName);
		const TArray<FString> ExistingExtensions = ExistingPackageExtensions(TargetBase);
		OutBackedUpExtensions.Add(Spec.PackageName, ExistingExtensions);
		const FString BackupBase = BackupBasePath(BackupRoot, Spec.PackageName);
		for (const FString& Extension : ExistingExtensions)
		{
			if (!CopyFile(TargetBase + Extension, BackupBase + Extension))
			{
				return false;
			}
		}
		return true;
	}

	void DeletePackageBundle(const FString& BasePath)
	{
		for (const TCHAR* Extension : PackageExtensions)
		{
			IFileManager::Get().Delete(*(BasePath + Extension), false, true, true);
		}
	}

	bool InstallPackage(const FAssetPackageSpec& Spec)
	{
		const FString TargetBase = PackageBasePath(FPaths::ProjectContentDir(), Spec.PackageName);
		DeletePackageBundle(TargetBase);
		bool bCopiedPrimary = false;
		for (const TCHAR* Extension : PackageExtensions)
		{
			const FString SourceFile = Spec.SourceBase + Extension;
			if (!IFileManager::Get().FileExists(*SourceFile))
			{
				continue;
			}
			if (!CopyFile(SourceFile, TargetBase + Extension))
			{
				return false;
			}
			if (FCString::Stricmp(Extension, TEXT(".uasset")) == 0)
			{
				bCopiedPrimary = true;
			}
		}
		return bCopiedPrimary;
	}

	bool RestoreBackup(
		const FString& BackupRoot,
		const FAssetPackageSpec& Spec,
		const TMap<FString, TArray<FString>>& BackedUpExtensions)
	{
		const FString TargetBase = PackageBasePath(FPaths::ProjectContentDir(), Spec.PackageName);
		DeletePackageBundle(TargetBase);
		const TArray<FString>* Extensions = BackedUpExtensions.Find(Spec.PackageName);
		if (Extensions == nullptr)
		{
			return true;
		}
		const FString BackupBase = BackupBasePath(BackupRoot, Spec.PackageName);
		for (const FString& Extension : *Extensions)
		{
			if (!CopyFile(BackupBase + Extension, TargetBase + Extension))
			{
				return false;
			}
		}
		return true;
	}

	bool VerifyRootPackages(
		const TMap<FString, FAssetPackageSpec>& Specs,
		TArray<FString>& OutFailures,
		TArray<FString>& OutWarnings)
	{
		TArray<FString> PackageNames;
		for (const TPair<FString, FAssetPackageSpec>& Pair : Specs)
		{
			if (Pair.Value.bIsRoot)
			{
				PackageNames.Add(Pair.Key);
			}
		}
		PackageNames.Sort();

		for (const FString& PackageName : PackageNames)
		{
			UPackage* LoadedPackage = LoadPackage(nullptr, *PackageName, LOAD_None);
			if (LoadedPackage == nullptr)
			{
				OutFailures.Add(PackageName);
				UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("LOAD_FAILED %s"), *PackageName);
				continue;
			}

			const FAssetPackageSpec& Spec = Specs.FindChecked(PackageName);
			if (FindObject<UObject>(LoadedPackage, *Spec.AssetName) == nullptr)
			{
				OutWarnings.Add(PackageName + TEXT(" loaded, but its expected primary object name was not found."));
			}
		}
		return OutFailures.IsEmpty();
	}

	TSharedRef<FJsonObject> BuildReport(
		const TMap<FString, FAssetPackageSpec>& Specs,
		const TArray<FString>& MissingPackages,
		const TArray<FString>& Failures,
		const TArray<FString>& Warnings,
		const TArray<FString>& PlannedInstallPackages,
		const TArray<FString>& PreservedExistingDependencies,
		const TArray<FString>& ExcludedRoots,
		const FString& SourceRoot,
		const FString& OnlyPackage,
		const FString& RootFilter,
		const bool bIncludeSoftReferences,
		const bool bOverwriteStaleDependencies,
		const FString& BackupRoot,
		const FString& Status)
	{
		TSharedRef<FJsonObject> Report = MakeShared<FJsonObject>();
		Report->SetStringField(TEXT("status"), Status);
		Report->SetStringField(TEXT("sourceRoot"), SourceRoot);
		Report->SetStringField(TEXT("onlyPackage"), OnlyPackage);
		Report->SetStringField(TEXT("rootFilter"), RootFilter);
		Report->SetStringField(TEXT("projectContent"), FPaths::ProjectContentDir());
		Report->SetStringField(TEXT("backupRoot"), BackupRoot);
		Report->SetBoolField(TEXT("unversionedCVarEnabled"), IsCVarEnabled(TEXT("s.AllowUnversionedContentInEditor")));
		Report->SetBoolField(TEXT("cookedDataCVarEnabled"), IsCVarEnabled(TEXT("cook.AllowCookedDataInEditorBuilds")));
		Report->SetBoolField(TEXT("softReferencesIncludedInClosure"), bIncludeSoftReferences);
		Report->SetBoolField(TEXT("overwriteStaleDependencies"), bOverwriteStaleDependencies);
		Report->SetStringField(TEXT("editability"), TEXT("not tested; cooked exports may lack editor-only graphs/data"));
		Report->SetStringField(TEXT("compileCookability"), TEXT("not tested"));

		TArray<FString> PackageNames;
		Specs.GenerateKeyArray(PackageNames);
		PackageNames.Sort();
		TArray<TSharedPtr<FJsonValue>> PackageValues;
		int32 RootCount = 0;
		int32 EdgeCount = 0;
		int32 SoftEdgeCount = 0;
		for (const FString& PackageName : PackageNames)
		{
			const FAssetPackageSpec& Spec = Specs.FindChecked(PackageName);
			TSharedRef<FJsonObject> PackageObject = MakeShared<FJsonObject>();
			PackageObject->SetStringField(TEXT("package"), Spec.PackageName);
			PackageObject->SetStringField(TEXT("assetName"), Spec.AssetName);
			PackageObject->SetStringField(TEXT("type"), Spec.Type);
			PackageObject->SetBoolField(TEXT("root"), Spec.bIsRoot);
			PackageObject->SetBoolField(TEXT("sourceAvailable"), Spec.bSourceAvailable);
			PackageObject->SetStringField(TEXT("sourceJson"), Spec.JsonFile);

			TArray<TSharedPtr<FJsonValue>> SidecarValues;
			for (const TCHAR* Extension : PackageExtensions)
			{
				const FString SourceFile = Spec.SourceBase + Extension;
				if (IFileManager::Get().FileExists(*SourceFile))
				{
					SidecarValues.Add(MakeShared<FJsonValueString>(Extension));
				}
			}
			PackageObject->SetArrayField(TEXT("sourceSidecars"), SidecarValues);

			const FString TargetBase = PackageBasePath(FPaths::ProjectContentDir(), Spec.PackageName);
			TArray<TSharedPtr<FJsonValue>> TargetSidecarValues;
			for (const TCHAR* Extension : PackageExtensions)
			{
				const FString TargetFile = TargetBase + Extension;
				if (IFileManager::Get().FileExists(*TargetFile))
				{
					TSharedRef<FJsonObject> TargetSidecar = MakeShared<FJsonObject>();
					TargetSidecar->SetStringField(TEXT("extension"), Extension);
					TargetSidecar->SetNumberField(TEXT("bytes"), IFileManager::Get().FileSize(*TargetFile));
					TargetSidecarValues.Add(MakeShared<FJsonValueObject>(TargetSidecar));
				}
			}
			PackageObject->SetArrayField(TEXT("existingTargetSidecars"), TargetSidecarValues);

			TArray<FString> Dependencies = Spec.Dependencies.Array();
			Dependencies.Sort();
			TArray<TSharedPtr<FJsonValue>> DependencyValues;
			for (const FString& Dependency : Dependencies)
			{
				DependencyValues.Add(MakeShared<FJsonValueString>(Dependency));
			}
			PackageObject->SetArrayField(TEXT("dependencies"), DependencyValues);
			TArray<FString> SoftDependencies = Spec.SoftDependencies.Array();
			SoftDependencies.Sort();
			TArray<TSharedPtr<FJsonValue>> SoftDependencyValues;
			for (const FString& Dependency : SoftDependencies)
			{
				SoftDependencyValues.Add(MakeShared<FJsonValueString>(Dependency));
			}
			PackageObject->SetArrayField(TEXT("softDependencies"), SoftDependencyValues);
			PackageObject->SetArrayField(TEXT("missingDependencies"), [&Spec]()
			{
				TArray<TSharedPtr<FJsonValue>> Values;
				for (const FString& Missing : Spec.MissingDependencies)
				{
					Values.Add(MakeShared<FJsonValueString>(Missing));
				}
				return Values;
			}());
			PackageValues.Add(MakeShared<FJsonValueObject>(PackageObject));
			RootCount += Spec.bIsRoot ? 1 : 0;
			EdgeCount += Spec.Dependencies.Num();
			SoftEdgeCount += Spec.SoftDependencies.Num();
		}

		Report->SetArrayField(TEXT("packages"), PackageValues);
		Report->SetNumberField(TEXT("packageCount"), Specs.Num());
		Report->SetNumberField(TEXT("rootCount"), RootCount);
		Report->SetNumberField(TEXT("dependencyEdges"), EdgeCount);
		Report->SetNumberField(TEXT("softDependencyEdges"), SoftEdgeCount);
		Report->SetNumberField(TEXT("plannedInstallCount"), PlannedInstallPackages.Num());
		Report->SetNumberField(TEXT("preservedExistingDependencyCount"), PreservedExistingDependencies.Num());
		Report->SetArrayField(TEXT("plannedInstallPackages"), [&PlannedInstallPackages]()
		{
			TArray<TSharedPtr<FJsonValue>> Values;
			for (const FString& PackageName : PlannedInstallPackages)
			{
				Values.Add(MakeShared<FJsonValueString>(PackageName));
			}
			return Values;
		}());
		Report->SetArrayField(TEXT("preservedExistingDependencies"), [&PreservedExistingDependencies]()
		{
			TArray<TSharedPtr<FJsonValue>> Values;
			for (const FString& PackageName : PreservedExistingDependencies)
			{
				Values.Add(MakeShared<FJsonValueString>(PackageName));
			}
			return Values;
		}());
		Report->SetArrayField(TEXT("excludedRoots"), [&ExcludedRoots]()
		{
			TArray<TSharedPtr<FJsonValue>> Values;
			for (const FString& PackageName : ExcludedRoots)
			{
				Values.Add(MakeShared<FJsonValueString>(PackageName));
			}
			return Values;
		}());
		Report->SetArrayField(TEXT("missingPackages"), [&MissingPackages]()
		{
			TArray<TSharedPtr<FJsonValue>> Values;
			for (const FString& Missing : MissingPackages)
			{
				Values.Add(MakeShared<FJsonValueString>(Missing));
			}
			return Values;
		}());
		Report->SetArrayField(TEXT("loadFailures"), [&Failures]()
		{
			TArray<TSharedPtr<FJsonValue>> Values;
			for (const FString& Failure : Failures)
			{
				Values.Add(MakeShared<FJsonValueString>(Failure));
			}
			return Values;
		}());
		Report->SetArrayField(TEXT("warnings"), [&Warnings]()
		{
			TArray<TSharedPtr<FJsonValue>> Values;
			for (const FString& Warning : Warnings)
			{
				Values.Add(MakeShared<FJsonValueString>(Warning));
			}
			return Values;
		}());
		return Report;
	}

	bool WriteReport(const TSharedRef<FJsonObject>& Report, FString& OutFilename)
	{
		OutFilename = FPaths::Combine(FPaths::ProjectSavedDir(), ReportRelativePath);
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutFilename), true);
		FString JsonText;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
		if (!FJsonSerializer::Serialize(Report, Writer))
		{
			return false;
		}
		return FFileHelper::SaveStringToFile(
			JsonText,
			*OutFilename,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	FString JournalFilename()
	{
		return FPaths::Combine(FPaths::ProjectSavedDir(), JournalRelativePath);
	}

	bool SaveJsonObject(const TSharedRef<FJsonObject>& Object, const FString& Filename)
	{
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
		FString JsonText;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
		return FJsonSerializer::Serialize(Object, Writer)
			&& FFileHelper::SaveStringToFile(
				JsonText,
				*Filename,
				FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	bool UpdateJournalStatus(const FString& NewStatus)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *JournalFilename()))
		{
			return false;
		}
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		TSharedPtr<FJsonObject> Journal;
		if (!FJsonSerializer::Deserialize(Reader, Journal) || !Journal.IsValid())
		{
			return false;
		}
		Journal->SetStringField(TEXT("status"), NewStatus);
		return SaveJsonObject(Journal.ToSharedRef(), JournalFilename());
	}

	bool ArchiveCompletedJournal(FString& OutError)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *JournalFilename()))
		{
			return true;
		}
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		TSharedPtr<FJsonObject> Journal;
		FString Status;
		if (!FJsonSerializer::Deserialize(Reader, Journal)
			|| !Journal.IsValid()
			|| !Journal->TryGetStringField(TEXT("status"), Status))
		{
			OutError = TEXT("Previous install journal is invalid; restore it manually before another Apply.");
			return false;
		}
		if (Status != TEXT("fresh-process-verify-passed") && Status != TEXT("rolled-back"))
		{
			OutError = TEXT("Previous install is not verified or rolled back; run VerifyOnly or Rollback before another Apply.");
			return false;
		}

		const FString ArchiveDirectory = FPaths::Combine(
			FPaths::ProjectSavedDir(), TEXT("RealAssetReconstruction/InstallJournals"));
		const FString ArchiveFilename = FPaths::Combine(
			ArchiveDirectory,
			TEXT("install-journal-") + FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S_"))
				+ FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT(".json"));
		if (!CopyFile(JournalFilename(), ArchiveFilename))
		{
			OutError = TEXT("Could not archive previous journal; no new assets were copied.");
			return false;
		}
		IFileManager::Get().Delete(*JournalFilename(), false, true, true);
		return true;
	}

	bool MarkJournalVerifiedForSpecs(const TMap<FString, FAssetPackageSpec>& Specs)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *JournalFilename()))
		{
			return false;
		}
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		TSharedPtr<FJsonObject> Journal;
		const TArray<TSharedPtr<FJsonValue>>* Packages = nullptr;
		if (!FJsonSerializer::Deserialize(Reader, Journal)
			|| !Journal.IsValid()
			|| !Journal->TryGetArrayField(TEXT("packages"), Packages)
			|| Packages == nullptr
			|| Packages->Num() > Specs.Num())
		{
			return false;
		}

		TSet<FString> JournalPackages;
		for (const TSharedPtr<FJsonValue>& Value : *Packages)
		{
			const TSharedPtr<FJsonObject> Package = Value.IsValid() ? Value->AsObject() : nullptr;
			FString PackageName;
			if (!Package.IsValid() || !Package->TryGetStringField(TEXT("package"), PackageName))
			{
				return false;
			}
			if (!Specs.Contains(PackageName))
			{
				return false;
			}
			JournalPackages.Add(PackageName);
		}
		return UpdateJournalStatus(TEXT("fresh-process-verify-passed"));
	}

	bool WriteInstallJournal(
		const FString& BackupRoot,
		const TMap<FString, TArray<FString>>& BackedUpExtensions,
		FString& OutError)
	{
		if (!ArchiveCompletedJournal(OutError))
		{
			return false;
		}
		TSharedRef<FJsonObject> Journal = MakeShared<FJsonObject>();
		Journal->SetStringField(TEXT("status"), TEXT("install-in-progress"));
		Journal->SetStringField(TEXT("backupRoot"), BackupRoot);
		TArray<FString> PackageNames;
		BackedUpExtensions.GenerateKeyArray(PackageNames);
		PackageNames.Sort();
		TArray<TSharedPtr<FJsonValue>> Packages;
		for (const FString& PackageName : PackageNames)
		{
			TSharedRef<FJsonObject> Package = MakeShared<FJsonObject>();
			Package->SetStringField(TEXT("package"), PackageName);
			TArray<TSharedPtr<FJsonValue>> Extensions;
			for (const FString& Extension : BackedUpExtensions.FindChecked(PackageName))
			{
				Extensions.Add(MakeShared<FJsonValueString>(Extension));
			}
			Package->SetArrayField(TEXT("previousSidecars"), Extensions);
			Packages.Add(MakeShared<FJsonValueObject>(Package));
		}
		Journal->SetArrayField(TEXT("packages"), Packages);
		return SaveJsonObject(Journal, JournalFilename());
	}

	bool RollbackFromJournal(FString& OutError, int32& OutRestoredPackages)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *JournalFilename()))
		{
			OutError = TEXT("No install journal exists; nothing can be rolled back automatically.");
			return false;
		}

		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		TSharedPtr<FJsonObject> Journal;
		if (!FJsonSerializer::Deserialize(Reader, Journal) || !Journal.IsValid())
		{
			OutError = TEXT("The install journal is invalid; preserve it and restore the recorded backup manually.");
			return false;
		}

		FString BackupRoot;
		const TArray<TSharedPtr<FJsonValue>>* Packages = nullptr;
		if (!Journal->TryGetStringField(TEXT("backupRoot"), BackupRoot)
			|| BackupRoot.IsEmpty()
			|| !Journal->TryGetArrayField(TEXT("packages"), Packages)
			|| Packages == nullptr)
		{
			OutError = TEXT("The install journal lacks a backup root or package list.");
			return false;
		}

		FString AllowedBackupRoot = FPaths::Combine(FPaths::ProjectSavedDir(), BackupRelativeRoot);
		FPaths::NormalizeDirectoryName(AllowedBackupRoot);
		AllowedBackupRoot = FPaths::ConvertRelativePathToFull(AllowedBackupRoot);
		FPaths::NormalizeFilename(AllowedBackupRoot);
		BackupRoot = FPaths::ConvertRelativePathToFull(BackupRoot);
		FPaths::NormalizeFilename(BackupRoot);
		if (!BackupRoot.StartsWith(AllowedBackupRoot + TEXT("/"), ESearchCase::IgnoreCase))
		{
			OutError = TEXT("The journal backup path is outside Saved/RealAssetReconstruction/Backups.");
			return false;
		}

		TMap<FString, TArray<FString>> RestorePlan;
		for (const TSharedPtr<FJsonValue>& PackageValue : *Packages)
		{
			const TSharedPtr<FJsonObject> Package = PackageValue.IsValid() ? PackageValue->AsObject() : nullptr;
			FString PackageName;
			const TArray<TSharedPtr<FJsonValue>>* Extensions = nullptr;
			if (!Package.IsValid()
				|| !Package->TryGetStringField(TEXT("package"), PackageName)
				|| NormalizePackageName(PackageName) != PackageName
				|| !Package->TryGetArrayField(TEXT("previousSidecars"), Extensions)
				|| Extensions == nullptr)
			{
				OutError = TEXT("The journal contains an invalid package entry; no files were changed.");
				return false;
			}

			if (RestorePlan.Contains(PackageName) || FindPackage(nullptr, *PackageName) != nullptr)
			{
				OutError = TEXT("A journal package is duplicated or already loaded; no files were changed: ") + PackageName;
				return false;
			}

			TArray<FString> PreviousExtensions;
			bool bHasPrimaryPackage = false;
			for (const TSharedPtr<FJsonValue>& ExtensionValue : *Extensions)
			{
				if (!ExtensionValue.IsValid() || ExtensionValue->Type != EJson::String)
				{
					OutError = TEXT("The journal contains a malformed sidecar list; no files were changed.");
					return false;
				}
				const FString Extension = ExtensionValue->AsString();
				bool bKnownExtension = false;
				for (const TCHAR* KnownExtension : PackageExtensions)
				{
					bKnownExtension |= Extension == KnownExtension;
				}
				if (!bKnownExtension || PreviousExtensions.Contains(Extension))
				{
					OutError = TEXT("The journal contains an invalid or duplicate sidecar extension; no files were changed.");
					return false;
				}
				PreviousExtensions.Add(Extension);
				bHasPrimaryPackage |= Extension == TEXT(".uasset");
			}
			if (!PreviousExtensions.IsEmpty() && !bHasPrimaryPackage)
			{
				OutError = TEXT("The journal lists sidecars without a primary .uasset; no files were changed.");
				return false;
			}

			const FString BackupBase = BackupBasePath(BackupRoot, PackageName);
			for (const FString& Extension : PreviousExtensions)
			{
				if (!IFileManager::Get().FileExists(*(BackupBase + Extension))
					|| IFileManager::Get().FileSize(*(BackupBase + Extension)) <= 0)
				{
					OutError = TEXT("A required backup sidecar is missing or empty; no project files were changed: ") + (BackupBase + Extension);
					return false;
				}
			}
			RestorePlan.Add(PackageName, MoveTemp(PreviousExtensions));
		}

		if (RestorePlan.IsEmpty())
		{
			OutError = TEXT("The journal has no package entries; no files were changed.");
			return false;
		}

		const FString StageRoot = FPaths::Combine(FPaths::GetPath(BackupRoot), TEXT("RollbackStage"), TEXT("Content"));
		TArray<FString> PackageNames;
		RestorePlan.GenerateKeyArray(PackageNames);
		PackageNames.Sort();
		TArray<FString> StagedFiles;
		for (const FString& PackageName : PackageNames)
		{
			const FString BackupBase = BackupBasePath(BackupRoot, PackageName);
			const FString StageBase = PackageBasePath(StageRoot, PackageName);
			for (const FString& Extension : RestorePlan.FindChecked(PackageName))
			{
				const FString StageFile = StageBase + Extension;
				if (!CopyFile(BackupBase + Extension, StageFile))
				{
					for (const FString& StagedFile : StagedFiles)
					{
						IFileManager::Get().Delete(*StagedFile, false, true, true);
					}
					OutError = TEXT("Could not stage every backup sidecar; project Content was not changed.");
					return false;
				}
				StagedFiles.Add(StageFile);
			}
		}

		TArray<FString> Failures;
		for (const FString& PackageName : PackageNames)
		{
			const FString TargetBase = PackageBasePath(FPaths::ProjectContentDir(), PackageName);
			const FString StageBase = PackageBasePath(StageRoot, PackageName);
			DeletePackageBundle(TargetBase);
			bool bRestored = true;
			for (const FString& Extension : RestorePlan.FindChecked(PackageName))
			{
				if (!CopyFile(StageBase + Extension, TargetBase + Extension))
				{
					bRestored = false;
				}
				IFileManager::Get().Delete(*(StageBase + Extension), false, true, true);
			}

			if (bRestored)
			{
				++OutRestoredPackages;
			}
			else
			{
				Failures.Add(PackageName);
			}
		}

		Journal->SetStringField(
			TEXT("status"),
			Failures.IsEmpty() ? TEXT("rolled-back") : TEXT("rollback-incomplete"));
		Journal->SetNumberField(TEXT("restoredPackageCount"), OutRestoredPackages);
		SaveJsonObject(Journal.ToSharedRef(), JournalFilename());
		if (!Failures.IsEmpty())
		{
			OutError = FString::Join(Failures, TEXT(", "));
			return false;
		}
		return true;
	}
}

UKardsRealAssetReconstructorCommandlet::UKardsRealAssetReconstructorCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UKardsRealAssetReconstructorCommandlet::Main(const FString& Params)
{
	using namespace KardsRealAssetReconstructor;
	FString ProbePackage;
	if (FParse::Value(*Params, TEXT("ProbePackage="), ProbePackage))
	{
		ProbePackage = NormalizePackageName(ProbePackage);
		if (!ProbePackage.StartsWith(TEXT("/Game/"), ESearchCase::IgnoreCase))
		{
			UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("ProbePackage must be a /Game long package name."));
			return 1;
		}

		UPackage* LoadedPackage = LoadPackage(nullptr, *ProbePackage, LOAD_None);
		if (LoadedPackage == nullptr)
		{
			UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("PROBE_LOAD_FAILED %s"), *ProbePackage);
			return 1;
		}

		UE_LOG(LogKardsRealAssetReconstructor, Display, TEXT("PROBE_LOAD_PASSED %s"), *ProbePackage);
		return 0;
	}

	if (FParse::Param(*Params, TEXT("Rollback")))
	{
		FString RollbackError;
		int32 RestoredPackages = 0;
		const bool bRolledBack = RollbackFromJournal(RollbackError, RestoredPackages);
		if (!bRolledBack)
		{
			UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Rollback failed: %s"), *RollbackError);
			return 9;
		}
		UE_LOG(LogKardsRealAssetReconstructor, Display, TEXT("Rollback restored %d package(s). Restart the editor before loading them."), RestoredPackages);
		return 0;
	}

	FString SourceRoot;
	FString AssetRegistryFile;
	FString OnlyPackage;
	FString RootFilter;
	FString ExcludeRootsArgument;
	if (!FParse::Value(*Params, TEXT("SourceRoot="), SourceRoot))
	{
		SourceRoot = DefaultSourceRoot;
	}
	if (!FParse::Value(*Params, TEXT("AssetRegistry="), AssetRegistryFile))
	{
		AssetRegistryFile = FPaths::Combine(FPaths::GetPath(SourceRoot), TEXT("AssetRegistry.json"));
	}
	FPaths::NormalizeDirectoryName(SourceRoot);
	SourceRoot = FPaths::ConvertRelativePathToFull(SourceRoot);
	FPaths::NormalizeFilename(SourceRoot);
	AssetRegistryFile = FPaths::ConvertRelativePathToFull(AssetRegistryFile);
	FParse::Value(*Params, TEXT("OnlyPackage="), OnlyPackage);
	FParse::Value(*Params, TEXT("RootFilter="), RootFilter);
	FParse::Value(*Params, TEXT("ExcludeRoots="), ExcludeRootsArgument);
	if (!RootFilter.IsEmpty()
		&& !RootFilter.Equals(TEXT("All"), ESearchCase::IgnoreCase)
		&& !RootFilter.Equals(TEXT("Audio"), ESearchCase::IgnoreCase)
		&& !RootFilter.Equals(TEXT("Effects"), ESearchCase::IgnoreCase)
		&& !RootFilter.Equals(TEXT("Niagara"), ESearchCase::IgnoreCase)
		&& !RootFilter.Equals(TEXT("Cascade"), ESearchCase::IgnoreCase)
		&& !RootFilter.Equals(TEXT("ParticleSystem"), ESearchCase::IgnoreCase)
		&& !RootFilter.Equals(TEXT("VfxAssets"), ESearchCase::IgnoreCase)
		&& !RootFilter.Equals(TEXT("EffectBlueprints"), ESearchCase::IgnoreCase)
		&& !RootFilter.Equals(TEXT("BlueprintEffects"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Invalid RootFilter. Use All, Audio, Effects, VfxAssets, Niagara, Cascade, EffectBlueprints, or BlueprintEffects."));
		return 1;
	}
	if (!OnlyPackage.IsEmpty())
	{
		OnlyPackage = NormalizePackageName(OnlyPackage);
		if (OnlyPackage.IsEmpty())
		{
			UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("OnlyPackage must be a valid /Game long package name."));
			return 1;
		}
	}
	if (!SourceRoot.EndsWith(TEXT("/Exports/kards/Content"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("SourceRoot must be the current Exports/kards/Content directory: %s"), *SourceRoot);
		return 1;
	}

	TMap<FString, FAssetPackageSpec> Specs;
	FString Error;
	const bool bIncludeBlueprintEffects = FParse::Param(*Params, TEXT("IncludeBlueprintEffects"))
		|| RootFilter.Equals(TEXT("BlueprintEffects"), ESearchCase::IgnoreCase);
	const bool bIncludeSoftReferences = FParse::Param(*Params, TEXT("IncludeSoftReferences"));
	if (!CollectSeeds(SourceRoot, AssetRegistryFile, bIncludeBlueprintEffects, Specs, Error))
	{
		UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("%s"), *Error);
		return 1;
	}
	if (!RootFilter.IsEmpty() && !RootFilter.Equals(TEXT("All"), ESearchCase::IgnoreCase))
	{
		TArray<FString> RemoveRoots;
		for (const TPair<FString, FAssetPackageSpec>& Pair : Specs)
		{
			if (!MatchesRootFilter(Pair.Value, RootFilter))
			{
				RemoveRoots.Add(Pair.Key);
			}
		}
		for (const FString& PackageName : RemoveRoots)
		{
			Specs.Remove(PackageName);
		}
		if (Specs.IsEmpty())
		{
			UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("RootFilter matched no eligible exports: %s"), *RootFilter);
			return 1;
		}
	}
	TArray<FString> ExcludedRoots;
	if (!ExcludeRootsArgument.IsEmpty())
	{
		TArray<FString> RequestedExclusions;
		ExcludeRootsArgument.ParseIntoArray(RequestedExclusions, TEXT("|"), true);
		for (const FString& RequestedExclusion : RequestedExclusions)
		{
			const FString PackageName = NormalizePackageName(RequestedExclusion);
			if (PackageName.IsEmpty() || !Specs.Contains(PackageName))
			{
				UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("ExcludeRoots contains a package that is not a selected root: %s"), *RequestedExclusion);
				return 1;
			}
			if (!ExcludedRoots.Contains(PackageName))
			{
				ExcludedRoots.Add(PackageName);
				Specs.Remove(PackageName);
			}
		}
		ExcludedRoots.Sort();
		if (Specs.IsEmpty())
		{
			UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("ExcludeRoots removed every selected root."));
			return 1;
		}
	}
	if (!OnlyPackage.IsEmpty())
	{
		const FAssetPackageSpec* SelectedSpec = Specs.Find(OnlyPackage);
		if (SelectedSpec == nullptr)
		{
			UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("OnlyPackage is not an eligible root in the current manifest: %s"), *OnlyPackage);
			return 1;
		}
		TMap<FString, FAssetPackageSpec> SelectedSpecs;
		SelectedSpecs.Add(OnlyPackage, *SelectedSpec);
		Specs = MoveTemp(SelectedSpecs);
	}

	TArray<FString> MissingPackages;
	DiscoverPackageClosure(SourceRoot, bIncludeSoftReferences, Specs, MissingPackages);
	int32 MissingSourceDependencyCount = 0;
	for (const TPair<FString, FAssetPackageSpec>& Pair : Specs)
	{
		if (!Pair.Value.bSourceAvailable && !Pair.Value.bIsRoot)
		{
			if (MissingSourceDependencyCount < 40)
			{
				UE_LOG(LogKardsRealAssetReconstructor, Warning, TEXT("Dependency has no exported package: %s"), *Pair.Key);
			}
			++MissingSourceDependencyCount;
		}
	}
	if (MissingSourceDependencyCount > 40)
	{
		UE_LOG(LogKardsRealAssetReconstructor, Warning, TEXT("Additional unresolved dependency packages omitted from log: %d"), MissingSourceDependencyCount - 40);
	}

	const bool bApply = FParse::Param(*Params, TEXT("Apply"));
	const bool bVerifyOnly = FParse::Param(*Params, TEXT("VerifyOnly"));
	const bool bOverwriteExisting = FParse::Param(*Params, TEXT("OverwriteExisting"));
	const bool bOverwriteDependencies = FParse::Param(*Params, TEXT("OverwriteDependencies"));
	const bool bOverwriteStaleDependencies = FParse::Param(*Params, TEXT("OverwriteStaleDependencies"));
	if (bOverwriteDependencies && bOverwriteStaleDependencies)
	{
		UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Use either -OverwriteDependencies or -OverwriteStaleDependencies, not both."));
		return 2;
	}
	const bool bCookedEnabled = IsCVarEnabled(TEXT("cook.AllowCookedDataInEditorBuilds"));
	const bool bUnversionedEnabled = IsCVarEnabled(TEXT("s.AllowUnversionedContentInEditor"));
	FString BackupRoot;
	TMap<FString, TArray<FString>> BackedUpExtensions;
	TArray<FString> Failures;
	TArray<FString> Warnings;
	TArray<FString> PlannedInstallPackages;
	TArray<FString> PreservedExistingDependencies;
	FString Status = TEXT("audit-only");
	for (const TPair<FString, FAssetPackageSpec>& Pair : Specs)
	{
		bool bPreserveExistingDependency = false;
		if (ShouldInstallPackage(Pair.Value, bOverwriteDependencies, bOverwriteStaleDependencies, bPreserveExistingDependency))
		{
			PlannedInstallPackages.Add(Pair.Key);
		}
		else if (bPreserveExistingDependency)
		{
			PreservedExistingDependencies.Add(Pair.Key);
		}
	}
	PlannedInstallPackages.Sort();
	PreservedExistingDependencies.Sort();

	if (bApply && bVerifyOnly)
	{
		UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Use either -Apply or -VerifyOnly, not both."));
		return 2;
	}

	if (bApply)
	{
		if (!bCookedEnabled || !bUnversionedEnabled)
		{
			UE_LOG(
				LogKardsRealAssetReconstructor,
				Error,
				TEXT("Apply refused. Restart the editor with cook.AllowCookedDataInEditorBuilds=1 and s.AllowUnversionedContentInEditor=1 in [ConsoleVariables]."));
			return 3;
		}

		for (const TPair<FString, FAssetPackageSpec>& Pair : Specs)
		{
			if (!Pair.Value.bSourceAvailable)
			{
				UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Apply refused; source .uasset is missing: %s"), *Pair.Key);
				return 4;
			}
		}
		for (const FString& PackageName : PlannedInstallPackages)
		{
			if (FindPackage(nullptr, *PackageName) != nullptr)
			{
				UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Apply refused; package is already loaded in this process: %s"), *PackageName);
				return 5;
			}
			const FString TargetBase = PackageBasePath(FPaths::ProjectContentDir(), PackageName);
			if (!ExistingPackageExtensions(TargetBase).IsEmpty() && !bOverwriteExisting)
			{
				UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Apply refused; existing target needs explicit -OverwriteExisting: %s"), *PackageName);
				return 5;
			}
		}

		BackupRoot = MakeBackupRoot();
		for (const FString& PackageName : PlannedInstallPackages)
		{
			if (!BackupPackage(BackupRoot, Specs.FindChecked(PackageName), BackedUpExtensions))
			{
				UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Backup failed before install: %s"), *PackageName);
				return 6;
			}
		}
		FString JournalError;
		if (!WriteInstallJournal(BackupRoot, BackedUpExtensions, JournalError))
		{
			UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Could not write install journal; no content was changed: %s"), *JournalError);
			return 6;
		}

		for (const FString& PackageName : PlannedInstallPackages)
		{
			if (!InstallPackage(Specs.FindChecked(PackageName)))
			{
				Failures.Add(PackageName + TEXT(" copy failed"));
				break;
			}
		}

		if (Failures.IsEmpty())
		{
			VerifyRootPackages(Specs, Failures, Warnings);
		}
		if (!Failures.IsEmpty())
		{
			bool bRollbackSucceeded = true;
			for (const FString& PackageName : PlannedInstallPackages)
			{
				bRollbackSucceeded &= RestoreBackup(BackupRoot, Specs.FindChecked(PackageName), BackedUpExtensions);
			}
			Status = bRollbackSucceeded ? TEXT("rolled-back-after-verification-failure") : TEXT("rollback-incomplete");
			UpdateJournalStatus(bRollbackSucceeded ? TEXT("rolled-back") : TEXT("rollback-incomplete"));
			UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Install verification failed. Rollback %s; restart the editor before further package operations."), bRollbackSucceeded ? TEXT("completed") : TEXT("incomplete"));
		}
		else
		{
			Status = TEXT("installed-and-root-packages-loaded");
			UpdateJournalStatus(TEXT("installed-awaiting-fresh-process-verification"));
		}
	}
	else if (bVerifyOnly)
	{
		if (!bCookedEnabled || !bUnversionedEnabled)
		{
			UE_LOG(
				LogKardsRealAssetReconstructor,
				Error,
				TEXT("VerifyOnly refused. Restart the editor with both cooked and unversioned content CVars set to 1."));
			return 3;
		}
		VerifyRootPackages(Specs, Failures, Warnings);
		Status = Failures.IsEmpty() ? TEXT("verify-only-loaded") : TEXT("verify-only-failed");
		if (Failures.IsEmpty() && !MarkJournalVerifiedForSpecs(Specs)
			&& IFileManager::Get().FileExists(*JournalFilename()))
		{
			UE_LOG(LogKardsRealAssetReconstructor, Warning, TEXT("Fresh-process load passed, but the install journal was not updated because its package scope differs."));
		}
	}

	TSharedRef<FJsonObject> Report = BuildReport(
		Specs,
		MissingPackages,
		Failures,
		Warnings,
		PlannedInstallPackages,
		PreservedExistingDependencies,
		ExcludedRoots,
		SourceRoot,
		OnlyPackage,
		RootFilter,
		bIncludeSoftReferences,
		bOverwriteStaleDependencies,
		BackupRoot,
		Status);
	FString ReportFile;
	if (!WriteReport(Report, ReportFile))
	{
		UE_LOG(LogKardsRealAssetReconstructor, Error, TEXT("Failed to write report."));
		return 7;
	}

	int32 RootCount = 0;
	for (const TPair<FString, FAssetPackageSpec>& Pair : Specs)
	{
		RootCount += Pair.Value.bIsRoot ? 1 : 0;
	}
	UE_LOG(
		LogKardsRealAssetReconstructor,
		Display,
		TEXT("Asset reconstruction status=%s, roots=%d, dependency closure=%d, planned install=%d, preserved dependencies=%d, unresolved=%d, failures=%d"),
		*Status,
		RootCount,
		Specs.Num(),
		PlannedInstallPackages.Num(),
		PreservedExistingDependencies.Num(),
		MissingPackages.Num(),
		Failures.Num());
	UE_LOG(LogKardsRealAssetReconstructor, Display, TEXT("Report: %s"), *ReportFile);
	if (!BackupRoot.IsEmpty())
	{
		UE_LOG(LogKardsRealAssetReconstructor, Display, TEXT("Backups: %s"), *BackupRoot);
	}
	return Failures.IsEmpty() ? 0 : 8;
}
