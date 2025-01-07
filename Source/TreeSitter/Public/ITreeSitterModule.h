// Copyright 2025 Mickael Daniel. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

struct TSLanguage;

UENUM()
enum class ETreeSitterLanguage : uint8
{
	JavaScript,
	Json,
	Markdown,
	MarkdownInline,
};

/**
 * Interface for the Concert Sync Server module.
 */
class ITreeSitterModule : public IModuleInterface
{
public:
	using FGetLanguageParser = const TSLanguage*(void);

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline ITreeSitterModule& Get()
	{
		static const FName ModuleName = "TreeSitter";
		return FModuleManager::LoadModuleChecked<ITreeSitterModule>(ModuleName);
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() during shutdown if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		static const FName ModuleName = "TreeSitter";
		return FModuleManager::Get().IsModuleLoaded(ModuleName);
	}

	/**
	 * Returns corresponding language parser, use this to get a reference onto `tree_sitter_json()` etc.
	 */
	virtual FGetLanguageParser* GetLanguageParser(const ETreeSitterLanguage InLanguage) = 0;
};
