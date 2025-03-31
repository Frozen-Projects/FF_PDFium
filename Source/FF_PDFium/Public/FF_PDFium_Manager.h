// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "FF_PDFium_Includes.h"

#include "FF_PDFium_Manager.generated.h"

class UPDFiumDoc;

UCLASS()
class FF_PDFIUM_API UFF_PDFium_ManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:

	FPDF_LIBRARY_CONFIG Config;
	bool bIsLibInitialized = false;
	
	// We have to store all created PDF documents because closing them before library destroy is our responsibility.
	TArray<UPDFiumDoc*> Array_PDFs;

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "PDFium - Is Initialized", ToolTip = "", Keywords = "pdfium, pdf, document, library, lib, is, initialized"), Category = "Frozen Forest|FF_PDFium|System")
	bool IsPDFiumInitialized() const;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from File", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, file"), Category = "Frozen Forest|FF_PDFium|System")
	virtual bool PDFium_Doc_Open_File(UPDFiumDoc*& Out_PDF, FString& ErrorCode, FString In_Path, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from Memory x64", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, memory"), Category = "Frozen Forest|FF_PDFium|System")
	virtual bool PDFium_Doc_Open_Memory_x64(UPDFiumDoc*& Out_PDF, FString& ErrorCode, UPARAM(ref)UBytesObject_64*& In_Bytes_Object, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from Memory x86", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, memory"), Category = "Frozen Forest|FF_PDFium|System")
	virtual bool PDFium_Doc_Open_Memory_x86(UPDFiumDoc*& Out_PDF, FString& ErrorCode, TArray<uint8> In_Bytes, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Create Document", ToolTip = "", Keywords = "pdfium, pdf, create, doc, document, pdf"), Category = "PDFium|Write")
	virtual bool PDFium_Create_Doc(UPDFiumDoc*& Out_PDF);
};