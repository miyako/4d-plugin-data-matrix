# 4d-plugin-data-matrix

4D implementation of [libdmtx 0.7.5](https://github.com/dmtx/libdmtx).

### Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|

### Version

<img src="https://cloud.githubusercontent.com/assets/1725068/18940649/21945000-8645-11e6-86ed-4a0f800e5a73.png" width="32" height="32" /> <img src="https://cloud.githubusercontent.com/assets/1725068/18940648/2192ddba-8645-11e6-864d-6d5692d55717.png" width="32" height="32" />

Read about [Datamatrix](http://jpgraph.net/download/manuals/chunkhtml/ch26.html).

### New

``JPG`` support.

### Features

Uses ``libpng`` or ``libjpeg`` to create native images. ``PNG`` offers the best in terms of size and quality, because the colorspace is ``PNG_COLOR_TYPE_PALETTE``. ``JPEG`` colorspace  is ``JCS_GRAYSCALE``, with quality ``100``, which is not as small as ``PNG``. ``SVG``, by contrast, is very large.

---

## Syntax

```
barcode:=DMTX (value;format;scheme;moduleSize;symbolSize;margin;DPI;data)
```

Parameter|Type|Description
------------|------------|----
value|TEXT|value to codify
format|LONGINT|see ``DMTX Export Type`` 
scheme|LONGINT|see ``DMTX Scheme``
moduleSize|LONGINT|size of each block
symbolSize|LONGINT|see ``DMTX Symbol Size``
margin|LONGINT|
DPI|LONGINT|
data|TEXT|base64 encoded data or SVG source (output)
barcode|PICTURE|data matrix (return)

* DMTX Export Type

```
DMTX Format JPG 0
DMTX Format SVG 1
DMTX Format PNG 2
```

* DMTX Scheme

```
DMTX Scheme ASCII 0
DMTX Scheme C40 1
DMTX Scheme Text 2
DMTX Scheme X12 3
DMTX Scheme Edifact 4
DMTX Scheme Base256 5
```

* DMTX Symbol Size

```
DMTX Symbol 10x10 0
DMTX Symbol 12x12 1
DMTX Symbol 14x14 2
DMTX Symbol 16x16 3
DMTX Symbol 18x18 4
DMTX Symbol 20x20 5
DMTX Symbol 22x22 6
DMTX Symbol 24x24 7
DMTX Symbol 26x26 8
DMTX Symbol 32x32 9
DMTX Symbol 36x36 10
DMTX Symbol 40x40 11
DMTX Symbol 44x44 12
DMTX Symbol 48x48 13
DMTX Symbol 52x52 14
DMTX Symbol 64x64 15
DMTX Symbol 72x72 16
DMTX Symbol 80x80 17
DMTX Symbol 88x88 18
DMTX Symbol 96x96 19
DMTX Symbol 104x104 20
DMTX Symbol 120x120 21
DMTX Symbol 132x132 22
DMTX Symbol 144x144 23
DMTX Symbol 8x18 24
DMTX Symbol 8x32 25
DMTX Symbol 12x26 26
DMTX Symbol 12x36 27
DMTX Symbol 16x36 28
DMTX Symbol 16x48 29
```

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
