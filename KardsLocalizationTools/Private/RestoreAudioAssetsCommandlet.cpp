#include "RestoreAudioAssetsCommandlet.h"

#include "AssetImportTask.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Dom/JsonObject.h"
#include "IAssetTools.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundNodeBranch.h"
#include "Sound/SoundNodeConcatenator.h"
#include "Sound/SoundNodeDelay.h"
#include "Sound/SoundNodeLooping.h"
#include "Sound/SoundNodeMixer.h"
#include "Sound/SoundNodeModulator.h"
#include "Sound/SoundNodeRandom.h"
#include "Sound/SoundNodeSoundClass.h"
#include "Sound/SoundNodeWaveParam.h"
#include "Sound/SoundNodeWavePlayer.h"
#include "Sound/SoundWave.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogAudioAssetRestorer, Log, All);

namespace AudioAssetRestorer
{
    const TCHAR* DefaultWaveRoot =
        TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kards/Content/Assets/Audio");
    const TCHAR* DefaultJsonRoot =
        TEXT("E:/Comet_work/Debug-win-x64/win-x64/Output/Exports/kardsC/Content/Assets/Audio");

    struct FRestoreStats
    {
        int32 WaveFiles = 0;
        int32 WaveMatched = 0;
        int32 WaveImported = 0;
        int32 WaveFailed = 0;
        int32 CueFiles = 0;
        int32 CueRestored = 0;
        int32 CueFailed = 0;
        int32 CueNodes = 0;
    };

    FString ToObjectPathFromWaveFile(const FString& WaveRoot, const FString& Filename)
    {
        FString Relative = Filename;
        FPaths::MakePathRelativeTo(Relative, *(WaveRoot / TEXT("")));
        FPaths::NormalizeFilename(Relative);
        Relative = FPaths::ChangeExtension(Relative, TEXT(""));
        Relative.RemoveFromEnd(TEXT("."));
        const FString PackageName = TEXT("/Game/Assets/Audio/") + Relative;
        return PackageName + TEXT(".") + FPaths::GetBaseFilename(Filename);
    }

    FString ToPackageNameFromExport(const FString& ExportPackage)
    {
        FString PackageName = ExportPackage;
        const FString Prefix = TEXT("kards/Content/");
        if (PackageName.StartsWith(Prefix, ESearchCase::IgnoreCase))
        {
            PackageName.RightChopInline(Prefix.Len(), EAllowShrinking::No);
        }
        return TEXT("/Game/") + PackageName;
    }

    bool ReadJsonExports(const FString& Filename, TArray<TSharedPtr<FJsonValue>>& OutExports)
    {
        FString Text;
        if (!FFileHelper::LoadFileToString(Text, *Filename))
        {
            return false;
        }
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
        return FJsonSerializer::Deserialize(Reader, OutExports);
    }

    FString ExtractReferencedName(const TSharedPtr<FJsonObject>& Reference)
    {
        if (!Reference.IsValid())
        {
            return {};
        }

        FString ObjectName;
        if (!Reference->TryGetStringField(TEXT("ObjectName"), ObjectName))
        {
            return {};
        }
        int32 ColonIndex = INDEX_NONE;
        if (ObjectName.FindLastChar(TEXT(':'), ColonIndex))
        {
            ObjectName.RightChopInline(ColonIndex + 1, EAllowShrinking::No);
        }
        int32 QuoteIndex = INDEX_NONE;
        if (ObjectName.FindLastChar(TEXT('\''), QuoteIndex))
        {
            ObjectName.LeftInline(QuoteIndex, EAllowShrinking::No);
        }
        else if (ObjectName.FindChar(TEXT('\''), QuoteIndex))
        {
            ObjectName.RightChopInline(QuoteIndex + 1, EAllowShrinking::No);
        }
        return ObjectName;
    }

    FString ReferenceToAssetPath(const TSharedPtr<FJsonObject>& Reference)
    {
        if (!Reference.IsValid())
        {
            return {};
        }

        FString AssetPathName;
        if (Reference->TryGetStringField(TEXT("AssetPathName"), AssetPathName)
            && AssetPathName.StartsWith(TEXT("/Game/")))
        {
            return AssetPathName;
        }

        FString ObjectPath;
        FString ObjectName;
        if (!Reference->TryGetStringField(TEXT("ObjectPath"), ObjectPath)
            || !Reference->TryGetStringField(TEXT("ObjectName"), ObjectName))
        {
            return {};
        }
        if (!ObjectPath.StartsWith(TEXT("kards/Content/"), ESearchCase::IgnoreCase))
        {
            return {};
        }

        int32 ExportIndex = INDEX_NONE;
        if (ObjectPath.FindLastChar(TEXT('.'), ExportIndex))
        {
            ObjectPath.LeftInline(ExportIndex, EAllowShrinking::No);
        }
        FString AssetName = ExtractReferencedName(Reference);
        int32 SubobjectIndex = INDEX_NONE;
        if (AssetName.FindLastChar(TEXT(':'), SubobjectIndex))
        {
            AssetName.RightChopInline(SubobjectIndex + 1, EAllowShrinking::No);
        }
        return ToPackageNameFromExport(ObjectPath) + TEXT(".") + AssetName;
    }

    TSharedPtr<FJsonObject> GetObjectField(const TSharedPtr<FJsonObject>& Object, const TCHAR* Name)
    {
        if (!Object.IsValid())
        {
            return nullptr;
        }
        const TSharedPtr<FJsonObject>* Value = nullptr;
        return Object->TryGetObjectField(Name, Value) && Value != nullptr ? *Value : nullptr;
    }

    float GetFloatField(const TSharedPtr<FJsonObject>& Object, const TCHAR* Name, const float DefaultValue)
    {
        double Value = DefaultValue;
        return Object.IsValid() && Object->TryGetNumberField(Name, Value)
            ? static_cast<float>(Value)
            : DefaultValue;
    }

    bool GetBoolField(const TSharedPtr<FJsonObject>& Object, const TCHAR* Name, const bool DefaultValue)
    {
        bool Value = DefaultValue;
        return Object.IsValid() && Object->TryGetBoolField(Name, Value) ? Value : DefaultValue;
    }

    FName GetNameField(const TSharedPtr<FJsonObject>& Object, const TCHAR* Name)
    {
        FString Value;
        return Object.IsValid() && Object->TryGetStringField(Name, Value) ? FName(*Value) : NAME_None;
    }

    TArray<float> GetFloatArray(const TSharedPtr<FJsonObject>& Object, const TCHAR* Name)
    {
        TArray<float> Output;
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (Object.IsValid() && Object->TryGetArrayField(Name, Values) && Values != nullptr)
        {
            Output.Reserve(Values->Num());
            for (const TSharedPtr<FJsonValue>& Value : *Values)
            {
                Output.Add(static_cast<float>(Value->AsNumber()));
            }
        }
        return Output;
    }

    UClass* SoundNodeClassForType(const FString& Type)
    {
        if (Type == TEXT("SoundNodeBranch")) return USoundNodeBranch::StaticClass();
        if (Type == TEXT("SoundNodeConcatenator")) return USoundNodeConcatenator::StaticClass();
        if (Type == TEXT("SoundNodeDelay")) return USoundNodeDelay::StaticClass();
        if (Type == TEXT("SoundNodeLooping")) return USoundNodeLooping::StaticClass();
        if (Type == TEXT("SoundNodeMixer")) return USoundNodeMixer::StaticClass();
        if (Type == TEXT("SoundNodeModulator")) return USoundNodeModulator::StaticClass();
        if (Type == TEXT("SoundNodeRandom")) return USoundNodeRandom::StaticClass();
        if (Type == TEXT("SoundNodeSoundClass")) return USoundNodeSoundClass::StaticClass();
        if (Type == TEXT("SoundNodeWaveParam")) return USoundNodeWaveParam::StaticClass();
        if (Type == TEXT("SoundNodeWavePlayer")) return USoundNodeWavePlayer::StaticClass();
        return nullptr;
    }

    void ApplyNodeProperties(
        USoundNode* Node,
        const TSharedPtr<FJsonObject>& Properties,
        TArray<FString>& OutWarnings)
    {
        if (USoundNodeBranch* Branch = Cast<USoundNodeBranch>(Node))
        {
            Branch->BoolParameterName = GetNameField(Properties, TEXT("BoolParameterName"));
        }
        else if (USoundNodeConcatenator* Concatenator = Cast<USoundNodeConcatenator>(Node))
        {
            Concatenator->InputVolume = GetFloatArray(Properties, TEXT("InputVolume"));
        }
        else if (USoundNodeDelay* Delay = Cast<USoundNodeDelay>(Node))
        {
            Delay->DelayMin = GetFloatField(Properties, TEXT("DelayMin"), Delay->DelayMin);
            Delay->DelayMax = GetFloatField(Properties, TEXT("DelayMax"), Delay->DelayMax);
        }
        else if (USoundNodeLooping* Looping = Cast<USoundNodeLooping>(Node))
        {
            Looping->bLoopIndefinitely = GetBoolField(
                Properties, TEXT("bLoopIndefinitely"), Looping->bLoopIndefinitely);
        }
        else if (USoundNodeMixer* Mixer = Cast<USoundNodeMixer>(Node))
        {
            Mixer->InputVolume = GetFloatArray(Properties, TEXT("InputVolume"));
        }
        else if (USoundNodeModulator* Modulator = Cast<USoundNodeModulator>(Node))
        {
            Modulator->PitchMin = GetFloatField(Properties, TEXT("PitchMin"), Modulator->PitchMin);
            Modulator->PitchMax = GetFloatField(Properties, TEXT("PitchMax"), Modulator->PitchMax);
        }
        else if (USoundNodeRandom* Random = Cast<USoundNodeRandom>(Node))
        {
            Random->Weights = GetFloatArray(Properties, TEXT("Weights"));
        }
        else if (USoundNodeSoundClass* SoundClassNode = Cast<USoundNodeSoundClass>(Node))
        {
            const FString Path = ReferenceToAssetPath(
                GetObjectField(Properties, TEXT("SoundClassOverride")));
            SoundClassNode->SoundClassOverride = LoadObject<USoundClass>(nullptr, *Path);
            if (!Path.IsEmpty() && SoundClassNode->SoundClassOverride == nullptr)
            {
                OutWarnings.Add(TEXT("Missing SoundClass ") + Path);
            }
        }
        else if (USoundNodeWaveParam* WaveParam = Cast<USoundNodeWaveParam>(Node))
        {
            WaveParam->WaveParameterName = GetNameField(Properties, TEXT("WaveParameterName"));
        }
        else if (USoundNodeWavePlayer* WavePlayer = Cast<USoundNodeWavePlayer>(Node))
        {
            WavePlayer->bLooping = GetBoolField(Properties, TEXT("bLooping"), false);
            const FString Path = ReferenceToAssetPath(
                GetObjectField(Properties, TEXT("SoundWaveAssetPtr")));
            USoundWave* Wave = LoadObject<USoundWave>(nullptr, *Path);
            WavePlayer->SetSoundWave(Wave);
            if (!Path.IsEmpty() && Wave == nullptr)
            {
                OutWarnings.Add(TEXT("Missing SoundWave ") + Path);
            }
        }
    }

    bool SaveAsset(UObject* Asset)
    {
        if (Asset == nullptr)
        {
            return false;
        }
        UPackage* Package = Asset->GetOutermost();
        Asset->MarkPackageDirty();
        const FString Filename = FPackageName::LongPackageNameToFilename(
            Package->GetName(), FPackageName::GetAssetPackageExtension());
        FSavePackageArgs Args;
        Args.TopLevelFlags = RF_Public | RF_Standalone;
        Args.SaveFlags = SAVE_NoError;
        Args.Error = GError;
        return UPackage::SavePackage(Package, Asset, *Filename, Args);
    }

    bool RestoreCue(
        const FString& JsonFile,
        int32& OutNodeCount,
        TArray<FString>& OutWarnings)
    {
        TArray<TSharedPtr<FJsonValue>> Exports;
        if (!ReadJsonExports(JsonFile, Exports) || Exports.IsEmpty())
        {
            OutWarnings.Add(TEXT("Invalid JSON ") + JsonFile);
            return false;
        }

        const TSharedPtr<FJsonObject> CueExport = Exports[0]->AsObject();
        FString Type;
        FString CueName;
        FString ExportPackage;
        if (!CueExport.IsValid()
            || !CueExport->TryGetStringField(TEXT("Type"), Type)
            || Type != TEXT("SoundCue")
            || !CueExport->TryGetStringField(TEXT("Name"), CueName)
            || !CueExport->TryGetStringField(TEXT("Package"), ExportPackage))
        {
            return false;
        }

        const FString PackageName = ToPackageNameFromExport(ExportPackage);
        const FString CuePath = PackageName + TEXT(".") + CueName;
        USoundCue* Cue = LoadObject<USoundCue>(nullptr, *CuePath);
        if (Cue == nullptr)
        {
            OutWarnings.Add(TEXT("Missing SoundCue placeholder ") + CuePath);
            return false;
        }

#if WITH_EDITOR
        Cue->CreateGraph();
        Cue->ClearGraph();
        for (USoundNode* ExistingNode : Cue->AllNodes)
        {
            if (ExistingNode != nullptr)
            {
                ExistingNode->Rename(
                    nullptr,
                    GetTransientPackage(),
                    REN_DontCreateRedirectors | REN_NonTransactional);
                ExistingNode->MarkAsGarbage();
            }
        }
        Cue->AllNodes.Reset();
#endif
        Cue->FirstNode = nullptr;

        TMap<FString, USoundNode*> NodesByExportName;
        TMap<FString, TSharedPtr<FJsonObject>> NodeExportsByName;
        for (int32 Index = 1; Index < Exports.Num(); ++Index)
        {
            const TSharedPtr<FJsonObject> NodeExport = Exports[Index]->AsObject();
            FString NodeType;
            FString ExportName;
            if (!NodeExport.IsValid()
                || !NodeExport->TryGetStringField(TEXT("Type"), NodeType)
                || !NodeExport->TryGetStringField(TEXT("Name"), ExportName))
            {
                continue;
            }

            UClass* NodeClass = SoundNodeClassForType(NodeType);
            if (NodeClass == nullptr)
            {
                OutWarnings.Add(FString::Printf(
                    TEXT("Unsupported node %s in %s"), *NodeType, *CuePath));
                return false;
            }

#if WITH_EDITOR
            USoundNode* Node = Cue->ConstructSoundNode<USoundNode>(NodeClass, false);
#else
            USoundNode* Node = NewObject<USoundNode>(Cue, NodeClass, NAME_None, RF_Transactional);
#endif
            Node->ChildNodes.Reset();
            ApplyNodeProperties(
                Node,
                GetObjectField(NodeExport, TEXT("Properties")),
                OutWarnings);
            NodesByExportName.Add(ExportName, Node);
            NodeExportsByName.Add(ExportName, NodeExport);
        }

        for (const TPair<FString, TSharedPtr<FJsonObject>>& Pair : NodeExportsByName)
        {
            USoundNode* const* NodePtr = NodesByExportName.Find(Pair.Key);
            USoundNode* Node = NodePtr != nullptr ? *NodePtr : nullptr;
            const TSharedPtr<FJsonObject> Properties = GetObjectField(Pair.Value, TEXT("Properties"));
            const TArray<TSharedPtr<FJsonValue>>* Children = nullptr;
            if (Node == nullptr
                || !Properties.IsValid()
                || !Properties->TryGetArrayField(TEXT("ChildNodes"), Children)
                || Children == nullptr)
            {
                continue;
            }

            for (const TSharedPtr<FJsonValue>& ChildValue : *Children)
            {
                const FString ChildName = ExtractReferencedName(ChildValue->AsObject());
                USoundNode* const* Child = NodesByExportName.Find(ChildName);
                Node->ChildNodes.Add(Child != nullptr ? *Child : nullptr);
                if (Child == nullptr)
                {
                    OutWarnings.Add(FString::Printf(
                        TEXT("Missing child %s in %s"), *ChildName, *CuePath));
                }
            }
        }

        const TSharedPtr<FJsonObject> CueProperties = GetObjectField(CueExport, TEXT("Properties"));
        const FString FirstNodeName = ExtractReferencedName(
            GetObjectField(CueProperties, TEXT("FirstNode")));
        if (USoundNode* const* FirstNode = NodesByExportName.Find(FirstNodeName))
        {
            Cue->FirstNode = *FirstNode;
        }
        else
        {
            OutWarnings.Add(TEXT("Missing FirstNode in ") + CuePath);
            return false;
        }

        Cue->VolumeMultiplier = GetFloatField(
            CueProperties, TEXT("VolumeMultiplier"), Cue->VolumeMultiplier);
        Cue->bPrimeOnLoad = GetBoolField(CueProperties, TEXT("bPrimeOnLoad"), Cue->bPrimeOnLoad);
        Cue->bExcludeFromRandomNodeBranchCulling = GetBoolField(
            CueProperties,
            TEXT("bExcludeFromRandomNodeBranchCulling"),
            Cue->bExcludeFromRandomNodeBranchCulling);

        const FString SoundClassPath = ReferenceToAssetPath(
            GetObjectField(CueProperties, TEXT("SoundClassObject")));
        if (!SoundClassPath.IsEmpty())
        {
            Cue->SoundClassObject = LoadObject<USoundClass>(nullptr, *SoundClassPath);
            if (Cue->SoundClassObject == nullptr)
            {
                OutWarnings.Add(TEXT("Missing cue SoundClass ") + SoundClassPath);
            }
        }

        const FString AttenuationPath = ReferenceToAssetPath(
            GetObjectField(CueProperties, TEXT("AttenuationSettings")));
        if (!AttenuationPath.IsEmpty())
        {
            Cue->AttenuationSettings = LoadObject<USoundAttenuation>(nullptr, *AttenuationPath);
            if (Cue->AttenuationSettings == nullptr)
            {
                OutWarnings.Add(TEXT("Missing attenuation ") + AttenuationPath);
            }
        }

#if WITH_EDITOR
        Cue->LinkGraphNodesFromSoundNodes();
#else
        Cue->CacheAggregateValues();
#endif
        OutNodeCount = NodesByExportName.Num();
        return SaveAsset(Cue);
    }

    void WriteReport(
        const FRestoreStats& Stats,
        const TArray<FString>& Warnings,
        const FString& WaveRoot,
        const FString& JsonRoot)
    {
        const FString ReportDirectory = FPaths::ProjectDir() / TEXT("Out/RestoredAudio");
        IFileManager::Get().MakeDirectory(*ReportDirectory, true);
        FString Report = FString::Printf(
            TEXT("WaveRoot\t%s\nJsonRoot\t%s\n")
            TEXT("WaveFiles\t%d\nWaveMatched\t%d\nWaveImported\t%d\nWaveFailed\t%d\n")
            TEXT("CueFiles\t%d\nCueRestored\t%d\nCueFailed\t%d\nCueNodes\t%d\n")
            TEXT("Warnings\t%d\n\n"),
            *WaveRoot,
            *JsonRoot,
            Stats.WaveFiles,
            Stats.WaveMatched,
            Stats.WaveImported,
            Stats.WaveFailed,
            Stats.CueFiles,
            Stats.CueRestored,
            Stats.CueFailed,
            Stats.CueNodes,
            Warnings.Num());
        for (const FString& Warning : Warnings)
        {
            Report += Warning + TEXT("\n");
        }
        FFileHelper::SaveStringToFile(
            Report,
            *(ReportDirectory / TEXT("restore-audio-report.txt")),
            FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    }
}

URestoreAudioAssetsCommandlet::URestoreAudioAssetsCommandlet()
{
    IsClient = false;
    IsEditor = true;
    IsServer = false;
    LogToConsole = true;
    ShowErrorCount = true;
}

int32 URestoreAudioAssetsCommandlet::Main(const FString& Params)
{
    using namespace AudioAssetRestorer;

    FString WaveRoot;
    FString JsonRoot;
    if (!FParse::Value(*Params, TEXT("WaveRoot="), WaveRoot))
    {
        WaveRoot = DefaultWaveRoot;
    }
    if (!FParse::Value(*Params, TEXT("JsonRoot="), JsonRoot))
    {
        JsonRoot = DefaultJsonRoot;
    }
    FPaths::NormalizeDirectoryName(WaveRoot);
    FPaths::NormalizeDirectoryName(JsonRoot);

    FRestoreStats Stats;
    TArray<FString> Warnings;
    TArray<FString> WaveFiles;
    IFileManager::Get().FindFilesRecursive(WaveFiles, *WaveRoot, TEXT("*.wav"), true, false);
    Stats.WaveFiles = WaveFiles.Num();
    WaveFiles.Sort();

    TArray<UAssetImportTask*> ImportTasks;
    ImportTasks.Reserve(WaveFiles.Num());
    for (const FString& WaveFile : WaveFiles)
    {
        const FString ObjectPath = ToObjectPathFromWaveFile(WaveRoot, WaveFile);
        USoundWave* ExistingWave = LoadObject<USoundWave>(nullptr, *ObjectPath);
        if (ExistingWave == nullptr)
        {
            ++Stats.WaveFailed;
            Warnings.Add(TEXT("Missing SoundWave placeholder ") + ObjectPath);
            continue;
        }
        ++Stats.WaveMatched;

        FString PackageName;
        FString AssetName;
        ObjectPath.Split(TEXT("."), &PackageName, &AssetName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
        UAssetImportTask* Task = NewObject<UAssetImportTask>();
        Task->Filename = WaveFile;
        Task->DestinationPath = FPackageName::GetLongPackagePath(PackageName);
        Task->DestinationName = AssetName;
        Task->bAutomated = true;
        Task->bReplaceExisting = true;
        Task->bReplaceExistingSettings = false;
        Task->bSave = true;
        Task->bAsync = false;
        ImportTasks.Add(Task);
    }

    FAssetToolsModule& AssetToolsModule =
        FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
    AssetToolsModule.Get().ImportAssetTasks(ImportTasks);
    for (UAssetImportTask* Task : ImportTasks)
    {
        const TArray<UObject*>& ImportedObjects = Task->GetObjects();
        USoundWave* Wave = ImportedObjects.Num() > 0 ? Cast<USoundWave>(ImportedObjects[0]) : nullptr;
        if (Wave != nullptr && Wave->GetDuration() > 0.0f)
        {
            ++Stats.WaveImported;
        }
        else
        {
            ++Stats.WaveFailed;
            Warnings.Add(TEXT("Failed to import ") + Task->Filename);
        }
    }

    TArray<FString> JsonFiles;
    IFileManager::Get().FindFilesRecursive(JsonFiles, *JsonRoot, TEXT("*.json"), true, false);
    JsonFiles.Sort();
    for (const FString& JsonFile : JsonFiles)
    {
        TArray<TSharedPtr<FJsonValue>> Exports;
        if (!ReadJsonExports(JsonFile, Exports) || Exports.IsEmpty())
        {
            continue;
        }
        const TSharedPtr<FJsonObject> FirstExport = Exports[0]->AsObject();
        FString Type;
        if (!FirstExport.IsValid()
            || !FirstExport->TryGetStringField(TEXT("Type"), Type)
            || Type != TEXT("SoundCue"))
        {
            continue;
        }

        ++Stats.CueFiles;
        int32 NodeCount = 0;
        if (RestoreCue(JsonFile, NodeCount, Warnings))
        {
            ++Stats.CueRestored;
            Stats.CueNodes += NodeCount;
        }
        else
        {
            ++Stats.CueFailed;
        }
    }

    WriteReport(Stats, Warnings, WaveRoot, JsonRoot);
    UE_LOG(
        LogAudioAssetRestorer,
        Display,
        TEXT("Audio restore: waves=%d/%d cues=%d/%d cueNodes=%d failed=%d"),
        Stats.WaveImported,
        Stats.WaveFiles,
        Stats.CueRestored,
        Stats.CueFiles,
        Stats.CueNodes,
        Stats.WaveFailed + Stats.CueFailed);
    return Stats.WaveFailed == 0 && Stats.CueFailed == 0 ? 0 : 2;
}
