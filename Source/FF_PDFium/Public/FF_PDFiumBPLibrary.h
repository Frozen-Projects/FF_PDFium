// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "FF_PDFium_Includes.h"
#include "FF_PDFiumBPLibrary.generated.h"

USTRUCT(BlueprintType)
struct FF_PDFIUM_API FPdfTextObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	FString Text_String = "";

	UPROPERTY(BlueprintReadOnly)
	FVector2D Text_Position  = FVector2D(0, 0);

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

UCLASS(BlueprintType)
class FF_PDFIUM_API UPDFiumDoc : public UObject
{
	GENERATED_BODY()

public:
	
	FPDF_DOCUMENT Document = NULL;

	// ~UPDFiumDoc start.
	void BeginDestroy();
	// ~UPDFiumDoc finish.
};

UCLASS(BlueprintType)
class FF_PDFIUM_API UPDFiumFont : public UObject
{
	GENERATED_BODY()

public:

	FPDF_FONT Font = NULL;

	// ~UPDFiumFont start.
	void BeginDestroy();
	// ~UPDFiumFont finish.
};

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegatePdfium, bool, bIsSuccessfull, FString, OutCode);

UCLASS()
class UFF_PDFiumBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Library Init", Keywords = "pdfium, pdf, document, library, lib, open, init"), Category = "PDFium|System")
	static FF_PDFIUM_API bool PDFium_LibInit(FString& Out_Code);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Library Close", Keywords = "pdfium, pdf, document, library, lib, close"), Category = "PDFium|System")
	static FF_PDFIUM_API bool PDFium_LibClose(FString& Out_Code);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "PDFium - Library State", ToolTip = "", Keywords = "pdfium, pdf, document, library, state, get, is, initialized"), Category = "PDFium|System")
	static FF_PDFIUM_API bool PDFium_LibState();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from File", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, file"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Doc_Open_File(UPDFiumDoc*& Out_PDF, FString& ErrorCode, FString In_Path, FString In_PDF_Password);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Open Document from Memory", ToolTip = "", Keywords = "pdfium, pdf, document, read, open, import, memory"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Doc_Open_Memory(UPDFiumDoc*& Out_PDF, FString& ErrorCode, UPARAM(ref)UBytesObject_64*& In_Bytes_Object, FString In_PDF_Password);

	/**
	* @param In_Sampling Default (also minimum) value is "1" but "2" gives best result for A4 sized PDF on 17 inch notebook screen. Bigger values is good for 3D widget like huge UIs.
	* @param Out_Pages Created textures don't have BITMAP headers. So, they don't work with other technologies. If you need real bitmaps, export UTexture2Ds with "ExtendedVars -> ExportT2dAsBitmap"
	* @param bUseMatrix Don't use this boolean. Currently it is exprimental and doesn't work.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Pages", Keywords = "pdfium, pdf, document, read, get, pages"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Get_Pages(FJsonObjectWrapper& Out_Code, TMap<UTexture2D*, FVector2D>& Out_Pages, UPARAM(ref)UPDFiumDoc*& In_PDF, int32 In_Sampling = 1, FColor BG_Color = FColor::White, bool bUseSrgb = true, bool bUseMatrix = false, bool bUseAlpha = true, bool bRenderAnnots = true);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Images", Keywords = "pdfium, pdf, document, read, get, images"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Get_Images(TMap<UTexture2D*, FVector2D>& Out_Images, UPARAM(ref)UPDFiumDoc*& In_PDF, int32 PageIndex, bool bUseSrgb = true);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get All Texts", Keywords = "pdfium, pdf, read, document, get, all, texts, string"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Get_All_Texts(TArray<FString>& Out_Texts, UPARAM(ref)UPDFiumDoc*& In_PDF);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Texts", ToolTip = "This will get all text objects of target page with their locations. Text object doesn't mean line. Some editors can create different text objects for some stylized texts.\nFunction will get all chars including spaces and line breaks. If you don't need them, you can filter them with your blueprint mechanics.\nIf you use this node with \"PDFium_Get_Images\", you can create a basic interactive PDF viewer.", Keywords = "pdfium, pdf, document, read, get, images"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Get_Texts(TArray<FPdfTextObject>& Out_Texts, UPARAM(ref)UPDFiumDoc*& In_PDF, int32 PageIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Links", Keywords = "pdfium, pdf, document, read, get, text, string, link, web, url"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Get_Links(TArray<FString>& Out_Links, UPARAM(ref)UPDFiumDoc*& In_PDF, int32 PageIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Select Text", Keywords = "pdfium, pdf, document, read, get, text, string, select, area"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Select_Text(FString& Out_Text, UPARAM(ref)UPDFiumDoc*& In_PDF, FVector2D Start, FVector2D End, int32 PageIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Get Pages Count and Sizes", ToolTip = "", Keywords = "pdfium, pdf, document, get, pages, count, size"), Category = "PDFium|Read")
	static FF_PDFIUM_API bool PDFium_Pages_Counts_Sizes(TArray<FVector2D>& PagesCount, UPARAM(ref)UPDFiumDoc*& In_PDF);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Create Document", ToolTip = "", Keywords = "pdfium, pdf, create, doc, document, pdf"), Category = "PDFium|Write")
	static FF_PDFIUM_API bool PDFium_Create_Doc(UPDFiumDoc*& Out_PDF);

	/*
	* @param Pages It will add pages to the document as length of the array. Each page's size will be equal to respective indexed Vector 2D
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Add Pages", Keywords = "pdfium, pdf, create, doc, document, pdf, add, pages"), Category = "PDFium|Write")
	static FF_PDFIUM_API bool PDFium_Pages_Add(UPARAM(ref)UPDFiumDoc*& In_PDF, TArray<FVector2D> Pages);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Delete Pages", Keywords = "pdfium, pdf, doc, document, pdf, delete, pages"), Category = "PDFium|Write")
	static FF_PDFIUM_API bool PDFium_Pages_Delete(UPARAM(ref)UPDFiumDoc*& In_PDF, int32 PageIndex = 0);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Load Standart Font", ToolTip = "These are 14 standart font which described on PDF Specs 1.7 page 416.\nPDFium (or PDF Specs 1.7) uses same library for both Arial and Helvetica. So, if there are problems with their visualizations or language supports, it is library's owner's responsibility.\n\nZapfDingbats and Symbol really convert your texts into some kind of drawings. It's not an error or language problem, it is what it is.\nActually kawaii btw.", Keywords = "pdfium, pdf, document, load, font, standart"), Category = "PDFium|Write")
	static FF_PDFIUM_API bool PDFium_Font_Load_Standart(UPDFiumFont*& Out_Font, UPARAM(ref)UPDFiumDoc*& In_PDF, EStandartFonts Font_Name = EStandartFonts::Helvetica);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Load External Font", ToolTip = "You have to use \"desktop fonts\" not \"web fonts\". For example, some Google Fonts (such as Google Variant of Roboto) don't work. You can download compatible fonts from here. https://all-free-download.com/font/ , https://www.fontspace.com/category/truetype , https://www.dafont.com/", Keywords = "pdfium, pdf, document, load, font, external, custom"), Category = "PDFium|Write")
	static FF_PDFIUM_API bool PDFium_Font_Load_External(UPDFiumFont*& Out_Font, UPARAM(ref)UPDFiumDoc*& In_PDF, FString Font_Path, EExternalFonts In_Font_Type = EExternalFonts::TrueType, bool bIsCid = false);

	/**
	* @param Position X value starts from left, Y value starts from bottom.
	* @param bUseCharcodes It switch between "FPDFText_SetCharcodes()" and "FPDFText_SetText()". When you enable CharCodes, you don't need to write your texts as ASCII decimals. System automatically converts it. Also it supports much more characters.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Add Texts", Keywords = "pdfium, pdf, create, doc, document, add, texts"), Category = "PDF_Reader|PDFium|Write")
	static FF_PDFIUM_API void PDFium_Add_Texts(FDelegatePdfium DelegateAddObject, UPARAM(ref)UPDFiumDoc*& In_PDF, UPARAM(ref)UPDFiumFont*& In_Font, FString In_Texts, FColor Text_Color = FColor::Black, FVector2D Position = FVector2D(0.0f, 0.0f), FVector2D Size = FVector2D(1.0f, 1.0f), FVector2D Rotation = FVector2D(0.0f, 0.0f), FVector2D Border = FVector2D(10.0f, 10.0f), int32 FontSize = 12, int32 PageIndex = 0, bool bUseCharcodes = true);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Draw Rectangle", ToolTip = "", Keywords = "pdfium, pdf, create, doc, document, add, draw, rectangle"), Category = "PDF_Reader|PDFium|Write")
	static FF_PDFIUM_API bool PDFium_Draw_Rectangle(UPARAM(ref)UPDFiumDoc*& In_PDF, FVector2D Position, FVector2D Anchor, FVector2D Size, FVector2D Rotation, FColor Color = FColor::Blue, int32 PageIndex = 0);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PDFium - Add Image", ToolTip = "", Keywords = "pdfium, pdf, create, doc, document, add, draw, image"), Category = "PDF_Reader|PDFium|Write")
	static FF_PDFIUM_API bool PDFium_Add_Image(UPARAM(ref)UPDFiumDoc*& In_PDF, FString& Out_Code, TArray<uint8> In_Bytes, FVector2D In_Size, FVector2D Position, FVector2D Rotation, int32 PageIndex = 0);

};