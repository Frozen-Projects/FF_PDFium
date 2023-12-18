// Fill out your copyright notice in the Description page of Project Settings.

#include "PDFium_Save_Memory.h"

THIRD_PARTY_INCLUDES_START
#include "fpdf_save.h"
THIRD_PARTY_INCLUDES_END

// Global bytes object for PDF file.
UBytesObject_64* Global_Bytes_Object = nullptr;

// Sets default values
APDFium_Save_Memory::APDFium_Save_Memory()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APDFium_Save_Memory::BeginPlay()
{
	Super::BeginPlay();
}

void APDFium_Save_Memory::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(Global_Bytes_Object))
	{
		Global_Bytes_Object = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void APDFium_Save_Memory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int Callback_Bytes(FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)
{
	if (!Global_Bytes_Object)
	{
		Global_Bytes_Object = NewObject<UBytesObject_64>();
	}

	Global_Bytes_Object->ByteArray.Append(static_cast<const uint8*>(pData), size);

	return size;
}

bool APDFium_Save_Memory::PDFium_Save_Bytes(UBytesObject_64*& Out_Bytes, UPARAM(ref)UPDFiumDoc*& In_PDF)
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

	// We need to clear byte array before executing new save.
	if (IsValid(Global_Bytes_Object))
	{
		Global_Bytes_Object->ByteArray.Empty();
	}

	FPDF_FILEWRITE Writer;
	memset(&Writer, 0, sizeof(FPDF_FILEWRITE));

	Writer.version = 1;
	Writer.WriteBlock = Callback_Bytes;
	FPDF_SaveAsCopy(In_PDF->Document, &Writer, FPDF_INCREMENTAL);

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