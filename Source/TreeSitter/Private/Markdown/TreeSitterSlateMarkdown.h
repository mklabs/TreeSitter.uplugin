// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

struct FTreeSitterNode;
struct TSNode;

namespace UE::TreeSitter
{
	/** Helper Function to Extract Text from a Node */
	FString ExtractNodeText(const TSNode& InNode, const FString& InSource);

	/** Recursive function to process the tree and create Slate widgets for each node */
	TSharedRef<SWidget> GenerateSlateWidgetsFromNode(const TSNode& InNode, const TSharedPtr<FTreeSitterNode>& InParent, const FString& InSource, const uint32 InDepth);

	/** Wrap the root node in a container (like SVerticalBox) to display Markdown content */
	TSharedRef<SWidget> GenerateMarkdownSlateWidget(const TSNode& InRootNode, const FString& InSource);
}
