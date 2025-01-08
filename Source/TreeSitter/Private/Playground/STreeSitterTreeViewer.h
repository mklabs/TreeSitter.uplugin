// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

#include "Widgets/Views/STreeView.h"

struct FTreeSitterNode;
struct TSNode;
struct TSLanguage;

using TSFieldId = uint16_t;

class STreeSitterTreeViewer : public SCompoundWidget
{
	using STreeSitterView = STreeView<TSharedPtr<FTreeSitterNode>>;
	
public:
	SLATE_BEGIN_ARGS(STreeSitterTreeViewer)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void UpdateTree(const TSNode& InRootNode, const FText& InCodeText);

	void ExpandTreeView(const TSharedRef<STreeSitterView>& InTreeView);
	void SetItemExpansionRecursive(const TSharedPtr<FTreeSitterNode>& InTreeItem, bool bInExpansionState);

private:
	TArray<TSharedPtr<FTreeSitterNode>> TreeItems;
	TSharedPtr<STreeSitterView> TreeView;

	// Copy of original source code to extract source info from
	FText CodeText;

	static TSharedRef<ITableRow> GenerateRow(TSharedPtr<FTreeSitterNode> InItem, const TSharedRef<STableViewBase>& InOwnerTable);

	static void GetTreeChildren(TSharedPtr<FTreeSitterNode> InItem, TArray<TSharedPtr<FTreeSitterNode>>& OutChildren);

	void PopulateTree(const TSNode& InNode, const TSharedPtr<FTreeSitterNode>& InParent, const uint32 InDepth);

	/** Tries to find the corresponding field names for this node, if any */
	static FString GetNodeFieldName(const TSNode& InNode);

	static TArray<TSFieldId> GetFieldIdsForLanguage(const TSLanguage* InLanguage);
};
