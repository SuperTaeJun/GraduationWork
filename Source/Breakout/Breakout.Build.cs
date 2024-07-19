// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Breakout : ModuleRules
{
	public Breakout(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableExceptions = true;

        PublicIncludePaths.AddRange(new string[] { "Breakout" });

        PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", 
			"InputCore", "EnhancedInput", "UMG", 
			"ProceduralMeshComponent", "Niagara" ,
			"GeometryCore","DynamicMesh","MovieScene"
			,"LevelSequence"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
