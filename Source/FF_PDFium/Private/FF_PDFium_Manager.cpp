// Fill out your copyright notice in the Description page of Project Settings.

#include "FF_PDFium_Manager.h"

// Custom Includes.
#include "FF_PDFium_Doc.h"

// Sets default values.
AFF_PDFium_Manager::AFF_PDFium_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void AFF_PDFium_Manager::BeginPlay()
{
	Super::BeginPlay();

	FString Out_Code;

	if (!this->PDFium_LibInit(Out_Code))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Out_Code);
	}
}

// Called when the game ends or when destroyed.
void AFF_PDFium_Manager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	FTimerHandle DelayTimer;
	FTimerDelegate DelayDelegate;
	DelayDelegate.BindUObject(this, &AFF_PDFium_Manager::PDFium_LibClose);
	//GEngine->GetCurrentPlayWorld()->GetTimerManager().SetTimer(DelayTimer, DelayDelegate, 0.2, false);
}

// Called every frame.
void AFF_PDFium_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AFF_PDFium_Manager::PDFium_LibState()
{
	return this->bIsPdfiumStarted;
}

bool AFF_PDFium_Manager::PDFium_LibInit(FString& Out_Code)
{
	if (this->bIsPdfiumStarted)
	{
		Out_Code = "Library already initialized.";
		return false;
	}

	FPDF_LIBRARY_CONFIG config;
	FMemory::Memset(&config, 0, sizeof(config));
	config.version = 2;
	config.m_pUserFontPaths = NULL;
	config.m_pIsolate = NULL;
	config.m_v8EmbedderSlot = 0;
	FPDF_InitLibraryWithConfig(&config);
	
	Out_Code = "Library successfully initialized.";
	this->bIsPdfiumStarted = true;

	return true;
}

void AFF_PDFium_Manager::PDFium_LibClose()
{
	if (!Array_PDFs.IsEmpty())
	{
		for (UPDFiumDoc* EachPDF : this->Array_PDFs)
		{
			if (EachPDF->Document)
			{
				FPDF_CloseDocument(EachPDF->Document);
			}
		}
	}

	FPDF_DestroyLibrary();
	this->bIsPdfiumStarted = false;
}

bool AFF_PDFium_Manager::PDFium_Doc_Open_File(UPDFiumDoc*& Out_PDF, FString& ErrorCode, FString In_Path, FString this_Password)
{
	if (!this->bIsPdfiumStarted)
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

	FPDF_DOCUMENT Temp_Document = FPDF_LoadDocument(TCHAR_TO_UTF8(*Path), TCHAR_TO_UTF8(*this_Password));

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

bool AFF_PDFium_Manager::PDFium_Doc_Open_Memory(UPDFiumDoc*& Out_PDF, FString& ErrorCode, UPARAM(ref)UBytesObject_64*& In_Bytes_Object, FString this_Password)
{
	if (!this->bIsPdfiumStarted)
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
	Out_PDF->SetManager(this);
	this->Array_PDFs.Add(Out_PDF);

	ErrorCode = "Success.";
	return true;
}

bool AFF_PDFium_Manager::PDFium_Create_Doc(UPDFiumDoc*& Out_PDF)
{
	if (!this->bIsPdfiumStarted)
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