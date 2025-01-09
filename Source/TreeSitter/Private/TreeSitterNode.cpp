// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "TreeSitterNode.h"

FTreeSitterNode::FTreeSitterNode(const TSNode& InNode, const uint32 InDepth)
{
	Node = InNode;
	
	Language = FName(ts_language_name(ts_node_language(InNode)));
	NodeType = FName(ts_node_type(InNode));
	NodeGrammarType = FName(ts_node_grammar_type(InNode));
	Symbol =  ts_node_symbol(InNode);
	GrammarSymbol = ts_node_grammar_symbol(InNode);
	StartByte = ts_node_start_byte(InNode);
	EndByte = ts_node_end_byte(InNode);
	StartPoint = ts_node_start_point(InNode);
	EndPoint = ts_node_end_point(InNode);
	bIsNull = ts_node_is_null(InNode);
	bIsMissing = ts_node_is_missing(InNode);
	bIsNamed = ts_node_is_named(InNode);
	bHasError = ts_node_has_error(InNode);
	bIsError = ts_node_is_error(InNode);
	bIsExtra = ts_node_is_extra(InNode);
	bHasChanges = ts_node_has_changes(InNode);
	Depth = InDepth;
}

