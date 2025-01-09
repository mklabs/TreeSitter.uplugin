// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterMarkdownBlockquote.h"

#include "Markdown/TreeSitterSlateMarkdown.h"
#include "TreeSitterNode.h"

#define LOCTEXT_NAMESPACE "TreeSitter"

STreeSitterMarkdownBlockquote::~STreeSitterMarkdownBlockquote()
{
}

void STreeSitterMarkdownBlockquote::Construct(const FArguments& InArgs, const TSharedRef<FTreeSitterNode>& InNode)
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
	const FString SourceText = OriginalSource.Mid(InNode->StartByte, InNode->EndByte - InNode->StartByte);

	const TSNode Paragraph = ts_node_child(TreeNode, 1);
	const FString Text = ts_node_is_null(Paragraph) ? SourceText : ExtractNodeText(Paragraph, OriginalSource);

	const FColor BorderColor = FColor::FromHex(TEXT("#3d444d"));
	const FColor TextColor = FColor::FromHex(TEXT("#9198a1"));
	
	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		// .MinWidth(1.f)
		.AutoWidth()
		.Padding(0.f, 0.f, 8.f, 0.f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ErrorReporting.Box"))
			.BorderBackgroundColor(BorderColor)
		]
		+ SHorizontalBox::Slot()
		// .FillWidth(1.f)
		.VAlign(VAlign_Bottom)
		// .Padding(0.f, 8.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Text))
			.ColorAndOpacity(TextColor)
			.Margin(FMargin(0.f, 8.f, 0.f, -8.f))
			// .LineHeightPercentage(2.f)
			// .Justification(ETextJustify::Center)
			.AutoWrapText(true)

		]
	];
}

#undef LOCTEXT_NAMESPACE