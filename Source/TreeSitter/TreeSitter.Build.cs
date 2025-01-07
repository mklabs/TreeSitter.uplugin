// Copyright 2024 Mickael Daniel. All Rights Reserved.

using UnrealBuildTool;

public class TreeSitter : ModuleRules
{
	public TreeSitter(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"TreeSitterLibrary"
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Projects", 
				"Slate",
				"SlateCore", 
			}
		);
		
		// TODO Move to Editor module
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd"
				}
			);
		}
	}
}