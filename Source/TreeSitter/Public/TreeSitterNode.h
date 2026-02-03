// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

#include "Templates/SharedPointer.h"
#include "tree_sitter/api.h"

struct FTreeSitterNode
{
	TArray<TSharedPtr<FTreeSitterNode>> Children;
	
	FName Language;
	FName NodeType;
	FName NodeGrammarType;
	TSSymbol Symbol;
	TSSymbol GrammarSymbol;
	uint32 StartByte;
	uint32 EndByte;
	TSPoint StartPoint;
	TSPoint EndPoint;

	FString SExpression;
	FString ExtractedSource;
	
	bool bIsNull;
	bool bIsNamed;
	bool bHasError;
	bool bIsError;
	bool bIsMissing;
	bool bIsExtra;
	bool bHasChanges;
	
	FName FieldName;
	uint32 Depth;

	/** Copy of original tree-sitter node (I have the feeling this is a bad idea) */
	TSNode Node;

	/** Ref to original raw source code to be able to extract code from start / end points */
	// TSharedPtr<FString> SourceCode;

	FTreeSitterNode() = default;
	virtual ~FTreeSitterNode() = default;

	// explicit FTreeSitterNode(const TSNode& InNode, const TSharedRef<FString>& InSourceCode, const uint32 InDepth = 0);
	explicit FTreeSitterNode(const TSNode& InNode, const uint32 InDepth = 0);
};
