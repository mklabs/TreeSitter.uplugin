// Copyright 2024 Mickael Daniel. All Rights Reserved.

#include "TreeSitterModule.h"

#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Markdown/Nodes/STreeSitterMarkdownHeading.h"
#include "Markdown/Nodes/STreeSitterMarkdownParagraph.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "TreeSitter.h"
#include "Markdown/STreeSitterMarkdownPlayground.h"
#include "Playground/STreeSitterPlayground.h"

#if WITH_LIVE_CODING
#include "ILiveCodingModule.h"
#endif

#define LOCTEXT_NAMESPACE "TreeSitter"

void FTreeSitterModule::StartupModule()
{
	RegisterConsoleCommands();

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
	//
	// TODO: Handle loading of dll exports for other parsers
	// TODO: See PythonScriptPluginPreload.cpp or WindowsStylusInputPlatformAPI.cpp
	// to load all libraries found in folder, using IFileManager to do the lookup

	LoadLanguageLibraryWithDLLExport(TEXT("javascript"), tree_sitter_javascript);
	LoadLanguageLibraryWithDLLExport(TEXT("json"), tree_sitter_json);
	LoadLanguageLibraryWithDLLExport(TEXT("libtree-sitter-markdown-inline.dll"), TEXT("tree_sitter_markdown_inline"), tree_sitter_markdown_inline);
	LoadLanguageLibraryWithDLLExport(TEXT("markdown"), tree_sitter_markdown);

	// Simple sanity check to make sure the DLL loaded correctly
	CheckTreeSitter();
	CheckTreeSitterMarkdown();

	RegisterCustomWidgetInstances();

#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::LoadModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		OnLiveReloadPatchCompleteHandle = LiveCoding->GetOnPatchCompleteDelegate().AddRaw(this, &FTreeSitterModule::OnLiveReloadComplete);
	}
#endif
}

void FTreeSitterModule::ShutdownModule()
{
	UnregisterCustomWidgetInstances();
	UnregisterConsoleCommands();

	for (void* DllHandle : ParserLibraryHandles)
	{
		FWindowsPlatformProcess::FreeDllHandle(DllHandle);
	}

	ParserLibraryHandles.Reset();
	SlateWindows.Reset();
	
#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::LoadModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		LiveCoding->GetOnPatchCompleteDelegate().Remove(OnLiveReloadPatchCompleteHandle);
	}
#endif
}

ITreeSitterModule::FGetLanguageParser* FTreeSitterModule::GetLanguageParser(const ETreeSitterLanguage InLanguage)
{
	switch (InLanguage)
	{
	case ETreeSitterLanguage::JavaScript:
		return tree_sitter_javascript;
	case ETreeSitterLanguage::Json:
		return tree_sitter_json;
	case ETreeSitterLanguage::Markdown:
		return tree_sitter_markdown;
	case ETreeSitterLanguage::MarkdownInline:
		return tree_sitter_markdown_inline;
	}

	return nullptr;
}

void FTreeSitterModule::RegisterCustomMarkdownWidget(const FName& InNodeName, const FTreeSitterOnGetCustomWidgetInstance& InCustomWidgetDelegate)
{
	if (InNodeName != NAME_None)
	{
		NodeNameToWidgetFactories.Add(InNodeName, InCustomWidgetDelegate);
	}
}

void FTreeSitterModule::UnregisterCustomMarkdownWidget(const FName& InNodeName)
{
	if (InNodeName != NAME_None)
	{
		NodeNameToWidgetFactories.Remove(InNodeName);
	}
}

TSharedRef<SWidget> FTreeSitterModule::CreateWidgetForNodeType(const ::FName& InNodeType, const TSharedRef<FTreeSitterNode>& InNode, const FString& InOriginalSource)
{
	if (const FTreeSitterOnGetCustomWidgetInstance* Factory = NodeNameToWidgetFactories.Find(InNodeType))
	{
		// Call the delegate to create the widget
		return Factory->Execute(InNode, InOriginalSource);
	}

	// Fallback: Return default widget
	return SNew(STextBlock)
		.Text(FText::Format(LOCTEXT("UnknownNodeType", "Unknown Node Type: {0}"), FText::FromString(InNodeType.ToString())));
}

bool FTreeSitterModule::HasCustomWidgetForNodeType(const FName& InNodeType)
{
	return NodeNameToWidgetFactories.Contains(InNodeType);
}

void FTreeSitterModule::OnLiveReloadComplete()
{
	// Reload widget instances on live reload, we may have changed the registration names
	UnregisterCustomWidgetInstances();
	RegisterCustomWidgetInstances();
}

void FTreeSitterModule::RegisterCustomWidgetInstances()
{
	// Register the paragraph widget
	RegisterCustomMarkdownWidget(
		TEXT("paragraph"),
		FTreeSitterOnGetCustomWidgetInstance::CreateStatic(&STreeSitterMarkdownParagraph::MakeInstance)
	);

	// // Register another widget
	RegisterCustomMarkdownWidget(
		TEXT("atx_heading"),
		FTreeSitterOnGetCustomWidgetInstance::CreateStatic(&STreeSitterMarkdownHeading::MakeInstance)
	);
}

void FTreeSitterModule::UnregisterCustomWidgetInstances()
{
	// for (auto [Name, Delegate]: NodeNameToWidgetFactories)
	// {
	// 	UnregisterCustomMarkdownWidget(Name);
	// }
	
	// for (const TPair<FName, FTreeSitterOnGetCustomWidgetInstance>& Tuple : NodeNameToWidgetFactories)
	// {
	// 	UnregisterCustomMarkdownWidget(Tuple.Key);
	// }

	NodeNameToWidgetFactories.Reset();
}

void FTreeSitterModule::RegisterConsoleCommands()
{
	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TreeSitter.Test"),
		TEXT("Prints out a simple test case AST"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FTreeSitterModule::ExecuteTestCommand),
		ECVF_Default
	));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TreeSitter.Playground"),
		TEXT("Opens the slate widget for testing"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FTreeSitterModule::ExecuteWidgetCommand),
		ECVF_Default
	));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TreeSitter.GenerateMarkdownSlateWidget"),
		TEXT("Opens the slate widget for testing"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FTreeSitterModule::ExecuteGenerateMarkdownSlateWidgetCommand),
		ECVF_Default
	));
}

void FTreeSitterModule::ExecuteTestCommand(const TArray<FString>& InArgs) const
{
	CheckTreeSitterMarkdown();
}

void FTreeSitterModule::ExecuteGenerateMarkdownSlateWidgetCommand(const TArray<FString>& InArgs)
{
	const FVector2f WindowSize(850.f, 650.f);

	const FText Title = LOCTEXT("WindowTitle", "TreeSitter Playground");

	const TSharedRef<SWidget> ContentWidget = SNew(STreeSitterMarkdownPlayground);
	const TSharedPtr<SWindow> Window = OpenWindow(ContentWidget, Title, WindowSize);
	Window->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FTreeSitterModule::HandleWindowClosed));
	SlateWindows.Add(Window);
}

TSharedPtr<SWindow> FTreeSitterModule::OpenWindow(const TSharedRef<SWidget>& InWidgetContent, const FText& InTitle, const FVector2f& InWindowSize)
{
	TSharedPtr<SWindow> Window = SNew(SWindow)
		.Title(InTitle)
		.HasCloseButton(true)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SupportsTransparency(EWindowTransparency::PerWindow)
		.InitialOpacity(1.f)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(InWindowSize)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.FillHeight(1)
				[
					InWidgetContent
				]
			]
		];

	Window->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FTreeSitterModule::HandleWindowClosed));

	if (FGlobalTabmanager::Get()->GetRootWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(Window.ToSharedRef(), FGlobalTabmanager::Get()->GetRootWindow().ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(Window.ToSharedRef());
	}

	return Window;
}

void FTreeSitterModule::HandleWindowClosed(const TSharedRef<SWindow>& InWindow)
{
	SlateWindows.RemoveAll([InWindow](const TSharedPtr<SWindow>& Window)
	{
		return Window == InWindow;
	});
}

void FTreeSitterModule::UnregisterConsoleCommands()
{
	for (IConsoleCommand* ConsoleCommand : ConsoleCommands)
	{
		IConsoleManager::Get().UnregisterConsoleObject(ConsoleCommand);
	}

	ConsoleCommands.Empty();
}

void FTreeSitterModule::ExecuteWidgetCommand(const TArray<FString>& InArgs)
{
	const FVector2f WindowSize(1280.f, 1080.f);

	const FText Title = LOCTEXT("WindowTitle", "TreeSitter Playground");

	const TSharedRef<SWidget> ContentWidget = SNew(STreeSitterPlayground);
	const TSharedPtr<SWindow> Window = OpenWindow(ContentWidget, Title, WindowSize);
	Window->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FTreeSitterModule::HandleWindowClosed));
	SlateWindows.Add(Window);
}

void* FTreeSitterModule::LoadLanguageLibraryHandle(const FString& InLibraryPath)
{
	// Get the base directory of this plugin
	const FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("TreeSitter"))->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/TreeSitterLibrary/Win64/languages") / InLibraryPath);
#endif

	return !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
}

void* FTreeSitterModule::LoadLanguageLibrary(const FString& InDLLName)
{
	void* DLLHandle = LoadLanguageLibraryHandle(InDLLName);
	if (!DLLHandle)
	{
		const FString Message = FString::Printf(TEXT("Failed to load %s"), *InDLLName);
		UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
		return nullptr;
	}

	ParserLibraryHandles.Add(DLLHandle);
	return DLLHandle;
}

void* FTreeSitterModule::LoadLanguageLibraryWithDLLExport(const FString& InDLLName, const FString& InExportName, FGetLanguageParser*& OutExportHandle)
{
	void* DLLHandle = LoadLanguageLibrary(InDLLName);
	checkf(DLLHandle, TEXT("Could not load DLL from %s."), *InDLLName);

	GetDllExport(*InDLLName, DLLHandle, *InExportName, OutExportHandle);
	checkf(OutExportHandle, TEXT("Could not get DLL export '%s' from %s."), *InExportName, *InDLLName);
	return DLLHandle;
}

void* FTreeSitterModule::LoadLanguageLibraryWithDLLExport(const FString& InLanguageName, FGetLanguageParser*& OutExportHandle)
{
	const FString DLLName = FString::Printf(TEXT("libtree-sitter-%s.dll"), *InLanguageName);
	const FString ExportName = FString::Printf(TEXT("tree_sitter_%s"), *InLanguageName);

	return LoadLanguageLibraryWithDLLExport(DLLName, ExportName, OutExportHandle);
}

void FTreeSitterModule::CheckTreeSitter() const
{
	// Create a parser.
	TSParser* Parser = ts_parser_new();

	// Set the parser's language (JSON in this case).
	ts_parser_set_language(Parser, tree_sitter_json());

	// Build a syntax tree based on source code stored in a string.
	const char* SourceCode = "[1, null]";
	TSTree* Tree = ts_parser_parse_string(
		Parser,
		nullptr,
		SourceCode,
		strlen(SourceCode)
	);

	check(Tree);

	// Get the root node of the syntax tree.
	const TSNode RootNode = ts_tree_root_node(Tree);

	// Get some child nodes.
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
	UE_LOG(LogTemp, Display, TEXT("InParser pointer: %p"), Parser)

	// Print the syntax tree as an S-expression.
	char* String = ts_node_string(RootNode);
	UE_LOG(LogTemp, Display, TEXT("Syntax tree: %hs"), String);

	// Free all the heap-allocated memory.
	free(String);
	ts_tree_delete(Tree);
	ts_parser_delete(Parser);
}

void FTreeSitterModule::CheckTreeSitterMarkdown() const
{
	// Create a parser.
	TSParser* Parser = ts_parser_new();

	// Set the parser's language (JSON in this case).
	ts_parser_set_language(Parser, tree_sitter_markdown());

	// Build a syntax tree based on source code stored in a string.
	const char* Markdown = R"_Markdown(# Heading 1

Yo, is it okay like this ?

And what about **this** ?

## Heading 2

Yo

Here is a list of items:

- Foo
- Bar
- Baz is *Foobar* and ***foo*** 


## Heading 2

What about code blocks

### Heading 3

Some code

```cpp
FString Foo = TEXT("Foo");
```

---

Bottom line after HR
)_Markdown";

	TSTree* Tree = ts_parser_parse_string(
		Parser,
		nullptr,
		Markdown,
		strlen(Markdown)
	);

	check(Tree);

	// Double parse shenanigans
	// https://tree-sitter.github.io/tree-sitter/using-parsers/3-advanced-parsing.html#multi-language-documents
	// https://github.com/tree-sitter-grammars/tree-sitter-markdown#standalone-usage

	// Find ranges of inline nodes
	// Get the root node of the syntax tree.
	const TSNode RootNode = ts_tree_root_node(Tree);

	TArray<TSRange> Ranges;
	GetInlineTSRanges(RootNode, Ranges);

	ts_parser_set_language(Parser, tree_sitter_markdown_inline());
	const bool bSuccessParse = ts_parser_set_included_ranges(Parser, Ranges.GetData(), Ranges.Num());
	// constexpr bool bSuccessParse = false;
	const TSTree* InlineTree = ts_parser_parse_string(Parser, nullptr, Markdown, strlen(Markdown));
	
	// Print the syntax tree as an S-expression.
	const FString RootSexp = ts_node_string(RootNode);
	const TSNode InlineRootNode = ts_tree_root_node(InlineTree);
	char* InlineSexp = ts_node_string(InlineRootNode);
	
	// UE_LOG(LogTemp, Display, TEXT("Block: %s"), *RootSexp);
	// UE_LOG(LogTemp, Display, TEXT("Inline: %hs"), InlineSexp);
	
	DebugASTNodeInfo(Markdown, RootNode);
	
	UE_LOG(LogTemp, Display, TEXT("--- DebugASTNodeInfo below InlineRootNode - SuccessParse: %s"), *LexToString(bSuccessParse));
	DebugASTNodeInfo(Markdown, InlineRootNode);

	// Free all the heap-allocated memory.
	// free(String);
	ts_tree_delete(Tree);
	ts_parser_delete(Parser);
}

FString FTreeSitterModule::GetNodeTextForRanges(const FString& InSource, const TSPoint& InStartPoint, const TSPoint& InEndPoint)
{
	FString LocalSource = InSource;

	TArray<FString> Lines;
	InSource.ParseIntoArray(Lines, TEXT("\n"), false);

	const int32 StartRow = InStartPoint.row;
	const int32 StartColumn = InStartPoint.column;

	const int32 EndRow = InEndPoint.row;
	const int32 EndColumn = InEndPoint.column;

	if (!Lines.IsValidIndex(StartRow) || !Lines.IsValidIndex(EndRow))
	{
		return {};
	}

	const FString StartLine = Lines[StartRow];
	FString EndLine = Lines[EndRow];

	const int32 Count = StartLine.Len() - EndColumn;
	return StartLine.Mid(StartColumn, Count);
}

bool FTreeSitterModule::GetInlineTSRanges(const TSNode& InNode, TArray<TSRange>& OutRanges)
{
	if (strcmp(ts_node_type(InNode), "inline") == 0)
	{
		OutRanges.Add(TSRange({
			.start_point = ts_node_start_point(InNode),
			.end_point = ts_node_end_point(InNode),
			.start_byte = ts_node_start_byte(InNode),
			.end_byte = ts_node_end_byte(InNode)
		}));
	}
	
	const uint32 ChildCount = ts_node_child_count(InNode);
	// has children?
	if (ChildCount <= 0)
	{
		return false;
	}

	// TODO: Pretty sure I'd have to recursively search for those, inline nodes in inline nodes possible ?
	for (uint32 i = 0; i < ChildCount; ++i)
	{
		const TSNode CurrentNode = ts_node_child(InNode, i);
		if (const uint32_t ChildChildCount = ts_node_child_count(CurrentNode); ChildChildCount > 0)
		{
			GetInlineTSRanges(CurrentNode, OutRanges);
			continue;
		}
			
		if (strcmp(ts_node_type(CurrentNode), "inline") == 0)
		{
			OutRanges.Add(TSRange({
				.start_point = ts_node_start_point(CurrentNode),
				.end_point = ts_node_end_point(CurrentNode),
				.start_byte = ts_node_start_byte(CurrentNode),
				.end_byte = ts_node_end_byte(CurrentNode)
			}));
		}
	}

	return true;
}

void FTreeSitterModule::DebugASTNodeInfo(const FString& InSource, const TSNode& InNode, const FString& InPadding)
{
	constexpr bool bDisplayExpression = false;
	auto DebugNode = [InPadding, InSource](const TSNode& Node, const FString& InPrefix = TEXT(""))
	{
		const FString NodeType = ts_node_type(Node);
		const FString NodeString = ts_node_string(Node);
		const TSSymbol NodeSymbol = ts_node_symbol(Node);
		// const TSSymbol NodeSymbol = ts_node_grammar_symbol(Node);

		const TSPoint StartPoint = ts_node_start_point(Node);
		const TSPoint EndPoint = ts_node_end_point(Node);

		if (bDisplayExpression)
		{
			UE_LOG(LogTemp, Display, TEXT("%s[%d]%s%s - %s"), *InPrefix, NodeSymbol, *InPadding, *NodeType, *NodeString);
		}
		else
		{
			UE_LOG(
				LogTemp,
				Display,
				TEXT("%s[%d]%s%s ; [%d, %d] - [%d, %d] - %s"),
				*InPrefix,
				NodeSymbol,
				*InPadding,
				*NodeType,
				StartPoint.row,
				StartPoint.column,
				EndPoint.row,
				EndPoint.column,
				*GetNodeTextForRanges(InSource, StartPoint, EndPoint)
			);
		}
	};
	
	// if (strcmp(NodeType, "inline") == 0)
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("INLINE"), *InPadding, NodeType, *NodeString);
	// 	return;
	// }
	
	// has children?
	const uint32 ChildCount = ts_node_child_count(InNode);
	if (ChildCount <= 0)
	{
		// For leaf nodes (no children), print the text content
		DebugNode(InNode);
		return;
	}

	DebugNode(InNode, TEXT(""));
	for (uint32 i = 0; i < ChildCount; i++)
	{
		TSNode ChildNode = ts_node_child(InNode, i);
		const char* ChildType = ts_node_type(ChildNode);
		char* ChildNodeString = ts_node_string(ChildNode);
		// const TSSymbol ChildSymbol = ts_node_symbol(ChildNode);
		const TSSymbol ChildSymbol = ts_node_grammar_symbol(ChildNode);
		
		// UE_LOG(LogTemp, Display, TEXT("\t %hs (%hs)"), ChildType, ChildNodeString);

		// if (strcmp(ChildType, "inline") == 0)
		// {
		// 	continue;
		// }

		if (const uint32_t ChildChildCount = ts_node_child_count(ChildNode); ChildChildCount > 0)
		{
			DebugASTNodeInfo(InSource, ChildNode, InPadding + TEXT("\t"));
		}
		else
		{
			// For leaf nodes (no children), print the text content
			DebugNode(ChildNode);
		}
	}
}

IMPLEMENT_MODULE(FTreeSitterModule, TreeSitter);

#undef LOCTEXT_NAMESPACE