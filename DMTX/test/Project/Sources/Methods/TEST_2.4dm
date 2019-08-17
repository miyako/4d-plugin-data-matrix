//%attributes = {"preemptive":"capable"}
$moduleSize:=2
$margin:=2
$DPI:=96
$data:=""  //out param (svg source code)


$value:="18000000011522042528120000355250541000002277743896"
$image:=DMTX ($value;DMTX Format PNG;DMTX Scheme ASCII;$moduleSize;DMTX Symbol 16x48;$margin;$DPI;$data)
$image:=$image/$image/$image/$image
$text:=DMTX Read image ($image)

ALERT:C41($text)

C_COLLECTION:C1488($texts)
$texts:=DMTX Read images ($image)

For each ($text;$texts)
	ALERT:C41($text)
End for each 