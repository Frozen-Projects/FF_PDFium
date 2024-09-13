#pragma once

UENUM(BlueprintType)
enum class EPDFiumSaveTypes : uint8
{
	Incremental		UMETA(DisplayName = "Incremental"),
	NoIncremental	UMETA(DisplayName = "NoIncremental"),
	RemoveSecurity	UMETA(DisplayName = "RemoveSecurity"),
};
ENUM_CLASS_FLAGS(EPDFiumSaveTypes)

UENUM(BlueprintType)
enum class EPDFiumSaveVersion : uint8
{
	PDF_14		UMETA(DisplayName = "PDF_14"),
	PDF_15		UMETA(DisplayName = "PDF_15"),
	PDF_17		UMETA(DisplayName = "PDF_17"),
};
ENUM_CLASS_FLAGS(EPDFiumSaveVersion)