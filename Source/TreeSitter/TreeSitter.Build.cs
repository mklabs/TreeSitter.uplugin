// Copyright 2024 Mickael Daniel. All Rights Reserved.

using UnrealBuildTool;

public class TreeSitter : ModuleRules
{
	public TreeSitter(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects",
				"TreeSitterLibrary",
			}
		);
	}
}