// Copyright 2024 Mickael Daniel. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

struct TSParser;
struct TSLanguage;

class FTreeSitterModule : public IModuleInterface
{
public:

	//~ Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface

private:
	TArray<void*> ParserLibraryHandles;
	
	using FFuncGetSomething = const TSLanguage*(void);
	FFuncGetSomething* tree_sitter_json = nullptr;
	
	static void* LoadLibraryHandle();
	static void* LoadLanguageLibraryHandle(const FString& InLibraryPath);
	void CheckTreeSitter(TSParser* InParser) const;

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
