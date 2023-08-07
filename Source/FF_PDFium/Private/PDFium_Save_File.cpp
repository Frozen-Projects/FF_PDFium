// Fill out your copyright notice in the Description page of Project Settings.

#include "PDFium_Save_File.h"

#include "Misc/FileHelper.h"

THIRD_PARTY_INCLUDES_START
#include "fpdf_save.h"
#include "fpdf_edit.h"
THIRD_PARTY_INCLUDES_END

// Global bytes array for saving PDF files.
TArray64<uint8> PDF_Bytes;

// Sets default values
APDFium_Save_File::APDFium_Save_File()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APDFium_Save_File::BeginPlay()
{
	Super::BeginPlay();
}

void APDFium_Save_File::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PDF_Bytes.Empty();
	
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void APDFium_Save_File::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int Callback_File(FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)
{
	PDF_Bytes.Append(static_cast<const uint8*>(pData), size);

	return size;
}

bool APDFium_Save_File::PDFium_Save_File(UPARAM(ref)UPDFiumDoc*& In_PDF, FString Export_Path)
{
	if (UFF_PDFiumBPLibrary::PDFium_LibState() == false)
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
	
	// We need to clear byte array before executing new save.
	PDF_Bytes.Empty();

	FPDF_FILEWRITE Writer;
	memset(&Writer, 0, sizeof(FPDF_FILEWRITE));

	Writer.version = 1;
	Writer.WriteBlock = Callback_File;

	FPDF_SaveAsCopy(In_PDF->Document, &Writer, FPDF_INCREMENTAL);
	
	if (PDF_Bytes.Num() > 0)
	{
		FFileHelper::SaveArrayToFile(PDF_Bytes, *Export_Path);

		// We need to clear byte array after executing save.
		PDF_Bytes.Empty();

		return true;
	}
	
	else
	{
		return false;
	}
}