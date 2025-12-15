#pragma once

// External Plugin.
#include "ExtendedVarsBPLibrary.h"

// UE Includes.
#include "JsonObjectWrapper.h"
#include "JsonUtilities.h"

#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"

#include "ImageCore.h"
#include "ImageUtils.h"  
#include "Engine/TextureRenderTarget2D.h"

#include "Subsystems/GameInstanceSubsystem.h"

THIRD_PARTY_INCLUDES_START
// https://github.com/bblanchon/pdfium-binaries/releases
#ifdef _WIN64
#include <Windows/AllowWindowsPlatformTypes.h>
#include "fpdfview.h"
#include <Windows/HideWindowsPlatformTypes.h>
#else
#include "fpdfview.h"
#endif

#include "fpdf_edit.h"
#include "fpdf_save.h"
#include "fpdf_text.h"
#include "fpdf_formfill.h"
THIRD_PARTY_INCLUDES_END