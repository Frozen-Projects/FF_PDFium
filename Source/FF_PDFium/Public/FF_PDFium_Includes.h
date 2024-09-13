#pragma once

/*
*	PDFium Binaries
*	https://github.com/bblanchon/pdfium-binaries/releases
*/

// External Plugin.
#include "ExtendedVarsBPLibrary.h"

// Custom Includes.
#include "FF_PDFium_Enums.h"

// UE Includes.
#include "JsonObjectWrapper.h"
#include "JsonUtilities.h"

#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"

#include "ImageCore.h"
#include "ImageUtils.h"  
#include "Engine/TextureRenderTarget2D.h"

#ifdef _WIN64
THIRD_PARTY_INCLUDES_START
#include <Windows/AllowWindowsPlatformTypes.h>
#include "fpdfview.h"
#include <Windows/HideWindowsPlatformTypes.h>
#include "fpdf_edit.h"
#include "fpdf_save.h"
#include "fpdf_text.h"
#include "fpdf_formfill.h"
THIRD_PARTY_INCLUDES_END
#endif

#ifdef __ANDROID__
THIRD_PARTY_INCLUDES_START
#include "fpdfview.h"
#include "fpdf_save.h"
#include "fpdf_edit.h"
#include "fpdf_text.h"
#include "fpdf_formfill.h"
THIRD_PARTY_INCLUDES_END
#endif