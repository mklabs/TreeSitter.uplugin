// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

#include "Engine/TimerHandle.h"
#include "Widgets/SCompoundWidget.h"

class FTreeSitterParser;
class STreeSitterCodeEditor;
class STreeSitterMarkdown;

class STreeSitterMarkdownPlayground : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STreeSitterMarkdownPlayground)
		{
		}

	SLATE_END_ARGS()
	
	virtual ~STreeSitterMarkdownPlayground() override;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<STreeSitterCodeEditor> CodeEditor;
	TSharedPtr<STreeSitterMarkdown> MarkdownPreview;
	
	FTimerHandle DebounceTimerHandle;
	
	FText CodeText;
	
	void OnCodeChanged(const FText& NewText);
	void ProcessPendingCode() const;
};
