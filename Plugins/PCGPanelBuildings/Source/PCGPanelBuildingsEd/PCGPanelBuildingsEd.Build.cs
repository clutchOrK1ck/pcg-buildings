using UnrealBuildTool;

public class PCGPanelBuildingsEd : ModuleRules
{
    public PCGPanelBuildingsEd(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "PCGPanelBuildings",
                "MessageLog",
                "UnrealEd",
            }
        );
    }
}