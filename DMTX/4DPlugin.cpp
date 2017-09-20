/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : data-matrix
 #	author : miyako
 #	2015/11/17
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

void PluginMain(PA_long32 selector, PA_PluginParameters params)
{
	try
	{
		PA_long32 pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (PA_long32 pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
// --- data-matrix

		case 1 :
			DMTX(pResult, pParams);
			break;

		case 2 :
			DMTX_Read_image(pResult, pParams);
			break;

	}
}

#pragma mark -

namespace barcode
{
	PA_Picture reduceImage(PackagePtr pParams, uint32_t index, int size = 512)
	{
		PA_Picture p = *(PA_Picture *)(pParams[index - 1]);
		//create thumbnail
		PA_Variable args[5];
		args[0] = PA_CreateVariable(eVK_Picture);
		args[1] = PA_CreateVariable(eVK_Picture);
		args[2] = PA_CreateVariable(eVK_Longint);
		args[3] = PA_CreateVariable(eVK_Longint);
		PA_SetPictureVariable(&args[0], p);
		PA_SetLongintVariable(&args[2], size);
		PA_SetLongintVariable(&args[3], size);
		PA_SetLongintVariable(&args[4], Scaled_to_fit_proportional);
		PA_ExecuteCommandByID(CREATE_THUMBNAIL, args, 5);
		p = PA_GetPictureVariable(args[1]);
		PA_SetPictureVariable(&args[0], NULL);
		PA_SetPictureVariable(&args[1], NULL);
		PA_ClearVariable(&args[0]);
		PA_ClearVariable(&args[1]);
		PA_ClearVariable(&args[2]);
		PA_ClearVariable(&args[3]);
		PA_ClearVariable(&args[4]);
		return p;
	}
	
	PA_long32 imageSize(PA_Picture p)
	{
		//create thumbnail
		PA_Variable args[1];
		args[0] = PA_CreateVariable(eVK_Picture);
		PA_SetPictureVariable(&args[0], p);
		PA_long32 size = PA_GetLongintVariable(PA_ExecuteCommandByID(Picture_size, args, 1));
		PA_SetPictureVariable(&args[0], NULL);
		PA_ClearVariable(&args[0]);
		
		return size;
	}
	
	Image createImage(PA_Picture p)
	{
		return (Image)PA_CreateNativePictureForScreen(p);
	}
	void disposeImage(Image image)
	{
		if(image)
		{
#if VERSIONMAC
			CGImageRelease(image);
#else
			image->Dispose();
#endif
		}
	}
	void getBuf(Image image, Buf &buf, int *width, int *height)
	{
#if VERSIONMAC
		size_t w = CGImageGetWidth(image);
		size_t h = CGImageGetHeight(image);
		*width = (int)w;
		*height = (int)h;
		size_t size = w * h;
		buf.resize(size);
		
		CGContextRef ctx = NULL;
		CGColorSpaceRef colorSpace = NULL;
		
		size_t bitmapBytesPerRow   = (w * 4);
		size_t bitmapByteCount     = (bitmapBytesPerRow * h);
		
		std::vector<uint8_t> bitmapData(bitmapByteCount);
		
		colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
		
		if (colorSpace){
			
			ctx = CGBitmapContextCreate ((void *)&bitmapData[0],
																	 w,
																	 h,
																	 8,      // bits per component
																	 bitmapBytesPerRow,
																	 colorSpace,
																	 kCGImageAlphaPremultipliedFirst);
			
			CGColorSpaceRelease(colorSpace);
			
		}
		
		if (ctx){
			
			CGRect rect = {{0,0},{w,h}};
			
			CGContextDrawImage(ctx, rect, image);
			
			size_t *pixels = (size_t *)CGBitmapContextGetData (ctx);
			
			size_t pixel, y8;
			size_t i = 0;
#if __LP64__
			BOOL alt = false;
			size_t j = 0;
#endif
			for(size_t y = 0; y < h; y++) {
				for(size_t x = 0; x < w; x++) {
#if __LP64__
					if(!alt)
					{
						pixel = pixels[j];
						y8 = (pixel >> 24) & 0xFF;
					}
					else
					{
						pixel = pixels[j];
						y8 = (pixel >> 56) & 0xFF;
						j++;
					}
					buf[i] = y8;
					i++;
					alt = !alt;
#else
					pixel = pixels[i];
					y8 = (pixel >> 24) & 0xFF;
					buf[i] = y8;
					i++;
#endif
				}
			}
			CGContextRelease(ctx);
		}
		
#else
		size_t w = image->GetWidth();
		size_t h = image->GetHeight();
		*width = w;
		*height = h;
		size_t size = w * h;
		buf.resize(size);
		
		uint32_t y8;
		size_t i = 0;
		
		for(size_t y = 0; y < h; y++) {
			for(size_t x = 0; x < w; x++) {
				
				Gdiplus::Color c;
				image->GetPixel(x,y,&c);
				
				y8 = c.GetR();
				buf[i] = y8;
				i++;
			}
		}
#endif
	}
}

// ---------------------------------- data-matrix ---------------------------------

#pragma mark SVG

void toSVG(DmtxEncode *enc, int margin, int size, int dpi, C_TEXT &_dump, sLONG_PTR *pResult)
{
	if(enc)
	{
		if(enc->image)
		{
			size_t width = dmtxImageGetProp(enc->image, DmtxPropWidth);
			size_t height = dmtxImageGetProp(enc->image, DmtxPropHeight);
			size_t bytesPerPixel = dmtxImageGetProp(enc->image, DmtxPropBytesPerPixel);
			
			margin = margin * size;
			char _size[33];
			sprintf(_size, "%d", size);
			
			char _width[33];
			sprintf(_width, "%d", (int)(margin + (width * size) + margin));

			char _height[33];
			sprintf(_height, "%d", (int)(margin + (height * size) + margin));
			
			char _dpi[33];
			sprintf(_dpi, "%d", dpi);
			
			char _margin[33];
			sprintf(_margin, "%d", margin);
			
			char _x[33];
			char _y[33];
			
			std::string svg;
			
			svg += "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
			svg += "<svg width=\"";
			svg += _width;
			svg += "\" height=\"";
			svg += _height;
			svg += "\" viewBox=\"0 0 ";
			svg += _width;
			svg += " ";
			svg += _height;
			svg += "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:ns4d=\"http://www.4d.com\" ns4d:DPI=\"";
			svg += _dpi;
			svg += "\">\n";
//			svg += "<g shape-rendering=\"auto\" stroke-width=\"0\" stroke=\"none\" transform=\"translate(";
									svg += "<g shape-rendering=\"crispEdges\" stroke-width=\"0\" stroke=\"none\" transform=\"translate(";
			svg += _margin;
			svg += ",";
			svg += _margin;
			svg += ")\">\n";
			
			unsigned char *p;
			int x, y = 0, i, j;
			
			p = enc->image->pxl;
			
			for(i = 0; i < height; ++i)
			{
				//breathe every 8KO
				if((i % 0x2000)==0) {
					PA_YieldAbsolute();
				}
				x = 0;
				sprintf(_y, "%d", y);
				
				for(j = 0; j < width; ++j)
				{
					sprintf(_x, "%d", x);
					
					if(*p&1)
					{
						svg += "<rect fill=\"white\" stroke=\"white\" height=\"";
						svg += _size;
						svg += "\" width=\"";
						svg += _size;
						svg += "\" ";
					}else{
						svg += "<rect fill=\"black\" stroke=\"black\" height=\"";
						svg += _size;
						svg += "\" width=\"";
						svg += _size;
						svg += "\" ";
					}
					
					svg += "x=\"";
					svg += _x;
					svg += "\" ";
					svg += "y=\"";
					svg += _y;
					svg += "\" />\n";
					
					p+=bytesPerPixel;
					x = x + size;
					
				}
				y = y + size;
			}
			svg += "</g>\n";
			svg += "</svg>\n";
			
			CUTF8String _svg((const uint8_t *)svg.c_str());
			
			_dump.setUTF8String(&_svg);
			
			PA_Picture picture = PA_CreatePicture((void *)svg.c_str(), (PA_long32)svg.size());
			*(PA_Picture*) pResult = picture;
		}
	}
}

#pragma mark JPG

namespace JPG
{
	
	struct my_error_mgr {
		struct jpeg_error_mgr pub;
		jmp_buf setjmp_buffer;
		int     jump_set;
	};
	
	typedef struct my_error_mgr *my_error_ptr;
	
	typedef struct {
		struct jpeg_destination_mgr pub; /* public fields */
		
		unsigned char **buf_ptr;
		size_t *bufsize_ptr;
		size_t incsize;
		
		unsigned char *buf;
		size_t bufsize;
		
	} jpeg_memory_destination_mgr;
	
	typedef jpeg_memory_destination_mgr *jpeg_memory_destination_ptr;
	
	void my_error_exit(j_common_ptr cinfo)
	{
		
	}
	
	void my_output_message (j_common_ptr cinfo)
	{
		
	}
	
	boolean jpeg_memory_empty_output_buffer (j_compress_ptr cinfo)
	{
		jpeg_memory_destination_ptr dest = (jpeg_memory_destination_ptr) cinfo->dest;
		unsigned char *newbuf;
		/* abort if incsize is 0 (no expansion of buffer allowed) */
		if (dest->incsize == 0) return _false;
		/* otherwise, try expanding buffer... */
		newbuf = (unsigned char *)realloc(dest->buf,dest->bufsize + dest->incsize);
		if (!newbuf) return _false;
		dest->pub.next_output_byte = newbuf + dest->bufsize;
		dest->pub.free_in_buffer = dest->incsize;
		dest->buf = newbuf;
		dest->bufsize += dest->incsize;
		dest->incsize *= 2;
		return _true;
	}
	
	void jpeg_memory_init_destination (j_compress_ptr cinfo)
	{
		jpeg_memory_destination_ptr dest = (jpeg_memory_destination_ptr) cinfo->dest;
		dest->pub.next_output_byte = dest->buf;
		dest->pub.free_in_buffer = dest->bufsize;
	}
	
	void jpeg_memory_term_destination (j_compress_ptr cinfo)
	{
		jpeg_memory_destination_ptr dest = (jpeg_memory_destination_ptr) cinfo->dest;
		*dest->buf_ptr = dest->buf;
		*dest->bufsize_ptr = dest->bufsize - dest->pub.free_in_buffer;
	}
	
	void jpeg_memory_dest (j_compress_ptr cinfo, unsigned char **bufptr, size_t *bufsizeptr, size_t incsize)
	{
		jpeg_memory_destination_ptr dest;
		
		/* allocate destination manager object for compress object, if needed */
		if (!cinfo->dest) {
			cinfo->dest = (struct jpeg_destination_mgr *)
			(*cinfo->mem->alloc_small) ( (j_common_ptr) cinfo,
																	JPOOL_PERMANENT,
																	sizeof(jpeg_memory_destination_mgr) );
		}
		
		dest = (jpeg_memory_destination_ptr)cinfo->dest;
		
		dest->buf_ptr = bufptr;
		dest->buf = *bufptr;
		dest->bufsize_ptr = bufsizeptr;
		dest->bufsize = *bufsizeptr;
		dest->incsize = incsize;
		
		dest->pub.init_destination = jpeg_memory_init_destination;
		dest->pub.empty_output_buffer = jpeg_memory_empty_output_buffer;
		dest->pub.term_destination = jpeg_memory_term_destination;
	}
}
	
	void toJPG(DmtxEncode *enc, int margin, int size, int dpi, C_TEXT &_dump, sLONG_PTR *pResult)
{
	if(enc)
	{
		if(enc->image)
		{
			size_t width = dmtxImageGetProp(enc->image, DmtxPropWidth);
			size_t height = dmtxImageGetProp(enc->image, DmtxPropHeight);
			size_t bytesPerPixel = dmtxImageGetProp(enc->image, DmtxPropBytesPerPixel);
			
			struct jpeg_compress_struct cinfo;
			struct JPG::my_error_mgr jcerr;

			/* initialize compression object */
			cinfo.err = jpeg_std_error(&jcerr.pub);
			jpeg_create_compress(&cinfo);
			jcerr.pub.error_exit = JPG::my_error_exit;
			jcerr.pub.output_message = JPG::my_output_message;
			jcerr.jump_set = 0;
			
			if (setjmp(jcerr.setjmp_buffer))
			{
				/* error handler for compress failures */
				jpeg_abort_compress(&cinfo);
				jcerr.jump_set=0;
			} else {
				jcerr.jump_set=1;
			}
			
			unsigned char *row, *p, *q;
			int x, y, xx, yy, bit;
			int realwidth, realHeight;
			
			realwidth = (int)((width + margin * 2) * size);
			realHeight = (int)((height + margin * 2) * size);
			size_t outbuffersize = (realwidth * realHeight * bytesPerPixel) + 32768;
			unsigned char *outbuffer = (unsigned char *)malloc(outbuffersize);
			
			if(outbuffer)
			{
				JPG::jpeg_memory_dest(&cinfo, &outbuffer, &outbuffersize, 65536);
				
				cinfo.in_color_space = JCS_GRAYSCALE;
				cinfo.input_components = 1;
				cinfo.image_width = (JDIMENSION)realwidth;
				cinfo.image_height = (JDIMENSION)realHeight;
				jpeg_set_defaults(&cinfo);
				//jpeg_set_defaults overrides density
				cinfo.density_unit = 1;   /* Inches */
				cinfo.X_density = dpi;
				cinfo.Y_density = dpi;
				cinfo.write_JFIF_header = _true;
//				jpeg_set_quality(&cinfo, 100, _true);
				jpeg_set_quality(&cinfo, 0, _true);
				jpeg_simple_progression(&cinfo);
				cinfo.optimize_coding = _true;
				jpeg_start_compress(&cinfo, _true);
				
				int bytes_per_line = (int)(bytesPerPixel * realwidth);
				JSAMPLE *jpegbuf = (JSAMPLE *)malloc(bytes_per_line);
				
				if(jpegbuf)
				{
					row = (unsigned char *)malloc((realwidth + 7) / 8);
					if(row)
					{
						JSAMPLE *buf8 = (JSAMPLE *)malloc(bytes_per_line * 8);
						if(buf8)
						{
							/* top margin */
							memset(buf8, 0xff, bytes_per_line * 8);
							for(int m=0; m<(margin * size); m++) {
								jpeg_write_scanlines(&cinfo, &buf8, 1);
							}
							
							/* data */
							p = enc->image->pxl;
							for(y=0; y<height; y++) {
								if((y % 0x2000)==0) {
									PA_YieldAbsolute();
								}
								bit = 7;
								memset(row, 0xff, (realwidth + 7) / 8);
								q = row;
								q += margin * size / 8;
								bit = 7 - (margin * size % 8);
								for(x=0; x<width; x++) {
									for(xx=0; xx<size; xx++) {
										*q ^= (!(*p & 1)) << bit;
										bit--;
										if(bit < 0) {
											q++;
											bit = 7;
										}
									}
									p+=bytesPerPixel;
								}
								
								for(int col1 = 0; col1 < bytes_per_line; col1++)
								{
									for(int col8 = 0; col8 < 8; col8++)
									{
										buf8[col1 * 8 + col8] = row[col1] & (1 << (8 - col8 - 1)) ? 0xff : 0;
									}
								}
								
								for(yy=0; yy<size; yy++) {
									jpeg_write_scanlines(&cinfo, &buf8, 1);
								}
							}
							/* bottom margin */
							memset(buf8, 0, bytes_per_line * 8);
							for(int m=0; m<(margin * size); m++) {
								jpeg_write_scanlines(&cinfo, &buf8, 1);
							}
							free(buf8);
						}//buf8
						free(row);
					}//row
					
					free(jpegbuf);
				}//jpegbuf
				jpeg_finish_compress(&cinfo);
				jpeg_destroy_compress(&cinfo);
				
				PA_Picture picture = PA_CreatePicture((void *)outbuffer, (PA_long32)outbuffersize);
				*(PA_Picture*) pResult = picture;

				C_BLOB jpg;
				jpg.setBytes((const uint8_t *)outbuffer, (unsigned int)barcode::imageSize(picture));
				jpg.toB64Text(&_dump);

				free(outbuffer);
			}//outbuffer
		}
	}
}

#pragma mark PNG

void toPNG(DmtxEncode *enc, int margin, int size, int dpi, C_TEXT &_dump, sLONG_PTR *pResult)
{
	if(enc)
	{
		if(enc->image)
		{
			size_t width = dmtxImageGetProp(enc->image, DmtxPropWidth);
			size_t height = dmtxImageGetProp(enc->image, DmtxPropHeight);
			size_t bytesPerPixel = dmtxImageGetProp(enc->image, DmtxPropBytesPerPixel);
			
			unsigned int fg_color[4] = {0, 0, 0, 255};
			unsigned int bg_color[4] = {255, 255, 255, 255};
			
			png_structp png_ptr;
			png_infop info_ptr;
			png_colorp palette;
			png_byte alpha_values[2];
			unsigned char *row, *p, *q;
			int x, y, xx, yy, bit;
			int realwidth, realHeight;
			
			realwidth = (int)((width + margin * 2) * size);
			realHeight = (int)((height + margin * 2) * size);
			row = (unsigned char *)malloc((realwidth + 7) / 8);
			if(row != NULL) {
				
				png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
				if(png_ptr != NULL) {
					
					info_ptr = png_create_info_struct(png_ptr);
					if(info_ptr != NULL) {
						
						if(setjmp(png_jmpbuf(png_ptr))) {
							png_destroy_write_struct(&png_ptr, &info_ptr);
						}else{
							
							palette = (png_colorp) malloc(sizeof(png_color) * 2);
							if(palette != NULL) {
								
								palette[0].red   = fg_color[0];
								palette[0].green = fg_color[1];
								palette[0].blue  = fg_color[2];
								palette[1].red   = bg_color[0];
								palette[1].green = bg_color[1];
								palette[1].blue  = bg_color[2];
								alpha_values[0] = fg_color[3];
								alpha_values[1] = bg_color[3];
								png_set_PLTE(png_ptr, info_ptr, palette, 2);
								png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);
								
								C_BLOB png;
								
								png_set_write_fn(png_ptr, (png_voidp)&png, write_data_fn, output_flush_fn);
								
								png_set_IHDR(png_ptr, info_ptr,
														 realwidth, realHeight,
														 1,
														 PNG_COLOR_TYPE_PALETTE,
														 PNG_INTERLACE_NONE,
														 PNG_COMPRESSION_TYPE_DEFAULT,
														 PNG_FILTER_TYPE_DEFAULT);
								png_set_pHYs(png_ptr, info_ptr,
														 dpi * INCHES_PER_METER,
														 dpi * INCHES_PER_METER,
														 PNG_RESOLUTION_METER);
								
								png_write_info(png_ptr, info_ptr);
								
								/* top margin */
								memset(row, 0xff, (realwidth + 7) / 8);
								for(y=0; y<margin * size; y++) {
									png_write_row(png_ptr, row);
								}
								
								/* data */
								p = enc->image->pxl;
								for(y=0; y<height; y++) {
									if((y % 0x2000)==0) {
										PA_YieldAbsolute();
									}
									bit = 7;
									memset(row, 0xff, (realwidth + 7) / 8);
									q = row;
									q += margin * size / 8;
									bit = 7 - (margin * size % 8);
									for(x=0; x<width; x++) {
										for(xx=0; xx<size; xx++) {
											*q ^= (!(*p & 1)) << bit;
											bit--;
											if(bit < 0) {
												q++;
												bit = 7;
											}
										}
										p+=bytesPerPixel;
									}
									for(yy=0; yy<size; yy++) {
										png_write_row(png_ptr, row);
									}
								}
								
								/* bottom margin */
								memset(row, 0xff, (realwidth + 7) / 8);
								for(y=0; y<margin * size; y++) {
									png_write_row(png_ptr, row);
								}
								
								png_write_end(png_ptr, info_ptr);
								png_destroy_write_struct(&png_ptr, &info_ptr);
								
								free(row);
								free(palette);
								
								PA_Picture picture = PA_CreatePicture((void *)png.getBytesPtr(), png.getBytesLength());
								*(PA_Picture*) pResult = picture;
								
								png.toB64Text(&_dump);
								
							}
						}
					}
				}
			}
		}
	}
}

#pragma mark -

void write_data_fn(png_structp png_ptr, png_bytep buf, png_size_t size)
{
	C_BLOB *blob = (C_BLOB *)png_get_io_ptr(png_ptr);
	blob->addBytes((const uint8_t *)buf, (uint32_t)size);
}

void output_flush_fn(png_structp png_ptr)
{
	
}

#pragma mark -

void DMTX(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT inData;
	C_LONGINT inOutputFormat;
	C_LONGINT inScheme;
	C_LONGINT inModuleSize;
	C_LONGINT inSymbolSize;
	C_LONGINT inMarginSize;
	C_LONGINT inDPI;
	C_TEXT outData;

	inData.fromParamAtIndex(pParams, 1);
	inOutputFormat.fromParamAtIndex(pParams, 2);
	inScheme.fromParamAtIndex(pParams, 3);
	inModuleSize.fromParamAtIndex(pParams, 4);
	inSymbolSize.fromParamAtIndex(pParams, 5);
	inMarginSize.fromParamAtIndex(pParams, 6);
	inDPI.fromParamAtIndex(pParams, 7);
	
	output_type_t type = (output_type_t)inOutputFormat.getIntValue();// == DMTX_OUTPUT_SVG ? DMTX_OUTPUT_SVG : DMTX_OUTPUT_PNG;
	DmtxScheme scheme = (DmtxScheme)inScheme.getIntValue();
	DmtxSymbolSize symbolSize = (DmtxSymbolSize)inSymbolSize.getIntValue();
	unsigned int moduleSize = inModuleSize.getIntValue();
	moduleSize = moduleSize ? moduleSize : 1;
	unsigned int marginSize = inMarginSize.getIntValue();
	int dpi = inDPI.getIntValue(); dpi = dpi > 0 ? dpi : 72;
	
	DmtxEncode *enc = dmtxEncodeCreate();
	dmtxEncodeSetProp(enc, DmtxPropScheme, scheme);
	dmtxEncodeSetProp(enc, DmtxPropModuleSize, moduleSize);
	dmtxEncodeSetProp(enc, DmtxPropMarginSize, marginSize);
	dmtxEncodeSetProp(enc, DmtxPropSizeRequest, symbolSize);
	
	CUTF8String str;
	inData.copyUTF8String(&str);
	int n = (int)str.size();
	unsigned char *s = (unsigned char *)str.c_str();
	
	if(dmtxEncodeDataMatrix(enc, n, s)){
		
		switch(type){
			case DMTX_OUTPUT_PNG:
				toPNG(enc, marginSize, moduleSize, dpi, outData, pResult);
				break;
				
			case DMTX_OUTPUT_SVG:
				toSVG(enc, marginSize, moduleSize, dpi, outData, pResult);
				break;
			default:
				toJPG(enc, marginSize, moduleSize, dpi, outData, pResult);
				break;
		}
		
	}
	
	dmtxEncodeDestroy(&enc);
	
	outData.toParamAtIndex(pParams, 8);
}

void DMTX_Read_image(sLONG_PTR *pResult, PackagePtr pParams)
{
	ARRAY_TEXT values;
	C_TEXT returnValue;
	
	values.setSize(1);
	
	barcode::Image image;
	
	if(1)
	{
		PA_Picture p = barcode::reduceImage(pParams, 1);
		image = barcode::createImage(p);
		PA_DisposePicture(p);//important that we do this if we reduceImage!
	}
	else
	{
		PA_Picture p = *(PA_Picture *)(pParams[0]);
		image = barcode::createImage(p);
	}
	
	barcode::Buf buf;
	int w = 0, h = 0;
	barcode::getBuf(image, buf, &w, &h);
	
	DmtxImage *img = dmtxImageCreate(&buf[0], w, h, DmtxPack8bppK);
	
	if(img)
	{
		DmtxDecode *dec = dmtxDecodeCreate(img, 1);
		if(dec)
		{
			
			DmtxTime timeout = dmtxTimeNow();
			timeout.sec++;
			
			DmtxRegion *reg;
			while((reg = dmtxRegionFindNext(dec, &timeout)) != 0)
			{
				PA_YieldAbsolute();
				DmtxMessage *msg = dmtxDecodeMatrixRegion(dec, reg, DmtxUndefined);
				if(msg)
				{
					values.appendUTF8String(msg->output, (uint32_t)msg->outputSize);
					if(!returnValue.getUTF16Length())
						returnValue.setUTF8String(msg->output, (uint32_t)msg->outputSize);
					dmtxMessageDestroy(&msg);
				}
				dmtxRegionDestroy(&reg);
				timeout = dmtxTimeNow();
				timeout.sec++;
			}
			dmtxDecodeDestroy(&dec);
		}
		dmtxImageDestroy(&img);
	}
	
	barcode::disposeImage(image);
	
	values.toParamAtIndex(pParams, 2);
	returnValue.setReturn(pResult);
}
