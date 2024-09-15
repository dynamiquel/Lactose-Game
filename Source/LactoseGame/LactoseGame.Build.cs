// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LactoseGame : ModuleRules
{
	public LactoseGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
            {
			    "Core", 
			    "CoreUObject", 
			    "Engine", 
			    "InputCore", 
			    "EnhancedInput", 
			    "DiscordGame", 
			    "LactoseGrpc",
			    "TurboLinkGrpc"
            }
        );
		
		/*PrivateDependencyModuleNames.AddRange(new[]
		{
			
		});*/

	}
}
