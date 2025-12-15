#include "FF_PDFium_Font.h"

void UPDFiumFont::BeginDestroy()
{
	if (this->Font)
	{
		FPDFFont_Close(this->Font);
	}

	Super::BeginDestroy();
}