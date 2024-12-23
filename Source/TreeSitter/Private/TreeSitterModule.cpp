// Copyright 2024 Mickael Daniel. All Rights Reserved.

#include "TreeSitterModule.h"

#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "TreeSitter.h"

void FTreeSitterModule::StartupModule()
{

	// Using a shared library below
	//
	// Note: See Build.cs file and issues with linker and multiple defined symbols error
	//
	// 		LibraryHandle = LoadLibraryHandle();
	// 		if (!LibraryHandle)
	// 		{
	// 			const FString Message = TEXT("Failed to load tree-sitter.dll");
	// 			UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
	// 			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
	// 			return;
	// 		}

	// Load language parsers libraries
	// TODO: See PythonScriptPluginPreload.cpp or WindowsStylusInputPlatformAPI.cpp
	// to load all libraries found in folder, using IFileManager to do the lookup
	
	const FString JsonDllName = TEXT("libtree-sitter-json.dll");
	void* DLLHandle = LoadLanguageLibraryHandle(JsonDllName);
	if (!DLLHandle)
	{
		const FString Message = TEXT("Failed to load tree-sitter.dll");
		UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
		return;
	}

	ParserLibraryHandles.Add(DLLHandle);

	// TODO: Handle loading of dll exports for other parsers
	// TODO: Compile and link markdown parser that's missing in Win64/languages
	GetDllExport(*JsonDllName, DLLHandle, TEXT("tree_sitter_json"), tree_sitter_json);
	
	// Simple sanity check to make sure the DLL loaded correctly
	TSParser* Parser = ts_parser_new();
	CheckTreeSitter(Parser);
	
	if (Parser)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully loaded tree-sitter.dll"));
	}
	else
	{
		const FString Message = TEXT("Loading of tree-sitter.dll produced unexpected result");
		UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
	}
}

void FTreeSitterModule::ShutdownModule()
{
	for (void* DllHandle : ParserLibraryHandles)
	{
		FWindowsPlatformProcess::FreeDllHandle(DllHandle);
	}

	ParserLibraryHandles.Reset();
}

void* FTreeSitterModule::LoadLibraryHandle()
{
	// Get the base directory of this plugin
	const FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("TreeSitter"))->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TreeSitterLibrary/dynamic/bin/tree-sitter.dll"));;
#endif

	return !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
}

void* FTreeSitterModule::LoadLanguageLibraryHandle(const FString& InLibraryPath)
{
	// Get the base directory of this plugin
	const FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("TreeSitter"))->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TreeSitterLibrary/Win64/languages") / InLibraryPath);;
#endif

	return !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
}

void FTreeSitterModule::CheckTreeSitter(TSParser* InParser) const
{
	// Create a parser.
	// TSParser* Parser = ts_parser_new();
	
	// Set the parser's language (JSON in this case).
	ts_parser_set_language(InParser, tree_sitter_json());

	// Build a syntax tree based on source code stored in a string.
	const char* SourceCode = "[1, null]";
	const TSTree* Tree = ts_parser_parse_string(
		InParser,
		nullptr,
		SourceCode,
		strlen(SourceCode)
	);

	check(Tree);

	// Get the root node of the syntax tree.
	const TSNode RootNode = ts_tree_root_node(Tree);

	// Get some child nodes.Tree = {const TSTree *} 0x000001c4639c0710 {root={data={is_inline=false, visible=false, named=false, ...}, ptr=0x000001c463f99760 {={={ref_count=1, padding={bytes=0, extent={row=0, column=0}}, size={bytes=9, extent={row=0, column=9}}, ...}, ={external_scanner_state={={={long_data=...}, ...}}}, ...}}}, ...}... View
	const TSNode ArrayNode = ts_node_named_child(RootNode, 0);
	const TSNode NumberNode = ts_node_named_child(ArrayNode, 0);

	// Check that the nodes have the expected types.
	check(strcmp(ts_node_type(RootNode), "document") == 0);
	check(strcmp(ts_node_type(ArrayNode), "array") == 0);
	check(strcmp(ts_node_type(NumberNode), "number") == 0);

	// Check that the nodes have the expected child counts.
	check(ts_node_child_count(RootNode) == 1);
	check(ts_node_child_count(ArrayNode) == 5);
	check(ts_node_named_child_count(ArrayNode) == 2);
	check(ts_node_child_count(NumberNode) == 0);

	UE_LOG(LogTemp, Display, TEXT("Tree pointer: %p"), Tree)
	UE_LOG(LogTemp, Display, TEXT("InParser pointer: %p"), InParser)
}

IMPLEMENT_MODULE(FTreeSitterModule, TreeSitter);
