// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterMarkdownPlayground.h"

#include "Playground/STreeSitterCodeEditor.h"
#include "Markdown/STreeSitterMarkdown.h"

STreeSitterMarkdownPlayground::~STreeSitterMarkdownPlayground()
{
}

void STreeSitterMarkdownPlayground::Construct(const FArguments& InArgs)
{
	const FString InitialMarkdown = TEXT("# Hello World\nThis is a paragraph.\n\n- Item 1\n- Item 2");
	
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(5.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			// Left pane, takes half width
			.FillWidth(0.5f)
			[
				SAssignNew(CodeEditor, STreeSitterCodeEditor)
				.OnTextChanged(this, &STreeSitterMarkdownPlayground::OnCodeChanged)
				.InitialText(FText::FromString(InitialMarkdown))
			]

			+ SHorizontalBox::Slot()
			// Right pane, takes other half
			.FillWidth(0.5f)
			[
				SAssignNew(MarkdownPreview, STreeSitterMarkdown)
				.InitialMarkdown(InitialMarkdown)
			]
		]
	];
}

void STreeSitterMarkdownPlayground::OnCodeChanged(const FText& NewText)
{
	CodeText = NewText;
	
	if (GEditor)
	{
		constexpr bool bLoop = false;
		constexpr float DebounceRate = 0.1f;
		GEditor->GetTimerManager()->ClearTimer(DebounceTimerHandle);
		GEditor->GetTimerManager()->SetTimer(DebounceTimerHandle, FTimerDelegate::CreateSP(this, &STreeSitterMarkdownPlayground::ProcessPendingCode), DebounceRate, bLoop);
	}
}

void STreeSitterMarkdownPlayground::ProcessPendingCode() const
{
	MarkdownPreview->SetMarkdownSource(CodeText.ToString());
}
