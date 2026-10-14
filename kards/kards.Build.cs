using UnrealBuildTool;

public class kards : ModuleRules
{
    public kards(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;
        CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Warning;

        PublicDependencyModuleNames.AddRange(new string[] {
            "BlueprintJson",
            "Core",
            "CoreUObject",
            "Engine",
            "EnhancedInput",
            "GameplayTags",
            "KardsCore",
            "Niagara",
            "UMG",
        });

    }
}

