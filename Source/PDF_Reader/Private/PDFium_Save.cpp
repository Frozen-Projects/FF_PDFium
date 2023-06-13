// Fill out your copyright notice in the Description page of Project Settings.

#include "PDFium_Save.h"

#include "Misc/FileHelper.h"

THIRD_PARTY_INCLUDES_START
#include <fstream>
#include "fpdf_edit.h"
THIRD_PARTY_INCLUDES_END

// Global path variable for saving PDF files.
FString Global_Export_Path;

// Global bytes variable for PDF bytes.
UBytesObject_64* Global_Bytes_Object;

// Sets default values
APDFium_Save::APDFium_Save()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APDFium_Save::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APDFium_Save::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int Callback_File(FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)
{
	TArray64<uint8> PDF_Bytes;
	PDF_Bytes.SetNum(size);
	FMemory::Memcpy(PDF_Bytes.GetData(), pData, size);
	FFileHelper::SaveArrayToFile(PDF_Bytes, *Global_Export_Path);
	PDF_Bytes.Empty();

	return size;
}

int Callback_Bytes(FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)
{
	TArray64<uint8> PDF_Bytes;
	PDF_Bytes.SetNum(size);
	FMemory::Memcpy(PDF_Bytes.GetData(), pData, size);
	
	Global_Bytes_Object = NewObject<UBytesObject_64>();
	Global_Bytes_Object->ByteArray = PDF_Bytes;

	return size;
}

bool APDFium_Save::PDFium_Save_File(UPARAM(ref)UPDFiumDoc*& In_PDF, FString Export_Path)
{
	if (UPDF_ReaderBPLibrary::PDFium_LibState() == false)
	{
		return false;
	}

	if (IsValid(In_PDF) == false)
	{
		return false;
	}

	if (!In_PDF->Document)
	{
		return false;
	}

	if (Export_Path.IsEmpty() == true)
	{
		return false;
	}

	Global_Export_Path = Export_Path;

	FPDF_FILEWRITE Writer;
	memset(&Writer, 0, sizeof(FPDF_FILEWRITE));

	Writer.version = 1;
	Writer.WriteBlock = Callback_File;

	FPDF_SaveAsCopy(In_PDF->Document, &Writer, 3);

	return true;
}

bool APDFium_Save::PDFium_Save_Bytes(UBytesObject_64*& Out_Bytes, UPARAM(ref)UPDFiumDoc*& In_PDF)
{
	if (UPDF_ReaderBPLibrary::PDFium_LibState() == false)
	{
		return false;
	}

	if (IsValid(In_PDF) == false)
	{
		return false;
	}

	if (!In_PDF->Document)
	{
		return false;
	}

	FPDF_FILEWRITE Writer;
	memset(&Writer, 0, sizeof(FPDF_FILEWRITE));

	Writer.version = 1;
	Writer.WriteBlock = Callback_Bytes;
	FPDF_SaveAsCopy(In_PDF->Document, &Writer, 3);

	if (Global_Bytes_Object->ByteArray.Num() > 0)
	{
		Out_Bytes = Global_Bytes_Object;
		return true;
	}

	else
	{
		return false;
	}
}