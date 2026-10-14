#include "CreateEffectPlaceholdersCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Camera/CameraShakeBase.h"
#include "Dom/JsonObject.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "GameFramework/Actor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemFactoryNew.h"
#include "Particles/ParticleSystem.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "VisualCardActionBaseStub.h"

DEFINE_LOG_CATEGORY_STATIC(LogEffectPlaceholderCreator, Log, All);

namespace EffectPlaceholderCreator
{
    const TCHAR* DefaultSourceRoot =
        TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kardsC/Content");

    enum class EPlaceholderKind : uint8
    {
        NiagaraSystem,
        ParticleSystem,
        ActorBlueprint,
        VisualActionBlueprint,
        CameraShakeBlueprint
    };

    struct FPlaceholderSpec
    {
        EPlaceholderKind Kind = EPlaceholderKind::NiagaraSystem;
        FString PackageName;
        FString AssetName;
        FString SourceFile;
    };

    const TCHAR* LexToString(const EPlaceholderKind Kind)
    {
        switch (Kind)
        {
        case EPlaceholderKind::NiagaraSystem:
            return TEXT("NiagaraSystem");
        case EPlaceholderKind::ParticleSystem:
            return TEXT("ParticleSystem");
        case EPlaceholderKind::ActorBlueprint:
            return TEXT("ActorBlueprint");
        case EPlaceholderKind::VisualActionBlueprint:
            return TEXT("VisualActionBlueprint");
        case EPlaceholderKind::CameraShakeBlueprint:
            return TEXT("CameraShakeBlueprint");
        default:
            return TEXT("Unknown");
        }
    }

    bool LoadJsonExports(const FString& Filename, TArray<TSharedPtr<FJsonValue>>& OutExports)
    {
        FString JsonText;
        if (!FFileHelper::LoadFileToString(JsonText, *Filename))
        {
            return false;
        }

        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
        return FJsonSerializer::Deserialize(Reader, OutExports);
    }

    FString ToGamePackageName(const FString& ExportPackage, const FString& SourceRoot, const FString& SourceFile)
    {
        FString PackageName = ExportPackage;
        const FString ExportPrefix = TEXT("kards/Content/");
        if (PackageName.StartsWith(ExportPrefix, ESearchCase::IgnoreCase))
        {
            PackageName.RightChopInline(ExportPrefix.Len(), EAllowShrinking::No);
            return TEXT("/Game/") + PackageName;
        }

        FString RelativeFilename = SourceFile;
        FPaths::MakePathRelativeTo(RelativeFilename, *(SourceRoot / TEXT("")));
        RelativeFilename = FPaths::ChangeExtension(RelativeFilename, TEXT(""));
        RelativeFilename.ReplaceInline(TEXT("\\"), TEXT("/"));
        return TEXT("/Game/") + RelativeFilename;
    }

    bool IsEffectBlueprintFile(const FString& RelativeFilename)
    {
        return RelativeFilename.StartsWith(TEXT("Blueprints/Effects/"), ESearchCase::IgnoreCase)
            || RelativeFilename.StartsWith(TEXT("Effects/"), ESearchCase::IgnoreCase);
    }

    bool IsCameraShakeBlueprintFile(const FString& RelativeFilename)
    {
        return RelativeFilename.StartsWith(
            TEXT("examples/GrenadePack/Blueprints/CameraShake"), ESearchCase::IgnoreCase);
    }

    void AddSpec(TMap<FString, FPlaceholderSpec>& Specs, FPlaceholderSpec&& Spec)
    {
        const FString Key = Spec.PackageName + TEXT(".") + Spec.AssetName;
        if (!Specs.Contains(Key))
        {
            Specs.Add(Key, MoveTemp(Spec));
        }
    }

    bool CollectSpecs(const FString& SourceRoot, TArray<FPlaceholderSpec>& OutSpecs, FString& OutError)
    {
        TArray<FString> JsonFiles;
        IFileManager::Get().FindFilesRecursive(JsonFiles, *SourceRoot, TEXT("*.json"), true, false);
        if (JsonFiles.IsEmpty())
        {
            OutError = FString::Printf(TEXT("No JSON exports were found below %s."), *SourceRoot);
            return false;
        }

        TMap<FString, FPlaceholderSpec> Specs;
        for (const FString& JsonFile : JsonFiles)
        {
            FString RelativeFilename = JsonFile;
            FPaths::MakePathRelativeTo(RelativeFilename, *(SourceRoot / TEXT("")));
            RelativeFilename.ReplaceInline(TEXT("\\"), TEXT("/"));

            FString JsonText;
            if (!FFileHelper::LoadFileToString(JsonText, *JsonFile))
            {
                continue;
            }

            const bool bMayContainSystem =
                JsonText.Contains(TEXT("\"Type\": \"NiagaraSystem\""), ESearchCase::CaseSensitive)
                || JsonText.Contains(TEXT("\"Type\": \"ParticleSystem\""), ESearchCase::CaseSensitive);
            const bool bMayContainBlueprint = IsEffectBlueprintFile(RelativeFilename)
                || IsCameraShakeBlueprintFile(RelativeFilename);
            if (!bMayContainSystem && !bMayContainBlueprint)
            {
                continue;
            }

            TArray<TSharedPtr<FJsonValue>> Exports;
            if (!LoadJsonExports(JsonFile, Exports))
            {
                UE_LOG(LogEffectPlaceholderCreator, Warning, TEXT("Skipping invalid JSON: %s"), *JsonFile);
                continue;
            }

            for (const TSharedPtr<FJsonValue>& ExportValue : Exports)
            {
                const TSharedPtr<FJsonObject> Export = ExportValue.IsValid() ? ExportValue->AsObject() : nullptr;
                if (!Export.IsValid())
                {
                    continue;
                }

                FString Type;
                FString Name;
                FString ExportPackage;
                if (!Export->TryGetStringField(TEXT("Type"), Type)
                    || !Export->TryGetStringField(TEXT("Name"), Name)
                    || !Export->TryGetStringField(TEXT("Package"), ExportPackage))
                {
                    continue;
                }

                FPlaceholderSpec Spec;
                Spec.PackageName = ToGamePackageName(ExportPackage, SourceRoot, JsonFile);
                Spec.AssetName = Name;
                Spec.SourceFile = JsonFile;

                if (Type == TEXT("NiagaraSystem"))
                {
                    Spec.Kind = EPlaceholderKind::NiagaraSystem;
                    AddSpec(Specs, MoveTemp(Spec));
                    continue;
                }

                if (Type == TEXT("ParticleSystem"))
                {
                    Spec.Kind = EPlaceholderKind::ParticleSystem;
                    AddSpec(Specs, MoveTemp(Spec));
                    continue;
                }

                if (Type != TEXT("BlueprintGeneratedClass") || !bMayContainBlueprint)
                {
                    continue;
                }

                if (Spec.AssetName.RemoveFromEnd(TEXT("_C"), ESearchCase::CaseSensitive) == false)
                {
                    continue;
                }

                if (IsCameraShakeBlueprintFile(RelativeFilename))
                {
                    Spec.Kind = EPlaceholderKind::CameraShakeBlueprint;
                }
                else
                {
                    const TSharedPtr<FJsonObject>* SuperStruct = nullptr;
                    FString SuperName;
                    if (Export->TryGetObjectField(TEXT("SuperStruct"), SuperStruct)
                        && SuperStruct != nullptr
                        && SuperStruct->IsValid())
                    {
                        (*SuperStruct)->TryGetStringField(TEXT("ObjectName"), SuperName);
                    }
                    Spec.Kind = SuperName.Contains(TEXT("VisualCardActionBase"), ESearchCase::IgnoreCase)
                        ? EPlaceholderKind::VisualActionBlueprint
                        : EPlaceholderKind::ActorBlueprint;
                }
                AddSpec(Specs, MoveTemp(Spec));
            }
        }

        Specs.GenerateValueArray(OutSpecs);
        OutSpecs.Sort([](const FPlaceholderSpec& Left, const FPlaceholderSpec& Right)
        {
            const int32 PackageComparison = Left.PackageName.Compare(Right.PackageName, ESearchCase::CaseSensitive);
            return PackageComparison == 0
                ? Left.AssetName.Compare(Right.AssetName, ESearchCase::CaseSensitive) < 0
                : PackageComparison < 0;
        });
        return true;
    }

    UClass* GetBlueprintParent(const EPlaceholderKind Kind)
    {
        switch (Kind)
        {
        case EPlaceholderKind::VisualActionBlueprint:
            return AVisualCardActionBaseStub::StaticClass();
        case EPlaceholderKind::CameraShakeBlueprint:
            return UCameraShakeBase::StaticClass();
        case EPlaceholderKind::ActorBlueprint:
        default:
            return AActor::StaticClass();
        }
    }

    UObject* CreateAsset(const FPlaceholderSpec& Spec, UPackage* Package)
    {
        const FName AssetName(*Spec.AssetName);
        switch (Spec.Kind)
        {
        case EPlaceholderKind::NiagaraSystem:
        {
            UNiagaraSystem* System = NewObject<UNiagaraSystem>(
                Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
            UNiagaraSystemFactoryNew::InitializeSystem(System, false);
            return System;
        }
        case EPlaceholderKind::ParticleSystem:
            return NewObject<UParticleSystem>(
                Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
        case EPlaceholderKind::ActorBlueprint:
        case EPlaceholderKind::VisualActionBlueprint:
        case EPlaceholderKind::CameraShakeBlueprint:
            return FKismetEditorUtilities::CreateBlueprint(
                GetBlueprintParent(Spec.Kind),
                Package,
                AssetName,
                BPTYPE_Normal,
                UBlueprint::StaticClass(),
                UBlueprintGeneratedClass::StaticClass(),
                FName(TEXT("CreateEffectPlaceholders")));
        default:
            return nullptr;
        }
    }

    bool SaveAsset(const FPlaceholderSpec& Spec, UObject* Asset)
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
        const TArray<FPlaceholderSpec>& Specs,
        const TArray<FPlaceholderSpec>& CreatedSpecs,
        const TSet<FString>& ExistingObjects)
    {
        const FString ReportDirectory = FPaths::ProjectDir() / TEXT("Out/EffectPlaceholders");
        IFileManager::Get().MakeDirectory(*ReportDirectory, true);

        FString Inventory = TEXT("Status\tType\tObjectPath\tSourceJson\n");
        for (const FPlaceholderSpec& Spec : Specs)
        {
            const FString ObjectPath = Spec.PackageName + TEXT(".") + Spec.AssetName;
            const TCHAR* Status = ExistingObjects.Contains(ObjectPath) ? TEXT("Existing") : TEXT("Created");
            Inventory += FString::Printf(
                TEXT("%s\t%s\t%s\t%s\n"),
                Status,
                LexToString(Spec.Kind),
                *ObjectPath,
                *Spec.SourceFile);
        }
        FFileHelper::SaveStringToFile(
            Inventory, *(ReportDirectory / TEXT("effect-placeholder-inventory.tsv")),
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

        FString CreatedPackages;
        for (const FPlaceholderSpec& Spec : CreatedSpecs)
        {
            CreatedPackages += Spec.PackageName + TEXT("\n");
        }
        FFileHelper::SaveStringToFile(
            CreatedPackages, *(ReportDirectory / TEXT("created-packages.txt")),
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

        FString AllPlaceholderPackages;
        for (const FPlaceholderSpec& Spec : Specs)
        {
            AllPlaceholderPackages += Spec.PackageName + TEXT("\n");
        }
        FFileHelper::SaveStringToFile(
            AllPlaceholderPackages, *(ReportDirectory / TEXT("placeholder-packages.txt")),
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    }
}

UCreateEffectPlaceholdersCommandlet::UCreateEffectPlaceholdersCommandlet()
{
    IsClient = false;
    IsEditor = true;
    IsServer = false;
    LogToConsole = true;
    ShowErrorCount = true;
}

int32 UCreateEffectPlaceholdersCommandlet::Main(const FString& Params)
{
    using namespace EffectPlaceholderCreator;

    FString SourceRoot;
    if (!FParse::Value(*Params, TEXT("SourceRoot="), SourceRoot))
    {
        SourceRoot = DefaultSourceRoot;
    }
    FPaths::NormalizeDirectoryName(SourceRoot);

    TArray<FPlaceholderSpec> Specs;
    FString Error;
    if (!CollectSpecs(SourceRoot, Specs, Error))
    {
        UE_LOG(LogEffectPlaceholderCreator, Error, TEXT("%s"), *Error);
        return 1;
    }

    TArray<FPlaceholderSpec> CreatedSpecs;
    TSet<FString> ExistingObjects;
    int32 FailedCount = 0;
    for (const FPlaceholderSpec& Spec : Specs)
    {
        const FString ObjectPath = Spec.PackageName + TEXT(".") + Spec.AssetName;
        if (FPackageName::DoesPackageExist(Spec.PackageName))
        {
            ExistingObjects.Add(ObjectPath);
            UE_LOG(LogEffectPlaceholderCreator, Display, TEXT("EXISTS  %s"), *ObjectPath);
            continue;
        }

        UPackage* Package = CreatePackage(*Spec.PackageName);
        UObject* Asset = Package != nullptr ? CreateAsset(Spec, Package) : nullptr;
        if (Asset == nullptr || !SaveAsset(Spec, Asset))
        {
            ++FailedCount;
            UE_LOG(LogEffectPlaceholderCreator, Error, TEXT("FAILED  %s"), *ObjectPath);
            continue;
        }

        CreatedSpecs.Add(Spec);
        UE_LOG(LogEffectPlaceholderCreator, Display, TEXT("CREATED %s"), *ObjectPath);
    }

    WriteReports(Specs, CreatedSpecs, ExistingObjects);
    UE_LOG(
        LogEffectPlaceholderCreator,
        Display,
        TEXT("Effect placeholders: discovered=%d, created=%d, existing=%d, failed=%d"),
        Specs.Num(),
        CreatedSpecs.Num(),
        ExistingObjects.Num(),
        FailedCount);
    return FailedCount == 0 ? 0 : 2;
}
