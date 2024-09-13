#include "FF_PDFium_Font.h"

void UPDFiumFont::BeginDestroy()
{
	FPDFFont_Close(this->Font);

	Super::BeginDestroy();
}