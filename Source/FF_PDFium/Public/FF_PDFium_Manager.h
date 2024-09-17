// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "FF_PDFium_Includes.h"

#include "FF_PDFium_Manager.generated.h"

class UPDFiumDoc;

UCLASS()
class FF_PDFIUM_API AFF_PDFium_Manager : public AActor
{
	GENERATED_BODY()

private:

	FPDF_LIBRARY_CONFIG config;
	bool bIsPdfiumStarted = false;

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game ends or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	UPROPERTY()
	TArray<UPDFiumDoc*> Array_PDFs;

	// Sets default values for this actor's properties.
	AFF_PDFium_Manager();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "PDFium - Library State", ToolTip = "", Keywords = "pdfium, pdf, document, library, state, get, is, initialized"), Category = "PDFium|System")
	virtual bool PDFium_LibState();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Library Init", Keywords = "pdfium, pdf, document, library, lib, open, init"), Category = "PDFium|System")
	virtual bool PDFium_LibInit(FString& Out_Code);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Library Close", Keywords = "pdfium, pdf, document, library, lib, close"), Category = "PDFium|System")
	virtual void PDFium_LibClose();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from File", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, file"), Category = "PDFium|Read")
	virtual bool PDFium_Doc_Open_File(UPDFiumDoc*& Out_PDF, FString& ErrorCode, FString In_Path, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from Memory", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, memory"), Category = "PDFium|Read")
	virtual bool PDFium_Doc_Open_Memory(UPDFiumDoc*& Out_PDF, FString& ErrorCode, UPARAM(ref)UBytesObject_64*& In_Bytes_Object, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Create Document", ToolTip = "", Keywords = "pdfium, pdf, create, doc, document, pdf"), Category = "PDFium|Write")
	virtual bool PDFium_Create_Doc(UPDFiumDoc*& Out_PDF);

};
