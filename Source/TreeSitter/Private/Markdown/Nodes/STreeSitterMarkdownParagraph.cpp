// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterMarkdownParagraph.h"

#include "TreeSitterNode.h"

STreeSitterMarkdownParagraph::~STreeSitterMarkdownParagraph()
{
}

void STreeSitterMarkdownParagraph::Construct(const FArguments& InArgs, const TSharedRef<FTreeSitterNode>& InNode)
{
	const FString OriginalSource = InArgs._InitialMarkdown;
	const FString Text = OriginalSource.Mid(InNode->StartByte, InNode->EndByte - InNode->StartByte);
	// const FString Text = UE::TreeSitter::ExtractNodeText(InNode, InArgs._InitialMarkdown);

	ChildSlot
	[
		SNew(STextBlock)
		.Text(FText::FromString(Text))
		.AutoWrapText(true)
	];
}
