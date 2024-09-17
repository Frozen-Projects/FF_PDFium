#include "FF_PDFium_Font.h"

// Custom Includes.
#include "FF_PDFium_Doc.h"

void UPDFiumFont::BeginDestroy()
{
	if (IsValid(this->Document))
	{
		this->Document->Array_Fonts.Remove(this);
	}

	if (this->Font)
	{
		FPDFFont_Close(this->Font);
	}

	Super::BeginDestroy();
}