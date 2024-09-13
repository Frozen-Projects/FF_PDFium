// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "FF_PDFiumBPLibrary.h"

#include "FF_PDFium_Save.generated.h"

UCLASS()
class FF_PDFIUM_API APDFium_Save : public AActor
{
	GENERATED_BODY()
	
private:

	static int Callback_Writer(FPDF_FILEWRITE* pThis, const void* pData, unsigned long size);

public:	
	
	// Sets default values for this actor's properties.
	APDFium_Save();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Save File", ToolTip = "", Keywords = "pdfium, pdf, doc, document, save, file"), Category = "PDF_Reader|Write")
	virtual bool PDFium_Save_File(UPARAM(ref)UPDFiumDoc*& In_PDF, FString Export_Path, EPDFiumSaveTypes In_SaveType = EPDFiumSaveTypes::Incremental, EPDFiumSaveVersion In_Version = EPDFiumSaveVersion::PDF_17);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Save Bytes", ToolTip = "", Keywords = "pdfium, pdf, doc, document, save, bytes"), Category = "PDF_Reader|Write")
	virtual bool PDFium_Save_Bytes(UBytesObject_64*& Out_Bytes, UPARAM(ref)UPDFiumDoc*& In_PDF, EPDFiumSaveTypes In_SaveType = EPDFiumSaveTypes::Incremental, EPDFiumSaveVersion In_Version = EPDFiumSaveVersion::PDF_17);

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};
