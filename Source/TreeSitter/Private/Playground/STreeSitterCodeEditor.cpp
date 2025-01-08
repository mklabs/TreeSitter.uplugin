// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "STreeSitterCodeEditor.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"


void STreeSitterCodeEditor::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(EditBox, SMultiLineEditableTextBox)
		.Text(InArgs._InitialText)
		.OnTextChanged(InArgs._OnTextChanged)
	];
}

const TSharedPtr<SMultiLineEditableTextBox>& STreeSitterCodeEditor::GetEditBox() const
{
	return EditBox;
}
