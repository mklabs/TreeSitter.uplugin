// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

struct FTreeSitterNode;
class FTreeSitterParser;
class SBorder;

struct FTreeSitterMarkdownTableListItem
{
	// FName ColumnName;
	// FString CellContent;

	TMap<FName, FString> ColumnNamesToCellContent;
};

class STreeSitterMarkdownTableRow : public SMultiColumnTableRow<TSharedPtr<FString>>
{
	SLATE_BEGIN_ARGS(STreeSitterMarkdownTableRow)
		{
		}

		SLATE_ARGUMENT(TSharedPtr<FString>, Item)
	SLATE_END_ARGS()
	
	/** Constructs this widget with InArgs and OwnerTable */
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FTreeSitterMarkdownTableListItem>& InListItem);

	/** Generates appropriate widget for the given column name ID */
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;
private:
	TSharedPtr<FTreeSitterMarkdownTableListItem> ListItem;
};

class STreeSitterMarkdownTable : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STreeSitterMarkdownTable)
		{
		}

		SLATE_ARGUMENT(FString, InitialMarkdown)

	SLATE_END_ARGS()

	virtual ~STreeSitterMarkdownTable() override;

	void Construct(const FArguments& InArgs, const TSharedRef<FTreeSitterNode>& InNode);

	// Static method to create an instance, used by the custom widget registry
	static TSharedRef<SWidget> MakeInstance(const TSharedRef<FTreeSitterNode>& InNode, const FString& InSource)
	{
		return SNew(STreeSitterMarkdownTable, InNode).InitialMarkdown(InSource);
	}

private:

	TSharedPtr<SListView<TSharedPtr<FTreeSitterMarkdownTableListItem>>> ListView;
	TArray<TSharedPtr<FTreeSitterMarkdownTableListItem>> ListItems;

	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FTreeSitterMarkdownTableListItem> InCells, const TSharedRef<STableViewBase>& TableViewBase);
};
