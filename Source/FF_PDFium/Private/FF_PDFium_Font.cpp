#include "FF_PDFium_Font.h"

// Custom Includes.
#include "FF_PDFium_Doc.h"

void UPDFiumFont::BeginDestroy()
{
	if (this->Font)
	{
		FPDFFont_Close(this->Font);
	}

	Super::BeginDestroy();
}