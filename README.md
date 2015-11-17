# 4d-plugin-data-matrix

About
---
4D implementation of [libdmtx](http://libdmtx.sourceforge.net).

###TODO

The "read" feature is not implemented.

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
