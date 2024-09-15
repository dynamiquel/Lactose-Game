// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LactoseGame : ModuleRules
{
	public LactoseGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
            {
			    "Core", 
			    "CoreUObject", 
			    "Engine", 
			    "InputCore", 
			    "EnhancedInput", 
			    "DiscordGame"
		    }
        );
	}
}
