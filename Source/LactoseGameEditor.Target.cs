// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LactoseGameEditorTarget : TargetRules
{
	public LactoseGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("LactoseGame");
		RegisterModulesCreatedByRider();
	}

	void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] { "LactoseTests", "LactoseDebug" });
	}
}
