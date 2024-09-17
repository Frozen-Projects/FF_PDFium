#pragma once

// Custom Includes.
#include "FF_PDFium_Includes.h"

#include "FF_PDFium_Font.generated.h"

class UPDFiumDoc;

UCLASS(BlueprintType)
class FF_PDFIUM_API UPDFiumFont : public UObject
{
	GENERATED_BODY()

public:

	UPDFiumDoc* Document = nullptr;
	FPDF_FONT Font = NULL;

	// ~UPDFiumFont start.
	void BeginDestroy();
	// ~UPDFiumFont finish.
};