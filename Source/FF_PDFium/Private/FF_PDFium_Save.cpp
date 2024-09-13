// Fill out your copyright notice in the Description page of Project Settings.

#include "FF_PDFium_Save.h"

// Global bytes array for saving PDF files.
static TArray64<uint8> PDF_Bytes;

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

void APDFium_Save::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PDF_Bytes.Empty();
	
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void APDFium_Save::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int APDFium_Save::Callback_Writer(FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)
{
	PDF_Bytes.Append(static_cast<const uint8*>(pData), size);

	return size;
}

bool APDFium_Save::PDFium_Save_File(UPARAM(ref)UPDFiumDoc*& In_PDF, FString Export_Path, EPDFiumSaveTypes In_SaveType, EPDFiumSaveVersion In_Version)
{
	if (!UFF_PDFiumBPLibrary::PDFium_LibState())
	{
		return false;
	}

	if (!IsValid(In_PDF))
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
	Writer.WriteBlock = APDFium_Save::Callback_Writer;
	Writer.version = 1;

	int Flags = FPDF_INCREMENTAL;
	switch (In_SaveType)
	{
	case EPDFiumSaveTypes::Incremental:
		Flags = FPDF_INCREMENTAL;
		break;

	case EPDFiumSaveTypes::NoIncremental:
		Flags = FPDF_NO_INCREMENTAL;
		break;

	case EPDFiumSaveTypes::RemoveSecurity:
		Flags = FPDF_REMOVE_SECURITY;
		break;

	default:
		Flags = FPDF_INCREMENTAL;
		break;
	}

	int Version = 17;
	switch (In_Version)
	{
	case EPDFiumSaveVersion::PDF_14:
		Version = 14;
		break;
	case EPDFiumSaveVersion::PDF_15:
		Version = 15;
		break;
	case EPDFiumSaveVersion::PDF_17:
		Version = 17;
		break;
	default:
		Version = 17;
		break;
	}

	FPDF_BOOL RetVal = FPDF_SaveWithVersion(In_PDF->Document, &Writer, Flags, Version);

	if (PDF_Bytes.Num() > 0 && RetVal == 1)
	{
		FFileHelper::SaveArrayToFile(PDF_Bytes, *Export_Path);
		
		PDF_Bytes.Empty();
		return true;
	}
	
	else
	{
		PDF_Bytes.Empty();
		return false;
	}
}

bool APDFium_Save::PDFium_Save_Bytes(UBytesObject_64*& Out_Bytes, UPARAM(ref)UPDFiumDoc*& In_PDF, EPDFiumSaveTypes In_SaveType, EPDFiumSaveVersion In_Version)
{
	if (!UFF_PDFiumBPLibrary::PDFium_LibState())
	{
		return false;
	}

	if (!IsValid(In_PDF))
	{
		return false;
	}

	if (!In_PDF->Document)
	{
		return false;
	}

	// We need to clear byte array before executing new save.
	PDF_Bytes.Empty();

	FPDF_FILEWRITE Writer;
	memset(&Writer, 0, sizeof(FPDF_FILEWRITE));
	Writer.WriteBlock = APDFium_Save::Callback_Writer;
	Writer.version = 1;

	int Flags = FPDF_INCREMENTAL;
	switch (In_SaveType)
	{
	case EPDFiumSaveTypes::Incremental:
		Flags = FPDF_INCREMENTAL;
		break;

	case EPDFiumSaveTypes::NoIncremental:
		Flags = FPDF_NO_INCREMENTAL;
		break;

	case EPDFiumSaveTypes::RemoveSecurity:
		Flags = FPDF_REMOVE_SECURITY;
		break;

	default:
		Flags = FPDF_INCREMENTAL;
		break;
	}

	int Version = 17;
	switch (In_Version)
	{
		case EPDFiumSaveVersion::PDF_14:
			Version = 14;
			break;
		case EPDFiumSaveVersion::PDF_15:
			Version = 15;
			break;
		case EPDFiumSaveVersion::PDF_17:
			Version = 17;
			break;
		default:
			Version = 17;
			break;
	}

	FPDF_BOOL RetVal = FPDF_SaveWithVersion(In_PDF->Document, &Writer, Flags, Version);

	if (PDF_Bytes.Num() > 0 && RetVal == 1)
	{
		UBytesObject_64* Temp_PDF = NewObject<UBytesObject_64>();
		FMemory::Memcpy(Temp_PDF->ByteArray.GetData(), PDF_Bytes.GetData(), PDF_Bytes.Num());
		Out_Bytes = Temp_PDF;

		PDF_Bytes.Empty();
		return true;
	}

	else
	{
		PDF_Bytes.Empty();
		return false;
	}
}
