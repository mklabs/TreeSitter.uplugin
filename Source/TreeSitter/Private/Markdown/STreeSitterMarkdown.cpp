// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterMarkdown.h"

#include "ITreeSitterModule.h"
#include "TreeSitterParser.h"
#include "TreeSitterSlateMarkdown.h"
#include "tree_sitter/api.h"

STreeSitterMarkdown::~STreeSitterMarkdown()
{
	Parser.Reset();
}

void STreeSitterMarkdown::Construct(const FArguments& InArgs)
{
	MarkdownSource = MakeShared<FString>(InArgs._InitialMarkdown);
	
	Parser = MakeShared<FTreeSitterParser>();
	Parser->SetLanguage(ETreeSitterLanguage::Markdown);

    ChildSlot
    [
		SAssignNew(Container, SBorder)
		.Padding(8.f, 8.f)
		.BorderImage(FAppStyle::GetBrush("Border"))
		[
			GenerateMarkdownSlateWidget()
		]
    ];
}

const TSharedPtr<FString>& STreeSitterMarkdown::GetMarkdownSource() const
{
	return MarkdownSource;
}

void STreeSitterMarkdown::SetMarkdownSource(const FString& InMarkdownSource) const
{
	const FString CurrentSource = GetMarkdownSourceText();
	if (CurrentSource == InMarkdownSource)
	{
		return;
	}
	
	*MarkdownSource = InMarkdownSource;

	Container->ClearContent();
	Container->SetContent(GenerateMarkdownSlateWidget());
}

FString STreeSitterMarkdown::GetMarkdownSourceText() const
{
	return MarkdownSource.IsValid() ? *MarkdownSource : TEXT("");
}

TSharedRef<SWidget> STreeSitterMarkdown::GenerateMarkdownSlateWidget() const
{
	check(Parser.IsValid());
	
	TSTree* Tree = Parser->Parse(GetMarkdownSourceText());
	const TSNode RootNode = ts_tree_root_node(Tree);

	TSharedRef<SWidget> Widget = UE::TreeSitter::GenerateMarkdownSlateWidget(RootNode, MarkdownSource.ToSharedRef());
    ts_tree_delete(Tree);

	return Widget;
}

