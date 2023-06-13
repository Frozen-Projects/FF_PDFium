# USED LIBRARIES
Windows and Android Binaries<br />
https://github.com/bblanchon/pdfium-binaries

# DEPENDENCIES
1- In order to view online PDF files, you have to use a HTTP Client plugin. You can use ours or get one from Github/Unreal Marketplace or write your own.<br />
https://github.com/FF-Projects-UE/HTTP_Client

2- <b>Extended Variables</b> for bytes and font related functions<br />
https://github.com/FF-Projects-UE/ExtendedVars

# RELATED PLUGINS
1- UE_LibHaru (Use this if you want to create PDF with images.)
https://github.com/FF-Projects-UE/UE_LibHaru

# TARGET PLATFORM
* Plugin is for Unreal Engine 5.1 and up.
* This is a "runtime" plugin. It won't work on editor.
* It supports both Windows and Android.

# HELPER FUNCTIONS
Unreal blueprints doesn't support TArray64<uint8> (more than 2GB size). So, in order to support this kind of large PDF loadings, we capsulated that array to an <b>BytesObject_32 and BytesObject_32 with Extended Variables plugin.</b>.

# PDFIUM SYSTEM FUNCTIONS
* <b>PDFium - LibInit / PDF_LibClose / PDF_LibState</b>: It will initialize, close and give state of PDFium library.

# PDFIUM READ FUNCTIONS
* <b>PDFium - File Open</b> : This will open PDF from given bytes.

* <b>PDFium - PDFium Get Pages</b>: It will give a Map of (Key: Texture2D, Value: Width and Height) pages as textures.

* <b>PDFium - PDFium Get All Texts</b>: It will get all texts of all pages.

* <b>PDFium - PDFium Get Texts</b>: It will get all texts of <b>target page</b> with their styles.
	* Transform (Location, size): Some editors use "1" point for font size and change text object's size with transform matrixes. So, if you want to re-create that PDF file with only font size, you will get very small textures.
	* Font Size
	* Font Name
	* Font Color
	* Runtime Font Object : It is a container of UFont pointer. Because we have to release it and UFontFace when we want to close game.

* <b>PDFium - Select Text</b>: You can think this as a somekind of text selection. It will get text/strings in a rectangle area.
	* You have to define that rectangle's size and location according with PDF's size and location.
	* Don't use your cursor location directly.
	* Sample math: (Cursor Location - PDF Location at screen) * Sampling (value which you used on Get Pages)

* <b>PDFium Get Links</b>: It will give all "weblinks" in specified page. It won't give internal bridges. It will give weblinks. This is limitation of PDFium.

* <b>PDFium Get Images</b>: It will extract all images as a TMap<UTexture2D, FVector2D (locations>. But images have to be <b>FPDF_PAGEOBJ_IMAGE</b>

* <b>PDFium - Get Page Count and Sizes </b>: It will give an array of FVector2D. Lenght of array will be equal to page size, each elements contains size of documentation.

* <b>PDFium- File Close</b>: It will close opened document.

# PDFIUM WRITE FUNCTIONS
* <b>PDFium - Create PDF</b>: It creates a new and empty PDF file.

* <b>Add Pages</b>: It adds pages to target PDF file with defined size. (Array count is page count and each vector element of array is its size)

* <b>PDFium - Load Standart Font</b>: There are 14 standart fonts embedded to PDF libraries according to PDF Spec 1.7 page 416. This function allows to select one them.

* <b>PDFium - Load External Font</b>: You have to use <b>desktop fonts</b> not <b>web fonts</b>. For example, some Google Fonts (such as Google Variant of Roboto) don't work. You can download compatible fonts from here. https://all-free-download.com/font/ , https://www.fontspace.com/category/truetype , https://www.dafont.com/

* <b>PDFium - Add Texts</b>: It adds text objects to target page. PDFium doesn't support line break at default but we integrated a parse system. So it has auto wrap feature. Neverthless, we suggest you that be careful about your layout (for example when will you create a line break or not)
	* Text Color
	* Position X = Horizontal position.
	* Position Y = Vertical position but it starts from bottom. If your PDF page's height is 800 pixel, top will be 800 and bottom will be 0.
	* Shear X and Y = It strecth your text object. Scaling won't be vectoral. So we don't suggest it.
	* Rotation X and Y = We don't know how it works right now.
	* Border X and Y = It defines vertical and horizontal borders.
	* Use Charcodes: Frontend usage from blueprints are same but at backend it switches between ASCII Decimal based "FPDFText_SetCharcodes()" and "FPDFText_SetText()". Charcodes has better character support.

* <b>PDFium - Draw Rectangle</b>: It will draw a rectangle on PDF file with given location, size and color.

* <b>PDFium - Save File</b>: You need to spawn <b>PDFium_Save</b> actor class to save your PDF files. If you need to save multiple PDF files at the same time, you need to spawn that actor for each of them.

* <b>PDFium - Save Bytes</b>: You need to spawn <b>PDFium_Save</b> actor class to save your PDF as bytes. If you need to save multiple PDF at the same time, you need to spawn that actor for each of them.

# PDFIUM HINTS
* Do NOT close PDF document unless you absolutely sure that you won't use it anymore.
* When you close your game or finish your job with that document, close that PDF document.

# GENERAL HINTS
* if you want to "view" changes which you did on your newly created PDF file in UE5, you need to render it "again" and assign it to your widget or material.
* Look at plugin's content folder. There are example blueprints to show you how can you use this plugin.

# ROADMAP (DO NOT USE THEM UNTIL WE REMOVE THEM FROM ROADMAP SECTION !)
* PDFium - Rotation functions.

# LIMITATION
* Currently it doesn't support image adding. If you want to add images to PDF, use UE_LibHaru plugin which mentioned on <b>Related Projects</b> section. It has same functions with same usages with PDFium.<br /> Also you can get bytes of LibHaru based PDF to open it with PDFium.
* Some external web fonts cause crash. So, it would be good to test your fonts at development stage and give them to your customers.