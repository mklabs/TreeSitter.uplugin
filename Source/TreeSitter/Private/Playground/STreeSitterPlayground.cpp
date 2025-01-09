// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterPlayground.h"

#include "Editor.h"
#include "ITreeSitterModule.h"
#include "STreeSitterCodeEditor.h"
#include "STreeSitterTreeViewer.h"
#include "TreeSitterParser.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "tree_sitter/api.h"

#define LOCTEXT_NAMESPACE "TreeSitter"

static TMap<ETreeSitterLanguage, FString> Examples = {
	{ ETreeSitterLanguage::Json, TEXT(R"_JSON(
{
  "VersionName": "0.1.0",
  "FriendlyName": "Tree-sitter",
  "Description": "Third party plugin wrapper for Tree-sitter",
  "Modules": [
    {
      "Name": "TreeSitter",
      "Type": "Runtime",
      "LoadingPhase": "Default"
    }
  ]
}
)_JSON") },
	{ ETreeSitterLanguage::JavaScript, TEXT(R"_JSON(
function greet() { console.log('hello'); }

// Foo
const foo = 2;

class Foo {
    prop = true;
    foo = "";
    bar = "foobar";
    bar2: "foobar";
}
)_JSON") },
	{ ETreeSitterLanguage::Markdown, TEXT(R"_Markdown(
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

)_Markdown") },
	{ ETreeSitterLanguage::MarkdownInline, TEXT(R"_Markdown(
Baz is *Foobar* and ***foo*** 
)_Markdown") },
};

STreeSitterPlayground::~STreeSitterPlayground()
{
	Parser.Reset();
	CodeText.Reset();
}

void STreeSitterPlayground::Construct(const FArguments& InArgs)
{
	Parser = MakeShared<FTreeSitterParser>();
	Parser->SetLanguage(ETreeSitterLanguage::Json);

	CodeText = MakeShared<FString>();

	SelectedLanguage = UEnum::GetValueAsName(ETreeSitterLanguage::Json);
	AvailableLanguages = {
		UEnum::GetValueAsName(ETreeSitterLanguage::Json),
		UEnum::GetValueAsName(ETreeSitterLanguage::JavaScript),
		UEnum::GetValueAsName(ETreeSitterLanguage::Markdown),
		UEnum::GetValueAsName(ETreeSitterLanguage::MarkdownInline)
	};

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ComboLabel", "Language Parser:"))
			]
			+ SHorizontalBox::Slot()
			// Left pane, takes half width
			.FillWidth(0.5f)
			[
				SAssignNew(ComboBox, SComboBox<FName>)
				.ToolTipText(LOCTEXT("ComboToolTip", "Choose the language parser"))
				.OptionsSource(&AvailableLanguages)
				.InitiallySelectedItem(SelectedLanguage)
				.OnSelectionChanged(this, &STreeSitterPlayground::HandleSelectedLanguageChanged)
				.OnGenerateWidget_Static(&STreeSitterPlayground::MakeWidgetForComboBox)
				[
					SNew(STextBlock)
					.Text(this, &STreeSitterPlayground::GetSelectedLanguage)
				]
			]
			
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &STreeSitterPlayground::IsPreservingCode)
				.ToolTipText(LOCTEXT("CheckboxTooltip", "Check this box to preserve the code when changing languages. Uncheck to load pre-defined examples."))
				.OnCheckStateChanged(this, &STreeSitterPlayground::SetPreservingCode)
			]
		]

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
				.OnTextChanged(this, &STreeSitterPlayground::OnCodeChanged)
			]

			+ SHorizontalBox::Slot()
			// Right pane, takes other half
			.FillWidth(0.5f)
			[
				SAssignNew(TreeViewer, STreeSitterTreeViewer)
			]
		]
	];

	HandleSelectedLanguageChanged(SelectedLanguage, ESelectInfo::Direct);
}

void STreeSitterPlayground::OnCodeChanged(const FText& NewText)
{
	check(CodeText.IsValid());
	
	*CodeText = NewText.ToString();
	
	if (GEditor)
	{
		constexpr bool bLoop = false;
		constexpr float DebounceRate = 0.1f;
		GEditor->GetTimerManager()->ClearTimer(DebounceTimerHandle);
		GEditor->GetTimerManager()->SetTimer(DebounceTimerHandle, FTimerDelegate::CreateSP(this, &STreeSitterPlayground::ProcessPendingCode), DebounceRate, bLoop);
	}
}

void STreeSitterPlayground::ProcessPendingCode() const
{
	check(CodeText.IsValid());
	
	TSTree* Tree = Parser->Parse(*CodeText);
	const TSNode RootNode = ts_tree_root_node(Tree);

	TreeViewer->UpdateTree(RootNode, CodeText.ToSharedRef());
	ts_tree_delete(Tree);
}

void STreeSitterPlayground::HandleSelectedLanguageChanged(FName InSelectedLanguage, ESelectInfo::Type InSelectInfo)
{
	check(CodeText.IsValid());
	
	SelectedLanguage = InSelectedLanguage;

	const ETreeSitterLanguage Language = static_cast<ETreeSitterLanguage>(StaticEnum<ETreeSitterLanguage>()->GetValueByName(InSelectedLanguage));
	if (!bPreserveCode && Examples.Contains(Language))
	{
		const FString Example = Examples.FindChecked(Language);
		*CodeText = Example;
		CodeEditor->GetEditBox()->SetText(FText::FromString(*CodeText));
	}
	
	Parser->SetLanguage(Language);
	ProcessPendingCode();
}

TSharedRef<SWidget> STreeSitterPlayground::MakeWidgetForComboBox(FName InValue)
{
	// return SNew(STextBlock).Text(UEnum::GetDisplayValueAsText(InValue));
	return SNew(STextBlock).Text(FText::FromName(InValue));
}

FText STreeSitterPlayground::GetSelectedLanguage() const
{
	// return UEnum::GetDisplayValueAsText(SelectedLanguage);
	return FText::FromName(SelectedLanguage);
}

ECheckBoxState STreeSitterPlayground::IsPreservingCode() const
{
	return bPreserveCode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void STreeSitterPlayground::SetPreservingCode(const ECheckBoxState InCheckBoxState)
{
	bPreserveCode = InCheckBoxState == ECheckBoxState::Checked;
}

#undef LOCTEXT_NAMESPACE