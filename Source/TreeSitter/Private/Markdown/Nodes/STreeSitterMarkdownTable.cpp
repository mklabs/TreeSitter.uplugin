// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterMarkdownTable.h"

#include "Markdown/TreeSitterSlateMarkdown.h"
#include "TreeSitterNode.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"

#define LOCTEXT_NAMESPACE "TreeSitter"

void STreeSitterMarkdownTableRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FTreeSitterMarkdownTableListItem>& InListItem)
{
	ListItem = InListItem;
	SMultiColumnTableRow::Construct(
		FSuperRowType::FArguments().Padding(1.0f),
		InOwnerTableView
	);
}

TSharedRef<SWidget> STreeSitterMarkdownTableRow::GenerateWidgetForColumn(const FName& InColumnName)
{
	TSharedRef<SBox> BoxWrapper = SNew(SBox)
		.Padding(FMargin(4.0f, 0.0f))
		.VAlign(VAlign_Center);

	const TMap<FName, FString>& Columns = ListItem->ColumnNamesToCellContent;
	const FString CellContent = Columns.Contains(InColumnName) ? Columns.FindChecked(InColumnName) : TEXT("");

	BoxWrapper->SetContent(SNew(STextBlock)
		.Text(FText::FromString(CellContent))
		.ToolTipText(FText::FromName(InColumnName))
	);

	return BoxWrapper;
}

STreeSitterMarkdownTable::~STreeSitterMarkdownTable()
{
}

void STreeSitterMarkdownTable::Construct(const FArguments& InArgs, const TSharedRef<FTreeSitterNode>& InNode)
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

	// pipe_table_header in the grammar
	// TODO: Investigate doing queries instead

	const TSNode TableHeader = ts_node_child(TreeNode, 0);
	// const TSNode DelimiterRow = ts_node_child(TreeNode, 1);
	if (ts_node_is_null(TableHeader))
	{
		return;
	}

	TArray<FName> ColumnNames;
	const TSharedRef<SHeaderRow> HeaderRow = SNew(SHeaderRow);
		
	// Process each child item, each one being a column header
	for (uint32 i = 0; i < ts_node_child_count(TableHeader); i++)
	{
		TSNode Child = ts_node_child(TableHeader, i);

		// ignore anonymous nodes
		if (!ts_node_is_named(Child))
		{
			continue;
		}

		const FString ColumnName = ExtractNodeText(Child, OriginalSource);
		
		const SHeaderRow::FColumn::FArguments Column = SHeaderRow::Column(FName(*ColumnName))
			.DefaultLabel(FText::FromString(ColumnName))
			.FillWidth(0.2f);
		
		HeaderRow->AddColumn(Column);
		ColumnNames.Add(FName(*ColumnName));
	}

	ListItems.Reset();
	
	// First child is header row: pipe_table_header
	// Second one is the delimiter row: pipe_table_delimiter_row
	// Any rows coming after are table rows: pipe_table_row
	const uint32_t ChildCount = ts_node_child_count(TreeNode);
	for (uint32 i = 2; i < ChildCount; i++)
	{
		const TSNode Child = ts_node_child(TreeNode, i);
		
		// ignore null nodes
		if (ts_node_is_null(Child))
		{
			continue;
		}

		// ignore anonymous nodes
		if (!ts_node_is_named(Child))
		{
			continue;
		}
		
		// Each child of the rows are the actual cell
		TMap<FName, FString> ColumnNamesToCellContent;
		uint32 ValidIndex = 0;
		for (uint32 j = 0; j < ts_node_child_count(Child); j++)
		{
			const TSNode TableCell = ts_node_child(Child, j);
			
			// ignore null nodes or unammed nodes
			if (ts_node_is_null(TableCell) || !ts_node_is_named(TableCell))
			{
				continue;
			}

			const FName ColumnName = ColumnNames.IsValidIndex(ValidIndex) ? ColumnNames[ValidIndex] : NAME_None;
			const FString CellContent = ExtractNodeText(TableCell, OriginalSource);

			ColumnNamesToCellContent.Add(ColumnName, CellContent);

			ValidIndex++;
		}

		TSharedPtr<FTreeSitterMarkdownTableListItem> Item = MakeShared<FTreeSitterMarkdownTableListItem>();
		Item->ColumnNamesToCellContent = MoveTemp(ColumnNamesToCellContent);
		ListItems.Add(Item);
	}

	ChildSlot
	[
		SAssignNew(ListView, SListView<TSharedPtr<FTreeSitterMarkdownTableListItem>>)
		.ListItemsSource(&ListItems)
		.HeaderRow(HeaderRow)
		.OnGenerateRow(this, &STreeSitterMarkdownTable::GenerateRow)
	];
}

TSharedRef<ITableRow> STreeSitterMarkdownTable::GenerateRow(TSharedPtr<FTreeSitterMarkdownTableListItem> InCells, const TSharedRef<STableViewBase>& TableViewBase)
{
	// const FString Output = FString::Join(*InCells.Get(), TEXT(" - "));
	
	//
	// TSharedRef<STableRow<TSharedPtr<FString>>> TableRow = SNew(STableRow<TSharedPtr<FString>>, TableViewBase);
	//
	//
	// return SNew(STableRow<TSharedPtr<FText>>, TableViewBase)
	// 	.Padding(FMargin(16, 4, 16, 4))
	// 	[
	// 		SNew(STextBlock).Text(FText::FromString(Output))
	// 	];

	return SNew(STreeSitterMarkdownTableRow, TableViewBase, InCells);
}

#undef LOCTEXT_NAMESPACE
