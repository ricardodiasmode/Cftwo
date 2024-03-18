// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CftwoTarget : TargetRules
{
	public CftwoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		ExtraModuleNames.Add("Cftwo");

		BuildEnvironment = TargetBuildEnvironment.Unique;
		bUseLoggingInShipping = true;

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			ExtraModuleNames.Add("OnlineSubsystemGooglePlay");
			ExtraModuleNames.Add("OnlineSubsystem");
			ExtraModuleNames.Add("AndroidAdvertising");
		}
	}
}
