// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF_PDFiumBPLibrary.h"
#include "FF_PDFium.h"

// UE Mechanics Includes.
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"

#include "ImageCore.h"
#include "ImageUtils.h"  
#include "Engine/TextureRenderTarget2D.h"

THIRD_PARTY_INCLUDES_START
// PDFium Includes.
#include "fpdf_text.h"
#include "fpdf_edit.h"
#include "fpdf_formfill.h"
THIRD_PARTY_INCLUDES_END

// Global library initialization checker.
bool Global_IsPDFiumInitialized = false;

UFF_PDFiumBPLibrary::UFF_PDFiumBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}

bool UFF_PDFiumBPLibrary::PDFium_LibInit(FString& Out_Code)
{
	if (Global_IsPDFiumInitialized == true)
	{
		Out_Code = "Library already initialized";
		return false;
	}
	
	FPDF_LIBRARY_CONFIG config;
	FMemory::Memset(&config, 0, sizeof(config));

	config.version = 2;
	config.m_pUserFontPaths = NULL;
	config.m_pIsolate = NULL;
	config.m_v8EmbedderSlot = 0;
	FPDF_InitLibraryWithConfig(&config);

	Global_IsPDFiumInitialized = true;

	Out_Code = "Library successfully initialized.";
	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_LibClose(FString& Out_Code)
{
	if (Global_IsPDFiumInitialized == false)
	{
		Out_Code = "Library already closed.";
		return false;
	}

	Global_IsPDFiumInitialized = false;
	FPDF_DestroyLibrary();

	Out_Code = "Library successfully closed.";
	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_LibState()
{
	return Global_IsPDFiumInitialized;
}

bool UFF_PDFiumBPLibrary::PDFium_File_Close(UPARAM(ref)UPDFiumDoc*& In_PDF)
{
	if (Global_IsPDFiumInitialized == false)
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

	FPDF_CloseDocument(In_PDF->Document);
	In_PDF = nullptr;

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_File_Open(UPDFiumDoc*& Out_PDF, FString& ErrorCode, UPARAM(ref)UBytesObject_64*& In_Bytes_Object, FString In_PDF_Password)
{
	if (IsValid(In_Bytes_Object) == false)
	{
		ErrorCode = "Bytes object is invalid.";
		return false;
	}

	if (In_Bytes_Object->ByteArray.Num() == 0)
	{
		ErrorCode = "There is no bytes.";
		return false;
	}

	void* PDF_Data = In_Bytes_Object->ByteArray.GetData();
	size_t PDF_Data_Size = In_Bytes_Object->ByteArray.Num();

	UPDFiumDoc* PDF_Object = NewObject<UPDFiumDoc>();

	PDF_Object->Document = FPDF_LoadMemDocument64(PDF_Data, PDF_Data_Size, TCHAR_TO_UTF8(*In_PDF_Password));
	FPDF_LoadXFA(PDF_Object->Document);

	if (!PDF_Object->Document)
	{
		ErrorCode = "PDF is invalid.";
		return false;
	}
	
	Out_PDF = PDF_Object;

	ErrorCode = "Success.";
	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Get_Pages(TMap<UTexture2D*, FVector2D>& Out_Pages, UPARAM(ref)UPDFiumDoc*& In_PDF, int32 In_Sampling, FColor BG_Color, bool bUseSrgb, bool bUseMatrix, bool bUseAlpha, bool bRenderAnnots)
{	
	if (Global_IsPDFiumInitialized == false)
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

	double Sampling = 1;
	if (In_Sampling < 1)
	{
		Sampling = 1;
	}

	else
	{
		Sampling = In_Sampling;
	}

	for (int32 Index_Pages = 0; Index_Pages < FPDF_GetPageCount(In_PDF->Document); Index_Pages++)
	{		
		FPDF_PAGE PDF_Page = FPDF_LoadPage(In_PDF->Document, Index_Pages);
		
		const double PDF_Page_Width = FPDF_GetPageWidth(PDF_Page);
		const double PDF_Page_Height = FPDF_GetPageHeight(PDF_Page);
		const int32 Render_Width = PDF_Page_Width * Sampling;
		const int32 Render_Height = PDF_Page_Height * Sampling;
		const size_t Lenght = static_cast<size_t>(Render_Width * Render_Height * 4);

		FPDF_BITMAP PDF_Bitmap = FPDFBitmap_Create(Render_Width, Render_Height, (bUseAlpha ? FPDFBitmap_BGRA : FPDFBitmap_BGRx));
		FPDFBitmap_FillRect(PDF_Bitmap, 0, 0, Render_Width, Render_Height, BG_Color.ToPackedARGB());
		
		FPDF_FORMHANDLE Form_Handle;
		FMemory::Memset(&Form_Handle, 0, sizeof(Form_Handle));
		FPDF_FFLDraw(Form_Handle, PDF_Bitmap, PDF_Page, 0, 0, Render_Width, Render_Height, 0, 0);
		
		if (bUseMatrix)
		{
			FS_MATRIX Image_Matrix
			{ 
				static_cast<float>(Sampling),
				0.f,
				0.f,
				static_cast<float>(Sampling),
				0.f,
				0.f
			};
			
			FS_RECTF Rect
			{ 
				0.f,
				0.f,
				static_cast<float>(Render_Width), 
				static_cast<float>(Render_Height) 
			};
			
			FPDF_RenderPageBitmapWithMatrix(PDF_Bitmap, PDF_Page, &Image_Matrix, &Rect, (bRenderAnnots ? FPDF_ANNOT : FPDF_LCD_TEXT));
		}

		else
		{
			FPDF_RenderPageBitmap(PDF_Bitmap, PDF_Page, 0, 0, Render_Width, Render_Height, 0, (bRenderAnnots ? FPDF_ANNOT : FPDF_LCD_TEXT));
		}

		void* Buffer = FPDFBitmap_GetBuffer(PDF_Bitmap);
		
		UTexture2D* PDF_Texture = UTexture2D::CreateTransient(Render_Width, Render_Height, PF_B8G8R8A8);
		PDF_Texture->SRGB = bUseSrgb;
		FTexture2DMipMap& PDF_Texture_Mip = PDF_Texture->GetPlatformData()->Mips[0];
		void* PDF_Texture_Data = PDF_Texture_Mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(PDF_Texture_Data, Buffer, Lenght);

		PDF_Texture_Mip.BulkData.Unlock();
		PDF_Texture->UpdateResource();

		Out_Pages.Add(PDF_Texture, FVector2D(PDF_Page_Width, PDF_Page_Height));
		FPDFBitmap_Destroy(PDF_Bitmap);
		FPDF_ClosePage(PDF_Page);
	}

	if (Out_Pages.Num() > 0)
	{
		return true;
	}

	else
	{
		return false;
	}
}

bool UFF_PDFiumBPLibrary::PDFium_Get_Images(TMap<UTexture2D*, FVector2D>& Out_Images, UPARAM(ref)UPDFiumDoc*& In_PDF, int32 PageIndex, bool bUseSrgb)
{
	if (Global_IsPDFiumInitialized == false)
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

	FPDF_PAGE PDF_Page = FPDF_LoadPage(In_PDF->Document, PageIndex);

	int ObjectCount = FPDFPage_CountObjects(PDF_Page);
	for (int32 Index_Objects = 0; Index_Objects < ObjectCount; Index_Objects++)
	{
		FPDF_PAGEOBJECT Each_Object = FPDFPage_GetObject(PDF_Page, Index_Objects);
		if (FPDFPageObj_GetType(Each_Object) == FPDF_PAGEOBJ_IMAGE)
		{
			float Left = 0;
			float Bottom = 0;
			float Right = 0;
			float Top = 0;
			FPDFPageObj_GetBounds(Each_Object, &Left, &Bottom, &Right, &Top);
			FVector2D TextureSize = FVector2D((Right - Left), (Top - Bottom));
			
			FPDF_BITMAP Each_Bitmap = FPDFImageObj_GetRenderedBitmap(In_PDF->Document, PDF_Page, Each_Object);
			void* Each_Buffer = FPDFBitmap_GetBuffer(Each_Bitmap);
			
			if (!Each_Buffer)
			{
				return false;
			}

			size_t Each_Buffer_Lenght = static_cast<SIZE_T>(TextureSize.X * TextureSize.Y * 4);

			UTexture2D* Each_Image = UTexture2D::CreateTransient(TextureSize.X, TextureSize.Y, PF_B8G8R8A8);
			Each_Image->SRGB = bUseSrgb;
			FTexture2DMipMap& Each_Image_Mip = Each_Image->GetPlatformData()->Mips[0];
			void* Each_Image_Data = Each_Image_Mip.BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(Each_Image_Data, Each_Buffer, Each_Buffer_Lenght);

			Each_Image_Mip.BulkData.Unlock();
			Each_Image->UpdateResource();
			FPDFBitmap_Destroy(Each_Bitmap);

			Out_Images.Add(Each_Image, FVector2D(Left, Top));
		}
	}

	FPDF_ClosePage(PDF_Page);

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Get_All_Texts(TArray<FString>& Out_Texts, UPARAM(ref)UPDFiumDoc*& In_PDF)
{
	if (Global_IsPDFiumInitialized == false)
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

	for (int32 PageIndex = 0; PageIndex < FPDF_GetPageCount(In_PDF->Document); PageIndex++)
	{
		FPDF_PAGE PDF_Page = FPDF_LoadPage(In_PDF->Document, PageIndex);
		FPDF_TEXTPAGE PDF_TextPage = FPDFText_LoadPage(PDF_Page);

		int Count_Chars = FPDFText_CountChars(PDF_TextPage);
		int BufferLenght = (Count_Chars + 1) * 2;
		unsigned short* CharBuffer = (unsigned short*)malloc(static_cast<size_t>(BufferLenght));
		FPDFText_GetText(PDF_TextPage, 0, Count_Chars, CharBuffer);

		FString PageText;
		PageText.AppendChars((WIDECHAR*)CharBuffer, Count_Chars);
		Out_Texts.Add(PageText);

		FPDFText_ClosePage(PDF_TextPage);
		FPDF_ClosePage(PDF_Page);
		free(CharBuffer);
	}

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Get_Texts(TArray<FPdfTextObject>& Out_Texts, UPARAM(ref)UPDFiumDoc*& In_PDF, int32 PageIndex)
{
	if (Global_IsPDFiumInitialized == false)
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

	FPDF_PAGE PDF_Page = FPDF_LoadPage(In_PDF->Document, PageIndex);
	FPDF_TEXTPAGE Text_Page = FPDFText_LoadPage(PDF_Page);

	int ObjectCount = FPDFPage_CountObjects(PDF_Page);
	for (int32 Index_Objects = 0; Index_Objects < ObjectCount; Index_Objects++)
	{
		FPDF_PAGEOBJECT Each_Object = FPDFPage_GetObject(PDF_Page, Index_Objects);
		if (FPDFPageObj_GetType(Each_Object) == FPDF_PAGEOBJ_TEXT)
		{
			// Initialize text object structure.
			FPdfTextObject Text_Object;
			
			// Text String
			unsigned long Buffer_Lenght = FPDFTextObj_GetText(Each_Object, Text_Page, NULL, 0);
			FPDF_WCHAR* Buffer = (unsigned short*)malloc(Buffer_Lenght);
			FPDFTextObj_GetText(Each_Object, Text_Page, Buffer, Buffer_Lenght);

			FString PageText;
			PageText.AppendChars((WIDECHAR*)Buffer, ((Buffer_Lenght / 2) - 1));
			Text_Object.Text_String = PageText;

			// Text Position
			float Left = 0;
			float Bottom = 0;
			float Right = 0;
			float Top = 0;
			FPDFPageObj_GetBounds(Each_Object, &Left, &Bottom, &Right, &Top);
			Text_Object.Text_Position = FVector2D(Left, Top);
			
			// Text Size: Some PDF files use "1" as font size and change its size with transform matrix. So, we need to get that value, too.
			FS_MATRIX Object_Matrix;
			FPDFPageObj_GetMatrix(Each_Object, &Object_Matrix);
			Text_Object.Text_Size = FVector2D(Object_Matrix.a, Object_Matrix.d);

			// Text Color
			unsigned int Text_Color_Red = 0;
			unsigned int Text_Color_Green = 0;
			unsigned int Text_Color_Blue = 0;
			unsigned int Text_Color_Alpha = 0;
			FPDFPageObj_GetFillColor(Each_Object, &Text_Color_Red, &Text_Color_Green, &Text_Color_Blue, &Text_Color_Alpha);
			Text_Object.Text_Color = FColor(Text_Color_Red, Text_Color_Green, Text_Color_Blue, Text_Color_Alpha);

			// Font Name
			FPDF_FONT Font_Object = FPDFTextObj_GetFont(Each_Object);
			unsigned long Font_Name_Lenght = FPDFFont_GetFontName(Font_Object, NULL, 0);
			char* Font_Name = (char*)malloc(static_cast<size_t>(Font_Name_Lenght));
			FPDFFont_GetFontName(Font_Object, Font_Name, Font_Name_Lenght);
			Text_Object.Font_Name = Font_Name;

			// Font Size
			FPDFTextObj_GetFontSize(Each_Object, &Text_Object.Font_Size);

			// Font Bytes
			size_t Bufflen;
			memset(&Bufflen, 0, sizeof(size_t));
			FPDFFont_GetFontData(Font_Object, NULL, Bufflen, &Bufflen);
			
			TArray<uint8> Font_Bytes;
			Font_Bytes.SetNum(Bufflen);
			FPDFFont_GetFontData(Font_Object, Font_Bytes.GetData(), Bufflen, &Bufflen);
			Text_Object.Runtime_Font = UExtendedVarsBPLibrary::RuntimeFont_Load(Font_Bytes);
			
			// Font Flag, Font Italic and Width
			Text_Object.Font_Flags = FPDFFont_GetFlags(Font_Object);
			Text_Object.Font_Weight = FPDFFont_GetWeight(Font_Object);
			FPDFFont_GetItalicAngle(Font_Object, &Text_Object.Font_Italic);

			Out_Texts.Add(Text_Object);
		}
	}

	FPDFText_ClosePage(Text_Page);
	FPDF_ClosePage(PDF_Page);

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Get_Links(TArray<FString>& Out_Links, UPARAM(ref)UPDFiumDoc*& In_PDF, int32 PageIndex)
{
	if (Global_IsPDFiumInitialized == false)
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
	
	FPDF_PAGE PDF_Page = FPDF_LoadPage(In_PDF->Document, PageIndex);
	FPDF_TEXTPAGE PDF_TextPage = FPDFText_LoadPage(PDF_Page);

	FPDF_PAGELINK PDF_Links = FPDFLink_LoadWebLinks(PDF_TextPage);
	int32 Links_Count = FPDFLink_CountWebLinks(PDF_Links);

	if (Links_Count == 0)
	{
		FPDFLink_CloseWebLinks(PDF_Links);
		FPDFText_ClosePage(PDF_TextPage);
		
		return false;
	}

	for (int32 Index_Link = 0; Index_Link < Links_Count; Index_Link++)
	{
		int Count_Links = FPDFLink_GetURL(PDF_Links, Index_Link, NULL, 0);
		int BufferLenght = Count_Links * 2;
		unsigned short* CharBuffer = (unsigned short*)malloc(static_cast<size_t>(BufferLenght));
		FPDFLink_GetURL(PDF_Links, Index_Link, CharBuffer, BufferLenght);

		FString LinkText;
		LinkText.AppendChars((WIDECHAR*)CharBuffer, Count_Links);

		Out_Links.Add(LinkText);
		free(CharBuffer);
	}

	FPDFLink_CloseWebLinks(PDF_Links);
	FPDFText_ClosePage(PDF_TextPage);
	FPDF_ClosePage(PDF_Page);

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Select_Text(FString& Out_Text, UPARAM(ref)UPDFiumDoc*& In_PDF, FVector2D Start, FVector2D End, int32 PageIndex)
{
	if (Global_IsPDFiumInitialized == false)
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

	FPDF_PAGE PDF_Page = FPDF_LoadPage(In_PDF->Document, PageIndex);
	FPDF_TEXTPAGE PDF_TextPage = FPDFText_LoadPage(PDF_Page);

	int Count_Texts = FPDFText_GetBoundedText(PDF_TextPage, Start.X, Start.Y, End.X, End.Y, NULL, 0);
	int BufferLenght = (Count_Texts + 1) * 2;
	unsigned short* CharBuffer = (unsigned short*)malloc(static_cast<size_t>(BufferLenght));
	FPDFText_GetBoundedText(PDF_TextPage, Start.X, Start.Y, End.X, End.Y, CharBuffer, BufferLenght);
	
	FString SelectedText;
	SelectedText.AppendChars((WIDECHAR*)CharBuffer, Count_Texts);

	Out_Text = SelectedText;

	FPDFText_ClosePage(PDF_TextPage);
	FPDF_ClosePage(PDF_Page);
	free(CharBuffer);

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Pages_Counts_Sizes(TArray<FVector2D>& Out_Infos, UPARAM(ref)UPDFiumDoc*& In_PDF)
{
	if (Global_IsPDFiumInitialized == false)
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

	int32 PagesCount = FPDF_GetPageCount(In_PDF->Document);
	for (int32 Index_Pages = 0; Index_Pages < PagesCount; Index_Pages++)
	{
		FPDF_PAGE PDF_Page = FPDF_LoadPage(In_PDF->Document, Index_Pages);
		double PDF_Page_Width = FPDF_GetPageWidth(PDF_Page);
		double PDF_Page_Height = FPDF_GetPageHeight(PDF_Page);
		Out_Infos.Add(FVector2D(PDF_Page_Width, PDF_Page_Height));

		FPDF_ClosePage(PDF_Page);
	}

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Create_Doc(UPDFiumDoc*& Out_PDF)
{
	if (Global_IsPDFiumInitialized == false)
	{
		return false;
	}

	UPDFiumDoc* PDF_Object = NewObject<UPDFiumDoc>();
	PDF_Object->Document = FPDF_CreateNewDocument();

	Out_PDF = PDF_Object;

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Pages_Add(UPARAM(ref)UPDFiumDoc*& In_PDF, TArray<FVector2D> Pages)
{
	if (Global_IsPDFiumInitialized == false)
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

	int32 PageCount = FPDF_GetPageCount(In_PDF->Document);

	for (int32 Index_Pages = 0; Index_Pages < Pages.Num(); Index_Pages++)
	{
		FPDF_PAGE PDF_Page = FPDFPage_New(In_PDF->Document, PageCount + Index_Pages, Pages[Index_Pages].X, Pages[Index_Pages].Y);
		FPDFPage_GenerateContent(PDF_Page);
		
		FPDF_ClosePage(PDF_Page);
	}

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Pages_Delete(UPARAM(ref)UPDFiumDoc*& In_PDF, int32 PageIndex)
{
	if (Global_IsPDFiumInitialized == false)
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
	
	FPDF_PAGE TargetPage = FPDF_LoadPage(In_PDF->Document, PageIndex);
	if (!TargetPage)
	{
		return false;
	}
	
	FPDF_ClosePage(TargetPage);
	FPDFPage_Delete(In_PDF->Document, PageIndex);

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Font_Load_Standart(UPDFiumFont*& Out_Font, UPARAM(ref)UPDFiumDoc*& In_PDF, EStandartFonts Font_Name)
{
	if (Global_IsPDFiumInitialized == false)
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

	FString Name_String;
	switch (Font_Name)
	{
	case EStandartFonts::Helvetica:
		Name_String = "Helvetica";
		break;
	case EStandartFonts::Helvetica_Italic:
		Name_String = "Helvetica-Italic";
		break;
	case EStandartFonts::Helvetica_Bold:
		Name_String = "Helvetica-Bold";
		break;
	case EStandartFonts::Helvetica_BoldItalic:
		Name_String = "Helvetica-BoldItalic";
		break;
	case EStandartFonts::Times_Roman:
		Name_String = "Times-Roman";
		break;
	case EStandartFonts::Times_Bold:
		Name_String = "Times-Bold";
		break;
	case EStandartFonts::Times_BoldItalic:
		Name_String = "Times-BoldItalic";
		break;
	case EStandartFonts::Times_Italic:
		Name_String = "Times-Italic";
		break;
	case EStandartFonts::Courier:
		Name_String = "Courier";
		break;
	case EStandartFonts::Courier_Bold:
		Name_String = "Courier-Bold";
		break;
	case EStandartFonts::Courier_Oblique:
		Name_String = "Courier-Oblique";
		break;
	case EStandartFonts::Courier_BoldOblique:
		Name_String = "Courier-BoldOblique";
		break;
	case EStandartFonts::Symbol:
		Name_String = "Symbol";
		break;
	case EStandartFonts::ZapfDingbats:
		Name_String = "ZapfDingbats";
		break;
	default:
		Name_String = "Helvetica";
		break;
	}

	FPDF_FONT Font = FPDFText_LoadStandardFont(In_PDF->Document, TCHAR_TO_UTF8(*Name_String));

	if (!Font)
	{
		return false;
	}
	
	UPDFiumFont* FontObject = NewObject<UPDFiumFont>();
	FontObject->Font = Font;

	Out_Font = FontObject;

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Font_Load_External(UPDFiumFont*& Out_Font, UPARAM(ref)UPDFiumDoc*& In_PDF, FString Font_Path, EExternalFonts In_Font_Type, bool bIsCid)
{
	if (Global_IsPDFiumInitialized == false)
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

	if (Font_Path.IsEmpty() == true)
	{
		return false;
	}

	FPaths::MakeStandardFilename(Font_Path);
	if (FPaths::FileExists(Font_Path) == false)
	{
		return false;
	}

	TArray<uint8> ByteArray;
	FFileHelper::LoadFileToArray(ByteArray, *Font_Path);

	int FontType = 0;
	switch (In_Font_Type)
	{
	case EExternalFonts::TrueType:
		FontType = FPDF_FONT_TRUETYPE;
		break;
	case EExternalFonts::TYPE1:
		FontType = FPDF_FONT_TYPE1;
		break;
	default:
		FontType = FPDF_FONT_TRUETYPE;
		break;
	}

	FPDF_FONT External_Font = FPDFText_LoadFont(In_PDF->Document, reinterpret_cast<const uint8_t*>(ByteArray.GetData()), ByteArray.GetAllocatedSize(), FontType, bIsCid);
	UPDFiumFont* FontObject = NewObject<UPDFiumFont>();
	FontObject->Font = External_Font;

	Out_Font = FontObject;

	ByteArray.Empty();

	return true;
}

bool UFF_PDFiumBPLibrary::PDFium_Close_Font(UPARAM(ref)UPDFiumFont*& In_Font)
{
	if (Global_IsPDFiumInitialized == false)
	{
		return false;
	}

	if (IsValid(In_Font) == false)
	{
		return false;
	}

	FPDFFont_Close(In_Font->Font);

	return true;
}

void UFF_PDFiumBPLibrary::PDFium_Add_Texts(FDelegatePdfium DelegateAddObject, UPARAM(ref)UPDFiumDoc*& In_PDF, UPARAM(ref)UPDFiumFont*& In_Font, FString In_Texts, FColor Text_Color, FVector2D Position, FVector2D Size, FVector2D Rotation, FVector2D Border, int32 FontSize, int32 PageIndex, bool bUseCharcodes)
{
	if (Global_IsPDFiumInitialized == false)
	{
		DelegateAddObject.Execute(false, "PDFium Library haven't been initialized.");
	}

	if (IsValid(In_PDF) == false)
	{
		DelegateAddObject.Execute(false, "PDF object is not valid.");
	}

	if (!In_PDF->Document)
	{
		DelegateAddObject.Execute(false, "PDF document is not valid.");
	}

	if (In_Texts.IsEmpty() == true)
	{
		DelegateAddObject.Execute(false, "Text is empty.");
	}

	if (IsValid(In_Font) == false)
	{
		DelegateAddObject.Execute(false, "Font object is invalid.");
	}

	if (!In_Font->Font)
	{
		DelegateAddObject.Execute(false, "PDFium font is invalid.");
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [DelegateAddObject, &In_PDF, In_Font, In_Texts, Text_Color, Position, Size, Rotation, Border, FontSize, PageIndex, bUseCharcodes]()
		{
			FPDF_PAGE First_Page = FPDF_LoadPage(In_PDF->Document, PageIndex);
			TArray<FPDF_PAGE> Array_Pages;
			Array_Pages.Add(First_Page);

			int32 Acceptable_Horizontal = 2 * ((FPDF_GetPageWidth(First_Page) - Position.X - Border.X) / FontSize);
			int32 Acceptable_Vertical = (Position.Y - Border.Y) / FontSize;

			// Generate paragraphs.
			TArray<FString> Array_Paragraphs;
			if (In_Texts.Contains(LINE_TERMINATOR_ANSI) == true)
			{
				Array_Paragraphs = UKismetStringLibrary::ParseIntoArray(In_Texts, LINE_TERMINATOR_ANSI, false);
			}

			else
			{
				Array_Paragraphs.Add(In_Texts);
			}

			// Generate lines for text wrapping.
			TArray<FString> Array_Lines;
			for (int32 Index_Paragraphs = 0; Index_Paragraphs < Array_Paragraphs.Num(); Index_Paragraphs++)
			{
				FString Each_Paragraph = Array_Paragraphs[Index_Paragraphs];

				if (Each_Paragraph.IsEmpty())
				{
					Array_Lines.Add(" ");

					continue;
				}

				if (Each_Paragraph.Len() < Acceptable_Horizontal)
				{
					Array_Lines.Add(Each_Paragraph);

					continue;
				}

				bool bAllowChop = true;
				while (bAllowChop)
				{
					FString Each_Line = UKismetStringLibrary::Left(Each_Paragraph, Acceptable_Horizontal);
					Each_Paragraph = UKismetStringLibrary::RightChop(Each_Paragraph, Acceptable_Horizontal);

					if (UKismetStringLibrary::Left(Each_Line, 2) == "  ")
					{
						Array_Lines.Add(Each_Line.TrimEnd());
					}

					else
					{
						Array_Lines.Add(Each_Line.TrimStartAndEnd());
					}

					if (Each_Paragraph.Len() < Acceptable_Horizontal)
					{
						if (UKismetStringLibrary::Left(Each_Paragraph, 2) == "  ")
						{
							Array_Lines.Add(Each_Paragraph.TrimEnd());
						}

						else
						{
							Array_Lines.Add(Each_Paragraph.TrimStartAndEnd());
						}

						bAllowChop = false;
					}
				}
			}

			// Check if new pages are required or not.
			if (Array_Lines.Num() > Acceptable_Vertical)
			{
				FVector2D Page_Resolution = FVector2D(FPDF_GetPageWidth(First_Page), FPDF_GetPageHeight(First_Page));
				int32 Extra_Page_Count = (Array_Lines.Num() / Acceptable_Vertical);
				for (int32 Index_Extra_Page = 0; Index_Extra_Page < Extra_Page_Count; Index_Extra_Page++)
				{
					FPDF_PAGE New_Page = FPDFPage_New(In_PDF->Document, PageIndex + Index_Extra_Page, Page_Resolution.X, Page_Resolution.Y);
					FPDFPage_GenerateContent(New_Page);
					FPDF_ClosePage(New_Page);

					Array_Pages.Add(New_Page);
				}
			}

			// Generate text objects.
			int32 ActivePage = 0;
			for (int32 Index_Lines = 0; Index_Lines < Array_Lines.Num(); Index_Lines++)
			{
				FPDF_PAGEOBJECT TextObject = FPDFPageObj_CreateTextObj(In_PDF->Document, In_Font->Font, FontSize);

				FString Each_Line = Array_Lines[Index_Lines];

				if (bUseCharcodes)
				{
					TArray<FString> Array_Chars = UKismetStringLibrary::GetCharacterArrayFromString(Each_Line);
					int32 CharsCount = Array_Chars.Num();

					TArray<uint32_t> CharCodes;
					for (int32 Index_Chars = 0; Index_Chars < CharsCount; Index_Chars++)
					{
						FString Char = Array_Chars[Index_Chars];
						CharCodes.Add(UKismetStringLibrary::GetCharacterAsNumber(Char));
					}
					
					FPDFText_SetCharcodes(TextObject, CharCodes.GetData(), CharsCount);
				}

				else
				{
					FPDFText_SetText(TextObject, TCHAR_TO_UTF16(*Each_Line));
				}

				FPDFPageObj_SetFillColor(TextObject, Text_Color.R, Text_Color.G, Text_Color.B, Text_Color.A);
				FPDFPageObj_Transform(TextObject, Size.X, Rotation.X, Rotation.Y, Size.Y, Position.X, ((Position.Y * (ActivePage + 1)) - (FontSize * Index_Lines)));
				FPDFPage_InsertObject(Array_Pages[ActivePage], TextObject);
				FPDFPage_GenerateContent(Array_Pages[ActivePage]);

				// We need counts not index.
				if (((Index_Lines + 1) / (ActivePage + 1)) == Acceptable_Vertical)
				{
					ActivePage += 1;
				}
			}

			// Close all pages after writing.
			for (int32 Index_Page = 0; Index_Page < Array_Pages.Num(); Index_Page++)
			{
				FPDF_ClosePage(Array_Pages[Index_Page]);
			}

			AsyncTask(ENamedThreads::GameThread, [DelegateAddObject]()
				{
					DelegateAddObject.ExecuteIfBound(true, "Text objects successfully added.");
				}
			);
		}
	);
}

bool UFF_PDFiumBPLibrary::PDFium_Draw_Rectangle(UPARAM(ref)UPDFiumDoc*& In_PDF, FVector2D Position, FVector2D Anchor, FVector2D Size, FVector2D Rotation, FColor Color, int32 PageIndex)
{
	if (Global_IsPDFiumInitialized == false)
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

	int BaseResolution = 256;

	FPDF_PAGE PDF_Page = FPDF_LoadPage(In_PDF->Document, PageIndex);
	FPDF_PAGEOBJECT Image_Object = FPDFPageObj_NewImageObj(In_PDF->Document);

	FPDF_BITMAP Bitmap = FPDFBitmap_CreateEx(BaseResolution, BaseResolution, FPDFBitmap_BGRA, NULL, (int)(BaseResolution * 4));
	FPDFBitmap_FillRect(Bitmap, Anchor.X, Anchor.Y, BaseResolution, BaseResolution, Color.ToPackedBGRA());
	FPDFImageObj_SetBitmap(&PDF_Page, PageIndex, Image_Object, Bitmap);

	FS_MATRIX Image_Matrix
	{
		static_cast<float>(Size.X),
		static_cast<float>(Rotation.X),
		static_cast<float>(Rotation.Y),
		static_cast<float>(Size.Y),
		static_cast<float>(Position.X),
		static_cast<float>(Position.Y),
	};

	FPDFPageObj_SetMatrix(Image_Object, &Image_Matrix);
	FPDFPage_InsertObject(PDF_Page, Image_Object);
	FPDFPage_GenerateContent(PDF_Page);
	FPDFBitmap_Destroy(Bitmap);
	FPDF_ClosePage(PDF_Page);

	return true;
}

bool PDF_Image_Callback(UObject* Target_Image, FPDF_PAGEOBJECT Image_Object, FVector2D& Out_Size)
{
	UTexture2D* Texture2D = Cast<UTexture2D>(Target_Image);
	TArray64<uint8_t> Bytes;

	if (Texture2D)
	{
		Out_Size = FVector2D(Texture2D->GetSizeX(), Texture2D->GetSizeY());

#if WITH_EDITOR

		FImage Image;
		FImageUtils::GetTexture2DSourceImage(Texture2D, Image);

		Bytes.SetNum(Image.RawData.Num());
		FMemory::Memcpy(Bytes.GetData(), Image.RawData.GetData(), Image.RawData.Num());

#else

		if (Texture2D->GetPixelFormat() == EPixelFormat::PF_B8G8R8A8 && Texture2D->CompressionSettings.GetIntValue() == 5 || Texture2D->CompressionSettings.GetIntValue() == 7)
		{
			FTexture2DMipMap& Texture_Mip = Texture2D->GetPlatformData()->Mips[0];
			void* Texture_Data = Texture_Mip.BulkData.Lock(LOCK_READ_WRITE);

			int64 BufferSize = Out_Size.X * Out_Size.Y * 4;
			if (BufferSize > Texture_Mip.BulkData.GetBulkDataSize())
			{
				UE_LOG(LogTemp, Display, TEXT("PDFium PDF : Texture settings are not compatible."))

				Texture_Mip.BulkData.Unlock();

				return NULL;
			}

			Bytes.SetNum(BufferSize);
			FMemory::Memcpy(Bytes.GetData(), (uint8_t*)Texture_Data, BufferSize);

			Texture_Mip.BulkData.Unlock();

			UE_LOG(LogTemp, Display, TEXT("PDFium PDF : Texture2D image insertion from runtime."))
		}

		else
		{
			UE_LOG(LogTemp, Display, TEXT("PDFium PDF : Texture settings are not compatible."))

			return NULL;
		}

#endif // WITH_EDITOR
	}

	UTextureRenderTarget2D* TRT2D = Cast<UTextureRenderTarget2D>(Target_Image);

	if (TRT2D)
	{
		Out_Size = FVector2D(TRT2D->SizeX, TRT2D->SizeY);
		FImageUtils::GetRawData(TRT2D, Bytes);

		UE_LOG(LogTemp, Display, TEXT("PDFium PDF : TextureRenderTarget2D image insertion."))
	}

	if (!Texture2D && !TRT2D)
	{
		UE_LOG(LogTemp, Display, TEXT("PDFium PDF : There is no image to read."))

		return false;
	}

	// Stride Function.
	int32 BytesPerPixel = 4;
	int32 Width_Bytes = Out_Size.X * BytesPerPixel;
	int32 Padding = (BytesPerPixel - (Width_Bytes) % BytesPerPixel) % BytesPerPixel;
	int32 Stride = (Width_Bytes)+Padding;

	FPDF_BITMAP PDF_Bitmap = FPDFBitmap_CreateEx(Out_Size.X, Out_Size.Y, FPDFBitmap_BGRA, Bytes.GetData(), Stride);
	FPDF_BOOL Result = FPDFImageObj_SetBitmap(NULL, 0, Image_Object, PDF_Bitmap);

	if (Result == 1)
	{
		return true;
	}

	else
	{
		return false;
	}
}

bool UFF_PDFiumBPLibrary::PDFium_Add_Image(UPARAM(ref)UPDFiumDoc*& In_PDF, UObject* In_Texture, FVector2D Position, FVector2D Rotation, int32 PageIndex)
{
	if (Global_IsPDFiumInitialized == false)
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

	if (IsValid(In_Texture) == false)
	{
		return false;
	}

	FPDF_PAGE PDF_Page = FPDF_LoadPage(In_PDF->Document, PageIndex);
	FPDF_PAGEOBJECT Image_Object = FPDFPageObj_NewImageObj(In_PDF->Document);

	FVector2D Size;
	if (!PDF_Image_Callback(In_Texture, Image_Object, Size))
	{
		return false;
	}

	FS_MATRIX Image_Matrix
	{
		static_cast<float>(Size.X),
		static_cast<float>(Rotation.X),
		static_cast<float>(Rotation.Y),
		static_cast<float>(Size.Y),
		static_cast<float>(Position.X),
		static_cast<float>(Position.Y),
	};

	FPDF_BOOL Result = FPDF_ERR_SUCCESS;
	Result = FPDFPageObj_SetMatrix(Image_Object, &Image_Matrix);
	FPDFPage_InsertObject(PDF_Page, Image_Object);
	Result = FPDFPage_GenerateContent(PDF_Page);
	FPDF_ClosePage(PDF_Page);

	if (Result == 1)
	{
		return true;
	}

	else
	{
		return false;
	}
}