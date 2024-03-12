// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Cftwo : ModuleRules
{
	public Cftwo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Niagara", "AIModule", "Perlin_Noise", "ProceduralMeshComponent", "Foliage", "Core", "Networking", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	
        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "OnlineSubsystem" });
            DynamicallyLoadedModuleNames.Add("OnlineSubsystemGooglePlay");
        }
    }
}
