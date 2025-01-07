// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

#include "Widgets/Views/STreeView.h"
#include "tree_sitter/api.h"

struct FTreeSitterTreeNode
{
	TArray<TSharedPtr<FTreeSitterTreeNode>> Children;
	
	FString Language;
	FString NodeType;
	FString NodeGrammarType;
	TSSymbol Symbol;
	TSSymbol GrammarSymbol;
	uint32 StartByte;
	uint32 EndByte;
	TSPoint StartPoint;
	TSPoint EndPoint;

	FString SExpression;
	FString ExtractedSource;
	
	bool bIsNull;
	bool bIsNamed;
	bool bHasError;
	bool bIsError;
	bool bIsMissing;
	bool bIsExtra;
	bool bHasChanges;
	
	FString FieldName;
	uint32 Depth;

	FTreeSitterTreeNode() = default;

	// explicit FTreeSitterTreeNode(const FString& InNodeType)
	// 	: NodeType(InNodeType)
	// {
	// }
};

class STreeSitterTreeViewer : public SCompoundWidget
{
	using STreeSitterView = STreeView<TSharedPtr<FTreeSitterTreeNode>>;
	
public:
	SLATE_BEGIN_ARGS(STreeSitterTreeViewer)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void UpdateTree(const TSNode& InRootNode, const FText& InCodeText);

	void ExpandTreeView(const TSharedRef<STreeSitterView>& InTreeView);
	void SetItemExpansionRecursive(const TSharedPtr<FTreeSitterTreeNode>& InTreeItem, bool bInExpansionState);

private:
	TArray<TSharedPtr<FTreeSitterTreeNode>> TreeItems;
	TSharedPtr<STreeSitterView> TreeView;

	// Copy of original source code to extract source info from
	FText CodeText;

	static TSharedRef<ITableRow> GenerateRow(TSharedPtr<FTreeSitterTreeNode> InItem, const TSharedRef<STableViewBase>& InOwnerTable);

	static void GetTreeChildren(TSharedPtr<FTreeSitterTreeNode> InItem, TArray<TSharedPtr<FTreeSitterTreeNode>>& OutChildren);

	void PopulateTree(const TSNode& InNode, const TSharedPtr<FTreeSitterTreeNode>& InParent, const uint32 InDepth);

	/** Tries to find the corresponding field names for this node, if any */
	static FString GetNodeFieldName(const TSNode& InNode);

	static TArray<TSFieldId> GetFieldIdsForLanguage(const TSLanguage* InLanguage);
};
