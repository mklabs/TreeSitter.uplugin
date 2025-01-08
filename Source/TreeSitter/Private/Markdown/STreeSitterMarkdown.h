﻿// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

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

	const FString& GetMarkdownSource() const;
	void SetMarkdownSource(const FString& InMarkdownSource);

private:
	TSharedPtr<FTreeSitterParser> Parser;
	TSharedPtr<SBorder> Container;
	FString MarkdownSource;

	static TSharedRef<SWidget> GenerateMarkdownSlateWidget(const TSharedRef<FTreeSitterParser>& InParser, const FString& InSource);
};