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
	MarkdownSource = InArgs._InitialMarkdown;
	
	Parser = MakeShared<FTreeSitterParser>();
	Parser->SetLanguage(ETreeSitterLanguage::Markdown);

    ChildSlot
    [
		SAssignNew(Container, SBorder)
		.Padding(8.f, 8.f)
		.BorderImage(FAppStyle::GetBrush("Border"))
		[
			GenerateMarkdownSlateWidget(Parser.ToSharedRef(), MarkdownSource)
		]
    ];
}

const FString& STreeSitterMarkdown::GetMarkdownSource() const
{
	return MarkdownSource;
}

void STreeSitterMarkdown::SetMarkdownSource(const FString& InMarkdownSource)
{
	if (MarkdownSource == InMarkdownSource)
	{
		return;
	}
	
	MarkdownSource = InMarkdownSource;

	Container->ClearContent();
	Container->SetContent(GenerateMarkdownSlateWidget(Parser.ToSharedRef(), MarkdownSource));
}

TSharedRef<SWidget> STreeSitterMarkdown::GenerateMarkdownSlateWidget(const TSharedRef<FTreeSitterParser>& InParser, const FString& InSource)
{
	TSTree* Tree = InParser->Parse(InSource);
	const TSNode RootNode = ts_tree_root_node(Tree);

	TSharedRef<SWidget> Widget = UE::TreeSitter::GenerateMarkdownSlateWidget(RootNode, InSource);
    ts_tree_delete(Tree);

	return Widget;
}

