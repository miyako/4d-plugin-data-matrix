/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.h
 #	source generated by 4D Plugin Wizard
 #	Project : data-matrix
 #	author : miyako
 #	2015/11/17
 #
 # --------------------------------------------------------------------------------*/



// --- data-matrix
void DMTX(sLONG_PTR *pResult, PackagePtr pParams);
void DMTX_Read_image(sLONG_PTR *pResult, PackagePtr pParams);

enum output_type_t
{
	DMTX_OUTPUT_JPG = 0,
	DMTX_OUTPUT_SVG = 1,
	DMTX_OUTPUT_PNG = 2
};

namespace barcode
{
#if VERSIONMAC
	typedef CGImageRef Image;
#else
	typedef Gdiplus::Bitmap *Image;
#endif
	typedef std::vector<uint8_t> Buf;
}

#include "dmtx.h"
#include "png.h"
#include "jpeglib.h"

#define INCHES_PER_METER (100.0/2.54)
#define Scaled_to_fit_proportional (5)
#define CREATE_THUMBNAIL (679)
#define Picture_size (356)

void toSVG(DmtxEncode *enc, int margin, int size, int dpi, C_TEXT &_dump, sLONG_PTR *pResult);
void toPNG(DmtxEncode *enc, int margin, int size, int dpi, C_TEXT &_dump, sLONG_PTR *pResult);

void write_data_fn(png_structp png_ptr, png_bytep buf, png_size_t size);
void output_flush_fn(png_structp png_ptr);
