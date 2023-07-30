// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FF_PDFIUM_API FF_PDFium_CharCodes
{
public:
	FF_PDFium_CharCodes();
	~FF_PDFium_CharCodes();

	static inline TMap<FString, uint32_t> Global_Char_To_ASCII;

	static inline TMap<uint32_t, FString> Global_ASCII_to_Char;

	static void DefineCharcodes();
};
