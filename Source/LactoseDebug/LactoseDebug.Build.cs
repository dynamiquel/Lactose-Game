using UnrealBuildTool;

public class LactoseDebug : ModuleRules
{
    public LactoseDebug(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "DebugOverlay",
                "LactoseGame"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "ImGui", 
                "DiscordGame",
                "HTTP"
            }
        );
        
        PrivateIncludePaths.AddRange(
            new string[]
            {
                "LactoseGame/Private",
            });
    }
}