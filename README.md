![version](https://img.shields.io/badge/version-17%2B-3E8B93)
![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm%20|%20win-64&color=blue)
[![license](https://img.shields.io/github/license/miyako/4d-plugin-data-matrix)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/4d-plugin-data-matrix/total)

**Note**: for v17 and earlier, move `manifest.json` to `Contents`

# 4d-plugin-data-matrix
Barcode generator based on [libdmtx 0.7.5](https://github.com/dmtx/libdmtx).

### Syntax

[miyako.github.io](https://miyako.github.io/2019/08/17/4d-plugin-data-matrix.html)

### Discussion

It seems the the library built using CMAKE on Mac does not work; using old copy. Passing array is still not thread safe. Must use collection instead (~~**TODO**~~). ``CREATE THUMBNAIL`` via ``PA_ExecuteCommandByID`` seems OK in preemptive mode;, but ``CGImageGetWidth`` crashes. probably because ``PA_CreateNativePictureForScreen`` is not thread-safe. Need to perform offscreen BMP conversion (~~**TODO**~~).

* Read about [Datamatrix](http://jpgraph.net/download/manuals/chunkhtml/ch26.html).

* Uses ``libpng`` or ``libjpeg`` to create native images. ``PNG`` offers the best in terms of size and quality, because the colorspace is ``PNG_COLOR_TYPE_PALETTE``. ``JPEG`` colorspace  is ``JCS_GRAYSCALE``, with quality ``100``, which is not as small as ``PNG``. ``SVG``, by contrast, is very large.

## Examples

```
$moduleSize:=2
$margin:=2
$DPI:=96
$data:=""  //out param (svg source code)

$value:="18000000011522042528120000355250541000002277743896"
$image:=DMTX ($value;DMTX Format PNG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
$image:=$image/$image/$image/$image
$text:=DMTX Read image ($image;$texts)
```

```
$moduleSize:=2
$margin:=2
$DPI:=96
$data:=""

$value:="18000000011522042528120000355250541000002277743896"

$image:=DMTX ($value;DMTX Format JPG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
$jpg_size:=Picture size($image)  //4,701 (not as small because it is grayscale)
$jpg_base64_size:=Length($data)  //6,355
WRITE PICTURE FILE(System folder(Desktop)+"sample_dmtx.jpg";$image;".jpg")

$image:=DMTX ($value;DMTX Format PNG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
$png_size:=Picture size($image)  //314 (very small because we use palette)
$png_base64_length:=Length($data)  //425
WRITE PICTURE FILE(System folder(Desktop)+"sample_dmtx.png";$image;".png")

$image:=DMTX ($value;DMTX Format SVG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
$svg_size:=Picture size($image)  //260,594 (very large because it is text) 
$svg_xml_length:=Length($data)  //260,594
WRITE PICTURE FILE(System folder(Desktop)+"sample_dmtx.svg";$image;".svg")
```
