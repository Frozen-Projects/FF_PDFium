// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class PDF_READER_API PDF_CharCodes
{
public:
	PDF_CharCodes();
	~PDF_CharCodes();

	static inline TMap<FString, uint32_t> Global_Char_To_ASCII;

	static inline TMap<uint32_t, FString> Global_ASCII_to_Char;

	static void DefineCharcodes();
};
