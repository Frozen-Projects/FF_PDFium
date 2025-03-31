// Fill out your copyright notice in the Description page of Project Settings.

#include "FF_PDFium_Manager.h"

// Custom Includes.
#include "FF_PDFium_Doc.h"

void UFF_PDFium_ManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (this->bIsLibInitialized)
	{
		return;
	}

	try
	{
		FMemory::Memset(&Config, 0, sizeof(Config));
		Config.version = 2;
		Config.m_pUserFontPaths = nullptr;
		Config.m_pIsolate = nullptr;
		Config.m_v8EmbedderSlot = 0;

		FPDF_InitLibraryWithConfig(&Config);
		this->bIsLibInitialized = true;
	}

	catch (const std::exception& Exception)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(Exception.what()));
	}
}

void UFF_PDFium_ManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (!this->bIsLibInitialized)
	{
		return;
	}

	if (!this->Array_PDFs.IsEmpty())
	{
		for (UPDFiumDoc* EachDoc : this->Array_PDFs)
		{
			if (IsValid(EachDoc))
			{
				EachDoc->ConditionalBeginDestroy();
			}
		}

		this->Array_PDFs.Empty();
	}

	FPDF_DestroyLibrary();
	this->bIsLibInitialized = false;
}

bool UFF_PDFium_ManagerSubsystem::IsPDFiumInitialized() const
{
	return this->bIsLibInitialized;
}

bool UFF_PDFium_ManagerSubsystem::PDFium_Doc_Open_File(UPDFiumDoc*& Out_PDF, FString& ErrorCode, FString In_Path, FString In_PDF_Password)
{
	if (!this->bIsLibInitialized)
	{
		return false;
	}

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

	FPDF_DOCUMENT Temp_Document = FPDF_LoadDocument(TCHAR_TO_UTF8(*Path), TCHAR_TO_UTF8(*In_PDF_Password));

	if (!Temp_Document)
	{
		ErrorCode = "PDF is invalid.";
		return false;
	}

	FPDF_LoadXFA(Temp_Document);

	Out_PDF = NewObject<UPDFiumDoc>();
	Out_PDF->Document = Temp_Document;
	Out_PDF->SetManager(this);
	this->Array_PDFs.Add(Out_PDF);

	ErrorCode = "Success.";
	return true;
}

bool UFF_PDFium_ManagerSubsystem::PDFium_Doc_Open_Memory_x64(UPDFiumDoc*& Out_PDF, FString& ErrorCode, UPARAM(ref)UBytesObject_64*& In_Bytes_Object, FString In_PDF_Password)
{
	if (!this->bIsLibInitialized)
	{
		return false;
	}

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

	const size_t PDF_Data_Size = In_Bytes_Object->ByteArray.Num();

	UPDFiumDoc* TempObject = NewObject<UPDFiumDoc>();

	if (!TempObject->SetBuffer(PDF_Data_Size, In_Bytes_Object->ByteArray.GetData()))
	{
		TempObject->ConditionalBeginDestroy();
		ErrorCode = "There was a problem while setting pdfium buffer !";
		return false;
	}

	FPDF_DOCUMENT Temp_Document = FPDF_LoadMemDocument64(TempObject->GetBuffer(), TempObject->GetSize(), TCHAR_TO_UTF8(*In_PDF_Password));

	if (!Temp_Document)
	{
		TempObject->ConditionalBeginDestroy();
		ErrorCode = "PDF is invalid.";
		return false;
	}

	FPDF_LoadXFA(Temp_Document);

	TempObject->Document = Temp_Document;
	TempObject->SetManager(this);
	this->Array_PDFs.Add(TempObject);

	Out_PDF = TempObject;
	ErrorCode = "Successful.";

	return true;
}

bool UFF_PDFium_ManagerSubsystem::PDFium_Doc_Open_Memory_x86(UPDFiumDoc*& Out_PDF, FString& ErrorCode, TArray<uint8> In_Bytes, FString In_PDF_Password)
{
	if (!this->bIsLibInitialized)
	{
		return false;
	}

	if (In_Bytes.IsEmpty())
	{
		ErrorCode = "Byte array is empty !";
		return false;
	}

	const size_t PDF_Data_Size = In_Bytes.Num();

	UPDFiumDoc* TempObject = NewObject<UPDFiumDoc>();
	if (!TempObject->SetBuffer(PDF_Data_Size, In_Bytes.GetData()))
	{
		TempObject->ConditionalBeginDestroy();
		ErrorCode = "There was a problem while setting pdfium buffer !";
		return false;
	}

	FPDF_DOCUMENT Temp_Document = FPDF_LoadMemDocument64(TempObject->GetBuffer(), TempObject->GetSize(), TCHAR_TO_UTF8(*In_PDF_Password));

	if (!Temp_Document)
	{
		TempObject->ConditionalBeginDestroy();
		ErrorCode = "PDF is invalid.";
		return false;
	}

	FPDF_LoadXFA(Temp_Document);

	TempObject->Document = Temp_Document;
	TempObject->SetManager(this);
	this->Array_PDFs.Add(TempObject);

	Out_PDF = TempObject;
	ErrorCode = "Successful.";
	return true;
}

bool UFF_PDFium_ManagerSubsystem::PDFium_Create_Doc(UPDFiumDoc*& Out_PDF)
{
	if (!this->bIsLibInitialized)
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
	Out_PDF->SetManager(this);
	this->Array_PDFs.Add(Out_PDF);

	return true;
}