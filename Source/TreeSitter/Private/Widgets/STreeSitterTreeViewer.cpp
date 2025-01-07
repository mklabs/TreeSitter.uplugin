// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterTreeViewer.h"

#include "tree_sitter/api.h"
#include "Styling/CoreStyle.h"

void STreeSitterTreeViewer::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.Padding(4.f)
		.BorderImage(FAppStyle::GetBrush("Border"))
		[
			SAssignNew(TreeView, STreeSitterView)
			.TreeItemsSource(&TreeItems)
			.OnGenerateRow_Static(&STreeSitterTreeViewer::GenerateRow)
			.OnGetChildren_Static(&STreeSitterTreeViewer::GetTreeChildren)
		]
	];
	
	if (ensure(TreeView.IsValid()))
	{
		ExpandTreeView(TreeView.ToSharedRef());
	}
}

void PrintNode(TSNode Node, uint32 Depth) {
    // Indentation for current depth
    FString Indent = FString::ChrN(Depth * 2, ' ');

    // Get node type and range
    FString NodeType = UTF8_TO_TCHAR(ts_node_type(Node));
	TSPoint StartPoint = ts_node_start_point(Node);
	
    uint32 StartRow = StartPoint.row;
    uint32 StartCol = StartPoint.column;
    uint32 EndRow = StartPoint.row;
    uint32 EndCol = StartPoint.column;

    // Print opening line
    if (ts_node_is_named(Node))
    {
	    UE_LOG(LogTemp, Log, TEXT("%s(%s ; [%u, %u] - [%u, %u] javascript"),
	           *Indent, *NodeType, StartRow, StartCol, EndRow, EndCol
	    );
    }

    // Recursively print children
    uint32 ChildCount = ts_node_child_count(Node);
    for (uint32 i = 0; i < ChildCount; i++) {
        PrintNode(ts_node_child(Node, i), Depth + 1);
    }

    // Print closing parentheses
    UE_LOG(LogTemp, Log, TEXT("%s)"), *Indent);
}

void STreeSitterTreeViewer::UpdateTree(const TSNode& InRootNode, const FText& InCodeText)
{
	CodeText = InCodeText;

	TreeItems.Empty();
	PopulateTree(InRootNode, nullptr, 0);
	TreeView->RequestTreeRefresh();
	ExpandTreeView(TreeView.ToSharedRef());
}

void STreeSitterTreeViewer::ExpandTreeView(const TSharedRef<STreeSitterView>& InTreeView)
{
	for (const TSharedPtr<FTreeSitterTreeNode>& Item : TreeItems)
	{
		SetItemExpansionRecursive(Item, true);
	}
}

void STreeSitterTreeViewer::SetItemExpansionRecursive(const TSharedPtr<FTreeSitterTreeNode>& InTreeItem, bool bInExpansionState)
{
	if (InTreeItem.IsValid())
	{
		TreeView->SetItemExpansion(InTreeItem, bInExpansionState);

		for (TSharedPtr<FTreeSitterTreeNode>& ChildModel : InTreeItem->Children)
		{
			SetItemExpansionRecursive(ChildModel, bInExpansionState);
		}
	}
}

TSharedRef<ITableRow> STreeSitterTreeViewer::GenerateRow(TSharedPtr<FTreeSitterTreeNode> InItem, const TSharedRef<STableViewBase>& InOwnerTable)
{
	// (program ; [1, 0] - [10, 0] javascript
	// parameters: (formal_parameters ; [1, 14] - [1, 16] javascript

	// Indentation for current depth
	const bool bHasChildren = !InItem->Children.IsEmpty();
	constexpr bool bShowParenthesis = false;
	
	const FString OpeningParenthesis = bShowParenthesis ? TEXT("(") : TEXT("");
	const FString ClosingParenthesis = bShowParenthesis ?
		bHasChildren ? TEXT("") : FString::ChrN(InItem->Depth, TEXT(')')) :
		TEXT("");

	const FString LineOutput = FString::Printf(
		TEXT("%s%s%s ; [%d, %d] - [%d, %d] %s"),
		*OpeningParenthesis,
		*InItem->NodeType,
		*ClosingParenthesis,
		InItem->StartPoint.row,
		InItem->StartPoint.column,
		InItem->EndPoint.row,
		InItem->EndPoint.column,
		*InItem->Language
	);

	const FString FieldName = InItem->FieldName;
	const bool bHasFieldName = !FieldName.IsEmpty();

	// name: (identifier) ; [1, 9] - [1, 14] javascript
	// parameters: (formal_parameters ; [1, 14] - [1, 16] javascript
	const FString NamedLineOutput = FString::Printf(
		TEXT("%s: %s%s%s ; [%d, %d] - [%d, %d] %s"),
		*InItem->FieldName,
		*OpeningParenthesis,
		*InItem->NodeType,
		*ClosingParenthesis,
		InItem->StartPoint.row,
		InItem->StartPoint.column,
		InItem->EndPoint.row,
		InItem->EndPoint.column,
		*InItem->Language
	);

	// const FMargin Padding = InItem->Depth == 0 ? FMargin(40.f, 40.f, 4.f, 4.f) : FMargin(4.f, 4.f, 4.f, 4.f);
	const FMargin Padding = FMargin(4.f, 4.f, 4.f, 4.f);
	return SNew(STableRow<TSharedPtr<FTreeSitterTreeNode>>, InOwnerTable)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.Padding(Padding)
			[
				SNew(STextBlock)
				// .Text(FText::FromString(InItem->NodeType))
				.Text(FText::FromString(bHasFieldName ? NamedLineOutput : LineOutput))
				.TextStyle(FAppStyle::Get(), "MessageLog")
			]
		];
}

void STreeSitterTreeViewer::GetTreeChildren(TSharedPtr<FTreeSitterTreeNode> InItem, TArray<TSharedPtr<FTreeSitterTreeNode>>& OutChildren)
{
	OutChildren = InItem->Children;
}

void STreeSitterTreeViewer::PopulateTree(const ::TSNode& InNode, const TSharedPtr<FTreeSitterTreeNode>& InParent, const uint32 InDepth)
{
	const char* UTF8Source = TCHAR_TO_UTF8(*CodeText.ToString());
	
	const TSharedRef<FTreeSitterTreeNode> NewNode = MakeShared<FTreeSitterTreeNode>();
	NewNode->Language = FString(ts_language_name(ts_node_language(InNode)));
	NewNode->NodeType = FString(ts_node_type(InNode));
	NewNode->NodeGrammarType = FString(ts_node_grammar_type(InNode));
	NewNode->Symbol =  ts_node_symbol(InNode);
	NewNode->GrammarSymbol = ts_node_grammar_symbol(InNode);
	NewNode->StartByte = ts_node_start_byte(InNode);
	NewNode->EndByte = ts_node_end_byte(InNode);
	NewNode->StartPoint = ts_node_start_point(InNode);
	NewNode->EndPoint = ts_node_end_point(InNode);
	NewNode->bIsNull = ts_node_is_null(InNode);
	NewNode->bIsMissing = ts_node_is_missing(InNode);
	NewNode->bIsNamed = ts_node_is_named(InNode);
	NewNode->bHasError = ts_node_has_error(InNode);
	NewNode->bIsError = ts_node_is_error(InNode);
	NewNode->bIsExtra = ts_node_is_extra(InNode);
	NewNode->bHasChanges = ts_node_has_changes(InNode);
	NewNode->Depth = InDepth;

	char* NodeString = ts_node_string(InNode);
	NewNode->SExpression = FString(UTF8_TO_TCHAR(NodeString));
	free(NodeString);

	{
		const uint32 SubstringLength = NewNode->EndByte - NewNode->StartByte;
		auto StringConversion = StringCast<TCHAR>(UTF8Source + NewNode->StartByte, SubstringLength);
		NewNode->ExtractedSource = StringConversion.Get();
	}
	
	NewNode->FieldName = GetNodeFieldName(InNode);

	if (NewNode->bIsNamed)
	{
		if (InParent)
		{
			InParent->Children.Add(NewNode);
		}
		else
		{
			TreeItems.Add(NewNode);
		}
	}

	const uint32 ChildCount = ts_node_child_count(InNode);
	for (uint32 i = 0; i < ChildCount; i++)
	{
		PopulateTree(ts_node_child(InNode, i), NewNode, InDepth + 1);
	}
}

FString STreeSitterTreeViewer::GetNodeFieldName(const TSNode& InNode)
{
	FString FieldName;
	const TSNode NodeParent = ts_node_parent(InNode);
	if (ts_node_is_null(NodeParent))
	{
		return MoveTemp(FieldName);
	}
	
	// TODO This can be done only once when language parser is set
	const TSLanguage* NodeLanguage = ts_node_language(InNode);
	TArray<TSFieldId> FieldIds = GetFieldIdsForLanguage(NodeLanguage);
	
	for (const TSFieldId Id : FieldIds)
	{
		const TSNode FoundNode = ts_node_child_by_field_id(NodeParent, Id);
		if (!ts_node_is_null(FoundNode) && ts_node_eq(FoundNode, InNode))
		{
			FieldName = FString(ts_language_field_name_for_id(NodeLanguage, Id));
			break;
		}
	}

	return MoveTemp(FieldName);
}

TArray<TSFieldId> STreeSitterTreeViewer::GetFieldIdsForLanguage(const TSLanguage* InLanguage)
{
	const uint32 Field_Count = ts_language_field_count(InLanguage);
	
	TArray<TSFieldId> FieldIds;
	for (uint32 i = 1; i < Field_Count; ++i)
	{
		const char* FieldNameId = ts_language_field_name_for_id(InLanguage, i);
		const TSFieldId FieldId = ts_language_field_id_for_name(InLanguage, FieldNameId, strlen(FieldNameId));
		FieldIds.Add(FieldId);
	}

	return MoveTemp(FieldIds);
}
