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

	if (strcmp(NodeType, "atx_heading") == 0)
	{
		// Extract heading level

		// Get first child, which is expected to be an atx_h[1-6]_marker
		const TSNode HeadingMarkerNode = ts_node_child(InNode, 0);
		int32 HeadingLevel = 1;

		if (!ts_node_is_null(HeadingMarkerNode))
		{
			// Try to extract the heading level info out of the marker type:
			// atx_h1_marker => H1
			// atx_h2_marker => H2
			// etc.
			const FString HeadingNodeType = FString(ts_node_type(HeadingMarkerNode));
			const FString HeadingLevelString = HeadingNodeType.Mid(5, 1);
			HeadingLevel = FCString::Atoi(*HeadingLevelString);
		}

		// Extract inner heading_content text
		constexpr const TCHAR* FieldNameKey = TEXT("heading_content");
		const char* FieldName = TCHAR_TO_UTF8(FieldNameKey);

		const TSNode HeadingContentNode = ts_node_child_by_field_name(InNode, FieldName, strlen(FieldName));
		const FString Content = ts_node_is_null(HeadingContentNode) ? TEXT("") : ExtractNodeText(HeadingContentNode, InSource.Get());

		// Adjust font size based on level
		const float FontSize = 24 - (HeadingLevel * 2);

		return SNew(STextBlock)
			.Text(FText::FromString(Content))
			.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), FontSize));
	}

	// block_quote
	if (strcmp(NodeType, "block_quote") == 0)
	{
		// const TSNode BlockquoteMarker = ts_node_child(InNode, 0);
		const TSNode Paragraph = ts_node_child(InNode, 1);
		
		const FString Text = ts_node_is_null(Paragraph) ? ExtractNodeText(InNode, InSource.Get()) : ExtractNodeText(Paragraph, InSource.Get());

		return SNew(SBorder)
		.Padding(8.f)
		.BorderImage(FAppStyle::GetBrush("Border"))
		[
			SNew(STextBlock)
			.Text(FText::FromString(Text))
			.AutoWrapText(true)
		];
	}

	if (strcmp(NodeType, "paragraph") == 0)
	{
		const FString Text = ExtractNodeText(InNode, InSource.Get());

		return SNew(STextBlock)
			.Text(FText::FromString(TEXT("Old one")))
			.AutoWrapText(true);
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
