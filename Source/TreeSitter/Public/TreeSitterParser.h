// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

enum class ETreeSitterLanguage : uint8;
struct TSLanguage;
struct TSParser;
struct TSTree;

class TREESITTER_API FTreeSitterParser : public TSharedFromThis<FTreeSitterParser>
{
public:
	FTreeSitterParser();
	~FTreeSitterParser();

	bool SetLanguage(const TSLanguage* Language) const;
	bool SetLanguage(const ETreeSitterLanguage InLanguage) const;
	TSTree* Parse(const FString& SourceCode) const;

private:
	TSParser* Parser;
};
