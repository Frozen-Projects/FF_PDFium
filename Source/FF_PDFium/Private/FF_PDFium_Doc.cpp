#include "FF_PDFium_Doc.h"

// Custom Includes.
#include "FF_PDFium_Manager.h"
#include "FF_PDFium_Font.h"

TArray64<uint8> UPDFiumDoc::SaveBytes64(bool& bIsSaveSuccessful, EPDFiumSaveTypes In_SaveType, EPDFiumSaveVersion In_Version)
{
	thread_local TArray64<uint8> ThreadLocalPDFBytes;
	ThreadLocalPDFBytes.Reset();

	auto Callback_Writer = [](FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)->int
		{
			ThreadLocalPDFBytes.Append(static_cast<const uint8*>(pData), size);
			return size;
		};

	FPDF_FILEWRITE Writer;
	memset(&Writer, 0, sizeof(FPDF_FILEWRITE));
	Writer.WriteBlock = Callback_Writer;
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

	FPDF_BOOL RetVal = FPDF_SaveWithVersion(this->Document, &Writer, Flags, Version);

	if (ThreadLocalPDFBytes.IsEmpty() || RetVal != 1)
	{
		bIsSaveSuccessful = false;
		return TArray64<uint8>();
	}

	bIsSaveSuccessful = true;
	return MoveTemp(ThreadLocalPDFBytes);
}

TArray<uint8> UPDFiumDoc::SaveBytes(bool& bIsSaveSuccessful, EPDFiumSaveTypes In_SaveType, EPDFiumSaveVersion In_Version)
{
	thread_local TArray<uint8> ThreadLocalPDFBytes;
	ThreadLocalPDFBytes.Reset();

	auto Callback_Writer = [](FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)->int
		{
			ThreadLocalPDFBytes.Append(static_cast<const uint8*>(pData), size);
			return size;
		};

	FPDF_FILEWRITE Writer;
	memset(&Writer, 0, sizeof(FPDF_FILEWRITE));
	Writer.WriteBlock = Callback_Writer;
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

	FPDF_BOOL RetVal = FPDF_SaveWithVersion(this->Document, &Writer, Flags, Version);

	if (ThreadLocalPDFBytes.IsEmpty() || !RetVal)
	{
		bIsSaveSuccessful = false;
		return TArray<uint8>();
	}

	bIsSaveSuccessful = true;
	return ThreadLocalPDFBytes;
}

void UPDFiumDoc::BeginDestroy()
{
	if (!this->Array_Fonts.IsEmpty())
	{
		for (int32 Index_Font = 0; Index_Font < this->Array_Fonts.Num(); Index_Font++)
		{
			UPDFiumFont* EachFont = this->Array_Fonts[Index_Font];

			if (IsValid(EachFont))
			{
				EachFont->ConditionalBeginDestroy();
			}
		}

		this->Array_Fonts.Empty();
	}

	if (this->Document)
	{
		FPDF_CloseDocument(this->Document);
	}

	free(this->PDF_Data);
	Super::BeginDestroy();
}

bool UPDFiumDoc::SetManager(UFF_PDFium_ManagerSubsystem* In_Manager)
{
	if (!IsValid(In_Manager))
	{
		return false;
	}

	this->Manager = In_Manager;
	return true;
}

UFF_PDFium_ManagerSubsystem* UPDFiumDoc::GetManager()
{
	return this->Manager;
}

bool UPDFiumDoc::SetBuffer(const size_t Size, void* Data)
{
	if (!Data)
	{
		return false;
	}

	if (Size == 0)
	{
		return false;
	}

	this->PDF_Data = malloc(Size);
	FMemory::Memcpy(this->PDF_Data, Data, Size);
	this->PDF_Data_Size = Size;
	return true;
}

void* UPDFiumDoc::GetBuffer()
{
	if (!this->PDF_Data)
	{
		return nullptr;
	}
	
	if (this->PDF_Data_Size == 0)
	{
		return nullptr;
	}

	return this->PDF_Data;
}

size_t UPDFiumDoc::GetSize()
{
	return this->PDF_Data_Size;
}

bool UPDFiumDoc::PDFium_Get_Pages(FJsonObjectWrapper& Out_Code, TMap<UTexture2D*, FVector2D>& Out_Pages, int32 In_Sampling, FColor BG_Color, bool bUseSrgb, bool bUseMatrix, bool bUseAlpha, bool bRenderAnnots)
{
	FJsonObjectWrapper TempCode;
	Out_Code = TempCode;

	TempCode.JsonObject->SetStringField("PluginName", "FF_PDFium");
	TempCode.JsonObject->SetStringField("ClassName", "UFF_PDFiumBPLibrary");
	TempCode.JsonObject->SetStringField("FunctionName", "PDFium_Get_Pages");
	TempCode.JsonObject->SetStringField("AdditionalInfo", "");

	if (!IsValid(this->Manager))
	{
		TempCode.JsonObject->SetStringField("Description", "PDFium manager is not valid.");
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		TempCode.JsonObject->SetStringField("Description", "PDFium has not been initialized.");
		return false;
	}

	if (!this->Document)
	{
		TempCode.JsonObject->SetStringField("Description", "PDFium \"file\" is not valid.");
		return false;
	}

	const double Sampling = In_Sampling < 1 ? 1 : In_Sampling;
	TArray<int32> SkippedPages;

	for (int32 Index_Pages = 0; Index_Pages < FPDF_GetPageCount(this->Document); Index_Pages++)
	{
		FPDF_PAGE PDF_Page = FPDF_LoadPage(this->Document, Index_Pages);

		const double PDF_Page_Width = FPDF_GetPageWidth(PDF_Page);
		const double PDF_Page_Height = FPDF_GetPageHeight(PDF_Page);

		if (PDF_Page_Width <= 0 || PDF_Page_Height <= 0)
		{
			SkippedPages.Add(Index_Pages);

			FPDF_ClosePage(PDF_Page);
			continue;
		}

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

		if (!Buffer)
		{
			SkippedPages.Add(Index_Pages);

			FPDFBitmap_Destroy(PDF_Bitmap);
			FPDF_ClosePage(PDF_Page);
			continue;
		}

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

	const bool IsSuccessfull = Out_Pages.Num() > 0 ? true : false;
	TempCode.JsonObject->SetStringField("Description", IsSuccessfull ? "PDF rendered successfully." : "There was a problem while rendering PDF file.");

	auto GetSkippedPages = [](TArray<int32> In_Skipped)->FString
		{
			if (In_Skipped.IsEmpty())
			{
				return FString();
			}

			FString Temp_String;
			const int32 LastIndex = In_Skipped.Num() - 1;

			for (int32 Index_Skipped = 0; Index_Skipped < In_Skipped.Num(); Index_Skipped++)
			{
				Temp_String += FString::FromInt(In_Skipped[Index_Skipped]);

				if (Index_Skipped != LastIndex)
				{
					Temp_String += "-";
				}
			}

			return Temp_String;
		};

	TempCode.JsonObject->SetStringField("AdditionalInfo", SkippedPages.IsEmpty() ? "" : "SkippedPages : " + GetSkippedPages(SkippedPages));

	return IsSuccessfull;
}

bool UPDFiumDoc::PDFium_Get_Images(TMap<UTexture2D*, FVector2D>& Out_Images, int32 PageIndex, bool bUseSrgb)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	FPDF_PAGE PDF_Page = FPDF_LoadPage(this->Document, PageIndex);

	const int ObjectCount = FPDFPage_CountObjects(PDF_Page);

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

			FPDF_BITMAP Each_Bitmap = FPDFImageObj_GetRenderedBitmap(this->Document, PDF_Page, Each_Object);
			void* Each_Buffer = FPDFBitmap_GetBuffer(Each_Bitmap);

			if (!Each_Buffer)
			{
				continue;
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

bool UPDFiumDoc::PDFium_Get_All_Texts(TArray<FString>& Out_Texts)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	for (int32 PageIndex = 0; PageIndex < FPDF_GetPageCount(this->Document); PageIndex++)
	{
		FPDF_PAGE PDF_Page = FPDF_LoadPage(this->Document, PageIndex);
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
		CharBuffer = nullptr;
	}

	return true;
}

bool UPDFiumDoc::PDFium_Get_Texts(TArray<FPdfTextObject>& Out_Texts, int32 PageIndex)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	FPDF_PAGE PDF_Page = FPDF_LoadPage(this->Document, PageIndex);
	FPDF_TEXTPAGE Text_Page = FPDFText_LoadPage(PDF_Page);

	const int ObjectCount = FPDFPage_CountObjects(PDF_Page);
	for (int32 Index_Objects = 0; Index_Objects < ObjectCount; Index_Objects++)
	{
		FPDF_PAGEOBJECT Each_Object = FPDFPage_GetObject(PDF_Page, Index_Objects);
		if (FPDFPageObj_GetType(Each_Object) == FPDF_PAGEOBJ_TEXT)
		{
			// Initialize text object structure.
			FPdfTextObject Text_Object;

			// Text String
			unsigned long Buffer_Lenght = FPDFTextObj_GetText(Each_Object, Text_Page, NULL, 0);
			FPDF_WCHAR* CharBuffer = (unsigned short*)malloc(Buffer_Lenght);
			FPDFTextObj_GetText(Each_Object, Text_Page, CharBuffer, Buffer_Lenght);

			FString PageText;
			PageText.AppendChars((WIDECHAR*)CharBuffer, ((Buffer_Lenght / 2) - 1));
			Text_Object.Text_String = PageText;

			free(CharBuffer);
			CharBuffer = nullptr;

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
			unsigned long Font_Name_Lenght = FPDFFont_GetFamilyName(Font_Object, NULL, 0);
			char* Font_Name = (char*)malloc(static_cast<size_t>(Font_Name_Lenght));
			FPDFFont_GetFamilyName(Font_Object, Font_Name, Font_Name_Lenght);
			Text_Object.Font_Name.AppendChars(Font_Name, Font_Name_Lenght);

			// Font Size
			FPDFTextObj_GetFontSize(Each_Object, &Text_Object.Font_Size);

			// Font Bytes
			size_t Bufflen;
			memset(&Bufflen, 0, sizeof(size_t));
			FPDFFont_GetFontData(Font_Object, NULL, Bufflen, &Bufflen);

			TArray<uint8> Font_Bytes;
			Font_Bytes.SetNum(Bufflen);
			FPDFFont_GetFontData(Font_Object, Font_Bytes.GetData(), Bufflen, &Bufflen);
			Text_Object.Runtime_Font = UExtendedVarsBPLibrary::Runtime_Font_Load(Font_Bytes, Text_Object.Font_Name);

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

bool UPDFiumDoc::PDFium_Get_Links(TArray<FString>& Out_Links, int32 PageIndex)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	FPDF_PAGE PDF_Page = FPDF_LoadPage(this->Document, PageIndex);
	FPDF_TEXTPAGE PDF_TextPage = FPDFText_LoadPage(PDF_Page);

	FPDF_PAGELINK PDF_Links = FPDFLink_LoadWebLinks(PDF_TextPage);
	const int32 Links_Count = FPDFLink_CountWebLinks(PDF_Links);

	if (Links_Count == 0)
	{
		FPDFLink_CloseWebLinks(PDF_Links);
		FPDFText_ClosePage(PDF_TextPage);

		return false;
	}

	for (int32 Index_Link = 0; Index_Link < Links_Count; Index_Link++)
	{
		const int Count_Links = FPDFLink_GetURL(PDF_Links, Index_Link, NULL, 0);
		const int BufferLenght = Count_Links * 2;
		unsigned short* CharBuffer = (unsigned short*)malloc(static_cast<size_t>(BufferLenght));
		FPDFLink_GetURL(PDF_Links, Index_Link, CharBuffer, BufferLenght);

		FString LinkText;
		LinkText.AppendChars((WIDECHAR*)CharBuffer, Count_Links);

		Out_Links.Add(LinkText);
		free(CharBuffer);
		CharBuffer = nullptr;
	}

	FPDFLink_CloseWebLinks(PDF_Links);
	FPDFText_ClosePage(PDF_TextPage);
	FPDF_ClosePage(PDF_Page);

	return true;
}

bool UPDFiumDoc::PDFium_Select_Text(FString& Out_Text, FVector2D Start, FVector2D End, int32 PageIndex)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	FPDF_PAGE PDF_Page = FPDF_LoadPage(this->Document, PageIndex);
	FPDF_TEXTPAGE PDF_TextPage = FPDFText_LoadPage(PDF_Page);

	const int Count_Texts = FPDFText_GetBoundedText(PDF_TextPage, Start.X, Start.Y, End.X, End.Y, NULL, 0);
	const int BufferLenght = (Count_Texts + 1) * 2;
	unsigned short* CharBuffer = (unsigned short*)malloc(static_cast<size_t>(BufferLenght));
	FPDFText_GetBoundedText(PDF_TextPage, Start.X, Start.Y, End.X, End.Y, CharBuffer, BufferLenght);

	FString SelectedText;
	SelectedText.AppendChars((WIDECHAR*)CharBuffer, Count_Texts);

	Out_Text = SelectedText;

	FPDFText_ClosePage(PDF_TextPage);
	FPDF_ClosePage(PDF_Page);
	free(CharBuffer);
	CharBuffer = nullptr;

	return true;
}

bool UPDFiumDoc::PDFium_Pages_Counts_Sizes(TArray<FVector2D>& Out_Infos)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	const int32 PagesCount = FPDF_GetPageCount(this->Document);

	for (int32 Index_Pages = 0; Index_Pages < PagesCount; Index_Pages++)
	{
		FPDF_PAGE PDF_Page = FPDF_LoadPage(this->Document, Index_Pages);
		const double PDF_Page_Width = FPDF_GetPageWidth(PDF_Page);
		const double PDF_Page_Height = FPDF_GetPageHeight(PDF_Page);
		Out_Infos.Add(FVector2D(PDF_Page_Width, PDF_Page_Height));

		FPDF_ClosePage(PDF_Page);
	}

	return true;
}

bool UPDFiumDoc::PDFium_Pages_Add(TArray<FVector2D> Pages)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	const int32 PageCount = FPDF_GetPageCount(this->Document);

	for (int32 Index_Pages = 0; Index_Pages < Pages.Num(); Index_Pages++)
	{
		FPDF_PAGE PDF_Page = FPDFPage_New(this->Document, PageCount + Index_Pages, Pages[Index_Pages].X, Pages[Index_Pages].Y);
		FPDFPage_GenerateContent(PDF_Page);

		FPDF_ClosePage(PDF_Page);
	}

	return true;
}

bool UPDFiumDoc::PDFium_Pages_Delete(int32 PageIndex)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	FPDF_PAGE TargetPage = FPDF_LoadPage(this->Document, PageIndex);

	if (!TargetPage)
	{
		return false;
	}

	FPDF_ClosePage(TargetPage);
	FPDFPage_Delete(this->Document, PageIndex);

	return true;
}

bool UPDFiumDoc::PDFium_Font_Load_Standart(UPDFiumFont*& Out_Font, EStandartFonts Font_Name)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
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

	FPDF_FONT Font = FPDFText_LoadStandardFont(this->Document, TCHAR_TO_UTF8(*Name_String));

	if (!Font)
	{
		return false;
	}

	Out_Font = NewObject<UPDFiumFont>();
	Out_Font->Font = Font;
	this->Array_Fonts.Add(Out_Font);

	return true;
}

bool UPDFiumDoc::PDFium_Font_Load_External(UPDFiumFont*& Out_Font, FString Font_Path, EExternalFonts In_Font_Type, bool bIsCid)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	if (Font_Path.IsEmpty() == true)
	{
		return false;
	}

	FString Path = FPlatformFileManager::Get().GetPlatformFile().ConvertToAbsolutePathForExternalAppForRead(*Font_Path);

	if (FPaths::FileExists(Path) == false)
	{
		return false;
	}

	TArray<uint8> ByteArray;
	FFileHelper::LoadFileToArray(ByteArray, *Path);

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

	FPDF_FONT Font = FPDFText_LoadFont(this->Document, reinterpret_cast<const uint8_t*>(ByteArray.GetData()), ByteArray.GetAllocatedSize(), FontType, bIsCid);

	if (!Font)
	{
		return false;
	}

	ByteArray.Empty();

	Out_Font = NewObject<UPDFiumFont>();
	Out_Font->Font = Font;
	this->Array_Fonts.Add(Out_Font);

	return true;
}

void UPDFiumDoc::PDFium_Add_Texts(FDelegatePdfium DelegateAddObject, UPARAM(ref)UPDFiumFont*& In_Font, FString In_Texts, FColor Text_Color, FVector2D Position, FVector2D Size, FVector2D Rotation, FVector2D Border, int32 FontSize, int32 PageIndex, bool bUseCharcodes)
{
	if (!IsValid(this->Manager))
	{
		DelegateAddObject.Execute(false, "PDFium manager is not valid.");
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		DelegateAddObject.Execute(false, "PDFium Library haven't been initialized.");
	}

	if (!this->Document)
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

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [DelegateAddObject, this, In_Font, In_Texts, Text_Color, Position, Size, Rotation, Border, FontSize, PageIndex, bUseCharcodes]()
		{
			FPDF_PAGE First_Page = FPDF_LoadPage(this->Document, PageIndex);
			TArray<FPDF_PAGE> Array_Pages;
			Array_Pages.Add(First_Page);

			const int32 Acceptable_Horizontal = 2 * ((FPDF_GetPageWidth(First_Page) - Position.X - Border.X) / FontSize);
			const int32 Acceptable_Vertical = (Position.Y - Border.Y) / FontSize;

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
					FPDF_PAGE New_Page = FPDFPage_New(this->Document, PageIndex + Index_Extra_Page, Page_Resolution.X, Page_Resolution.Y);
					FPDFPage_GenerateContent(New_Page);
					FPDF_ClosePage(New_Page);

					Array_Pages.Add(New_Page);
				}
			}

			// Generate text objects.
			int32 ActivePage = 0;
			for (int32 Index_Lines = 0; Index_Lines < Array_Lines.Num(); Index_Lines++)
			{
				FPDF_PAGEOBJECT TextObject = FPDFPageObj_CreateTextObj(this->Document, In_Font->Font, FontSize);

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

bool UPDFiumDoc::PDFium_Draw_Rectangle(FVector2D Position, FVector2D Anchor, FVector2D Size, FVector2D Rotation, FColor Color, int32 PageIndex)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	const int BaseResolution = 256;

	FPDF_PAGE PDF_Page = FPDF_LoadPage(this->Document, PageIndex);
	FPDF_PAGEOBJECT Image_Object = FPDFPageObj_NewImageObj(this->Document);

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

bool UPDFiumDoc::PDFium_Add_Image(FString& Out_Code, TArray<uint8> In_Bytes, FVector2D In_Size, FVector2D Position, FVector2D Rotation, int32 PageIndex)
{
	if (!IsValid(this->Manager))
	{
		Out_Code = "PDFium manager is not valid.";
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	if (In_Bytes.IsEmpty())
	{
		Out_Code = "Bytes' size shouldn't be zero.";
		return false;
	}

	if (In_Size.X == 0 || In_Size.Y == 0)
	{
		Out_Code = "Image's size components shouldn't be zero.";
		return false;
	}

	// If byte array is PNG, first 8 bytes will be "89504e470d0a1a0a" and PDFium doesn't support it.
	if (UExtendedVarsBPLibrary::Bytes_x86_To_Hex(In_Bytes, 0, 7, false) == "89504e470d0a1a0a")
	{
		Out_Code = "PDFium doesn't support PNG files. Please use JPEG/JPG or raw buffer.";
		return false;
	}

	// If byte array is BMP, first two bytes will be "424d" and PDFium doesn't support it.
	else if (UExtendedVarsBPLibrary::Bytes_x86_To_Hex(In_Bytes, 0, 1, false) == "424d")
	{
		Out_Code = "PDFium doesn't support BMP files. Please use JPEG/JPG or raw buffer.";
		return false;
	}

	FPDF_PAGE PDF_Page = FPDF_LoadPage(this->Document, PageIndex);
	FPDF_PAGEOBJECT Image_Object = FPDFPageObj_NewImageObj(this->Document);

	FPDF_BOOL Result = false;
	FString ImageType = "";

	// If byte array is JPG/JPEG, first two bytes will be "ffd8" and last two bytes will be "ffd9"
	if (UExtendedVarsBPLibrary::Bytes_x86_To_Hex(In_Bytes, 0, 1, false) == "ffd8" && UExtendedVarsBPLibrary::Bytes_x86_To_Hex(In_Bytes, (In_Bytes.Num() - 2), (In_Bytes.Num() - 1), false) == "ffd9")
	{
		auto Callback_Jpeg = [](void* CallbackContext, unsigned long position, unsigned char* ImportBuffer, unsigned long size)-> int
			{
				if (!CallbackContext)
				{
					UE_LOG(LogTemp, Warning, TEXT("Texture Buffer is not valid !"));
					return 0;
				}

				FMemory::Memcpy(ImportBuffer, static_cast<uint8*>(CallbackContext) + position, size);
				return 1;
			};

		FPDF_FILEACCESS Jpeg_Access;
		memset(&Jpeg_Access, 0, sizeof(Jpeg_Access));
		Jpeg_Access.m_FileLen = In_Bytes.Num();
		Jpeg_Access.m_Param = In_Bytes.GetData();
		Jpeg_Access.m_GetBlock = Callback_Jpeg;

		Result = FPDFImageObj_LoadJpegFileInline(&PDF_Page, 1, Image_Object, &Jpeg_Access);

		ImageType = "\"Jpeg\"";
	}

	// Raw image
	else
	{
		// Stride Function.
		const int32 BytesPerPixel = sizeof(FColor);
		const int32 Width_Bytes = In_Size.X * BytesPerPixel;
		const int32 Padding = (BytesPerPixel - (Width_Bytes) % BytesPerPixel) % BytesPerPixel;
		const int32 Stride = (Width_Bytes)+Padding;

		if (In_Bytes.Num() != (In_Size.X * In_Size.Y * BytesPerPixel))
		{
			FPDFPageObj_Destroy(Image_Object);
			FPDF_ClosePage(PDF_Page);

			Out_Code = "Unknown image buffer.";
			return false;
		}

		FPDF_BITMAP PDF_Bitmap = FPDFBitmap_CreateEx(In_Size.X, In_Size.Y, FPDFBitmap_BGRA, In_Bytes.GetData(), Stride);

		Result = FPDFImageObj_SetBitmap(NULL, 0, Image_Object, PDF_Bitmap);

		ImageType = "\"Raw buffer\"";
	}

	if (Result != 1)
	{
		FPDFPageObj_Destroy(Image_Object);
		FPDF_ClosePage(PDF_Page);

		Out_Code = "Image add is not successful.";
		return false;
	}

	FS_MATRIX Image_Matrix
	{
		static_cast<float>(In_Size.X),
		static_cast<float>(Rotation.X),
		static_cast<float>(Rotation.Y),
		static_cast<float>(In_Size.Y),
		static_cast<float>(Position.X),
		static_cast<float>(Position.Y),
	};

	Result = FPDFPageObj_SetMatrix(Image_Object, &Image_Matrix);

	if (Result != 1)
	{
		FPDFPageObj_Destroy(Image_Object);
		FPDF_ClosePage(PDF_Page);

		Out_Code = "Image matrix definition is not successful.";
		return false;
	}

	FPDFPage_InsertObject(PDF_Page, Image_Object);

	Result = FPDFPage_GenerateContent(PDF_Page);
	if (Result != 1)
	{
		FPDFPageObj_Destroy(Image_Object);
		FPDF_ClosePage(PDF_Page);

		Out_Code = "PDF content update is not successful.";
		return false;
	}

	Out_Code = "Image insert as " + ImageType + " is successful.";

	FPDF_ClosePage(PDF_Page);
	return true;
}

void UPDFiumDoc::PDFium_Save_File(FDelegatePdfium DelegateSave, FString Export_Path, EPDFiumSaveTypes In_SaveType, EPDFiumSaveVersion In_Version)
{
	if (!IsValid(this->Manager))
	{
		DelegateSave.ExecuteIfBound(false, "PDFium manager is not valid.");
		return;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		DelegateSave.ExecuteIfBound(false, "PDFium Library haven't been initialized.");
		return;
	}

	if (!this->Document)
	{
		DelegateSave.ExecuteIfBound(false, "PDF document is not valid.");
		return;
	}

	FString TempPath = Export_Path;
	FPaths::MakeStandardFilename(TempPath);

	if (TempPath.IsEmpty())
	{
		DelegateSave.ExecuteIfBound(false, "Export path is empty.");
		return;
	}

	const FString Directory = FPaths::GetPath(TempPath);

	if (!FPaths::DirectoryExists(Directory))
	{
		DelegateSave.ExecuteIfBound(false, "Export directory doesn't exist.");
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, DelegateSave, In_SaveType, In_Version, TempPath]()
		{
			bool bSaveSucceeded = false;
			TArray64<uint8> PDF_Bytes = this->SaveBytes64(bSaveSucceeded, In_SaveType, In_Version);

			if (!bSaveSucceeded || PDF_Bytes.IsEmpty())
			{
				PDF_Bytes.Reset();

				AsyncTask(ENamedThreads::GameThread, [DelegateSave]()
					{
						DelegateSave.ExecuteIfBound(false, "PDF save is not successful.");
					}
				);

				return;
			}
			
			bSaveSucceeded = FFileHelper::SaveArrayToFile(MoveTemp(PDF_Bytes), *TempPath);

			AsyncTask(ENamedThreads::GameThread, [DelegateSave, bSaveSucceeded, TempPath]()
				{
					DelegateSave.ExecuteIfBound(bSaveSucceeded, bSaveSucceeded ? "PDF file saved successfully to " + TempPath : "PDF file save is not successful.");
				}
			);
		}
	);
}

bool UPDFiumDoc::PDFium_Save_Bytes_x64(UBytesObject_64*& Out_Bytes, EPDFiumSaveTypes In_SaveType, EPDFiumSaveVersion In_Version)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	bool bIsSaveSuccessful = false;
	TArray64<uint8> PDF_Bytes = this->SaveBytes64(bIsSaveSuccessful, In_SaveType, In_Version);

	if (!bIsSaveSuccessful || PDF_Bytes.IsEmpty())
	{
		PDF_Bytes.Reset();
		return false;
	}

	Out_Bytes = NewObject<UBytesObject_64>();
	Out_Bytes->ByteArray = MoveTemp(PDF_Bytes);
	PDF_Bytes.Reset();

	return true;
}

bool UPDFiumDoc::PDFium_Save_Bytes_x86(TArray<uint8>& Out_Bytes, EPDFiumSaveTypes In_SaveType, EPDFiumSaveVersion In_Version)
{
	if (!IsValid(this->Manager))
	{
		return false;
	}

	if (!this->Manager->IsPDFiumInitialized())
	{
		return false;
	}

	if (!this->Document)
	{
		return false;
	}

	bool bIsSaveSuccessful = false;
	TArray<uint8> PDF_Bytes = this->SaveBytes(bIsSaveSuccessful, In_SaveType, In_Version);

	if (!bIsSaveSuccessful || PDF_Bytes.IsEmpty())
	{
		PDF_Bytes.Reset();
		return false;
	}

	Out_Bytes.Reset();
	Out_Bytes = MoveTemp(PDF_Bytes);
	PDF_Bytes.Reset();
	
	return true;
}
