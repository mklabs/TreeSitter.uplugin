﻿// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

struct FTreeSitterNode;
class FTreeSitterParser;
class SBorder;

class STreeSitterMarkdownParagraph : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STreeSitterMarkdownParagraph)
		{
		}

		SLATE_ARGUMENT(FString, InitialMarkdown)

	SLATE_END_ARGS()
	
	virtual ~STreeSitterMarkdownParagraph() override;

	void Construct(const FArguments& InArgs, const TSharedRef<FTreeSitterNode>& InNode);

	// Static method to create an instance, used by the custom widget registry
	static TSharedRef<SWidget> MakeInstance(const TSharedRef<FTreeSitterNode>& InNode, const FString& InSource)
	{
		return SNew(STreeSitterMarkdownParagraph, InNode).InitialMarkdown(InSource);
	}
};
