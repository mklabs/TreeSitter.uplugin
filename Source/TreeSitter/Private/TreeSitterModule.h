// Copyright 2024 Mickael Daniel. All Rights Reserved.

#pragma once

#include "ITreeSitterModule.h"

struct TSLanguage;
struct TSNode;
struct TSParser;
struct TSRange;
struct TSPoint;

class FTreeSitterModule : public ITreeSitterModule
{
public:

	//~ Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface
	
	//~ Begin ITreeSitterModule
	virtual FGetLanguageParser* GetLanguageParser(const ETreeSitterLanguage InLanguage) override;
	//~ End ITreeSitterModule

private:
	/** References of registered console commands via IConsoleManager */
	TArray<IConsoleCommand*> ConsoleCommands;
	
	/** Reference to opened slate widget */
	TSharedPtr<SWindow> SlateWindow;

	/** Called from StartupModule and sets up console commands for the plugin via IConsoleManager */
	void RegisterConsoleCommands();
	
	/** Called from ShutdownModule and clears out previously registered console commands */
	void UnregisterConsoleCommands();
	
	void ExecuteWidgetCommand(const TArray<FString>& InArgs);
	void ExecuteTestCommand(const TArray<FString>& InArgs) const;
	
	TSharedPtr<SWindow> OpenWindow(const TSharedRef<SWidget>& InWidgetContent, const FText& InTitle = FText::GetEmpty(), const FVector2f& InWindowSize = FVector2f(1280.f, 1080.f));
	void HandleWindowClosed(const TSharedRef<SWindow>& InWindow);
	
	TArray<void*> ParserLibraryHandles;

	using FGetLanguageParser = const TSLanguage*(void);
	FGetLanguageParser* tree_sitter_javascript = nullptr;
	FGetLanguageParser* tree_sitter_json = nullptr;
	FGetLanguageParser* tree_sitter_markdown = nullptr;
	FGetLanguageParser* tree_sitter_markdown_inline = nullptr;
	
	static void* LoadLanguageLibraryHandle(const FString& InLibraryPath);

	void* LoadLanguageLibrary(const FString& InDLLName);
	
	void* LoadLanguageLibraryWithDLLExport(const FString& InDLLName, const FString& InExportName, FGetLanguageParser*& OutExportHandle);
	void* LoadLanguageLibraryWithDLLExport(const FString& InLanguageName, FGetLanguageParser*& OutExportHandle);
	
	void CheckTreeSitter() const;
	void CheckTreeSitterMarkdown() const;

	static FString GetNodeTextForRanges(const FString& InSource, const TSPoint& InStartPoint, const TSPoint& InEndPoint);

	static bool GetInlineTSRanges(const TSNode& InNode, TArray<TSRange>& OutRanges);
	
	static void DebugASTNodeInfo(const FString& InSource, const TSNode& InNode, const FString& InPadding = TEXT(""));

	template<typename FFuncType>
	static void GetDllExport(const TCHAR* DllName, void* DllHandle, const TCHAR* ExportName, FFuncType& ExportHandle)
	{
		void* FuncPtr = FPlatformProcess::GetDllExport(DllHandle, ExportName);
		if (FuncPtr)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Retrieved DLL export '%s' from %s."), ExportName, DllName);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not get DLL export '%s' from %s."), ExportName, DllName);
		}
		ExportHandle = static_cast<FFuncType>(FuncPtr);
	};
};
