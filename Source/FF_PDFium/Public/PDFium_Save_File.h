// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "FF_PDFiumBPLibrary.h"

#include "PDFium_Save_File.generated.h"

UCLASS()
class FF_PDFIUM_API APDFium_Save_File : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APDFium_Save_File();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Save File", ToolTip = "", Keywords = "pdfium, pdf, doc, document, save, file"), Category = "PDF_Reader|Write")
	virtual bool PDFium_Save_File(UPARAM(ref)UPDFiumDoc*& In_PDF, FString Export_Path);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
