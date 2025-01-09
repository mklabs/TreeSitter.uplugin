// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "TreeSitterSlateMarkdown.h"

#include "ITreeSitterModule.h"
#include "TreeSitterNode.h"
#include "tree_sitter/api.h"

FString UE::TreeSitter::ExtractNodeText(const TSNode& InNode, const FString& InSource)
{
	const uint32 StartByte = ts_node_start_byte(InNode);
	const uint32 EndByte = ts_node_end_byte(InNode);

	// Extract text range from source string
	return InSource.Mid(StartByte, EndByte - StartByte);
}

TSharedRef<SWidget> UE::TreeSitter::GenerateSlateWidgetsFromNode(const TSNode& InNode, const TSharedPtr<FTreeSitterNode>& InParent, const TSharedRef<FString>& InSource, const uint32 InDepth)
{
	const char* NodeType = ts_node_type(InNode);
	
	const TSharedRef<FTreeSitterNode> NewNode = MakeShared<FTreeSitterNode>(InNode, InDepth);
	FSlateFontInfo SlateFontInfo = FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 10);

	ITreeSitterModule& TreeSitterModule = ITreeSitterModule::Get();

	if (TreeSitterModule.HasCustomWidgetForNodeType(NewNode->NodeType))
	{
		TSharedRef<SWidget> Widget = TreeSitterModule.CreateWidgetForNodeType(NodeType, NewNode, InSource.Get());
		return Widget;
	}

	if (strcmp(NodeType, "list") == 0)
	{
		TSharedRef<SVerticalBox> ListBox = SNew(SVerticalBox);

		// Process each list item
		for (uint32 i = 0; i < ts_node_child_count(InNode); i++)
		{
			TSNode Child = ts_node_child(InNode, i);
			ListBox->AddSlot()
			.AutoHeight()
			[
				GenerateSlateWidgetsFromNode(Child, NewNode, InSource, InDepth + 1) // Recursive call
			];
		}

		return ListBox;
	}

	if (strcmp(NodeType, "list_item") == 0)
	{
		const FString Text = ExtractNodeText(InNode, InSource.Get());

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("•"))) // Bullet point
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Text))
			];
	}

	TSharedRef<SVerticalBox> Container = SNew(SVerticalBox);
	for (uint32 i = 0; i < ts_node_child_count(InNode); i++)
	{
		Container->AddSlot()
		.AutoHeight()
		[
			// Recursive call
			GenerateSlateWidgetsFromNode(ts_node_child(InNode, i), NewNode, InSource, InDepth + 1)
		];
	}
	
	return Container;
}

TSharedRef<SWidget> UE::TreeSitter::GenerateMarkdownSlateWidget(const TSNode& InRootNode, const TSharedRef<FString>& InSource)
{
	return GenerateSlateWidgetsFromNode(InRootNode, nullptr, InSource, 0);
}
