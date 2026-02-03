// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

class FTreeSitterParser;
class SBorder;
struct FTreeSitterNode;

class STreeSitterMarkdownHeading : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STreeSitterMarkdownHeading)
		{
		}

		SLATE_ARGUMENT(FString, InitialMarkdown)

	SLATE_END_ARGS()

	virtual ~STreeSitterMarkdownHeading() override;

	void Construct(const FArguments& InArgs, const TSharedRef<FTreeSitterNode>& InNode);

	// Static method to create an instance, used by the custom widget registry
	static TSharedRef<SWidget> MakeInstance(const TSharedRef<FTreeSitterNode>& InNode, const FString& InSource)
	{
		return SNew(STreeSitterMarkdownHeading, InNode).InitialMarkdown(InSource);
	}
};
