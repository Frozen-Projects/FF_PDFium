// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF_PDFium.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FFF_PDFiumModule"

void FFF_PDFiumModule::StartupModule()
{
#ifdef _WIN64
	const FString BasePluginDir = IPluginManager::Get().FindPlugin("FF_PDFium")->GetBaseDir();
	const FString DLL_Path = FPaths::Combine(*BasePluginDir, TEXT("Source/FF_PDFium/ThirdParty/pdfium/Windows/lib/pdfium.dll"));
	PDFium_Handle = FPlatformProcess::GetDllHandle(*DLL_Path);

	if (PDFium_Handle != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("pdfium.dll loaded successfully!"));
	}

	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("pdfium.dll failed to load!"));
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