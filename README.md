![version](https://img.shields.io/badge/version-17%2B-3E8B93)
![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm%20|%20win-64&color=blue)
[![license](https://img.shields.io/github/license/miyako/4d-plugin-data-matrix)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/4d-plugin-data-matrix/total)

# 4d-plugin-data-matrix

Generate and read [Data Matrix](https://en.wikipedia.org/wiki/Data_Matrix) 2D barcodes in 4D. The plugin wraps [libdmtx 0.7.5](https://github.com/dmtx/libdmtx) (statically linked). [DMTX](#dmtx) encodes a text into a `Picture` in JPEG, PNG or SVG format, and also gives you the encoded file as text (SVG source or base64). [DMTX Read image](#dmtx-read-image) and [DMTX Read images](#dmtx-read-images) find and decode symbols in a `Picture`. PNG output uses `libpng` (1-bit, 2-colour palette), JPEG output uses `libjpeg` (grayscale).

| Command | Returns | Purpose |
|---|---|---|
| [DMTX](#dmtx) | `Picture` | Encode a text as a Data Matrix symbol (JPEG, PNG or SVG) |
| [DMTX Read image](#dmtx-read-image) | `Text` | Decode the first Data Matrix symbol found in a picture |
| [DMTX Read images](#dmtx-read-images) | `Collection` | Decode every Data Matrix symbol found in a picture |

**Platforms:** macOS (Apple Silicon and Intel), Windows 64-bit. 4D v17 or later.

Releases: [github.com/miyako/4d-plugin-data-matrix/releases](https://github.com/miyako/4d-plugin-data-matrix/releases) · Original article: [miyako.github.io](https://miyako.github.io/2019/08/17/4d-plugin-data-matrix.html)

---

## Requirements & platform notes

- **Failure is silent.** No command raises a 4D error. [DMTX](#dmtx) returns an empty picture (and sets `data` to `""`) when it can't encode; the read commands return `""` or an empty collection when they find nothing. Test the result with `Picture size` or `Length`.
- **All eight parameters of [DMTX](#dmtx) are read on every call.** There is no short form.
- **The commands are thread-safe** (`threadSafe: true` in the manifest), so you can call them from preemptive processes. The plugin's own test methods are declared `"preemptive":"capable"`.
- **Decoding blocks the calling process** while libdmtx scans the picture. Each search is limited to about one second (see [DMTX Read images](#dmtx-read-images) for how that adds up). In a cooperative process the 4D interface doesn't respond during that time.
- **Sizes are in pixels, and the module size counts twice** (see [DMTX](#dmtx)): one module of the symbol is `moduleSize × moduleSize` pixels wide, not `moduleSize`.
- The text is encoded as UTF-8 bytes, and the read commands interpret the decoded bytes as UTF-8.
- The universal macOS binary (arm64 + x86_64) built by this repository's release workflow requires macOS 11.5 or later.

---

## DMTX

### Syntax

```
DMTX ( value ; format ; scheme ; moduleSize ; symbolSize ; margin ; dpi ; data ) -> Picture
```

| Parameter | Type | Description |
|---|---|---|
| `value` | Text | The text to encode, converted to UTF-8 bytes. An empty text can't be encoded. |
| `format` | Longint | `DMTX Format JPG` (0), `DMTX Format SVG` (1) or `DMTX Format PNG` (2). Any other value produces JPEG. |
| `scheme` | Longint | Encodation scheme, a `DMTX Scheme` constant (see below). |
| `moduleSize` | Longint | Size factor, 1 to 100. `0` is treated as `1`. See the description for how it scales the output. |
| `symbolSize` | Longint | Symbol size, a `DMTX Symbol Size` constant (see below), or one of the automatic values. |
| `margin` | Longint | Quiet zone factor, 0 to 1000. See the description. |
| `dpi` | Longint | Resolution written in the file's metadata. `0` or a negative value means `72`. Values above `65535` are clamped to `65535`. |
| `data` | Text | Variable that receives the encoded file as text: the SVG source for SVG, base64 of the file for PNG and JPEG. `""` on failure. |
| Result | Picture | The barcode, or an empty picture on failure. |

### Description

`DMTX` encodes `value` with libdmtx and writes the symbol as a picture in the format you choose:

- **JPEG**: grayscale (`JCS_GRAYSCALE`), quality 100, progressive, with `dpi` as the JFIF density.
- **PNG**: 1 bit per pixel with a black and white palette (`PNG_COLOR_TYPE_PALETTE`), with `dpi` stored in the `pHYs` chunk. This is by far the smallest of the three.
- **SVG**: one `<rect>` per pixel of the symbol, black or white, with `shape-rendering="crispEdges"` and `dpi` in a `ns4d:DPI` attribute on the root element. It is large, but it is plain text you can post-process in 4D.

**Size of the output.** libdmtx first draws each module as `moduleSize` pixels and adds `margin` pixels of quiet zone on each side. The plugin then scales that drawing by `moduleSize` again and adds `margin × moduleSize` more pixels on each side. So:

- one module is `moduleSize × moduleSize` pixels wide;
- the quiet zone on each side is `2 × margin × moduleSize` pixels;
- the picture is `columns × moduleSize² + 4 × margin × moduleSize` pixels wide, and likewise high with rows.

For example, `DMTX Symbol 16x48` (16 rows, 48 columns) with `moduleSize` 2 and `margin` 2 gives a 208 × 80 pixel picture.

**Limits.** From the version after 2.7.0, `DMTX` returns an empty picture when `moduleSize` is negative or above 100, when `margin` is negative or above 1000, or when the result would be more than 32767 pixels wide or high. Earlier versions passed such values on unchecked, which could crash 4D.

**The `data` parameter** is filled from the version after 2.7.0. In 2.x versions up to 2.7.0 it was computed but never returned, so the variable kept its previous value.

**Encodation schemes** (`DMTX Scheme` theme):

| Constant | Value | Comment |
|---|---|---|
| `DMTX Scheme ASCII` | 0 | General purpose; digit pairs take one codeword. |
| `DMTX Scheme C40` | 1 | Optimised for upper-case letters and digits. |
| `DMTX Scheme Text` | 2 | Optimised for lower-case letters and digits. |
| `DMTX Scheme X12` | 3 | ANSI X12 character set. |
| `DMTX Scheme Edifact` | 4 | ASCII 32 to 94. |
| `DMTX Scheme Base256` | 5 | Arbitrary bytes. |

libdmtx also defines `-1` (automatic, best encodation) in its header. There is no constant for it; it worked in a test with the bundled libdmtx. `-2` (automatic, fast) is not implemented by libdmtx 0.7.5 and always fails.

**Symbol sizes** (`DMTX Symbol Size` theme), named rows × columns:

| Constants | Values |
|---|---|
| Square: `DMTX Symbol 10x10`, `12x12`, `14x14`, `16x16`, `18x18`, `20x20`, `22x22`, `24x24`, `26x26`, `32x32`, `36x36`, `40x40`, `44x44`, `48x48`, `52x52`, `64x64`, `72x72`, `80x80`, `88x88`, `96x96`, `104x104`, `120x120`, `132x132`, `144x144` | 0 to 23 |
| Rectangular: `DMTX Symbol 8x18`, `8x32`, `12x26`, `12x36`, `16x36`, `16x48` | 24 to 29 |

If `value` doesn't fit in the size you ask for, encoding fails and you get an empty picture. To let libdmtx choose, pass one of the automatic values from its header (no constants are provided): `-2` smallest square symbol, `-3` smallest rectangular symbol, `-1` automatic shape (it chose a square symbol in a test). Other values, including `-4`, fail.

### Example

From the plugin's own test method (`TEST.4dm`):

```4d
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
```

The plugin uses JPEG quality 100, so the `100%` figures in the JPEG comments are the relevant ones.

Let libdmtx pick the smallest square symbol, and check for failure:

```4d
C_PICTURE($image)
C_TEXT($data)

$image:=DMTX("https://example.com/item/42";DMTX Format PNG;DMTX Scheme ASCII;3;-2;1;144;$data)

If (Picture size($image)=0)
	ALERT("Could not encode the value")
End if
```

Use the SVG source directly, for example in an HTML page:

```4d
C_TEXT($svg;$html)

$image:=DMTX($value;DMTX Format SVG;DMTX Scheme ASCII;2;DMTX Symbol 16x48;2;96;$svg)
$html:="<html><body>"+$svg+"</body></html>"
```

---

## DMTX Read image

### Syntax

```
DMTX Read image ( image ) -> Text
```

| Parameter | Type | Description |
|---|---|---|
| `image` | Picture | Picture that contains a Data Matrix symbol. |
| Result | Text | The decoded text of the first symbol found, or `""`. |

### Description

The command first reduces `image` to a thumbnail of at most 512 × 512 pixels (4D's `CREATE THUMBNAIL`, scaled to fit, proportional), converts it to grayscale, and asks libdmtx for the first symbol it can find within about one second. The decoded bytes are returned as UTF-8 text.

- **On macOS**, the thumbnail is drawn into a 512 × 512 buffer, so a non-square picture is stretched to a square. Data Matrix decoding tolerates that in principle; the plugin doesn't test it.
- **On Windows**, the thumbnail's pixels are read as they are; the part of the 512 × 512 buffer outside a non-square thumbnail stays black (inferred from the source).
- Because of the reduction to 512 pixels, a symbol that is small relative to a large picture may lose too much detail to be found (inferred). Crop the picture around the symbol first if decoding fails.

An empty picture, or picture data that can't be turned into an image, returns `""`. Before the version after 2.7.0, such a picture could crash 4D on macOS in compiled mode.

### Example

From the plugin's own test method (`TEST_2.4dm`), which stacks four copies of a symbol with the picture operator `/`:

```4d
$moduleSize:=2
$margin:=2
$DPI:=96
$data:=""  //out param (svg source code)


$value:="18000000011522042528120000355250541000002277743896"
$image:=DMTX ($value;\
DMTX Format PNG;\
DMTX Scheme ASCII;\
$moduleSize;\
DMTX Symbol 16x48;\
$margin;\
$DPI;\
$data)

$image:=$image/$image/$image/$image
$text:=DMTX Read image ($image)

ALERT:C41($text)
```

Check the result:

```4d
C_TEXT($text)

$text:=DMTX Read image($image)
If ($text="")
	ALERT("No Data Matrix symbol found")
End if
```

---

## DMTX Read images

### Syntax

```
DMTX Read images ( image ) -> Collection
```

| Parameter | Type | Description |
|---|---|---|
| `image` | Picture | Picture that contains one or more Data Matrix symbols. |
| Result | Collection | One `Text` element per symbol decoded, in the order libdmtx finds them. Empty if none is found. |

### Description

Same preparation as [DMTX Read image](#dmtx-read-image) (thumbnail of at most 512 × 512 pixels, grayscale). Then the command keeps asking libdmtx for the next symbol until none is left. Each search gets its own one-second limit, which restarts after every region found, so the total time grows with the number of candidate regions in the picture. A region that libdmtx locates but can't decode is skipped.

### Example

From the plugin's own test method (`TEST_2.4dm`), continuing the example above:

```4d
C_COLLECTION:C1488($texts)
$texts:=DMTX Read images ($image)

For each ($text;$texts)
	ALERT:C41($text)
End for each 
```

---

## Error handling & troubleshooting

- **Empty picture from DMTX**: the text doesn't fit the requested `symbolSize` (try a larger size, or `-2`/`-3`), `value` is empty, the scheme can't encode the characters, or `moduleSize`/`margin` are out of range or make the picture larger than 32767 pixels. `data` is `""` in all these cases.
- **The barcode is much bigger than expected**: `moduleSize` applies twice, so a module is `moduleSize²` pixels. Small values (1 to 4) are usually enough.
- **`data` stays empty with 2.7.0 or earlier**: those versions don't return the 8th parameter. Update the plugin, or convert the picture to a BLOB and encode it yourself with 4D's own commands.
- **Read returns `""` or an empty collection**: no symbol was found within the time limit. Crop the area around the symbol or improve the contrast; the picture is reduced to 512 × 512 pixels before decoding.
- **The 4D interface stops responding while reading**: decoding runs in the calling process for up to about a second per search. Call the read commands from a worker or a preemptive process.
- **Non-ASCII text**: it is encoded as UTF-8 bytes. Other Data Matrix readers may interpret the bytes as ISO 8859-1 unless they assume UTF-8.

---

## Quick reference

```4d
C_PICTURE($image)
C_TEXT($data;$text)
C_COLLECTION($texts)

// encode: PNG, module 2, quiet zone 2, 96 dpi, smallest square symbol; $data = base64 PNG
$image:=DMTX("123456";DMTX Format PNG;DMTX Scheme ASCII;2;-2;2;96;$data)

// SVG; $data = SVG source
$image:=DMTX("123456";DMTX Format SVG;DMTX Scheme ASCII;2;DMTX Symbol 16x48;2;96;$data)

// decode
$text:=DMTX Read image($image)
$texts:=DMTX Read images($image)
```
