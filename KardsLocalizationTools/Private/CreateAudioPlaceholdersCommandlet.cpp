#include "CreateAudioPlaceholdersCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Metasound.h"
#include "MetasoundSource.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundWave.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogAudioPlaceholderCreator, Log, All);

namespace AudioPlaceholderCreator
{
    const TCHAR* DefaultSourceRoot =
        TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kardsC/Content/Assets/Audio");

    enum class EAudioKind : uint8
    {
        SoundWave,
        SoundCue,
        MetaSoundSource,
        MetaSoundPatch,
        SoundClass,
        SoundMix,
        SoundAttenuation
    };

    struct FAudioSpec
    {
        EAudioKind Kind = EAudioKind::SoundWave;
        FString PackageName;
        FString AssetName;
        FString SourceFile;
    };

    const TCHAR* LexToString(const EAudioKind Kind)
    {
        switch (Kind)
        {
        case EAudioKind::SoundWave: return TEXT("SoundWave");
        case EAudioKind::SoundCue: return TEXT("SoundCue");
        case EAudioKind::MetaSoundSource: return TEXT("MetaSoundSource");
        case EAudioKind::MetaSoundPatch: return TEXT("MetaSoundPatch");
        case EAudioKind::SoundClass: return TEXT("SoundClass");
        case EAudioKind::SoundMix: return TEXT("SoundMix");
        case EAudioKind::SoundAttenuation: return TEXT("SoundAttenuation");
        }
        return TEXT("Unknown");
    }

    TOptional<EAudioKind> ParseAudioKind(const FString& Type)
    {
        if (Type == TEXT("SoundWave")) return EAudioKind::SoundWave;
        if (Type == TEXT("SoundCue")) return EAudioKind::SoundCue;
        if (Type == TEXT("MetaSoundSource")) return EAudioKind::MetaSoundSource;
        if (Type == TEXT("MetaSoundPatch")) return EAudioKind::MetaSoundPatch;
        if (Type == TEXT("SoundClass")) return EAudioKind::SoundClass;
        if (Type == TEXT("SoundMix")) return EAudioKind::SoundMix;
        if (Type == TEXT("SoundAttenuation")) return EAudioKind::SoundAttenuation;
        return {};
    }

    FString ToGamePackageName(const FString& ExportPackage)
    {
        FString PackageName = ExportPackage;
        const FString ExportPrefix = TEXT("kards/Content/");
        if (PackageName.StartsWith(ExportPrefix, ESearchCase::IgnoreCase))
        {
            PackageName.RightChopInline(ExportPrefix.Len(), EAllowShrinking::No);
        }
        return TEXT("/Game/") + PackageName;
    }

    bool CollectSpecs(const FString& SourceRoot, TArray<FAudioSpec>& OutSpecs, FString& OutError)
    {
        TArray<FString> JsonFiles;
        IFileManager::Get().FindFilesRecursive(JsonFiles, *SourceRoot, TEXT("*.json"), true, false);
        if (JsonFiles.IsEmpty())
        {
            OutError = FString::Printf(TEXT("No audio JSON exports were found below %s."), *SourceRoot);
            return false;
        }

        TMap<FString, FAudioSpec> UniqueSpecs;
        for (const FString& JsonFile : JsonFiles)
        {
            FString JsonText;
            if (!FFileHelper::LoadFileToString(JsonText, *JsonFile)
                || !JsonText.Contains(TEXT("\"Package\": \"kards/Content/Assets/Audio/")))
            {
                continue;
            }

            TArray<TSharedPtr<FJsonValue>> Exports;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
            if (!FJsonSerializer::Deserialize(Reader, Exports))
            {
                UE_LOG(LogAudioPlaceholderCreator, Warning, TEXT("Skipping invalid JSON: %s"), *JsonFile);
                continue;
            }

            for (const TSharedPtr<FJsonValue>& ExportValue : Exports)
            {
                const TSharedPtr<FJsonObject> Export = ExportValue.IsValid() ? ExportValue->AsObject() : nullptr;
                FString Type;
                FString Name;
                FString ExportPackage;
                if (!Export.IsValid()
                    || !Export->TryGetStringField(TEXT("Type"), Type)
                    || !Export->TryGetStringField(TEXT("Name"), Name)
                    || !Export->TryGetStringField(TEXT("Package"), ExportPackage))
                {
                    continue;
                }
                const TOptional<EAudioKind> Kind = ParseAudioKind(Type);
                if (!Kind.IsSet())
                {
                    continue;
                }

                FAudioSpec Spec;
                Spec.Kind = Kind.GetValue();
                Spec.PackageName = ToGamePackageName(ExportPackage);
                Spec.AssetName = Name;
                Spec.SourceFile = JsonFile;
                UniqueSpecs.FindOrAdd(Spec.PackageName + TEXT(".") + Spec.AssetName, MoveTemp(Spec));
            }
        }

        UniqueSpecs.GenerateValueArray(OutSpecs);
        OutSpecs.Sort([](const FAudioSpec& Left, const FAudioSpec& Right)
        {
            const int32 PackageComparison = Left.PackageName.Compare(Right.PackageName, ESearchCase::CaseSensitive);
            return PackageComparison == 0
                ? Left.AssetName.Compare(Right.AssetName, ESearchCase::CaseSensitive) < 0
                : PackageComparison < 0;
        });
        return true;
    }

    UObject* CreateAsset(const FAudioSpec& Spec, UPackage* Package)
    {
        const FName AssetName(*Spec.AssetName);
        switch (Spec.Kind)
        {
        case EAudioKind::SoundWave:
            return NewObject<USoundWave>(
                Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
        case EAudioKind::SoundCue:
            return NewObject<USoundCue>(
                Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
        case EAudioKind::MetaSoundSource:
            return NewObject<UMetaSoundSource>(
                Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
        case EAudioKind::MetaSoundPatch:
            return NewObject<UMetaSoundPatch>(
                Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
        case EAudioKind::SoundClass:
            return NewObject<USoundClass>(
                Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
        case EAudioKind::SoundMix:
            return NewObject<USoundMix>(
                Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
        case EAudioKind::SoundAttenuation:
            return NewObject<USoundAttenuation>(
                Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
        }
        return nullptr;
    }

    bool SaveAsset(const FAudioSpec& Spec, UObject* Asset)
    {
        UPackage* Package = Asset != nullptr ? Asset->GetOutermost() : nullptr;
        if (Package == nullptr)
        {
            return false;
        }

        Asset->MarkPackageDirty();
        FAssetRegistryModule::AssetCreated(Asset);
        const FString PackageFilename = FPackageName::LongPackageNameToFilename(
            Spec.PackageName, FPackageName::GetAssetPackageExtension());
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(PackageFilename), true);

        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
        SaveArgs.SaveFlags = SAVE_NoError;
        SaveArgs.Error = GError;
        return UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs);
    }

    void WriteReports(
        const TArray<FAudioSpec>& Specs,
        const TArray<FAudioSpec>& CreatedSpecs,
        const TSet<FString>& ExistingObjects)
    {
        const FString ReportDirectory = FPaths::ProjectDir() / TEXT("Out/AudioPlaceholders");
        IFileManager::Get().MakeDirectory(*ReportDirectory, true);

        FString Inventory = TEXT("Status\tType\tObjectPath\tSourceJson\n");
        FString AllPackages;
        for (const FAudioSpec& Spec : Specs)
        {
            const FString ObjectPath = Spec.PackageName + TEXT(".") + Spec.AssetName;
            Inventory += FString::Printf(
                TEXT("%s\t%s\t%s\t%s\n"),
                ExistingObjects.Contains(ObjectPath) ? TEXT("Existing") : TEXT("Created"),
                LexToString(Spec.Kind),
                *ObjectPath,
                *Spec.SourceFile);
            AllPackages += Spec.PackageName + TEXT("\n");
        }
        FFileHelper::SaveStringToFile(
            Inventory, *(ReportDirectory / TEXT("audio-placeholder-inventory.tsv")),
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
        FFileHelper::SaveStringToFile(
            AllPackages, *(ReportDirectory / TEXT("audio-placeholder-packages.txt")),
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

        FString CreatedPackages;
        for (const FAudioSpec& Spec : CreatedSpecs)
        {
            CreatedPackages += Spec.PackageName + TEXT("\n");
        }
        FFileHelper::SaveStringToFile(
            CreatedPackages, *(ReportDirectory / TEXT("created-audio-packages.txt")),
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    }
}

UCreateAudioPlaceholdersCommandlet::UCreateAudioPlaceholdersCommandlet()
{
    IsClient = false;
    IsEditor = true;
    IsServer = false;
    LogToConsole = true;
    ShowErrorCount = true;
}

int32 UCreateAudioPlaceholdersCommandlet::Main(const FString& Params)
{
    using namespace AudioPlaceholderCreator;

    FString SourceRoot;
    if (!FParse::Value(*Params, TEXT("SourceRoot="), SourceRoot))
    {
        SourceRoot = DefaultSourceRoot;
    }
    FPaths::NormalizeDirectoryName(SourceRoot);

    TArray<FAudioSpec> Specs;
    FString Error;
    if (!CollectSpecs(SourceRoot, Specs, Error))
    {
        UE_LOG(LogAudioPlaceholderCreator, Error, TEXT("%s"), *Error);
        return 1;
    }

    TArray<FAudioSpec> CreatedSpecs;
    TSet<FString> ExistingObjects;
    int32 FailedCount = 0;
    for (const FAudioSpec& Spec : Specs)
    {
        const FString ObjectPath = Spec.PackageName + TEXT(".") + Spec.AssetName;
        if (FPackageName::DoesPackageExist(Spec.PackageName))
        {
            ExistingObjects.Add(ObjectPath);
            continue;
        }

        UPackage* Package = CreatePackage(*Spec.PackageName);
        UObject* Asset = Package != nullptr ? CreateAsset(Spec, Package) : nullptr;
        if (Asset == nullptr || !SaveAsset(Spec, Asset))
        {
            ++FailedCount;
            UE_LOG(LogAudioPlaceholderCreator, Error, TEXT("FAILED %s"), *ObjectPath);
            continue;
        }
        CreatedSpecs.Add(Spec);
    }

    WriteReports(Specs, CreatedSpecs, ExistingObjects);
    UE_LOG(
        LogAudioPlaceholderCreator,
        Display,
        TEXT("Audio placeholders: discovered=%d, created=%d, existing=%d, failed=%d"),
        Specs.Num(), CreatedSpecs.Num(), ExistingObjects.Num(), FailedCount);
    return FailedCount == 0 ? 0 : 2;
}
