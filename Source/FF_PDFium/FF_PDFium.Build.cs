// Some copyright should be here...

using System;
using System.IO;
using UnrealBuildTool;

public class FF_PDFium : ModuleRules
{
	public FF_PDFium(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
        bEnableExceptions = true;

        if (UnrealTargetPlatform.Win64 == Target.Platform)
        {
            bUseRTTI = true;

            PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "pdfium", "Win64", "include"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty", "pdfium", "Win64", "lib", "pdfium.dll.lib"));
            RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "ThirdParty", "pdfium", "Win64", "bin", "pdfium.dll"));
            PublicDelayLoadDLLs.Add("pdfium.dll");
        }

        if (UnrealTargetPlatform.Android == Target.Platform)
        {
            // We need it for #include "AndroidPlatform.h" in fpdview.h. Because default #define DLLEXPORT makes clash with Unreal's defination. We comment it out and include AndroidPlatform.h
            PublicIncludePaths.Add(Path.Combine(EngineDirectory, "Source", "Runtime", "Core", "Public", "Android"));
            PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "pdfium", "Android", "include"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty", "pdfium", "Android", "lib", "arm64-v8a", "libpdfium.so"));
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "FF_PDFium_UPL_Android.xml"));
        }

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
                "Slate",
				"SlateCore",
                "Projects",
                "RHI",
				"RenderCore",
				"ImageCore",
                "ExtendedVars",                
				"Json",
                "JsonUtilities",
                "JsonBlueprintUtilities",
			}
            );
	}
}