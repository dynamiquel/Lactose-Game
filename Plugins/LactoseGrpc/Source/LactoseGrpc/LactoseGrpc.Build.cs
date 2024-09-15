// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LactoseGrpc : ModuleRules
{
	public LactoseGrpc(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] 
			{
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] 
			{
				"../../Turbolink/Source/TurboLinkGrpc/Private",
				"../../Turbolink/Source/ThirdParty/protobuf/include",
				"../../Turbolink/Source/ThirdParty/grpc/include",
				"../../Turbolink/Source/ThirdParty/abseil/include"
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"TurboLinkGrpc"
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			}
		);
	}
}
