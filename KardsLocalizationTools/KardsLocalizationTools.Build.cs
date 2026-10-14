using UnrealBuildTool;

public class KardsLocalizationTools : ModuleRules
{
    public KardsLocalizationTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;
        PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "Public"));

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "AudioEditor",
            "AssetRegistry",
            "AssetTools",
            "BlueprintGraph",
            "Json",
            "JsonUtilities",
            "KardsCore",
            "kards",
            "Kismet",
            "MetasoundEngine",
            "Niagara",
            "NiagaraEditor",
            "UnrealEd"
        });
    }
}
