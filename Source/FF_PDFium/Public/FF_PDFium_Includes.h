#pragma once

/*
*	PDFium Binaries
*	https://github.com/bblanchon/pdfium-binaries/releases
*/

#include "ExtendedVarsBPLibrary.h"

#ifdef _WIN64
THIRD_PARTY_INCLUDES_START
#include <Windows/AllowWindowsPlatformTypes.h>
#include "fpdfview.h"
#include <Windows/HideWindowsPlatformTypes.h>
#include "fpdf_edit.h"
THIRD_PARTY_INCLUDES_END
#endif

#ifdef __ANDROID__
THIRD_PARTY_INCLUDES_START
#include "fpdfview.h"
THIRD_PARTY_INCLUDES_END
#endif