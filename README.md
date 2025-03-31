# DEPENDENCIES
1. Windows and Android Binaries<br />
https://github.com/bblanchon/pdfium-binaries

2. In order to view online PDF files, you have to use a HTTP Client plugin. You can use ours or get one from Github/Unreal Marketplace or write your own.<br />
https://github.com/FF-Projects-UE/HTTP_Client

3. <b>Extended Variables</b> for x64 bytes, external fonts and image adding<br />
https://github.com/FF-Projects-UE/ExtendedVars

# TARGET PLATFORM
This is a "runtime" plugin. It won't work on editor. It supports Windows and Android platforms as target.

# HELPER FUNCTIONS
Unreal blueprints doesn't support ``TArray64<uint8>`` and ``TArray<uint8>`` doesn't support more than 2 GB file size. So, if you want to bigger PDF files than 2 GB, you have to use ``BytesObject_x64`` from our ``Extended Variables`` plugin.< /br>
Add image function expects images as bytes array. You can use our ``Extended Variables`` plugin to get texture buffers, you can use another plugins from marketplace or you can write your own logic. Target texture shouldn't be ``DXT``variations. For detailed information, you can look at ``Extended Variables``'s ``README`` file.

# PDFIUM SYSTEM FUNCTIONS
* IsPDFiumIntialized
* Create Document
* Open PDF from file
* Open PDF from Memory

# PDFIUM READ FUNCTIONS
* <b>PDFium - PDFium Get Pages</b>: It will give a Map of (Key: Texture2D, Value: Width and Height) pages as textures.
* <b>PDFium - PDFium Get All Texts</b>: It will get all texts of all pages.
* <b>PDFium - PDFium Get Texts</b>: It will get all texts of target page with their styles.
	* Transform (Location, size): Some editors use "1" point for font size and change text object's size with transform matrixes. So, if you want to re-create that PDF file with only font size, you will get very small textures.
	* Font Size
	* Font Name
	* Font Color
	* Runtime Font Object : It is a container of UFont pointer. Because we have to release it and UFontFace when we want to close game.

* <b>PDFium - Select Text</b>: You can think this as a somekind of text selection. It will get text/strings in a rectangle area.
	* You have to define that rectangle's size and location according with PDF's size and location.
	* Don't use your cursor location directly.
	* ``Sample math: (Cursor Location - PDF Location at screen) * Sampling (value which you used on Get Pages)``

* <b>PDFium - Get Links</b>: It will give all <b>weblinks</b> in specified page. It won't give internal bridges. It will give weblinks. This is limitation of PDFium.
* <b>PDFium - Get Images</b>: It will extract all images as a ``TMap<UTexture2D, FVector2D> (FVector2D is images location)``. But images of PDF have to be ``FPDF_PAGEOBJ_IMAGE``
* <b>PDFium - Get Page Count and Sizes</b>: It will give an array of ``FVector2D``. Lenght of array will be equal to page size, each elements contains size of its respective page.

# PDFIUM WRITE FUNCTIONS
* <b>PDFium - Add Pages</b>: It adds pages to target PDF file with defined size. (Array count is page count and each vector element of array is its size)
* <b>PDFium - Load Standart Font</b>: There are 14 standart fonts embedded to PDF libraries according to ``PDF Spec 1.7 page 416``. This function allows to select one them.
* <b>PDFium - Load External Font</b>: You have to use desktop fonts not web fonts. For example, some <b>Google Fonts (such as Google Variant of Roboto) don't work</b>.< /br> You can download compatible fonts from these websites. < /br> https://all-free-download.com/font/ < /br> https://www.fontspace.com/category/truetype < /br> https://www.dafont.com/
* <b>PDFium - Add Texts</b>: It adds text objects to target page. PDFium doesn't support line break at default but we integrated a basic auto wrap feature. Neverthless, you need to be careful about your layout. (When will you create a line break or not)
	* Text Color
	* Position X = Horizontal position.
	* Position Y = Vertical position but it starts from bottom. If your PDF page's height is 800 pixel, top will be 800 and bottom will be 0.
	* Shear X and Y = It strecth your text object. Scaling won't be vectoral. So we don't suggest it.
	* Rotation X and Y = We don't know how it works right now.
	* Border X and Y = It defines vertical and horizontal borders.
	* Use Charcodes: Frontend usage from blueprints are same but at backend it switches between ASCII Decimal based ``FPDFText_SetCharcodes()`` and ``FPDFText_SetText()``. Charcodes has better character support.

* <b>PDFium - Add Image</b>: It expects byte array.
* <b>PDFium - Draw Rectangle</b>: It will draw a rectangle on PDF file with given location, size and color.
* <b>PDFium - Save File</b>
* <b>PDFium - Save Bytes</b>

# INSTRUCTIONS
* Look at plugin's content folder. There are example blueprints to show you how can you use this plugin.
* You have to PDFium Subsystem to Create and Open PDF files.
* Do NOT manually invalidate or set empty document objects !
* If you want to "view" changes which you did on your newly created PDF file in UE5, you need to render it "again" and assign it to your widget or material.

# ROADMAP
* <b>Rotation inputs</b>: Inputs are connected to internal functions but PDFium use a somekind of rotation matrix. So, use it if you know what you are doing. We will create a helper function about this.

# LIMITATION
* Some external web fonts can cause crash. So, it will be good to test your fonts at development stage and give them to your customers.