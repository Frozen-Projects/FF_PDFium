# USED LIBRARIES
Windows and Android Binaries<br />
https://github.com/bblanchon/pdfium-binaries

# DEPENDENCIES
1- In order to view online PDF files, you have to use a HTTP Client plugin. You can use ours or get one from Github/Unreal Marketplace or write your own.<br />
https://github.com/FF-Projects-UE/HTTP_Client

2- Extended Variables for bytes and font related functions<br />
https://github.com/FF-Projects-UE/ExtendedVars

# TARGET PLATFORM
* This is a "runtime" plugin. It won't work on editor.
* It supports Windows and Android for target platform.

# HELPER FUNCTIONS
* Unreal blueprints doesn't support TArray64<uint8> (more than 2GB size). So, in order to support this kind of large PDFs, we use ByteObject64 from our Extended Variables plugin..

# PDFIUM SYSTEM FUNCTIONS
* IsPDFiumIntialized
* Create Document
* Open PDF from file
Open PDF from Memory

# PDFIUM READ FUNCTIONS
* <b>PDFium - PDFium Get Pages</b>: It will give a Map of (Key: Texture2D, Value: Width and Height) pages as textures.
* <b>PDFium - PDFium Get All Texts<b>: It will get all texts of all pages.
* <b>PDFium - PDFium Get Texts<b>: It will get all texts of target page with their styles.
	* Transform (Location, size): Some editors use "1" point for font size and change text object's size with transform matrixes. So, if you want to re-create that PDF file with only font size, you will get very small textures.
	* Font Size
	* Font Name
	* Font Color
	* Runtime Font Object : It is a container of UFont pointer. Because we have to release it and UFontFace when we want to close game.

* <b>PDFium - Select Text<b>: You can think this as a somekind of text selection. It will get text/strings in a rectangle area.
	* You have to define that rectangle's size and location according with PDF's size and location.
	* Don't use your cursor location directly.
	* Sample math: (Cursor Location - PDF Location at screen) * Sampling (value which you used on Get Pages)

* <b>PDFium Get Links<b>: It will give all "weblinks" in specified page. It won't give internal bridges. It will give weblinks. This is limitation of PDFium.
* PDFium Get Images: It will extract all images as a TMap<UTexture2D, FVector2D (locations>. But images have to be FPDF_PAGEOBJ_IMAGE
* PDFium - Get Page Count and Sizes : It will give an array of FVector2D. Lenght of array will be equal to page size, each elements contains size of documentation.
* PDFium- File Close: It will close opened document.

# PDFIUM WRITE FUNCTIONS
* Add Pages: It adds pages to target PDF file with defined size. (Array count is page count and each vector element of array is its size)
* PDFium - Load Standart Font: There are 14 standart fonts embedded to PDF libraries according to PDF Spec 1.7 page 416. This function allows to select one them.
* PDFium - Load External Font: You have to use desktop fonts not web fonts. For example, some Google Fonts (such as Google Variant of Roboto) don't work. You can download compatible fonts from here. https://all-free-download.com/font/ , https://www.fontspace.com/category/truetype , https://www.dafont.com/
* PDFium - Add Texts: It adds text objects to target page. PDFium doesn't support line break at default but we integrated a parse system. So it has auto wrap feature. Neverthless, we suggest you that be careful about your layout (for example when will you create a line break or not)
	* Text Color
	* Position X = Horizontal position.
	* Position Y = Vertical position but it starts from bottom. If your PDF page's height is 800 pixel, top will be 800 and bottom will be 0.
	* Shear X and Y = It strecth your text object. Scaling won't be vectoral. So we don't suggest it.
	* Rotation X and Y = We don't know how it works right now.
	* Border X and Y = It defines vertical and horizontal borders.
	* Use Charcodes: Frontend usage from blueprints are same but at backend it switches between ASCII Decimal based "FPDFText_SetCharcodes()" and "FPDFText_SetText()". Charcodes has better character support.

* PDFium - Add Image: It can add BGRA8 UTexture2D and UTextureRenderTarget2D to PDF file. Textures shouldn't be DXT variations.
* PDFium - Draw Rectangle: It will draw a rectangle on PDF file with given location, size and color.
* PDFium - Save File: You need to spawn PDFium_Save actor class to save your PDF files. If you need to save multiple PDF files at the same time, you need to spawn that actor for each of them.
* PDFium - Save Bytes: You need to spawn PDFium_Save actor class to save your PDF as bytes. If you need to save multiple PDF at the same time, you need to spawn that actor for each of them.

# PDFIUM HINTS
* Do NOT close PDF document unless you absolutely sure that you won't use it anymore.
* When you close your game or finish your job with that document, close that PDF document.

# GENERAL HINTS
* if you want to "view" changes which you did on your newly created PDF file in UE5, you need to render it "again" and assign it to your widget or material.
* Look at plugin's content folder. There are example blueprints to show you how can you use this plugin.

# ROADMAP (DO NOT USE THEM UNTIL WE REMOVE THEM FROM ROADMAP SECTION !)
* PDFium - Rotation functions.

# LIMITATION
* Some external web fonts can cause crash. So, it would be good to test your fonts at development stage and give them to your customers.