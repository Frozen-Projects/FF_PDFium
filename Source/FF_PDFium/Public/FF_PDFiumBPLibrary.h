// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "FF_PDFium_Doc.h"
#include "FF_PDFiumBPLibrary.generated.h"

UCLASS()
class UFF_PDFiumBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, meta = (DisplayName = "PDFium - Library State", ToolTip = "", Keywords = "pdfium, pdf, document, library, state, get, is, initialized"), Category = "PDFium|System")
	static FF_PDFIUM_API bool PDFium_LibState();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Library Init", Keywords = "pdfium, pdf, document, library, lib, open, init"), Category = "PDFium|System")
	static FF_PDFIUM_API bool PDFium_LibInit(FString& Out_Code);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Library Close", Keywords = "pdfium, pdf, document, library, lib, close"), Category = "PDFium|System")
	static FF_PDFIUM_API bool PDFium_LibClose(FString& Out_Code);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from File", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, file"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Doc_Open_File(UPDFiumDoc*& Out_PDF, FString& ErrorCode, FString In_Path, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from Memory", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, memory"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Doc_Open_Memory(UPDFiumDoc*& Out_PDF, FString& ErrorCode, UPARAM(ref)UBytesObject_64*& In_Bytes_Object, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Create Document", ToolTip = "", Keywords = "pdfium, pdf, create, doc, document, pdf"), Category = "PDFium|Write")
	static FF_PDFIUM_API bool PDFium_Create_Doc(UPDFiumDoc*& Out_PDF);

};