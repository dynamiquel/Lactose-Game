// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Catalyst : ModuleRules
{
	public Catalyst(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange([
			"Core",
			"HTTP"
		]);
		
		PrivateDependencyModuleNames.AddRange([
			"CoreUObject",
			"Engine",
			"Json",
			"JsonSerialization",
			"JsonUtilities"
		]);
	}
}
