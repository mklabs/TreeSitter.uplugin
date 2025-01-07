// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

#include "Engine/TimerHandle.h"
#include "ITreeSitterModule.h"
#include "Widgets/SCompoundWidget.h"

class FTreeSitterParser;
class STreeSitterTreeViewer;
class STreeSitterCodeEditor;

class STreeSitterPlayground : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STreeSitterPlayground)
		{
		}

	SLATE_END_ARGS()

	virtual ~STreeSitterPlayground() override;

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<STreeSitterCodeEditor> CodeEditor;
	TSharedPtr<STreeSitterTreeViewer> TreeViewer;
	TSharedPtr<FTreeSitterParser> Parser;

	// TSharedPtr<SComboBox<ETreeSitterLanguage>> ComboBox;
	TSharedPtr<SComboBox<FName>> ComboBox;
	
	bool bPreserveCode = false;
	
	TArray<FName> AvailableLanguages;
	
	// Debounce variables
	FTimerHandle DebounceTimerHandle;

	// ETreeSitterLanguage SelectedLanguage = ETreeSitterLanguage::Json;
	FName SelectedLanguage;
	
	FText CodeText;

	void OnCodeChanged(const FText& NewText);
	void ProcessPendingCode() const;
	
	void HandleSelectedLanguageChanged(FName InSelectedLanguage, ESelectInfo::Type InSelectInfo);
	static TSharedRef<SWidget> MakeWidgetForComboBox(FName InValue);
	FText GetSelectedLanguage() const;
	
	ECheckBoxState IsPreservingCode() const;
	void SetPreservingCode(const ECheckBoxState InCheckBoxState);
};
