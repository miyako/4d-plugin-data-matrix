# 4d-plugin-data-matrix

4D implementation of [libdmtx](http://libdmtx.sourceforge.net).

##Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🆗|🆗|


* ~~TODO: The "read" feature is not implemented.~~

Commands
---

```c
// --- data-matrix
DMTX
DMTX_Read_image
```

Example
---

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
$data:=""  //out param (svg source code)

$value:="18000000011522042528120000355250541000002277743896"
$image:=DMTX ($value;DMTX Format SVG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
SET PICTURE TO PASTEBOARD($image)
$image:=$image/$image/$image/$image
$text:=DMTX Read image ($image;$texts)
```
