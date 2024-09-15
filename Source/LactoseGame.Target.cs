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
		ExtraModuleNames.Add("LactoseGame");
		RegisterModulesCreatedByRider();
	}

	void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] { "LactoseDebug" });
	}
}
