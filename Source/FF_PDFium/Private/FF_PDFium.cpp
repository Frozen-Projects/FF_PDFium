// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF_PDFium.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FFF_PDFiumModule"

DEFINE_LOG_CATEGORY(Log_Delay_Load);

void FFF_PDFiumModule::StartupModule()
{
#ifdef _WIN64
	const FString BasePluginDir = IPluginManager::Get().FindPlugin("FF_PDFium")->GetBaseDir();
	const FString DLL_Path = FPaths::Combine(*BasePluginDir, TEXT("Source/FF_PDFium/ThirdParty/pdfium/Win64/bin/pdfium.dll"));
	PDFium_Handle = FPlatformProcess::GetDllHandle(*DLL_Path);

	if (PDFium_Handle != nullptr)
	{
		UE_LOG(Log_Delay_Load, Log, TEXT("pdfium.dll loaded successfully!"));
	}

	else
	{
		UE_LOG(Log_Delay_Load, Fatal, TEXT("pdfium.dll failed to load!"));
	}
#endif
}

void FFF_PDFiumModule::ShutdownModule()
{
#ifdef _WIN64
	FPlatformProcess::FreeDllHandle(PDFium_Handle);
	PDFium_Handle = nullptr;
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFF_PDFiumModule, FF_PDFium)