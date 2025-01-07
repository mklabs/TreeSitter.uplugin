// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterPlayground.h"

#include "Editor.h"
#include "ITreeSitterModule.h"
#include "STreeSitterCodeEditor.h"
#include "STreeSitterTreeViewer.h"
#include "TreeSitterParser.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "tree_sitter/api.h"

STreeSitterPlayground::~STreeSitterPlayground()
{
	Parser.Reset();
}

static constexpr const TCHAR* ExampleJson = TEXT(R"_JSON(
				{
				    "uenumValue": "enumValueC"
				})_JSON");

static TMap<ETreeSitterLanguage, FString> Examples = {
	{ ETreeSitterLanguage::Json, TEXT(R"_JSON(
{
  "FileVersion": 3,
  "Version": 1,
  "VersionName": "1.0",
  "FriendlyName": "Tree-sitter",
  "Description": "Third party plugin wrapper for Tree-sitter",
  "Category": "Other",
  "CreatedBy": "Mickael Daniel",
  "CreatedByURL": "",
  "DocsURL": "",
  "MarketplaceURL": "",
  "CanContainContent": true,
  "IsBetaVersion": false,
  "IsExperimentalVersion": false,
  "Installed": false,
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
# Heading 1

Yo, is it okay like this ?

😁

Grug ❤️ Simplicity!

漢字

And what about **this** ?
Or this

## Heading 2

Yo

Here is a list of items:

- Foo
- Bar
- Baz is *Foobar* and ***foo*** 


## Heading 2

What about code blocks

### Heading 3

Some code

```cpp
FString Foo = TEXT("Foo");

FString Foo2 = TEXT("Foo2");


```

---

Bottom line after HR
)_Markdown") },
	{ ETreeSitterLanguage::MarkdownInline, TEXT(R"_Markdown(
Baz is *Foobar* and ***foo*** 
)_Markdown") },
};

void STreeSitterPlayground::Construct(const FArguments& InArgs)
{
	Parser = MakeShared<FTreeSitterParser>();
	Parser->SetLanguage(ITreeSitterModule::Get().GetLanguageParser(ETreeSitterLanguage::JavaScript)());

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

		// Slider on top
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
				.Text(NSLOCTEXT("TreeSitter", "ComboLabel", "Language Parser:"))
			]
			+ SHorizontalBox::Slot()
			// Left pane, takes half width
			.FillWidth(0.5f)
			[
				SAssignNew(ComboBox, SComboBox<FName>)
				.ToolTipText(NSLOCTEXT("TreeSitter", "ComboToolTip", "Choose the language parser"))
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
			[
				SNew(SCheckBox)
				.IsChecked(this, &STreeSitterPlayground::IsPreservingCode)
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
	CodeText = NewText;
	
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
	TSTree* Tree = Parser->Parse(CodeText.ToString());
	const TSNode RootNode = ts_tree_root_node(Tree);

	TreeViewer->UpdateTree(RootNode, CodeText);
	ts_tree_delete(Tree);
}

void STreeSitterPlayground::HandleSelectedLanguageChanged(FName InSelectedLanguage, ESelectInfo::Type InSelectInfo)
{
	SelectedLanguage = InSelectedLanguage;

	const ETreeSitterLanguage Language = static_cast<ETreeSitterLanguage>(StaticEnum<ETreeSitterLanguage>()->GetValueByName(InSelectedLanguage));
	if (!bPreserveCode && Examples.Contains(Language))
	{
		const FString Example = Examples.FindChecked(Language);
		CodeText = FText::FromString(Example);
		CodeEditor->GetEditBox()->SetText(CodeText);
	}

	
	Parser->SetLanguage(ITreeSitterModule::Get().GetLanguageParser(Language)());
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
