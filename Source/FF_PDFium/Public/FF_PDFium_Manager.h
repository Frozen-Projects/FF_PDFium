// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/EngineSubsystem.h"

#include "FF_PDFium_Includes.h"
#include "FF_PDFium_Enums.h"
#include "FF_PDFium_Font.h"

#include "FF_PDFium_Manager.generated.h"

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegatePdfium, bool, bIsSuccessfull, FString, OutCode);

USTRUCT(BlueprintType)
struct FF_PDFIUM_API FPdfTextObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	FString Text_String = "";

	UPROPERTY(BlueprintReadOnly)
	FVector2D Text_Position = FVector2D(0, 0);

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "Some PDF editors use \"1\" for font size and set size of text object. If you come across with that kind of PDF, use this."))
	FVector2D Text_Size = FVector2D(0, 0);

	UPROPERTY(BlueprintReadOnly)
	FColor Text_Color = FColor::Black;

	UPROPERTY(BlueprintReadOnly)
	FString Font_Name = "";

	UPROPERTY(BlueprintReadOnly)
	URuntimeFont* Runtime_Font = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float Font_Size = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Font_Flags = 0;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "600-700 ish values are bold."))
	int32 Font_Weight = 0;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "Values other than \"0\" means font is italic. Negative is italic to right and positive is italic to left."))
	int32 Font_Italic = 0;

};

UCLASS()
class FF_PDFIUM_API UPDFiumSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

private:

	FThreadSafeBool bIsLibInitialized = false;

public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool IsLibInitialized() const;
};

UCLASS()
class FF_PDFIUM_API APDF_Manager : public AActor
{
	GENERATED_BODY()

private:

	static inline FThreadSafeBool bIsLibInitialized = false;

	FPDF_DOCUMENT Document = NULL;

	// We need to store PDF data in memory because if we lose the file (PDF deleted/moved) we won't be able to run pdfium functions in there future.
	TArray64<uint8> PDF_Bytes_x64;

	UPROPERTY()
	TArray<UPDFiumFont*> Array_Fonts;

	virtual TArray64<uint8> SaveBytes64(bool& bIsSaveSuccessful, EPDFiumSaveTypes In_SaveType, EPDFiumSaveVersion In_Version);
	virtual TArray<uint8> SaveBytes(bool& bIsSaveSuccessful, EPDFiumSaveTypes In_SaveType, EPDFiumSaveVersion In_Version);

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game ends or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	// Sets default values for this actor's properties.
	APDF_Manager();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from File", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, file"), Category = "Frozen Forest|FF_PDFium|System")
	virtual bool PDFium_Doc_Open_File(FString& ErrorCode, FString In_Path, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from Memory x64", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, memory"), Category = "Frozen Forest|FF_PDFium|System")
	virtual bool PDFium_Doc_Open_Memory_x64(FString& ErrorCode, UPARAM(ref)UBytesObject_64*& In_Bytes_Object, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from Memory x86", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, memory"), Category = "Frozen Forest|FF_PDFium|System")
	virtual bool PDFium_Doc_Open_Memory_x86(FString& ErrorCode, TArray<uint8> In_Bytes, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Create Document", ToolTip = "", Keywords = "pdfium, pdf, create, doc, document, pdf"), Category = "Frozen Forest|FF_PDFium|System")
	virtual bool PDFium_Create_Doc();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Clear Document", ToolTip = "", Keywords = "pdfium, pdf, clear, doc, document, pdf"), Category = "Frozen Forest|FF_PDFium|System")
	virtual bool PDFium_Clear_Doc();

	/**
	* @param In_Sampling Default (also minimum) value is "1" but "2" gives best result for A4 sized PDF on 17 inch notebook screen. Bigger values is good for 3D widget like huge UIs.
	* @param Out_Pages Created textures don't have BITMAP headers. So, they don't work with other technologies. If you need real bitmaps, export UTexture2Ds with "ExtendedVars -> ExportT2dAsBitmap"
	* @param bUseMatrix Don't use this boolean. Currently it is exprimental and doesn't work.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Pages", Keywords = "pdfium, pdf, document, read, get, pages"), Category = "Frozen Forest|FF_PDFium|Read")
	virtual bool PDFium_Get_Pages(FJsonObjectWrapper& Out_Code, TMap<UTexture2D*, FVector2D>& Out_Pages, int32 In_Sampling = 1, FColor BG_Color = FColor::White, bool bUseSrgb = true, bool bUseMatrix = false, bool bUseAlpha = true, bool bRenderAnnots = true);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Images", Keywords = "pdfium, pdf, document, read, get, images"), Category = "Frozen Forest|FF_PDFium|Read")
	virtual bool PDFium_Get_Images(TMap<UTexture2D*, FVector2D>& Out_Images, int32 PageIndex, bool bUseSrgb = true);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get All Texts", Keywords = "pdfium, pdf, read, document, get, all, texts, string"), Category = "Frozen Forest|FF_PDFium|Read")
	virtual bool PDFium_Get_All_Texts(TArray<FString>& Out_Texts);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Texts", ToolTip = "This will get all text objects of target page with their locations. Text object doesn't mean line. Some editors can create different text objects for some stylized texts.\nFunction will get all chars including spaces and line breaks. If you don't need them, you can filter them with your blueprint mechanics.\nIf you use this node with \"PDFium_Get_Images\", you can create a basic interactive PDF viewer.", Keywords = "pdfium, pdf, document, read, get, images"), Category = "Frozen Forest|FF_PDFium|Read")
	virtual bool PDFium_Get_Texts(TArray<FPdfTextObject>& Out_Texts, int32 PageIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Links", Keywords = "pdfium, pdf, document, read, get, text, string, link, web, url"), Category = "Frozen Forest|FF_PDFium|Read")
	virtual bool PDFium_Get_Links(TArray<FString>& Out_Links, int32 PageIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Select Text", Keywords = "pdfium, pdf, document, read, get, text, string, select, area"), Category = "Frozen Forest|FF_PDFium|Read")
	virtual bool PDFium_Select_Text(FString& Out_Text, FVector2D Start, FVector2D End, int32 PageIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Pages Count and Sizes", ToolTip = "", Keywords = "pdfium, pdf, document, get, pages, count, size"), Category = "Frozen Forest|FF_PDFium|Read")
	virtual bool PDFium_Pages_Counts_Sizes(TArray<FVector2D>& PagesCount);

	/*
	* @param Pages It will add pages to the document as length of the array. Each page's size will be equal to respective indexed Vector 2D
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Add Pages", Keywords = "pdfium, pdf, create, doc, document, pdf, add, pages"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual bool PDFium_Pages_Add(TArray<FVector2D> Pages);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Delete Pages", Keywords = "pdfium, pdf, doc, document, pdf, delete, pages"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual bool PDFium_Pages_Delete(int32 PageIndex = 0);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Load Standart Font", ToolTip = "These are 14 standart font which described on PDF Specs 1.7 page 416.\nPDFium (or PDF Specs 1.7) uses same library for both Arial and Helvetica. So, if there are problems with their visualizations or language supports, it is library's owner's responsibility.\n\nZapfDingbats and Symbol really convert your texts into some kind of drawings. It's not an error or language problem, it is what it is.\nActually kawaii btw.", Keywords = "pdfium, pdf, document, load, font, standart"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual bool PDFium_Font_Load_Standart(UPDFiumFont*& Out_Font, EStandartFonts Font_Name = EStandartFonts::Helvetica);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Load External Font", ToolTip = "You have to use \"desktop fonts\" not \"web fonts\". For example, some Google Fonts (such as Google Variant of Roboto) don't work. You can download compatible fonts from here. https://all-free-download.com/font/ , https://www.fontspace.com/category/truetype , https://www.dafont.com/", Keywords = "pdfium, pdf, document, load, font, external, custom"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual bool PDFium_Font_Load_External(UPDFiumFont*& Out_Font, FString Font_Path, EExternalFonts In_Font_Type = EExternalFonts::TrueType, bool bIsCid = false);

	/**
	* @param Position Position X value starts from left, Y value starts from bottom.
	* @param bUseCharcodes It switch between "FPDFText_SetCharcodes()" and "FPDFText_SetText()". When you enable CharCodes, you don't need to write your texts as ASCII decimals. System automatically converts it. Also it supports much more characters.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Add Texts", Keywords = "pdfium, pdf, create, doc, document, add, texts"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual void PDFium_Add_Texts(FDelegatePdfium DelegateAddObject, UPARAM(ref)UPDFiumFont*& In_Font, FString In_Texts, FColor Text_Color = FColor::Black, FVector2D Position = FVector2D(0.0f, 0.0f), FVector2D Size = FVector2D(1.0f, 1.0f), FVector2D Rotation = FVector2D(0.0f, 0.0f), FVector2D Border = FVector2D(10.0f, 10.0f), int32 FontSize = 12, int32 PageIndex = 0, bool bUseCharcodes = true);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Draw Rectangle", ToolTip = "", Keywords = "pdfium, pdf, create, doc, document, add, draw, rectangle"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual bool PDFium_Draw_Rectangle(FVector2D Position, FVector2D Anchor, FVector2D Size, FVector2D Rotation, FColor Color = FColor::Blue, int32 PageIndex = 0);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Add Image", ToolTip = "", Keywords = "pdfium, pdf, create, doc, document, add, draw, image"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual bool PDFium_Add_Image(FString& Out_Code, TArray<uint8> In_Bytes, FVector2D In_Size, FVector2D Position, FVector2D Rotation, int32 PageIndex = 0);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Save File", ToolTip = "", Keywords = "pdfium, pdf, doc, document, save, file"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual void PDFium_Save_File(FDelegatePdfium DelegateSave, FString Export_Path, EPDFiumSaveTypes In_SaveType = EPDFiumSaveTypes::Incremental, EPDFiumSaveVersion In_Version = EPDFiumSaveVersion::PDF_17);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Save Bytes x64", ToolTip = "", Keywords = "pdfium, pdf, doc, document, save, bytes"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual bool PDFium_Save_Bytes_x64(UBytesObject_64*& Out_Bytes, EPDFiumSaveTypes In_SaveType = EPDFiumSaveTypes::Incremental, EPDFiumSaveVersion In_Version = EPDFiumSaveVersion::PDF_17);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Save Bytes x86", ToolTip = "", Keywords = "pdfium, pdf, doc, document, save, bytes"), Category = "Frozen Forest|FF_PDFium|Write")
	virtual bool PDFium_Save_Bytes_x86(TArray<uint8>& Out_Bytes, EPDFiumSaveTypes In_SaveType = EPDFiumSaveTypes::Incremental, EPDFiumSaveVersion In_Version = EPDFiumSaveVersion::PDF_17);

};