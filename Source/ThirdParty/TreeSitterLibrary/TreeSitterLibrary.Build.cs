// Copyright 2024 Mickael Daniel. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class TreeSitterLibrary : ModuleRules
{
	public TreeSitterLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		PublicIncludePaths.AddRange(new[]
		{
			Path.Combine(ModuleDirectory, "include"),
		});

		// As shared dynamic library
		// if (Target.Platform == UnrealTargetPlatform.Win64)
		// {
		// 	// Add the import library
		// 	PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "dynamic", "lib", "tree-sitter.lib"));
		//
		// 	// Ensure that the DLL is staged along with the executable
		// 	RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "Win64", "dynamic", "bin", "tree-sitter.dll"));
		//
		// 	// Delay-load the DLL, so we can load it from the right place first
		// 	PublicDelayLoadDLLs.Add("tree-sitter.dll");
		//
		// 	// Result:
		// 	// MSVCRT.lib(utility.obj): Error LNK2005 : atexit already defined in tree-sitter.lib(tree-sitter.dll)
		// 	// UnrealEditor-TreeSitter.dll: Error LNK1169 : one or more multiply defined symbols found
		// }

		// Static
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "static", "lib", "tree-sitter.lib"));
		}
		
		// Load additional parsers libraries
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "Win64", "languages", "libtree-sitter-json.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "Win64", "languages", "libtree-sitter-markdown.dll"));
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "Win64", "languages", "libtree-sitter-markdown-inline.dll"));
			
			// Delay-load the DLL, so we can load it from the right place first
			// PublicDelayLoadDLLs.Add("libtree-sitter-markdown.dll");
		}
	}
}