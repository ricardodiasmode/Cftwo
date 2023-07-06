// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Cftwo : ModuleRules
{
	public Cftwo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Foliage", "Core", "Networking", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });
	}
}
