// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FootSyncMarkers : ModuleRules
{
	public FootSyncMarkers(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"FootSyncMarkers/Public"
			});

		PrivateIncludePaths.AddRange(
			new string[] {
				"FootSyncMarkers/Private",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			    "AnimationModifiers",
                "Core",
                "CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			});
	}
}