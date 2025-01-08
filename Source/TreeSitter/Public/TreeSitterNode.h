// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

#include "tree_sitter/api.h"

struct FTreeSitterNode
{
	TArray<TSharedPtr<FTreeSitterNode>> Children;
	
	FString Language;
	FString NodeType;
	FString NodeGrammarType;
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
	
	FString FieldName;
	uint32 Depth;

	FTreeSitterNode() = default;

	explicit FTreeSitterNode(const TSNode& InNode, const uint32 InDepth = 0);
};
