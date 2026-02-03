// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterMarkdownPlayground.h"

#include "Components/VerticalBox.h"
#include "Editor.h"
#include "Playground/STreeSitterCodeEditor.h"
#include "Markdown/STreeSitterMarkdown.h"

STreeSitterMarkdownPlayground::~STreeSitterMarkdownPlayground()
{
}

void STreeSitterMarkdownPlayground::Construct(const FArguments& InArgs)
{
	// const FString InitialMarkdown = TEXT("# Hello World\nThis is a paragraph.\n\n- Item 1\n- Item 2");
	const FString InitialMarkdown = TEXT(R"_MD(

# TreeSitter.uplugin

> Unreal Engine plugin that integrates the [tree-sitter](https://tree-sitter.github.io) library as a third-party module for in-editor use.

## Table example


Bottom line after HR

---

| Column1 | Column2 | Column3 | Column4 | Column5 |
| --------------- | --------------- | --------------- | --------------- | --------------- |
| Item1.1 | Item2.1 | Item3.1 | Item4.1 | Item5.1 |
| Item1.2 | Item2.2 | Item3.2 | Item4.2 | Item5.2 |
| Item1.3 | Item2.3 | Item3.3 | Item4.3 | Item5.3 |
| Item1.4 | Item2.4 | Item3.4 | Item4.4 | Item5.4 |



## Todo

- [ ] Task item 1
- [ ] Task item 2
- [x] Task item 3
- [ ] Task item 4:
    - Subtask item 1
    - Subtask item 2

## Usage

```cpp
#include "TreeSitterParser.h"
#include "tree_sitter/api.h"

// Create the parser
TSharedPtr<FTreeSitterParser> Parser = MakeShared<FTreeSitterParser>();

// ...
```

Baz is *Foobar* and ***foo*** 

)_MD");
	
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
