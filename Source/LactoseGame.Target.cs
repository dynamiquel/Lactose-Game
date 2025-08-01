// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LactoseGameTarget : TargetRules
{
	public LactoseGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		
		// Built for C++23, which isn't officially supported by Unreal yet.
		CppStandard = CppStandardVersion.Latest;
		CStandard = CStandardVersion.Latest;
		
		ExtraModuleNames.AddRange([
			"LactoseGame",
			"LactoseDebug"
		]);
		
	}
}
