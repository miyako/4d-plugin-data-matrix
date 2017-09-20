# 4d-plugin-data-matrix

4D implementation of [libdmtx 0.7.5](https://github.com/dmtx/libdmtx).

### Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|

### Version

<img src="https://cloud.githubusercontent.com/assets/1725068/18940649/21945000-8645-11e6-86ed-4a0f800e5a73.png" width="32" height="32" /> <img src="https://cloud.githubusercontent.com/assets/1725068/18940648/2192ddba-8645-11e6-864d-6d5692d55717.png" width="32" height="32" />

Read about [Datamatrix](http://jpgraph.net/download/manuals/chunkhtml/ch26.html)

### New

``JPG`` support.

### Features

Uses ``libpng`` or ``libjpeg`` to create native images. ``PNG`` offers the best in terms of size and quality, because the colorspace is ``PNG_COLOR_TYPE_PALETTE``. ``JPEG`` colorspace  is ``JCS_GRAYSCALE``, with quality ``0``, but still not as small as ``PNG``.``SVG``, by contrast, is very large.

## Examples

```
$moduleSize:=2
$margin:=2
$DPI:=96
$data:=""  //out param (svg source code)

  //http://jpgraph.net/download/manuals/chunkhtml/ch26.html
If (Shift down)
$image:=DMTX ("1"*98;DMTX Format PNG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
Else 
$image:=DMTX ("1"*98;DMTX Format SVG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
  //rasterize
CONVERT PICTURE($image;".png")
End if 

SET PICTURE TO PASTEBOARD($image)
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
