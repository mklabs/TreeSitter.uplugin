// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "ITreeSitterModule.h"
#include "Misc/AutomationTest.h"

#include "TreeSitterParser.h"
#include "tree_sitter/api.h"

BEGIN_DEFINE_SPEC(FTreeSitterParserSpec, "TreeSitter.TreeSitterParser", EAutomationTestFlags::ProductFilter | EAutomationTestFlags_ApplicationContextMask)

	TSharedPtr<FTreeSitterParser> Parser;

END_DEFINE_SPEC(FTreeSitterParserSpec)

void FTreeSitterParserSpec::Define()
{
	BeforeEach([this]()
	{
		// Create the parser before each test
		Parser = MakeShared<FTreeSitterParser>();
		// Set language to JavaScript
		Parser->SetLanguage(ETreeSitterLanguage::JavaScript);
	});

	AfterEach([this]()
	{
		// Clean up the parser after each test
		Parser.Reset();
	});

	Describe("Parsing Source Code", [this]()
	{
		It("should parse valid JavaScript code and return a valid root node", [this]()
		{
			// Provide some source code
			const FString SourceCode = TEXT("let x = 42;");

			// Parse the code
			TSTree* Tree = Parser->Parse(SourceCode);
			const TSNode RootNode = ts_tree_root_node(Tree);

			// Root node should be valid
			TestTrue("Root node is valid", ts_node_is_null(RootNode) == false);
			TestEqual("Root node type is 'program'", FString(ts_node_type(RootNode)), TEXT("program"));

			// Clean up tree
			ts_tree_delete(Tree);
		});

		It("should handle empty source code and return an empty root node", [this]()
		{
			// Provide empty source code
			const FString SourceCode = TEXT("");

			// Parse the code
			TSTree* Tree = Parser->Parse(SourceCode);
			const TSNode RootNode = ts_tree_root_node(Tree);

			// Root node should still be valid but have no children
			TestTrue("Root node is valid", ts_node_is_null(RootNode) == false);
			TestEqual("Root node type is 'program'", FString(ts_node_type(RootNode)), TEXT("program"));
			TestEqual("Root node has no children", ts_node_child_count(RootNode), 0u);

			// Clean up tree
			ts_tree_delete(Tree);
		});

		It("should correctly parse nested structures", [this]()
		{
			// Provide some source code with nesting
			const FString SourceCode = TEXT("function greet() { console.log('hello'); }");

			// Parse the code
			TSTree* Tree = Parser->Parse(SourceCode);
			const TSNode RootNode = ts_tree_root_node(Tree);

			// Root node should have children
			TestTrue("Root node is valid", ts_node_is_null(RootNode) == false);
			TestEqual("Root node type is 'program'", FString(ts_node_type(RootNode)), TEXT("program"));

			const uint32 ChildCount = ts_node_child_count(RootNode);
			TestTrue("Root node has children", ChildCount > 0);

			// Get first child and check its type
			const TSNode FirstChild = ts_node_child(RootNode, 0);
			TestEqual("First child is 'function_declaration'", FString(ts_node_type(FirstChild)), TEXT("function_declaration"));

			// Clean up tree
			ts_tree_delete(Tree);
		});
	});

	Describe("Error Handling", [this]()
	{
		It("should return a valid tree for invalid source code", [this]()
		{
			// Provide invalid source code
			const FString SourceCode = TEXT("function () {");

			// Parse the code
			TSTree* Tree = Parser->Parse(SourceCode);
			const TSNode RootNode = ts_tree_root_node(Tree);

			// Tree should still return a valid root node
			TestTrue("Root node is valid", ts_node_is_null(RootNode) == false);
			TestEqual("Root node type is 'program'", FString(ts_node_type(RootNode)), TEXT("program"));

			// Clean up tree
			ts_tree_delete(Tree);
		});
	});
}
