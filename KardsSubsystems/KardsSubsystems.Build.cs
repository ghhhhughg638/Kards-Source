using UnrealBuildTool;

public class KardsSubsystems : ModuleRules
{
    public KardsSubsystems(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "BlueprintJson",
            "Core",
            "CoreUObject",
            "Engine",
            "JsonUtilities"
        });
    }
}
