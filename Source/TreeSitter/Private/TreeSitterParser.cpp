// Copyright 2025 Mickael Daniel. All Rights Reserved.

#include "TreeSitterParser.h"

#include "ITreeSitterModule.h"
#include "tree_sitter/api.h"

FTreeSitterParser::FTreeSitterParser()
	: Parser(ts_parser_new())
{
}

FTreeSitterParser::~FTreeSitterParser()
{
	ts_parser_delete(Parser);
}

bool FTreeSitterParser::SetLanguage(const TSLanguage* Language) const
{
	return ts_parser_set_language(Parser, Language);
}

bool FTreeSitterParser::SetLanguage(const ETreeSitterLanguage InLanguage) const
{
	return ts_parser_set_language(Parser, ITreeSitterModule::Get().GetLanguageParser(InLanguage)());
}

TSTree* FTreeSitterParser::Parse(const FString& SourceCode) const
{
	
	// Step 1: Convert FString to UTF-8
	const FTCHARToUTF8 UTF8String(*SourceCode);
	
	// Step 2: Get char* from UTF-8 wrapper
	// This char* valid as long as FTCHARToUTF8 object exists
	const char* Code = UTF8String.Get();
	
	return ts_parser_parse_string(Parser, nullptr, Code, strlen(Code));
}
