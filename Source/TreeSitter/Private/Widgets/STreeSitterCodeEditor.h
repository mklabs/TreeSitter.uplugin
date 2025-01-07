// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

#include "Framework/SlateDelegates.h"
#include "Widgets/SCompoundWidget.h"

class SMultiLineEditableTextBox;

class STreeSitterCodeEditor : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STreeSitterCodeEditor) {}
        SLATE_ARGUMENT(FText, InitialText)
        SLATE_EVENT(FOnTextChanged, OnTextChanged)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    const TSharedPtr<SMultiLineEditableTextBox>& GetEditBox() const;

private:
    TSharedPtr<SMultiLineEditableTextBox> EditBox;
};
