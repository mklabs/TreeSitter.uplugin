// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterMarkdownHeading.h"

#include "Markdown/TreeSitterSlateMarkdown.h"
#include "TreeSitterNode.h"

#define LOCTEXT_NAMESPACE "TreeSitter"

STreeSitterMarkdownHeading::~STreeSitterMarkdownHeading()
{
}

void STreeSitterMarkdownHeading::Construct(const FArguments& InArgs, const TSharedRef<FTreeSitterNode>& InNode)
{
	using namespace UE::TreeSitter;

	const TSNode& TreeNode = InNode->Node;
	if (ts_node_is_null(TreeNode))
	{
		ChildSlot
		[

			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("InvalidTreeNode", "Tree node is invalid for: {0}"), FText::FromName(InNode->NodeType)))
		];
		return;
	}
	
	const FString OriginalSource = InArgs._InitialMarkdown;
	const FString Text = OriginalSource.Mid(InNode->StartByte, InNode->EndByte - InNode->StartByte);
	
	// const FString Text = UE::TreeSitter::ExtractNodeText(InNode, InArgs._InitialMarkdown);
	// Extract heading level

	// Get first child, which is expected to be an atx_h[1-6]_marker
	const TSNode HeadingMarkerNode = ts_node_child(TreeNode, 0);
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

	const TSNode HeadingContentNode = ts_node_child_by_field_name(TreeNode, FieldName, strlen(FieldName));
	const FString Content = ts_node_is_null(HeadingContentNode) ? TEXT("") : ExtractNodeText(HeadingContentNode, OriginalSource);

	const FString FontName = FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf");

	TSharedRef<STextBlock> TextBlock = SNew(STextBlock)
		.Text(FText::FromString(Content));
		// .Font(FSlateFontInfo(FontName, FontSize));

	if (HeadingLevel == 1)
	{
		const FColor Color = FColor::FromHex(TEXT("#ababab"));
		// const FLinearColor Color = FLinearColor(0.046665f, 0.057805f, 0.074214f, 0.4f);
		
		TextBlock->SetFont(FSlateFontInfo(FontName, 24.f));

		ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 16.f, 0.f, 3.5f)
			[
				TextBlock
			]
			+SVerticalBox::Slot()
			.MinHeight(4.f)
			.Padding(0.f, 0.f, 0.f, 16.f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(Color)
			]
		];
		return;
	}
	
	if (HeadingLevel == 2)
	{
		const FColor Color = FColor::FromHex(TEXT("#3d444db3"));
		TextBlock->SetFont(FSlateFontInfo(FontName, 18.f));

		ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 16.f, 0.f, 3.5f)
			[
				TextBlock
			]
			+SVerticalBox::Slot()
			.MinHeight(2.f)
			.Padding(0.f, 0.f, 0.f, 16.f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(Color)
			]
		];
		return;
	}
	
	if (HeadingLevel >= 3)
	{
		// Adjust font size based on level
		const float FontSize = 24 - (HeadingLevel * 2);
		TextBlock->SetFont(FSlateFontInfo(FontName, FontSize));

		if (HeadingLevel == 6)
		{
			const FColor Color = FColor::FromHex(TEXT("#3d444db3"));
			TextBlock->SetColorAndOpacity(Color);
		}
		
		ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 16.f, 0.f, 3.5f)
			[
				TextBlock
			]
			+SVerticalBox::Slot()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNullWidget::NullWidget
			]
		];
		return;
	}


	ChildSlot
	[
		TextBlock
	];
}

#undef LOCTEXT_NAMESPACE