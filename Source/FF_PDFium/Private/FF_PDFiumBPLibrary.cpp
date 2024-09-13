// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF_PDFiumBPLibrary.h"
#include "FF_PDFium.h"

// Global library initialization checker.
bool Global_IsPDFiumInitialized = false;

UFF_PDFiumBPLibrary::UFF_PDFiumBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}

bool UFF_PDFiumBPLibrary::PDFium_LibState()
{
	return Global_IsPDFiumInitialized;
}

bool UFF_PDFiumBPLibrary::PDFium_LibInit(FString& Out_Code)
{
	if (Global_IsPDFiumInitialized == true)
	{
		Out_Code = "Library already initialized";
		return false;
	}
	
	FPDF_LIBRARY_CONFIG config;
	FMemory::Memset(&config, 0, sizeof(config));
	config.version = 2;
	config.m_pUserFontPaths = NULL;
	config.m_pIsolate = NULL;
	config.m_v8EmbedderSlot = 0;
	FPDF_InitLibraryWithConfig(&config);

	Global_IsPDFiumInitialized = true;

	Out_Code = "Library successfully initialized.";
	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_LibClose(FString& Out_Code)
{
	if (Global_IsPDFiumInitialized == false)
	{
		Out_Code = "Library already closed.";
		return false;
	}

	Global_IsPDFiumInitialized = false;
	FPDF_DestroyLibrary();

	Out_Code = "Library successfully closed.";
	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Doc_Open_File(UPDFiumDoc*& Out_PDF, FString& ErrorCode, FString In_Path, FString this_Password)
{
	if (In_Path.IsEmpty())
	{
		ErrorCode = "Path is empty.";
		return false;
	}

	FPaths::NormalizeFilename(In_Path);
	FString Path = FPlatformFileManager::Get().GetPlatformFile().ConvertToAbsolutePathForExternalAppForRead(*In_Path);

	if (!FPaths::FileExists(Path))
	{
		ErrorCode = "There is no file to open.";
		return false;
	}

	FPaths::MakePlatformFilename(Path);

	FPDF_DOCUMENT Temp_Document = FPDF_LoadDocument(TCHAR_TO_UTF8(*Path), TCHAR_TO_UTF8(*this_Password));

	if (!Temp_Document)
	{
		ErrorCode = "PDF is invalid.";
		return false;
	}

	FPDF_LoadXFA(Temp_Document);

	Out_PDF = NewObject<UPDFiumDoc>();
	Out_PDF->Document = Temp_Document;

	ErrorCode = "Success.";
	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Doc_Open_Memory(UPDFiumDoc*& Out_PDF, FString& ErrorCode, UPARAM(ref)UBytesObject_64*& In_Bytes_Object, FString this_Password)
{
	if (!IsValid(In_Bytes_Object))
	{
		ErrorCode = "Bytes object is invalid.";
		return false;
	}

	if (In_Bytes_Object->ByteArray.IsEmpty())
	{
		ErrorCode = "There is no bytes.";
		return false;
	}

	void* PDF_Data = In_Bytes_Object->ByteArray.GetData();
	const size_t PDF_Data_Size = In_Bytes_Object->ByteArray.Num();

	FPDF_DOCUMENT Temp_Document = FPDF_LoadMemDocument64(PDF_Data, PDF_Data_Size, TCHAR_TO_UTF8(*this_Password));

	if (!Temp_Document)
	{
		ErrorCode = "PDF is invalid.";
		return false;
	}

	FPDF_LoadXFA(Temp_Document);

	Out_PDF = NewObject<UPDFiumDoc>();
	Out_PDF->Document = Temp_Document;

	ErrorCode = "Success.";
	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Create_Doc(UPDFiumDoc*& Out_PDF)
{
	if (!Global_IsPDFiumInitialized)
	{
		return false;
	}

	FPDF_DOCUMENT Temp_Document = FPDF_CreateNewDocument();

	if (!Temp_Document)
	{
		return false;
	}

	Out_PDF = NewObject<UPDFiumDoc>();
	Out_PDF->Document = Temp_Document;

	return true;
}