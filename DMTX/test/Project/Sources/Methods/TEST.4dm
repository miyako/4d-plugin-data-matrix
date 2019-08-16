//%attributes = {}
$moduleSize:=2
$margin:=2
$DPI:=96
$data:=""

$value:="18000000011522042528120000355250541000002277743896"

$image:=DMTX ($value;DMTX Format JPG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
$jpg_size:=Picture size:C356($image)  //1,202 (100%=4,701) (not as small because it is grayscale)
$jpg_base64_size:=Length:C16($data)  //1,626 (100%=6,355)
WRITE PICTURE FILE:C680(System folder:C487(Desktop:K41:16)+"sample_dmtx.jpg";$image;".jpg")

$image:=DMTX ($value;DMTX Format PNG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
$png_size:=Picture size:C356($image)  //314 (very small because we use palette)
$png_base64_length:=Length:C16($data)  //425
WRITE PICTURE FILE:C680(System folder:C487(Desktop:K41:16)+"sample_dmtx.png";$image;".png")

$image:=DMTX ($value;DMTX Format SVG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
$svg_size:=Picture size:C356($image)  //260,594 (very large because it is text) 
$svg_xml_length:=Length:C16($data)  //260,594
WRITE PICTURE FILE:C680(System folder:C487(Desktop:K41:16)+"sample_dmtx.svg";$image;".svg")