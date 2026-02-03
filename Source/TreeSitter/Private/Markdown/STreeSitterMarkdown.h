// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once
#include "Widgets/SCompoundWidget.h"

class FTreeSitterParser;
class SBorder;

class STreeSitterMarkdown : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STreeSitterMarkdown)
		{
		}

		SLATE_ARGUMENT(FString, InitialMarkdown)

	SLATE_END_ARGS()
	
	virtual ~STreeSitterMarkdown() override;

	void Construct(const FArguments& InArgs);

	const TSharedPtr<FString>& GetMarkdownSource() const;
	void SetMarkdownSource(const FString& InMarkdownSource) const;
	
	FString GetMarkdownSourceText() const;

private:
	TSharedPtr<FTreeSitterParser> Parser;
	TSharedPtr<SBorder> Container;
	// FString MarkdownSource;
	
	TSharedPtr<FString> MarkdownSource;

	TSharedRef<SWidget> GenerateMarkdownSlateWidget() const;
};
