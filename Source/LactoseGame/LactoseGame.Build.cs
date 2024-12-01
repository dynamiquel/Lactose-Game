// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LactoseGame : ModuleRules
{
	public LactoseGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(new[]
        {
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"DiscordGame",
			"HTTP",
        }
        );
		
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Json",
			"JsonSerialization",
			"JsonUtilities"
		});
	}
}
