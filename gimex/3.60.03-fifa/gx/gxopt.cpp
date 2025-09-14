// (c) 1994-2004 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                              GXopt                               */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* History:                                                         */
/*  gxopt.cpp contains all options for gx. Any command line change  */
/*            should be done here, and reflected in the docs and    */
/*            helpstring.                                           */
/*------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include "gxopt.h"
#include "gx.h"
#include "spit.h"
#include "gxfilter.h"

void GXshowabout(GABOUT *about);

//----------------------------------------------
// document everything and create my major
// groups into which all other pages
// will get inserted.


/* \page o_creep creep
     \ingroup Image_Filters
     \par Syntax:
     <i> gx -creep<w>,<h> </i>
     \par
     creeper
     \par Example:
*/


//--------------------------------------------------------------------
// help information for gx. Any change to Spit_opts should be reflected here
// keep in sync do not allow to get out of sync.
char *optionhelp[]=
{
#ifdef GXRT
    "    -ext           select output file format .ext\n",
#else
/*
; ABSTRACT
;
; gx - graphics conversion
;
; DESCRIPTION
;@ */
    "    @file          response file, containing a list of options and files\n",
    "    =file          specify a output file ie source=dest\n"
    "                   %%d in <dest> uses frame index.  %s uses framename\n",
    "    -b0            disable banner\n",
    "    -v<n>          verbose level (aka -verbosity)\n"
    "                   0 no console output\n"
    "                   1 less output\n"
    "                   2 normal output (default)\n"
    "                   3 more (-v)\n"
    "                   4..6 diagnostics\n",
    "    -info<n>       show frame information (before processing)\n"
    "                   -1 use -v verbose level (default)\n"
    "                   0 turn off info option\n"
    "                   1 terse one line information\n"
    "                   2 one line information per frame (normal)\n"
    "                   3 detailed information per frame\n"
    "                   4 full frame information (diagnostic)\n",
    "    -w0            dont output file\n",
    "    -r             recurse subdirectories to find files\n"
    "    -r%ENV%        traverse environment variable ENV to find files\n",
    "    -openas<str>   open as specified file format\n",
#ifndef GCMP
    "    -reset         set to default options\n",
    "    -4             convert to 4 bit indexed\n"
    "    -8             convert to 8 bit indexed\n"
    "    -15            convert to 555 rgb\n"
    "    -16            convert to 565 rgb\n"
    "    -24            convert to 24 rgb\n"
    "    -32            convert to 32 rgb\n"
    "    -5a3           convert to 5a3 argb\n"
    "    -pscolred<n>   use photoshop to colour reduce from 8..32 bpp\n"
    "                   requires photoshop7\n"
    "    -yuy2          convert to YUY2\n"
    "    -a8            convert to alpha8 if supported\n"
    "    -bpp<n>        convert to n bits per pixel\n"
    "    -n<n>          convert to n colours (2..256)\n",
    "    -pack          pack output (default)\n",
    "    -pack0         dont pack output (aka -unpack)\n"
    "    -pack1         RLE pack (for most formats).  (default)\n"
    "    -pack2         refpack (for shape files only)\n"
    "    -pack3         texture compression (for shape files only)\n"
    "    -pack4         iframe texture pack (for shape files only)\n"
    "    -pack-1        auto select based on source\n"
    "    -pack('CODE')  fourcc code for AVI\n"
    "          'VP60'    VP6 codec\n"
    "          'CVID'    Cinepak 8,16,32\n"
    "          'IV50'    Indeo 24\n"
    "          'MSVC'    MS Video 1 8,16\n"
    "          'MP42'    MPeg 8,24,32\n"
    "          'MRLE'    MS RLE 8  -> this codec only supports 8bpp.\n"
    "          'MJPG'    Motion JPeg 24 -> www.jpg.com\n"
    "          'MJ2C'    Motion JPeg2000\n"
    "    -dxt           dxt compress. shortcut for -pack3\n"
    "          -dxt0     disable dxt compression. shortcut for -pack0\n"
    "          -dxt1     dxt1 opaque+transparent only. shortcut for -pack3 -25\n"
    "          -dxt3     dxt3 includes 4 bit alpha. shortcut for -pack3 -28\n"
    "          -dxt5     dxt5 bit interpolated alpha. shortcut for -pack3 -27\n"
    "                    see also: fdxt,gst\n",
    "    -fdxt          fast dxt compress. shortcut for -pack4\n"
    "          -fdxt0    disable dxt compression. shortcut for -pack0\n"
    "          -fdxt1    dxt1 opaque+transparent only. shortcut for -pack4 -25\n"
    "          -fdxt3    dxt3 includes 4 bit alpha. shortcut for -pack4 -28\n"
    "          -fdxt5    dxt5 bit interpolated alpha. shortcut for -pack4 -27\n",
    "    -gst<bw>,<db>  PS2 GS texture compression, can be used with fsh and ssh.\n"
    "                   <bw> - blockwidth when compressing. Accepted: 1,2,4,8\n"
    "                   <db> - detailbpp number of bits in the detail texture\n"
    "                          can be 1 or 2 bits\n"
    "                                                       \n"
    "                                       GST DESCRIPTION \n"
    "                                      -----------------\n"
    "                   GST is a texture compression scheme for encoding textures\n"
    "                   using anywhere from 1.5 to 4 bits per pixel (not including\n"
    "                   the palette), depending on the compression options used.\n"
    "                   Very little processing power is required to decompress a GST\n"
    "                   texture back into a palletized texture. In fact, on some\n"
    "                   consoles, this operation can be done mostly in hardware. A\n"
    "                   GST texture decompresses to an 8 bit texture from a base \n"
    "                   and detail component. The base texture is a small version\n"
    "                   of the original image whose dimensions depend on the block\n"
    "                   width chosen. For example, in the case of 4x2x2 the base\n"
    "                   texture will be width/4 x height/2. The detail texture is a\n"
    "                   one or two bit texture that modifies the index of the base\n"
    "                   texture. GST can achieve near 8 bit quality at roughly\n"
    "                   3 bits per pixel (422), a 70% decrease in size.\n"
    "                                                     \n"
    "                                         USAGE NOTES \n"
    "                                        -------------\n"
    "                   o  while GST is available in other shape file formats\n"
    "                      it is primarily intended for use with SSH.\n"
    "                   o  GST should always be used in conjunction with -swizzle\n"
    "                      when working with SSH, but only supports SSH swizzleing\n"
    "                   o  GST should only be used with mipmont, using GST with\n"
    "                      mipmap will result in non compressed.\n"
    "                      non collapsed mipmaps.\n"
    "                   o  GST textures must always be a multiple of 32x16\n"
    "                                                       \n"
    "                                        GST BIT DEPTHS \n"
    "                                       ----------------\n"
    "                   The following table shows some combinations of encoding\n"
    "                   values and the resulting storage requirements for various\n"
    "                   texture sizes:\n"
    "                       See the T&L document on GST for more information.\n"
    "                       (assumes a 1024 byte clut)\n"
    "                   blockwidth blockheight detailbpp total bpp  256x256 512x512\n"
    "                   ---------- ----------- --------- ---------  ------- -------\n"
    "                   2          2           2         4          33 KB   129 KB\n"
    "                   2          2           1         3          25 KB   97  KB\n"
    "                   4          2           2         3          25 KB   97  KB\n"
    "                   4          4           2         2.5        21 KB   81  KB\n"
    "                   4          2           1         2          17 KB   65  KB\n"
    "                   4          4           1         1.5        13 KB   49  KB\n",
    "    -filterdxt<n>       run image through dxt filter (n=1..5 dxt filter type\n",
    "                                        n=-1 choose based on alpha depth)\n",
    "    -grb<n>        8 bit grrrrbbb palettizer (n=1 horiz,2=vert,3=both)\n",
    "    -hflip         horizontally flip image\n",
    "    -vflip         vertically flip image\n",
    "    -sort          sort input names numerically (sgi style)\n",
    "    -sort2         sort input names alphabetically\n",
    "    -noremap       force colours not to be remapped\n",
    "    -charname      rename characters in font to ascii\n",
    "    -acopy0        copy alpha to RGB\n",
    "    -acopy1        copy max(r,g,b) to alpha\n",
    "    -extract       extract frames to seperate files\n",
    "    -grey          make image into grey scale (aka gray)\n",
    "    -alphacomment  imbed comment into alpha channel\n",
    "    -alphacomment2 extract comment from alpha channel\n",
    "    -invert        invert ARGB values\n",
    "    -invert<m>     xor ARGB values with specified colour mask\n",
    "    -tedge         extrude RGB onto transparent pixels\n",
    "    -subtype<n>    set subtype, -1 same, -2 same if shp (also -s)\n"
    "                   Can specify subtypes for shape using list of things\n"
    "                   you want and dont want seperated by , + or |\n"
    "                   use ! or 'no' for items you dont want\n"
    "      SHAPE SUBTYPES\n"
    "      -csm1        swizzled ps2 clut (default)\n"
    "      -csm2        linear ps2 clut\n"
    "      -swizzle     swizzle pixel order\n"
    "      -noswizzle   linear pixel order (aka unswizzle)\n"
    "      -pal         include a palette (aka -palette)\n"
    "      -nopal       do not include a palette (aka -palette)\n"
    "      -shp         include the shape (aka -shape,-image)\n"
    "      -noshp       do not include the shape\n"
    "      -hot         include hot spots (aka -hotspot)\n"
    "      -nohot       do not include hot spots\n"
//    "      -comment     include comments (aka -comm,-comment,-rem)\n"
// note -nocomment actually implemented as -comment''
    "      -nocomment   do not include comments (also see -comment)\n"
    "      -anim        include palette animation (aka -anm)\n"
    "      -noanim      do not include palette animation\n"
    "      -namedpal    name palettes in header (aka -namedpalette)\n"
    "      -nonamedpal  do not name palettes in header\n"
    "      -mipmapcollapse   make mipmaps into one shape (default, aka mmc)\n"
    "      -nomipmapcollapse do not make mipmaps into one shape\n"
    "      -dxtcollapse   make dxt into shapetype (default)\n"
    "      -nodxtcollapse do not make dxt into shape..leave as codec\n"
    "      -advanced      enable advanced image formats (shaders required)\n"
    "      -noadvanced    disable advanced image formats (default)\n"
    "      -sorted        sort the shape header (for VSH)\n"
    "      -nosorted      do not sort the shape header\n"
    "      -nonameelement do not output a long name element\n"
    "      -isdot3      flag a texture as dot3\n"
    "      -isembm      flag a texture as embm\n"
    "      -iscubemap   flag a texture as cubemap\n"
    "                   this flag is rather complex, user must ensure:\n"
    "                     1) texture is a multiple of 6\n"
    "                     2) for mipmaps, is a power of 2 as well\n"
    "                     3) for mipmaps, you do NOT generate\n"
    "                        mipmaps smaller than <anysize> x 6\n"
    "                        with the -mipmap option.\n"
    "                     This is extremly important for the cubemap\n"
    "                     collapse, gx will generate a cubemap but the\n"
    "                     texture will have black stripes at the bottom\n"
    "                     due to roundoff of the texture size during the collapse\n"
    "      -ismipmont   flag a texture as mipmont\n"
    "      -metalbin    flag a texture has metalbin\n"
    "      -isopaque    flag a texture being opaque\n"
    "      -istransparent flag a texture being 1 bpp transparent\n"
    "      -ishalfalpha flag a texture being halfalpha range (128=opaque) for ps2\n"
    "      -config      configure AVI codec advanced properties (for vp6)\n"
    "      FONT SUBTYPES\n"
    "      -char16      use CHARACTER16 font format (for ffn,xfn,sfn,gfn)\n"
    "      -powerof2    make texture power of 2 square\n"
    "      -maxwidth256 limit width of font texture page to 256\n",
    "    -userdata<n>    create a user data area in a shape file\n",
    "    -screen<c>,<o> screen mask image (1/2 transparent)\n",
    "                   <c> screen colour\n",
    "                   <o> orientation (0=upperleft,1=upperright)\n",
    "    -preten        preattenuate ARGB values\n"
    "    -preten1       unattenuate ARGB values\n",
    "    -tblack        make black transparent\n"
    "    -bblack        make black blue (0xff080808)\n"
    "    -twhite        make white transparent\n",
    "    -replace<s>,<r>,<sm>,<rm>,<sn> replace a colour with another colour\n"
    "                   <s> search colour ie WHITE\n"
    "                   <r> replace colour ie BLACK\n"
    "                   <sm> search mask ie 0xf8f8f8f8\n"
    "                   <rm> replace mask ie 0xffffffff\n"
    "                   <sn> search not 1=search for not s&sm\n",
    "    -palcopy<d>,<s> palette copy\n"
    "    -palmove<d>,<s> palette move (zero source)\n"
    "    -palswap<d>,<s> palette swap entries\n"
    "                   <d> destination palette entry\n"
    "                   <s> source palette range\n",
    "    -palgrad<s>,<e>,<min>,<max>,<mask> palette gradient\n"
    "                   <s> start colour\n"
    "                   <e> end colour\n"
    "                   <min> min argb colour value (see COLOURS)\n"
    "                   <max> max argb colour value (see COLOURS)\n",
    "                   <mask> mask argb colour value (see COLOURS)\n",
    "    -palgrad<s>,<e> palette gradient using existing palette entries\n"
    "                   <s> start colour\n"
    "                   <e> end colour\n"
    "    -halfalpha     reduce alpha to 1/2 (-pixel(a+1)/2,r,g,b)\n"
    "    -doublealpha   increase alpha to 2x (-pixel(min(255,(a*255+64)/128)),r,g,b)\n"
    "    -halfpixel     reduce pixel to 1/2 (-pixel(a+1)/2,(r+1)/2,(g+1)/2,(b+1)/2)\n",
    "    -fyi<ys>,<yo>,<cs>,<co>,<as>,<ao> adjust ayiq for ps2/xbox\n"
    "                   Adjust image for ps2 to/from xbox.  See MS whitepaper\n"
    "                   <ys> luminance scale\n"
    "                   <yo> luminance offset\n"
    "                   <cs> chrominance scale\n"
    "                   <co> chrominance offset\n"
    "                   <as> alpha scale\n"
    "                   <ao> alpha offset\n"
    "                   For ps2 to xbox/gc: -fyi1.0811,-20.68,1.115\n"
    "                   For xbox/gc to ps2: -fyi0.925,19.25,0.8968\n",
    "    -threshhold<n> bump values >= n to 255, and < n to 0\n",
    "    -pixel<r>,<g>,<b> replace pixels using expressions\n"
    "    -pixel<a>,<r>,<g>,<b>,<i>,<x>,<y> pixel expression\n"
    "                   <a> alpha expression\n"
    "                   <r> red expression\n"
    "                   <g> green expression\n"
    "                   <b> blue expression\n"
    "                   <i> index expression for 8 bit image\n"
    "                   <x> map destination x to source x with expression\n"
    "                   <y> map destination y to source y with expression\n"
    "                   Symbols available:\n"
    "                     a,r,g,b,x,argb,frame,framenum,width,height,bpp,\n"
    "                     startcolour,numcolours,colours,subtype,packed,\n"
    "                     quality,framesize,alphabits,redbits,greenbits,\n"
    "                     bluebits,centerx,centery,defaultx,defaulty,\n"
    "                     numhotspots,fps,dpi,ispal,framename\n"
    "                     hot0x,hot0y,hot1x,hot1y,hot2x,hot2y,hot3x,hot3y,\n"
    "                     hot4x,hot4y,hot5x,hot5y,hot6x,hot6y,hot7x,hot7y,\n"
    "                     hot8x,hot8y,hot9x,hot9y,hot10x,hot10y,hot11x,hot11y,\n"
    "                     hot12x,hot12y,hot13x,hot13y,hot14x,hot14y,\n"
    "                     hot15x,hot15y,hot16x,hot16y,hot17x,hot17y,\n"
    "                     hot18x,hot18y,hot19x,hot19y\n"
    "                   a_ prefix for auxilliary file variables (see -a)\n"
    "                   ispal is for expressions on the 8 bpp index <i> which are\n"
    "                     applied to both palette index and image\n"
    "                   Function available:\n"
    "                     min(...)     smallest of n values\n"
    "                     max(...)     largest of n values\n"
    "                     int(n)       cast to integer (truncate toward 0)\n"
    "                     round(n)     round to nearest integer\n"
    "                     floor(n)     truncate down to integer(negative away from 0)\n"
    "                     ceil(n)      truncate up to integer (negative toward 0)\n"
    "                     sin(n)       sine of radian angle\n"
    "                     cos(n)       cosine of radian angle\n"
    "                     tan(n)       tan of radian\n"
    "                     asin(n)      arcsine\n"
    "                     acos(n)      arccosine\n"
    "                     atan(n)      arctangent\n"
    "                     exp(n)       e raised to power of n\n"
    "                     log(n)       log base e of n\n"
    "                     log2(n)      log base 2 of n\n"
    "                     log10(n)     log base 10 of n\n"
    "                     sqrt(n)      square root of n\n"
    "                     rand(max)    float random up to/including max\n"
    "                     hypot(...)   hypotenous of n dimensional vertex\n"
    "                     floorpow2(x) next lower power of 2 of x\n"
    "                     ceilpow2(x)  next higher power of 2 of x\n"
    "                     pow(x,y)   x to the power of y\n",
    "    -safe1         ntsc safe colour fix using desaturation (dawson)\n"
    "      -safe2         ntsc safe colour fix using colour space shift (loken)\n"
    "      -safe2_grey<n> ignore colours with the same r,g,b within # threshold\n",
    "    -blur<n>       blur with n pixel radius\n",
    "    -dilate<n>     dilate with n pixel radius\n",
    "    -erode<n>      erode with n pixel radius\n",
    "    -equalize      equalize an image (maximize range of each channel)\n",
    "    -blurp<n>      blur with n pixel percentage radius\n",
    "    -gamma<n>      gamma correct (1.0 default)\n",
    "    -sharpen       sharpen using standard convolution kernel\n"
    "    -sharpen<n>    1 - (default) standard convolution kernel\n"
    "                   2 - gaussian kernel much clearer but introduces\n"
    "                       a lot of artifacts\n",
    "    -x<n>          default x-position\n"
    "    -y<n>          default y-position\n",
    "    -dx<n>         add n to default x-position\n"
    "    -dy<n>         add n to default y-position\n",
    "    -cx<n>         centre-point x-position\n"
    "    -cy<n>         centre-point y-position\n",
    "    -hotadjust0    adjust hot spots from absolute to center point relative\n"
    "                   same as -sethot(hot0x-centerx),(hot0y-centery)\n"
    "    -hotadjust1    adjust hot spots from relative to absolute\n"
    "                   same as -sethot(hot0x+centerx),(hot0y+centery)\n",
    "    -hotadd<n>,<x>,<y>       adjust hot spot n by x,y\n",
    "    -setvar=expr   set a user variable to a value (expression)\n",
    "    -sethot<n>,<x>,<y>...    set hot spots starting at 'n'\n"
    "    -sethot<x>,<y> set hot spots starting at first hot spot\n"
    "      <n>          starting hot spot - range constraint on numhot spots\n",
    "    -comment<str>  add comment for image.  str is printf formatter\n"
    "                     %s is used to access the old comment\n"
    "                     no argument turns off comment option\n"
    "                     empty string clears the comment. see also -nocomment\n",
    "    -shell<str>    launch shell command for every frame processed\n",
    "    -echo<str>     echo a string to the screen for every frame processed\n"
    "    -echo<form>,<expr> printf format an expression\n",
    "    -name<str>,<expr>    name shape\n"
    "      <str>          formatting string\n"
    "      <expr>         frame index range constraint (expression) for %d etc\n"
    "                     a_framename uses auxillary file framename for %s\n"
    "    -startindex<n>  starting frame number for output filenames\n",
    "    -sort<n>       sort filenames (0=off, 1=sort)\n"
    "    -hash<n>       hash names to n characters (default 512)\n",
    "    -d             dither.  if -555 is specified, low bits use jitter on\n"
    "                     32 bit image.  If -888 or -8888 is used, jitter if off\n"
    "      -d1            Floyd-Steinberg\n"
    "      -d2            Burkes\n"
    "      -d3            Stucki\n",
    "    -d2            randomize dither low bits. use -555 to specify bits\n",
    "    -return<expr>  set return value (ERROR LEVEL) for command line based\n"
    "                   on an expression.  Requires an image and has access\n"
    "                   to image information. default return is error count\n"
    "                   See also -pixel for variables/functions\n",
    "    -f<range>      frames to extract from source(default all)\n",
    "    -ff<range>     frames to filter from source(default all)\n"
    "    -if<expr>      process the frame if an expression is true\n",
    "    -fif<expr>     write image if an expression is true\n",
    "    -while<expr>   process the frame while an expression is true\n",
    "    -channel<n>    Photoshop channel to extract\n",
    "    -overwrite<n>  overwrite source if no dest (0=no,1=backup,2=overwrite)\n",
    "    -opaque<range> opaque colours\n",
    "    -black<range>  black out a range of colours (opaque black)\n",
    "    -t<range>      transparent colours\n",
    "    -l<range>      translucient colours\n",
    "    -avoid<range>  colours to avoid using in colour reduction\n",
    "    -a<file>       auxillary frames with specified file\n"
    "    -p<file>       read a palette to convert to from any image file\n"
    "      -p<file>,<frame> specify framenumber or framename to use\n"
    "    -log<file>     log file for output instead of stdout\n"
    "    +<file>        composite frames with specified file (or -+file)\n"
    "    -paste         composite paste frames\n"
    "    -add           composite add pixels\n"
    "    -sub           composite subtract pixels\n"
    "    -remove        composite remove pixels (clipped subtract)\n"
    "    -psnr          calculate PSNR of the image vs the composite file\n"
    "                   -psnr2 just show PSNRY and set return code to psnry\n"
    "    -blendmethod<n> composite method\n"
    "                   0 alpha blend unattenuated (default)\n"
    "                   1 paste\n"
    "                   2 sub\n"
    "                   3 add\n"
    "                   4 remove\n"
    "                   5 psnr calc\n"
    "                   6 alpha blend preattenuated\n"
    "                   7 multiply\n"
    "                   8 screen\n"
    "                   9 overlay\n"
    "                   10 alpha test (aka tmask) - stamp non-transparent pixels\n"
    "    -blendbehind   composite drawn behind image (swaps order)\n"
    "    -blend<n>      composite blend n percent (default 100%)\n"
    "    -blend<f>,<b>  foreground/background percentages\n"
    "    -blend<f>,<b>,<m> blend with channel mask\n",
    "    -q<n>          lossy compression quality 0..100\n"
    "                   0 bad, 100 perfect, -1 same as original, -2 auto (default)\n",
    "    -framesize<n>  set lossy compress level based on bytes in a frame\n"
    "                   for madcow, default is sqrt(width*height)*42\n"
    "    -datarate<n>   set datarate in term of KB/s.  Respects FPS if specified to\n"
    "                   convert to framesize.  Default is 29.97 fps\n"
    "    -gop<i>,<p>    set movie i and p frame intervals (15,3 recommended)\n",
    "    -dpi<n>        set dpi (dots per square inch)\n",
    "    -align<n>      shape data alignment\n",
    "    -fps<n>        set frames per second.  Common frame rates include:\n"
    "                   23.976          (24*1000/1001)\n"
    "                   24      Movie\n"
    "                   25      PAL/2\n"
    "                   29.97   NTSC/2  (30*1000/1001)\n"
    "                   30\n"
    "                   50      PAL\n"
    "                   59.94   NTSC    (60*1000/1001)\n"
    "                   60\n",
    "    -background<c> set a default background colour\n",
    "    -break<n>      break (int3) for debugging. 1=in options, 2=process image\n",
    "    -low           priority low to run in background\n",
    "    -high          priority high\n",
    "    -setcolour<c>,<val>        set palette entries to ARGB(0xaarrggbb)\n"
    "    -setcolour<c>,<val>,<mask> mask which channels are set(ie 0xffffffff)\n"
    "      <c>          colour range (ie 4..7)\n"
    "      <val>        value to set to (range constraint)\n"
    "      <mask>       channel mask\n",
    "    -rot<n>        rotate by 90 degrees. 1=90cw, 2=180, 3=90ccw\n",
    "    -rotate<n>     rotate by arbitrary degrees 0..360 (negative ok)\n",
    "    -transpose     transpose the image same as doing a hflip\n"
    "                   and a 270 degree rotate\n",
    "    -deint         deinterlace\n",
    "    -temporalfix   fix WMV temporal compression duplicate frames (grey frames)\n",
    "    -palsort<m>    m=method 0=histogram,1=argb,2=hls,3=weighthls,4=ihist,5=yuv\n"
    "                   sort in any order using the following symbols (uppercase\n"
    "                   sorts largest>smallest, lowercase sorts smallest>largest):\n"
    "                       A(alpha),R(red),G(green),B(blue),H(hue),S(saturation)\n"
    "                       L(lightness),Y(luminance),U(chrominance),V(chrominance)\n"
    "                       P(popularity(histogram))\n",
    "    -palsort<m>,<b> b=1 forces black to be colour 0,2=trans 255,3=trans 0\n",
    "    -wrap<x>,<y>   wrap image (latenight style)\n",
    "    -deband        dithering to remove banding\n",
    "    -deband<rgb>,<rad> rgb threshhold and pixel radius for dithering\n",
    "    -shadow<x1>,<x2>,<y>,<dx>,<dy> generate skewed shadow.\n"
    "                   x1,x2,y1 in percent (ie 100).  dx,dy are offsets\n",
    "    -dropshadow<x,y,z> Use a semi raytracer to dropshadow a glyph\n"
    "                       x,y,z specify light location in 3D coords.\n"
    "                       Glyph is texture mapped to a plane at <0,1,0>\n"
    "                       Camera at <0,1.10,0> pointing down y axis.\n"
    "                       BackPlane at <0,0,0> This combination\n"
    "                       allows any shadow from glyph to fall on the\n"
    "                       back plane and create the shadow effect.\n"
    "        -dslighttype<n>     0 - pointlight (default)\n"
    "                            1 - cylinder light.\n"
    "                            2 - spotlight.\n"
    "                            3 - area point light\n"
    "                            4 - area spotlight\n"
    "                            5 - area cylinderlight\n"
    "        -dslightdir<x,y,z> direction of light source. for \n"
    "                           directional light sources only!\n"
    "        -dsradius<n>       radius of cylindrical light.\n"
    "        -dsfalloff<n>      falloff of light source.\n"
    "        -dsFadePower<n>    fade power of spotlight.\n"
    "        -dsFadeDist<n> absolute fade distance of spotlight.\n"
    "        -dsAreaL<x,y,z,x,y,z,w,h> Build area light, with\n"
    "                                  Axis1 and Axis2 specifying rectangle\n"
    "                                  direction <x,y,z> preferable perp.\n"
    "                                  to each other.\n"
    "                                  w - number of light in Axis1 direct.\n"
    "                                  h - number of light in Axis2 direct.\n"
    "                                  NOTE large area creates large\n"
    "                                  smudged out shadows\n"
    "        -dsLTightness<n>   Tightness of spotlights and cylinderlights\n"
    "                           affects the rate of falloff of a light.\n"
    "        -dsTexPlaneOff<n>  Where down the y axis the texture plane\n"
    "                           is located. default 1.\n"
    "        -dsShadPlaneOff<n> Where down the y axis the shadow plane is\n"
    "                           located. default 0.\n"
    "        -dsLookAt<x,y,x> Point at which the camera focuses.\n"
    "                           default <0,0,0>\n"
    "        -dsshadcol<r,g,b>  colour of the shadow, values range from\n"
    "                           0 -> 1 default is <0,0,0> so shadow will\n"
    "                           be completly black.\n"
    "        -dsCamLoc<x,y,z>   Camera location, or vantage point in scene.\n"
    "                           values are in floating point.\n"
    "                           default is <0,2.0,0>    \n"
    "        -dsAntiAlias<n>    number of rays to shoot for each pixel.\n"
    "                           n - 1..3 (1 default)\n"
    "    -mkftexmap     make facial texture map\n"
    "                   takes each frame seperatly, crops out the central\n"
    "                   col. and inserts from left to right into final bitmap.\n",
    "    -swap<a>,<b>   swap 8 bit colours A and B\n",
    "    -zoom<w>,<h>   zoom to width/height specified (0 dont zoom)\n",
    "    -zoom<w>,<h>,<sharpness> sharpness in pixel percentage (100 default)\n",
    "    -scale<w>,<h>  point sample scale to width, height\n",
    "    -scale<w>      point sample scale to width (height proportional)\n",
    "    -mipmont       mipmap montage for ps2\n"
    "                   a montage is a collage of mipmaps in a manner that\n"
    "                   can be uploaded all at once.\n"
    "                   This is the only option when working with GST\n"
    "                   compression and mipmaps.\n"
	"    -mips          shortcut for -mipmap. takes same options.\n",
    "    -mipmap        mipmap with specified parameters. \n"
    "     -mipmap<levels>,<min>\n"
    "     -mipmap<levels>,<maxwidth>,<maxheight>\n"
    "     -mipmap<levels>,<maxwidth>,<maxheight>,<min>\n"
    "     -mipmap<levels>,<maxwidth>,<maxheight>,<min>,<trans>,<chroma>,<clear>\n"
    "     -mipmaps...   use bicubic filter (size filter)\n"
    "     -mipmapc...   use bicubic filter but global colour reduce\n"
    "                   across mipmaps to retain bit depth\n"
    "     -mipmapb...   use box filter (default - halfsize)\n"
    "     -mipmapp...   use old p2 method, (point sample) use only for <=8bpp images\n"
    "       levels      total number of levels including base texture\n"
    "       maxwidth..  size of top mipmap\n"
    "       min         stop when width or height hits this minimum pixel size\n"
    "       trans       transparency flag for size. ignored by halfsize (default)\n"
    "       chroma      chromakey. ignored\n"
    "       clear       clear last n mipmaps to background colour\n",
	
    "    -shrinktofit<m> crop to minimum rectangle (remove transparency)\n"
    "                   1   remove transparent pixels (alpha=0)\n"
    "                   2   use pixels with colour of upper left pixel\n"
    "                   3   remove both upper left and transparent pixels (default)\n",
    "    -autocenter    triangulate a center point based on 2 pixels on edge\n",
    "    -hsv<h>,<s>,<b>,<v> adjust hsb/v. h=180..180. s,b,v=-100..100\n",
    "    -contrast<c>,<b> adjust contrast brightness. -100..100\n",
    "    -chroma<tol>   chromakey to a tolerance (45 default)\n",
    "    -edge<colour>,<pixels>,<pits>  clean edge pits and spikes\n",
    "    -fill<colour>  flood fill starting at 0,0 with colour\n"
    "    -fill<x>,<y>,<colour>    flood fill starting at x,y with colour\n",
    "    -rect<x>,<y>,<w>,<h>,<c> draw rectangle\n",
    "    -outline<x>,<y>,<w>,<h>,<c> outline (normally for fonts)\n",
    "    -grid<x>,<y>,<w>,<h>,<c> draw grid\n",
    "    -line<x>,<y>,<x2>,<y2>,<c> draw line\n",
    "    -clut<x>,<y>   set clut position in vram\n",
    "    -crop          crop to specified dimensions (negatives ok)\n"
    "     -crop<n>      crop to specified number of pixels area\n"
    "     -crop<width>,<height>\n"
    "     -crop<width>,<height>,<colour>\n"
    "     -crop<x>,<y>,<width>,<height>\n"
    "     -crop<x>,<y>,<width>,<height>,<colour>\n"
    "     -crop<x>,<y>,<width>,<height>,<colour>,<clamp>\n"
    "     -crop<x>,<y>,<width>,<height>,<colour>,<clampx>,<clampy>\n"
    "       <clamp> 0 Fill with solid colour\n"
    "               1 Clamp to edge\n"
    "               2 Wrap around\n"
    "               3 Mirror\n"
    "     -cropby<left>,<top>,<right>,<bottom>,<colour>\n"
    "     -cropby...     crop by specified pixels (negatives for outward)\n"
    "     -cropto...     crop to specified alignment. (pos up, neg down)\n"
    "     -cropmax...    crop anything above maximum specified size\n"
    "     -crop<n>%...   crop to percentage (any arg)\n"
    "     -croppix<n>    crop to specified number of pixels area (aka -crop)\n"
    "     -cropby<n>%... crop by percentage\n"
    "     -cropby<left>,<top>,<right>,<bottom>,<colour>\n",
    "    -halfsize<n>   reduce image to half size 1=vertical,2=horizontal,3=both\n",
    "    -thumb<w>,<h>  reduce image to thumbnail. w/h default to 128,102\n",
    "    -size<w>,<h>     size to width/height specified (0 dont size)\n"
    "      -size<n>       size to specified number of pixels area (also -sizepix)\n"
    "      -size<n>%...   size to percentage (any arg)\n"
    "      -size<w>,<h>,<method0..6>,<trans>,<chromakey>,<sharpnessx>,<sharpnessy>\n"
    "       <method> 0 Lanczos    Good choice (sharper than Mitchell)\n"
    "                1 Mitchell   Good choice (B=1/3 C=1/3)\n"
    "                2 Cubic      Good choice (2|t|^3 - 3|t|^2 + 1 for -1 <= t <= 1)\n"
    "                3 Triangle   Linear interpolation\n"
    "                4 Box        Point sample with interpolation\n"
    "                5 Bell       Blurry\n"
    "                6 B-Spline   Blurry\n"
    "                7 Quadratic  Faster than cubics, good quality\n"
    "       <trans>       0 use alpha, 1 maintain if any pixel is transparent\n"
    "       <chromakey>   transparent colour for <trans> option\n"
    "       <sharpnessx>  sharpness horizontally. 100 default. 50 sharp. 200 blurry\n"
    "       <sharpnessy>  sharpness vertically.   100 default. 50 sharp. 200 blurry\n",
    "    -pointsize<s>    size for ttf true type font in points (72 dpi). default 24\n"
    "    -antialias<n>    specify antialiasing for ttf\n"
    "                     0=none, 1=antialias 8 bpp (default), 2=antialias 4 bpp\n"
    "    -style<n>       specify font style\n"
    "                    0 Regular\n"
    "                    1 Bold   (aka -bold)\n"
    "                    2 Italic (aka -italic)\n"
    "                    3 Bold/Italic\n"
    "                    4 Underline\n"
    "                    8 Strikeout\n"
    "    -italic         italic font (aka -style2)\n"
    "    -bold           bold font (aka -style1)\n"
    "    -charset<n>     specify font charset <n>\n"
    "                    ANSI            0\n"
    "                    DEFAULT         1\n"
    "                    SYMBOL          2\n"
    "                    SHIFTJIS        128\n"
    "                    HANGEUL         129\n"
    "                    HANGUL          129     (common asian format)\n"
    "                    GB2312          134\n"
    "                    CHINESEBIG5     136\n"
    "                    OEM             255\n"
    "                    JOHAB           130\n"
    "                    HEBREW          177\n"
    "                    ARABIC          178\n"
    "                    GREEK           161\n"
    "                    TURKISH         162\n"
    "                    VIETNAMESE      163\n"
    "                    THAI            222\n"
    "                    EASTEUROPE      238     (central european)\n"
    "                    RUSSIAN         204\n"
    "                    MAC             77\n"
    "                    BALTIC          186\n"
    "    -typeface<face> specify font face in unicode\n"
    "    -aspect<n>      specify font aspect ratio. 1=1:1.  0.5=half height\n"
    "    -tighten<n>     specify font tighten (spacing scale factor)\n"
    "                    1=none, 0.8 narrow, 1.2 spaced out\n"
    "    -tracking<n>    specify font tracking (spacing translation value)\n"
    "                    0=none, -1 narrow, 1 spaced out\n"
    "    -smallcaps<n>   specify smallcaps ratio. 0.8 typical. 0 disable\n"
    "                    makes lowercase use uppercase with this size ratio\n"
    "    -ascii<n>       1=use ascii, 0=unicode\n"
    "    -baseline       positions from baseline instead of upper left\n",
    "    -stereo<f>,<r> make stereogram from height field.\n"
    "       <f>         focus (default 63)\n"
    "       <r>         range (default 10)\n",
    "    -embm<t>,<s>,<f>,<w> create embm from height field\n"
    "       <t> - type of embm to create\n"
    "                1 - (default) UVL 888 format\n"
    "                2 - UV 8800 format (default)\n"
    "                difference values are signed with 0 being\n"
    "                no change in height\n"
    "        <s> scale height before generating normal (def 600/width)\n"
    "        <f> filter 0=3x3, 1=point.\n"
    "        <w> wrap 0=clamp, 1=wrap.\n"
    "    -gshembm       make GameCube embm from height field\n"
    "                   (biased by 128 unsigned char)\n",
    "    -unembm<n>     attempt to reconstruct the height field from embm data\n"
    "                   lossy conversion so attempt will produce greyscale image\n"
    "                      n - specifies where sea level is for the height field\n"
    "                          this is a percentage of the max height\n"
    "                          ie -unembm50  (50%)\n"
    "    -gshunembm<n>  attempt to reconstruct the height field\n"
    "                    (GSH ONLY) from embm data\n"
    "                    lossy conversion so attempt will produce greyscale image\n"
    "                      n - specifies where sea level is for the height field\n"
    "                           this is a percentage of the max height\n"
    "                           ie -unembm50  (50%)\n",
    "    -dot3<s>,<f>,<w> create normal map from height field\n"
    "                    s - scale height before generating normal (def 600/width)\n"
    "                    f - filter 0=3x3, 1=point.\n"
    "                    w - wrap 0=clamp, 1=wrap.\n"
    "    -undot3<n>     attempt to reconstruct heightfield from dot3 normalmap.\n"
    "                   n specifies starting point for heightfield. (default 127)\n",
    "    -semitrans<n>  transparency page value 0..5\n",
    "                   0 opaque, 1 50/50, 2 opaque, 3 add, 4 sub, 5 25/100\n",
    "    -flatten       Flatten all frames into one\n"
    "      -flatten2    Save x,y,w,h info as hot spots in flattened image\n",
    "    -flattened     Extracts only the flattened Photoshop PSD image\n",
    "    -layered       Treats texture files as layered images\n"
    "    -doinvisible   Flatten even the invisible layeres.\n",
    "    -cram<w>,<h>   cram the current shape with a vram page of w/h\n"
    "                   defaults to 1024x256\n"
    "    -cramtranspose   Let cram option do a transpose if necessary.\n"
    "    -cramrotate      Let cram option do rotation if necessary.\n"
    "    -cramcenterpoint Let cram update center point info as well\n"
    "    -cramcheckedge   Let cram option check edge boundaries and edges\n"
    "    -cramodd         Let cram option expand height to odd if width odd\n"
    "    -crambppsort     Let cram option sort by bitdepth\n"
    "    -cramclutsort    Let cram option sort by clut bit depth\n"
    "    -cramgroup<bit>,<clut>  specifies groupings of bitdepth and clut depth\n"
    "                            that should be lumped together when cramming\n"
    "    -cramtpage<w>,<h>Set cram texture page width & cram texture page height\n"
    "    -cramwrap        Set cram max width after which it will wrap\n"
    "    -cramwid         Cram option fails if width of tex. is more than tpage width\n"
    "    -cramspace<n>    add n pixels of space between the textures\n"
    "    -crampos<x>,<y>  Set cram x position Set cram y position\n"
    "    -cramnumfixed    Set number of shapes that are fixed and should not be moved.\n",
    "    -globalpalette Generates single palette for all frames.\n",
    "    -time          Display timing diagnostic (for colour reduction)\n",
    "    -pass<n>       number of passes of image processing to apply\n",
    "    -warp...       warp an image along a control line\n",
    "     -warp<type = 1><clampx><clampy><fillcolor><startx><starty><endx><endy>\n"
    "     -warp<type = 2><wavevertical><wavehorizontal>\n"
    "     -warp<type = 3><clampx><clampy><fillcolor><numberofLinePairs><LinePairs...>\n"
    "     -warp<type = 4><clampx><clampy><fillcolor><numberofLinePairs><LinePairs...>\n"
    "                   clamp codes:\n"
    "                   1 fill\n"
    "                   2 clamp\n"
    "                   warp types:\n"
    "                   1 regular single line quadrant warp.\n"
    "                   2 wave image.\n"
    "                   3 non bilinear multiple control line warp.\n"
    "                   4 bilinear control line warp.\n",
    "    -m<n>          colour reduction method -1,1..10 (1 default)\n",
    "      -m-1         -1 auto select method based on num colours or argb bits\n"
    "      -m1          Wu with 666 colour cube for 24 bit art\n"
    "      -m2          Fu with 3555 colour cube\n",
    "      -m3          Wu with 777 colour cube\n",
    "      -m4          PQ (default).  Best for most cases\n",
    "      -m7          exact simple histogram preserves alpha & order\n"
    "      -m8          Alpha - WU's with alpha\n"
    "      -m9          Wu with 555 colour cube\n"
    "      -m10         WuDa (Wu for alpha data) memory intensive version of 4DWu\n"
    "                   that handles images with arbitrary data in the alpha channel\n"
    "                   better than 4DWu.\n"
    "    -ma<n>         colour reduction method for translucient images -1,1..10\n",
    "      -ma-1        -1 auto select method based on num colours or argb bits\n"
    "      -ma2         Fu with 3555 colour cube\n",
    "      -ma4         PQ (default).  Best for alpha channel\n",
    "      -ma7         exact simple histogram preserves alpha & order\n"
    "      -ma8         Alpha - WU's with alpha\n"
    "      -ma10        WuDa (Wu for alpha data) memory intensive version of 4DWu\n"
    "                   that handles images with arbitrary data in the alpha channel\n"
    "                   better than 4DWu.\n"
    "      -exact       same as -m7, simple histogram colour reduction method\n"
    "      -wu5         same as -m9, 555 variance based colour cube\n"
    "      -wu6         same as -m1, 666 (default) variance based colour cube\n"
    "      -wu7         same as -m3, memory intensive version of Wu5 that produces\n"
    "                   marginally better results.\n"
    "      -pq          same as -m4, good for most types of art\n"
    "      -fu          same as -m2, runs Wu5 on 8 different alpha levels, good for\n"
    "                   images with limited transparency\n"
    "      -4dwu        same as -m8, Alpha, Wu with 6 bits of alpha precision\n"
    "                   - 6565 colour cube sensitive to channel depth.\n"
    "                   If green tint ensues from extra bit of precision in the\n"
    "                   green channel set image to 6555 and 4dwu will reduce\n"
    "                   with equal precision in each channel. Note also that 4dwu\n"
    "                   will only generate a single completly transparent colour as\n"
    "                   it assumes that perceptually, only one is required.\n"
    "      -wuda        same as -m10, Alpha, 6666 colour cube - memory intensive and\n"
    "                   slow but better for images with data in the alpha channel. \n"
    "                   WUDA: Wu for alpha data\n"
    "                   Wuda will generate several completly transparent colours\n"
    "                   if necessary.\n",
    "    -text<text>,<x>,<y>,<pointssize>,<bold>,<colour>,<truetype font>\n"
    "      <text>       ascii text to render into frame\n"
    "      <x>,<y>      position to render font (upper righthand corner)\n"
    "      <pointsize>  pointsize to render font\n"
    "      <colour>     colour value for text\n"
    "                   (8bpp does a best match in palette)\n"
    "      <bold>       flag, 0/1 turns bold on or off\n"
    "      <truetype font> file/pathname of font, including extension and folder\n",
    "\n",
/*
FILE FORMATS:"
    -act   ACT Photoshop Palette
    -anm   DPaint ANM
    -ash   Gameboy Advance Shape File
    -avi   Audio Visual Interleave
    -bif   Binary Info File
    -bmp   Windows BMP
    -bnr   Nintendo Banner
    -clp   ClipBoard Gimex
    -dds   DirectDraw Surface
    -eps   Encapsulated Postscript
    -ffn   PC Font File
    -fli   FLIC Animation
    -flm   Adobe FilmStrip
    -fsh   PC Shape File
    -fss   PC Streaming Shape
    -gfn   GameCube Font File
    -gif   Compuserve GIF
    -gmx   Gimex Images
    -gsh   GameCube Shape File
    -gss   GameCube Streaming Shape
    -hsi   HSI Raw
    -ico   Windows Icon Cursor
    -ijpeg Intel JPeg
    -jp2   JPeg2000
    -jpg   JPeg
    -lbm   Amiga ILBM
    -mad   Madcow Movie
    -mdec  MDEC Movie
    -mfn   Macintosh Font File
    -mov   QuickTime Movie
    -mpc   MPEG Video
    -msh   Mac Shape File
    -mss   Mac Streaming Shape
    -new   New Image
    -nil   Nil Gimex
    -pcx   ZSoft PCX
    -pfn   PSX Font File
    -pict  Macintosh Pict
    -pix   Alias Pix
    -png   Portable Network Graphics
    -ppm   Portable Pixel Map
    -psd   Adobe Photoshop 4.0
    -psh   PSX Shape File
    -qmt   Quake Model Texture Gimex
    -qt3   QuickTime3 Movie
    -ras   Sun Raster
    -raw   Erik Kiss RAW
    -rla   WaveFront RLA
    -sct   Scitex SCT
    -sfn   Sony2 Font File
    -sgi   SGI RGB Image File
    -sim   SoftImage PIC
    -ssh   PS2 Shape File
    -sss   PS2 Streaming Shape
    -tga   Targa
    -tgv   TGV Movie
    -tif   Tiff
    -tim   TIM PSX Texture Image
    -tpl   GameCube Texture
    -xfn   XBox Font File
    -xsh   XBox Shape File
    -xss   XBox Streaming Shape
    -yuv   ACOM YUV
*/
    "\n",
    "COLOUR SYNTAX:\n"
    "    There are 3 basic ways you can specify an ARGB colour\n"
    "    1. (R,G,B) or (A,R,G,B)\n"
    "    2. A single value, typically in hex form 0xAARRGGBB.\n"
    "    3. Using a predefined colour.  The predefined colours are:\n"
    "       BLACK,BLUE,GREEN,CYAN,RED,MAGENTA,BROWN,LIGHTGREY,DARKGREY,\n"
    "       LIGHTBLUE,LIGHTGREEN,LIGHTCYAN,LIGHTRED,LIGHTMAGENTA,YELLOW,\n"
    "       WHITE,GREY,TRANSPARENT\n"
    "       Predefined colours can optionally be prefixed with C_ or CA_\n",
    "\n",
    "RANGE SYNTAX:\n"
    "    [[!][<start>][..][<end>][+=<step>][,]]*\n"
    "    ie -f             all frames\n"
    "       -f5            frame 5\n"
    "       -f5..9         frames 5..9 inclusive\n"
    "       -f!10          all frames except 10\n"
    "       -f5,7,9        frames 5,7 and 9\n"
    "       -f10..98+=2    even frames in range between 10 and 98\n"
    "       -fbob,fred     named frames\n"
    "       -f!S*          not these wildcard frames\n",
    "\n",
#endif
    "    -h4            list of file formats\n",
    "    -history       history of changes\n",
    "    -help          launch web help\n",
    "    -opt?          show help on specified option\n",
    "    -?word         show help that contains specified word\n",
    "\n",
//    GIMEXTYPES,
// END ABSTRACT
#endif /* GXRT */
    0,
};

#define CR_WU6   1
#define CR_FU    2
#define CR_WU7   3
#define CR_PQ    4
#define CR_EXACT 7
#define CR_WU5   9
#define CR_4DWU  8
#define CR_WUDA 10



//--------------------------------------------------------------------
// spit opts is the callback for each option gx supports. Any change here
// should be reflected in the string documentation above and the gxabstract
// parser rerun to regenerate the options summary documentation.

char *SPIT_opt(const char *optptr)
{
    const char *d=NULL;
    int type;
    char OPTstr[SPITMAXLINE];

    // check for file formats. ie -tga

    if (*optptr!='.')
    {
        const int MAXTYPELEN=16;
        const char *nd = SPIT_getfield( OPTstr,optptr,1,0);

        if (strlen(OPTstr)<MAXTYPELEN)
        {
            type = GIMEX_find(optptr,&NAMEsubtype);
            if (type)
            {
                while (*optptr>' ' && *optptr!='?')
                    ++optptr;

                if (*optptr=='?' && OPTverbose)
                {
                    GABOUT *about;

                    about = GIMEX_about();
                    if (about)
                    {
                        GXshowabout(about);
                        SPIThelpgiven = 1;
                        gfree((char *)about);
                    }
                    ++optptr;
                }

                OPTtype = type;

                return((char *)optptr);
            }
        }
    }
    /*! \page o_Subtypes Subtypes
       \ingroup g_general_options
       \par Overview
       Many file formats support extra functionality that is not available by default. To access this
       special functionality gx requires the user turn on a subtype. Most subtypes are available through
       a simple flag at the command line.
       \par Example
           gx -swizzle john.jpg =out.fsh    <i>// turns on the swizzle subtype resulting in a swizzled FSH file.</i>
       \par Reference
           The following is a brief listing of known subtypes and their functionality<br>
          <b><i>SHAPE SUBTYPES</b></i><br>
          <i>These subtypes only apply to the shape file format</i><br>
          <b>-csm1</b>        swizzled ps2 clut (default)<br>
          <b>-csm2</b>        linear ps2 clut<br>
          <b>-swizzle</b>     swizzle pixel order<br>
          <b>-noswizzle</b>   linear pixel order (aka unswizzle)<br>
          <b>-pal</b>         include a palette (aka -palette)<br>
          <b>-nopal</b>       do not include a palette (aka -palette)<br>
          <b>-shp</b>         include the shape (aka -shape,-image)<br>
          <b>-noshp</b>       do not include the shape<br>
          <b>-hot</b>         include hot spots (aka -hotspot)<br>
          <b>-nohot</b>       do not include hot spots<br>
          <b>-nocomment</b>   do not include comments (also see -comment)<br>
          <b>-anim</b>        include palette animation (aka -anm)<br>
          <b>-noanim</b>      do not include palette animation<br>
          <b>-namedpal</b>    name palettes in header (aka -namedpalette)<br>
          <b>-nonamedpal</b>  do not name palettes in header<br>
          <b>-mipmapcollapse</b>   make mipmaps into one shape (default, aka mmc)<br>
          <b>-nomipmapcollapse</b> do not make mipmaps into one shape<br>
          <b>-dxtcollapse</b>   make dxt into shapetype (default)<br>
          <b>-nodxtcollapse</b> do not make dxt into shape..leave as codec<br>
          <b>-advanced</b>      enable advanced image formats (shaders required)<br>
          <b>-noadvanced</b>    disable advanced image formats (default)<br>
          <b>-sorted</b>        sort the shape header (for VSH)<br>
          <b>-nosorted</b>      do not sort the shape header<br>
          <b>-nonameelement</b> do not output a long name element<br>
          <b>-isdot3</b>      flag a texture as dot3<br>
          <b>-isembm</b>      flag a texture as embm<br>
          <b>-iscubemap</b>   flag a texture as cubemap<br>
                       this flag is rather complex, user must ensure:<br>
                         1) texture is a multiple of 6<br>
                         2) for mipmaps, is a power of 2 as well<br>
                         3) for mipmaps, you do NOT generate<br>
                            mipmaps smaller than <anysize> x 6<br>
                            with the -mipmap option.<br>
                         This is extremly important for the cubemap<br>
                         collapse, gx will generate a cubemap but the<br>
                         texture will have black stripes at the bottom<br>
                         due to roundoff of the texture size during the collapse<br>
          <b>-ismipmont</b>   flag a texture as mipmont<br>
          <b>-metalbin</b>    flag a texture has metalbin<br>
          <b>-isopaque</b>    flag a texture being opaque<br>
          <b>-istransparent</b> flag a texture being 1 bpp transparent<br>
          <b>-ishalfalpha</b> flag a texture being halfalpha range (128=opaque) for ps2<br>
          <b>-config      configure AVI codec advanced properties (for vp6)<br>
          <b><i>FONT SUBTYPES</b></i><br>
          <i>These subtypes only apply to the font file format</i><br>
          <b>-char16</b>      use CHARACTER16 font format (for ffn,xfn,sfn,gfn)<br>
          <b>-powerof2</b>    make texture power of 2 square<br>
          <b>-maxwidth256</b> limit width of font texture page to 256<br>
    */
    {
        int want=0;
        int nowant=0;
        int more;
        int ok;
        int i;
        const char *opt=optptr;
        static struct
        {
            char *name;
            int type;
        } subtypes[]=
        {
            "palette",-1,
            "pal",-1,
            "bitmap",-2,
            "image",-2,
            "shape",-2,
            "shp",-2,
            "hotspot",-4,
            "hot",-4,
//            "nocomment",8,
//            "comm",8,
//            "com",8,
            "rem",-8,
            "animation",-16,
            "anim",-16,
            "anm",-16,
            "tpage",-32,
            "namedpalette",256,
            "namedpal",256,
            "mipmapcollapse",-512,
            "mmc",-512,
            "sorted",1024,
            "twiddle",8192,
            "swizzle",8192,
            "extra",65536,
            "csm2",16384,
            "csm1",-16384,
            "nameelement",-32768,
            "isdot3",65536,
            "isembm",131072,
            "iscubemap",262144,
            "ismipmont",524288,
            "dxtcollapse",-1048576,
            "metalbin",      0x200000,
            "advanced",      0x400000,
            "isopaque",     0x1000000,
            "istransparent",0x2000000,
            "ishalfalpha",  0x4000000,
            "config",       0x8000000, // configure AVI codec advanced properties (ie vp6)

            // fonts
            "char16",      0x40000,
            "powerof2",    0x80000,
            "maxwidth256",0x100000,

            0,0,
        };

        do
        {
            bool notflag=false;
            if (*opt=='!')
            {
                notflag = true;
                ++opt;
            }
            else if (!strnicmp(opt,"no",2) || !strnicmp(opt,"un",2))
            {
                notflag = true;
                opt += 2;
            }
            ok = 0;
            more = 0;
            for (i=0; subtypes[i].type; ++i)
            {
                if (!strnicmp(opt,subtypes[i].name,strlen(subtypes[i].name)))
                {
                    type = subtypes[i].type;
                    if (type<0)
                    {
                        type = -type;
                        notflag = !notflag;
                    }

                    if (notflag)
                        nowant |= type;
                    else
                        want |= type;
                    opt += strlen(subtypes[i].name);
                    if (*opt=='+' || *opt==','|| *opt=='|')
                    {
                        ++opt;
                        ok = 1;
                        more = 1;
                    }
                    else if (*opt<=' ')
                    {
                        ok = 1;
                        more = 0;
                    }
                    break;
                }
            }
        } while (ok && more);

        if (ok)
        {
            OPTsubtypewant |= want;
            OPTsubtypenowant |= nowant;

            if (OPTverbose>=6)
                gprintf("subtype %x want %x, nowant %x\n",OPTsubtype,OPTsubtypewant,OPTsubtypenowant);

            return((char *)opt);
        }
    }

    switch (tolower(*optptr))
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':

colourbits:
        if (!strnicmp(optptr,"4dwu",4))
        {
            OPTquantmethod = CR_4DWU;
            return((char *)(optptr+4));
        }
        if (!strnicmp(optptr,"5a3",3))
        {
            OPTalphabits    =  3;
            OPTredbits      =  5;
            OPTgreenbits    =  5;
            OPTbluebits     =  5;
            return((char *)optptr+3);
        }
        else if (!strnicmp(optptr,"2101010",7))
        {
            OPTalphabits    =  2;
            OPTredbits      =  10;
            OPTgreenbits    =  10;
            OPTbluebits     =  10;
            return((char *)optptr+7);
        }
        else
        {
            int depth=OPTdepth;

            d = GX_getopt( &depth,optptr, "colours bits", 0, 8888);
            if (!depth)
            {
                OPTdepth = 0;       /* reset to auto mode */
                OPTalphabits = -1;
                OPTredbits = -1;
                OPTgreenbits = -1;
                OPTbluebits = -1;
            }
            else if (depth<=32)
                OPTdepth = depth;
            else
            {
                if (depth == 88)
                {
                    /* 88 is 8 bit alpha plus 8 bit intensity;
                       used for GSH I/A format */
                    OPTdepth        = 16;

                    OPTalphabits    =  8;
                    OPTredbits      =  8;
                    OPTgreenbits    =  0;
                    OPTbluebits     =  0;
                }
                else if (depth == 44)
                {
                    /* 44 is 4 bit alpha plus 4 bit intensity;
                       used for GSH I/A format */
                    OPTdepth        =  16; // just to keep true colour
                    OPTalphabits    =  4;
                    OPTredbits      =  4;
                    OPTgreenbits    =  0;
                    OPTbluebits     =  0;
                }
                else if ((depth%10     <0 || depth%10     >9)
                      || (depth/10%10  <0 || depth/10%10  >9)
                      || (depth/100%10 <0 || depth/100%10 >9)
                      || (depth/1000   <0 || depth/1000   >9))
                {
                    ++SPITerrorcount;
                    if (OPTverbose)
                    {
                        printf(TOOL_NAME " - INVALID DEPTH. Must be 1..32 or an RGB ie 555\n");
                        SPIThelpgiven = 1;
                    }
                }
                else
                {
                    OPTbluebits =  depth%10;
                    OPTgreenbits = depth/10%10;
                    OPTredbits =   depth/100%10;
                    OPTalphabits = depth/1000%10;
                }
            }
            return((char *)d);
        }

        case '+':
            return(SPIT_getfield( OPTcompositefile,SPIT_nextfield(optptr+1),1,0));

        case 'a':
            if (!strnicmp(optptr,"alphacomment",12))
            {
                return(GX_getopt( &OPTalphacomment,optptr+12, "Imbed comment in alpha channel", 0, 2));
            }
            else if (!strnicmp(optptr,"align",5))
            {
                d = SPIT_getfield( OPTalign,optptr+5,1,0);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"avoid",5))
            {
                return(SPIT_getrangeopt( OPTavoid,optptr+5, "avoid colours",-1,255));
            }
            else if (!strnicmp(optptr,"autocenter",10))
            {
                return(GX_getopt( &OPTautocenter,optptr+10, "auto triangulate center point", 0, 1));
            }
            else if (!strnicmp(optptr,"acopy",5))
            {
                return(GX_getopt( &OPTacopy,optptr+5, "copy alpha to other channels", 0, 1));
            }
            else if (!strnicmp(optptr,"add",3))
            {
                return(GX_getopt( &OPTadd,optptr+3, "composite add frames", 0, 1));
            }
            else if (!strnicmp(optptr,"a8",2))
            {
                //OPTdepth        =  8;
                OPTalphabits    =  8;
                OPTredbits      =  0;
                OPTgreenbits    =  0;
                OPTbluebits     =  0;
                OPTa8           =  1;
                return((char *)(optptr + 2));
            }
            else if (!strnicmp(optptr,"antialias",9))
            {
                return(SPIT_getfield( &OPTantialias[0],optptr+9, 1,0));
            }
            else if (!strnicmp(optptr,"aspect",6))
            {
                return(SPIT_getfield( &OPTaspect[0],optptr+6, 1,0));
            }
            else if (!strnicmp(optptr,"ascii",5))
            {
                return(SPIT_getfield( &OPTascii[0],optptr+5, 1,0));
            }
            return(SPIT_getfield( OPTauxillaryfile,SPIT_nextfield(optptr+1),1,0));
            break;

        case 'b':
            if (!strnicmp(optptr,"bblack",6))
            {
                type = 0;
                OPTreplaces  = OPTAUTO;
                d = GX_getopt( &type,optptr+6, "make black blue 0xff080808", 0, 1);
                if (type)
                {
                    OPTreplaces  = 0xff000000;
                    OPTreplacer  = 0xff080808;
                    OPTreplacesm = 0xf8f8f8f8;
                    OPTreplacerm = 0xffffffff;
                    OPTreplacesn = 0;
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"bpp",3))
            {
                optptr += 3;
                goto colourbits;
            }
            else if (!strnicmp(optptr,"background",10))
            {
                return(SPIT_getcolouropt(optptr+10, &OPTbackground, "background colour", 0x80000000, 0x7fffffff,1));
            }
            else if (!strnicmp(optptr,"blendbehind",11))
            {
                d = GX_getopt( &OPTblendbehind,optptr+11, "composite blend behind image", 0, 1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"blendmethod",11))
            {
                d = GX_getopt( &OPTblendmethod,optptr+11, "composite blend method", 0, 11);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"blend",5))
            {
                        d = GX_getopt( &OPTblendf,optptr+5, "composite blend foreground percentage", -200, 200);
                if (*d==',') d = GX_getopt( &OPTblendb,d+1, "composite blend background percentage", -200, 200);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPTblendmask, "composite blend channel mask", 0x80000000, 0x7fffffff,1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"blurp",5))
            {
                int blurp;
                d = SPIT_getfield( OPTblur,optptr+5,1,0);
                blurp = (int)(atof(OPTblur));
                sprintf(OPTblur,"%d/100",blurp);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"blur",4))
            {
                d = SPIT_getfield( OPTblur,optptr+4,1,0);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"black",5))
            {
                return(SPIT_getrangeopt( OPTblack,optptr+5, "black colours",-1,255));
            }
            else if (!strnicmp(optptr,"bold",4))
            {
                return(GX_getopt( &OPTbold,optptr+4, "bold font", 0, 1));
            }
            else if (!strnicmp(optptr,"baseline",8))
            {
                return(SPIT_getfield( &OPTbaseline[0],optptr+8, 1,0));
            }
            break;

        case 'c':
            if (!strnicmp(optptr,"contrast",8))
            {
                        d = GX_getopt( &OPTcontrastc,optptr+8, "adjust contrast", -100,100);
                if (*d==',') d = GX_getopt( &OPTcontrastb,d+1, "adjust brightness", -100,100);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"chroma",6))
            {
                d = GX_getopt( &OPTchroma,optptr+6, "chroma tolerance", 0,255);
                if (OPTchroma==1)
                    OPTchroma = 45;
                return((char *)d);
            }
            else if (!strnicmp(optptr,"comment",7))
            {
                if (*(optptr+7)>' ')
                    d = SPIT_getfield( OPTcomment,optptr+7,1,1);
                else
                {
                    d = optptr+7;
                    strcpy(OPTcomment,"%s");
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"channel",7))
            {
                return(GX_getopt( &OPTpschannel,optptr+7, "select photoshop PSD channel to start extract from", 0, 32));
            }
            /*! \page o_crop crop
              \ingroup g_filters
              \par Syntax:
              <i>-crop</i>          crop to specified dimensions (negatives ok) <br>
              <i>-crop<n></i>      crop to specified number of pixels area <br>
              <i>-crop<width>,<height></i> <br>
              <i>-crop<width>,<height>,<colour></i> <br>
              <i>-crop<x>,<y>,<width>,<height></i> <br>
              <i>-crop<x>,<y>,<width>,<height>,<colour></i> <br>
              <i>-crop<x>,<y>,<width>,<height>,<colour>,<clamp></i> <br>
              <i>-crop<x>,<y>,<width>,<height>,<colour>,<clampx>,<clampy></i> <br>
              <clamp> 0 Fill with solid colour <br>
              1 Clamp to edge <br>
              2 Wrap around <br>
              3 Mirror <br>
              <i>-cropby<left>,<top>,<right>,<bottom>,<colour></i> <br>
              <i>-cropby...</i>     crop by specified pixels (negatives for outward) <br>
              <i>-cropto...</i>     crop to specified alignment. (pos up, neg down) <br>
              <i>-cropmax...</i>    crop anything above maximum specified size <br>
              <i>-crop<n>%...</i>   crop to percentage (any arg) <br>
              <i>-croppix<n></i>    crop to specified number of pixels area (aka <i>-crop)</i> <br>
              <i>-cropby<n>%...</i> crop by percentage <br>
              <i>-cropby<left>,<top>,<right>,<bottom>,<colour></i> <br>
              \par Description
              The crop filter is amoung the most convoluted yet powerful filters that gx supports.
              The crop filter allows the user a variety of methods to crop, mirror, or expand an image.
              Cropping to an image size larger than the current dimensions will expand the image with
              a border in the specified colour. Cropping to a smaller image size can give different results
              depending on how the user calls the crop option.
              \note
              The % sign may cause problems in certain command interpreters. Be sure to escape or quote the
              surrounding expression if you choose to use the % cropping functionality.
              \par Example:
                  gx john.jpg "-crop50%" =out.tga    <i>// results in an image that is only 50% of the center region</i>
            */
            else if (!strnicmp(optptr,"crop",4))
            {
                char *td;
                int t;
                char OPTcroptemp[SPITMAXLINE];

                OPTcropw[0] = 0;//OPTAUTO;
                OPTcroph[0] = 0;// = OPTAUTO;
                OPTcropwby = 0;
                OPTcrophby = 0;
                OPTcropx[0] = 0;
                OPTcropy[0] = 0;
                OPTcropc = 0;
                OPTcropclampx = OPTAUTO;
                OPTcropclampy = OPTAUTO;
                d = GXgetscaleopt(optptr+4, OPTcropw, &OPTcropwby, OPTcroph, &OPTcrophby, "crop", 1, -32767, 32767);
                if (*d==',')
                {
                    td = SPIT_getfield( OPTcroptemp, d+1, 1,0);
                    if (*td==',')
                    {
                        strcpy(OPTcropx,OPTcropw);
                        strcpy(OPTcropy,OPTcroph);
                        OPTcropw[0] = 0;
                        OPTcroph[0] = 0;
                        d = GXgetscaleopt(d+1, OPTcropw, &t, OPTcroph, &t, "cropsize", 0, -32767, 32767);
                        if (*d==',') d = SPIT_getcolouropt(d+1, &OPTcropc, "crop colour", 0x80000000, 0x7fffffff,0);
                        if (*d==',') d = GX_getopt( &OPTcropclampx,d+1, "crop clamp x", 0, 3);
                        if (*d==',') d = GX_getopt( &OPTcropclampy,d+1, "crop clamp y", 0, 3);
                    }
                    else
                        d = SPIT_getcolouropt(d+1, &OPTcropc, "crop colour", 0x80000000, 0x7fffffff,1);
                }

                return((char *)d);
            }
            /*! \page o_clutx clutx
              \ingroup g_filters
              \par Syntax:
              <i> gx -clutx<w>,<h> </i>
              \par
              the Clutx option...
              \par Example:
            */

            else if (!strnicmp(optptr,"clutx",5))
            {
                d = GX_getopt( &OPTclutx,optptr+5, "clut x", 0, 1024);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"cluty",5))
            {
                d = GX_getopt( &OPTcluty,optptr+5, "clut y", 0, 512);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"clut",4))
            {
                d = GX_getopt( &OPTclutx,optptr+4, "clut x", 0, 1024);
                if (*d==',') d = GX_getopt( &OPTcluty,d+1, "clut y", 0, 512);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"charname",8))
            {
                return(GX_getopt( &OPTcharname,optptr+8, "use framename as name index (font style)", 0, 1));
            }
            else if (!strnicmp(optptr,"charmactodos",12))
            {
                return(GX_getopt( &OPTcharmactodos,optptr+12, "convert font characters from mac to dos ascii", 0, 1));
            }
            // dummy option to stop error mesages for these items.
            else if(!strnicmp(optptr,"csm",3))
            {
                char dummy[100];   // just grab the option off the options list, has been parsed earlier.
                return(SPIT_getfield( dummy,optptr,1,1));
            }
            else if(!strnicmp(optptr,"cramtranspose",13))
            {
                return(GX_getopt( &OPTcramtranspose,optptr+13, "Let cram option do a transpose", 0, 1));
            }
            else if(!strnicmp(optptr,"cramrotate",10))
            {
                return(GX_getopt( &OPTcramrotate,optptr+10, "Let cram option do rotation", 0, 1));
            }
            else if(!strnicmp(optptr,"cramcenterpoint",15))
            {
                return(GX_getopt( &OPTcramcenterpoint,optptr+15, "Let cram update center point info as well", 0, 1));
            }
            else if(!strnicmp(optptr,"cramcheckedge",13))
            {
                return(GX_getopt( &OPTcramcheckedge,optptr+13, "Let cram option check edge boundaries and edges", 0, 1));
            }
            else if(!strnicmp(optptr,"cramodd",7))
            {
                return(GX_getopt( &OPTcramoddodd,optptr+7, "Let cram option expand height if width odd", 0, 1));
            }
            else if(!strnicmp(optptr,"crambppsort",11))
            {
                return(GX_getopt( &OPTcrambppsort,optptr+11, "Let cram option sort by bitdepth", 0, 1));
            }
            else if(!strnicmp(optptr,"cramclutsort",12))
            {
                return(GX_getopt( &OPTcramclutsort,optptr+12, "Let cram option sort by clut bit depth", 0, 1));
            }
            else if(!strnicmp(optptr,"cramgroup",9))
            {
                OPTcramgroupsort++;   // group 0 holds all non grouped items
                                      // group 1 begins the real grouping.
                d = optptr+8;         // do one less so first iteration of d+1 can work.
                do
                {
                    d = GX_getopt( &OPTcramgroup[OPTcramgroupsort].image[OPTcramgroup[OPTcramgroupsort].num].bpp,d+1, "Bpp depth to group", 0, 32767);
                    if(*d == ',') {d = GX_getopt( &OPTcramgroup[OPTcramgroupsort].image[OPTcramgroup[OPTcramgroupsort].num].palette,d+1, "Bpp depth to group", 0, 32767);}
                    OPTcramgroup[OPTcramgroupsort].num++;
                }while(*d == ',');
                return((char *)d);

            }
            else if(!strnicmp(optptr,"cramtpage",9))
            {
                d = SPIT_getopt( &OPTcramtpagewidth,optptr+9, "cram texture page width", 0, 32767);
                if (*d==',') d = SPIT_getopt( &OPTcramtpageheight,d+1, "cram texture page height", 0, 32767);
                return((char *)d);
            }
            else if(!strnicmp(optptr,"cramwrap",8))
            {
                return(GX_getopt( &OPTcramtwrapwidth,optptr+8, "Set cram max width after which it will wrap", 0, 32767));
            }
            else if(!strnicmp(optptr,"cramwid",7))
            {
                return(GX_getopt( &OPTcramwidthin,optptr+7, "Cram option fails if width of texture is more than texture page width", 0, 1));
            }
            else if(!strnicmp(optptr,"cramspace",9))
            {
                return(GX_getopt( &OPTcramspace,optptr+9, "Put n pixels of space between textures.", 0, 32767));
            }
            else if(!strnicmp(optptr,"crampos",7))
            {
                d = SPIT_getopt( &OPTcramx,optptr+7, "Set cram x position", 0, 32767);
                if (*d==',') d = SPIT_getopt( &OPTcramy,d+1, "Set cram y position", 0, 32767);
                return((char *)d);
            }
            else if(!strnicmp(optptr,"cramnumfixed",12))
            {
                return(GX_getopt( &OPTcramnumfixed,optptr+12, "Set cram option number of shapes that are fixed and should not be moved.", 0, 32767));
            }
            else if(!strnicmp(optptr,"cram",4))
            {
                OPTcram = 1;
                d = SPIT_getopt( &OPTcramw,optptr+4, "cram width", -32768, 32767);
                if (*d==',') d = SPIT_getopt( &OPTcramh,d+1, "cram height", -32768, 32767);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"charset",7))
            {
                return(SPIT_getfield( &OPTcharset[0],optptr+7, 1,0));
            }
            else
            {
                switch(*(optptr+1) | (char)' ')
                {
                    case 'x':   return(SPIT_getpositionopt(optptr+2, &OPTcenterx[0], "centre x coordinate", -32768, 32767));
                    case 'y':   return(SPIT_getpositionopt(optptr+2, &OPTcentery[0], "centre y coordinate", -32768, 32767));
                    default:    return(GX_getopt( &OPTcodeddepth,optptr+1, "coded depth", 0, 8));

                }
            }

        case 'd':
            if (!strnicmp(optptr,"datarate",8))
            {
                d = SPIT_getfield( OPTdatarate,optptr+8,1,0);
                return((char *)d);
            }
            if (!strnicmp(optptr,"doublealpha",11))
            {
                return(GX_getopt( &OPTdoublealpha,optptr+11, "doublealpha", 0, 1));
            }
            if (!strnicmp(optptr,"doubledither",12))
            {
                return(GX_getopt( &OPTdoubledither,optptr+12, "double width with dither to argb4444", 0, 1));
            }
            if (!strnicmp(optptr,"doinvisible",11))
            {
                return(GX_getopt( &OPTnovisiblecheck,optptr+11, "treats invisible frames as visible for PSD files", 0, 1));
            }
            if(!strnicmp(optptr,"dropshadow",10))
            {
                OPTdropshadow = 1;
                d = SPIT_getfield( OPTDSLightLocationx,optptr+10,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSLightLocationy,d+1,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSLightLocationz,d+1,1,0);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dslighttype",11))
            {
                d = SPIT_getopt( &OPTDSLightType,optptr+11,"light type for drop shadowing",0,5);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dslightdir",10))
            {
                d = SPIT_getfield( OPTDSLightDirectionx,optptr+10,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSLightDirectiony,d+1,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSLightDirectionz,d+1,1,0);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dsradius",8))
            {
                 d = SPIT_getfield( OPTDSRadius,optptr+8,1,0);
                 return((char *)d);
            }
            if(!strnicmp(optptr,"dsfalloff",9))
            {
                 d = SPIT_getfield( OPTDSFalloff,optptr+9,1,0);
                 return((char *)d);
            }
            if(!strnicmp(optptr,"dsfadepower",11))
            {
                 d = SPIT_getfield( OPTDSFadePower,optptr+11,1,0);
                 return((char *)d);
            }
            if(!strnicmp(optptr,"dsfadedist",10))
            {
                 d = SPIT_getfield( OPTDSFadeDistance,optptr+10,1,0);
                 return((char *)d);
            }
            if(!strnicmp(optptr,"dsAreaL",7))
            {
                d = SPIT_getfield( OPTDSArLightAxis1x,optptr+7,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSArLightAxis1y,d+1,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSArLightAxis1z,d+1,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSArLightAxis2x,d+1,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSArLightAxis2y,d+1,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSArLightAxis2z,d+1,1,0);
                if(*d == ',') d = SPIT_getopt( &OPTDSArSizeW,d+1,"number of lights in Axis1 direction",1,100);
                if(*d == ',') d = SPIT_getopt( &OPTDSArSizeH,d+1,"number of lights in Axis2 direction",1,100);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dsLTightness",12))
            {
                d = SPIT_getfield( OPTDSLTightness,optptr+12,1,0);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dsTexPlaneOff",13))
            {
                d = SPIT_getfield( OPTDSTexPlaneOffset,optptr+13,1,0);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dsShadPlaneOff",14))
            {
                d = SPIT_getfield( OPTDSShaPlaneOffset,optptr+14,1,0);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dsLookAt",8))
            {
                d = SPIT_getfield( OPTDSLLookAtx,optptr+8,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSLLookAty,d+1,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSLLookAtz,d+1,1,0);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dsshadcol",9))
            {
                d = SPIT_getfield( OPTDSLightColourr,optptr+9,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSLightColourg,d+1,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSLightColourb,d+1,1,0);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dsCamLoc",8))
            {
                d = SPIT_getfield( OPTDSCamLocationx,optptr+8,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSCamLocationy,d+1,1,0);
                if(*d == ',') d = SPIT_getfield( OPTDSCamLocationz,d+1,1,0);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dsantialias",11))
            {
                d = SPIT_getopt(&OPTDSAntiAlias,optptr+11,"DropShadow - How Many rays to shoot for each pixel",1,4);
                return((char *)d);
            }
            if(!strnicmp(optptr,"dot3",4))
            {
                float val = 0;
                if(OPTsubtype < 0)
                    OPTsubtype = 0;
                OPTdot3[0] = 0;
                OPTdot3filter[0] = 0;
                OPTdot3wrap[0] = 0;

                d = SPIT_getfield(OPTdot3,optptr+4,1,0);
                if(*d == ',') d = SPIT_getfield( OPTdot3filter,d+1,1,0);  // filter method 0=3x3, 1=point
                if(*d == ',') d = SPIT_getfield( OPTdot3wrap,d+1,1,0);    // wrap 0=clamp, 1=wrap

                if (!OPTdot3[0])
                    strcpy(OPTdot3,"0");
                else if (!strcmp(OPTdot3,"0") && !OPTdot3filter[0]) // -dot30 to turn off dot3
                {
                    OPTdot3[0] = 0;
                    OPTdot3filter[0] = 0;
                }

                if (OPTdot3[0])
                    OPTsubtype |= 65536;

                return((char *)d);
            }
            if (!strnicmp(optptr,"dxt",3))
            {
                int dxt=0;
                d = GX_getopt2( &dxt,optptr+3, "dxt compress", -1, 5, -1);

                if (dxt)
                {
                    strcpy(OPTpack,"3"); // for shape files
                    if (dxt==3 || dxt==2)
                    {
                        OPTdepth = 28;
                    }
                    else if (dxt==5 || dxt==4)
                    {
                        OPTdepth = 27;
                    }
                    else if (dxt==1)
                    {
                        OPTdepth = 25;
                    }
                    // else -1 let bpp autoselect format
                }
                else
                    strcpy(OPTpack,"");
                return((char *)d);
            }
            if (!strnicmp(optptr,"dx",2))
            {
                return(SPIT_getpositionopt(optptr+2, &OPTdefaultdx[0], "add to default x coordinate", -32768, 32767));
            }
            if (!strnicmp(optptr,"dy",2))
            {
                return(SPIT_getpositionopt(optptr+2, &OPTdefaultdy[0], "add to default y coordinate", -32768, 32767));
            }
            if (!strnicmp(optptr,"dpi",3))
            {
                d = SPIT_getfield( OPTdpi,optptr+3,1,0);
                return((char *)d);
            }
            if (!strnicmp(optptr,"deint",5))
            {
                return(GX_getopt( &OPTdeint,optptr+5, "deinterlace", 0,1));
            }
            if (!strnicmp(optptr,"deband",6))
            {
                OPTdebandrgbdist = 15;
                OPTdebandradius =  8;
                d = GX_getopt( &OPTdebandrgbdist,optptr+6, "deband rgb distance", 0, 32767);
                if (*d==',')
                {
                    d = GX_getopt( &OPTdebandradius,d+1, "deband pixel radius", 0, 32767);
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"dilate",6))
            {
                d = SPIT_getfield( OPTdilate,optptr+6,1,0);
                return((char *)d);
            }

            return(GX_getopt( &OPTdither,optptr+1, "dither", 0, 3));

        case 'e':
            if (!strnicmp(optptr,"echo",4))
            {
                return(SPIT_getfield( OPTecho,optptr+4,1,1));
            }
            else if (!strnicmp(optptr,"embm",4))
            {
                float val = 0;
                if(OPTsubtype < 0)
                    OPTsubtype = 0;
                OPTembmscale[0] = 0;
                OPTembmfilter[0] = 0;
                OPTembmwrap[0] = 0;

                d = GX_getopt2( &OPTembm,optptr+4, "embm from height", 0, 2, 2);
                if(*d == ',') d = SPIT_getfield(OPTembmscale,d+1,1,0);                  // scale value
                if(*d == ',') d = SPIT_getfield( OPTembmfilter,d+1,1,0);  // filter method 0=3x3, 1=point
                if(*d == ',') d = SPIT_getfield( OPTembmwrap,d+1,1,0);    // wrap 0=clamp, 1=wrap

                if (OPTembm)
                    OPTsubtype |= 131072;

                return((char *)d);
            }
            else if (!strnicmp(optptr,"extract",7))
            {
                return(GX_getopt( &OPTextract,optptr+7, "extract frames to seperate files", 0, 1));
            }
            else if (!strnicmp(optptr,"edge",4))
            {
                OPTedgep = 1;
                OPTedges = 0;
                d = SPIT_getcolouropt(optptr+4, &OPTedgec, "edge background colour", 0x80000000, 0x7fffffff,0);
                if (*d==',') d = GX_getopt( &OPTedgep,d+1, "edge pixels", 1, 32);
                if (*d==',') d = GX_getopt( &OPTedges,d+1, "edge pits and/or spikes", 0, 2);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"erode",5))
            {
                d = SPIT_getfield( OPTerode,optptr+5,1,0);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"equalize",8))
            {
                d = SPIT_getfield( OPTequalize,optptr+8,1,0);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"exact",5))
            {
                OPTquantmethod = CR_EXACT;
                return((char *)(optptr+5));
            }
            break;

        case 'f':
            if (!strnicmp(optptr,"framesize",9))
            {
                d = SPIT_getfield( OPTframesize,optptr+9,1,0);
//                d = GX_getopt( &OPTframesize,optptr+9, "framesize", 0, 1024*1024);
                return((char *)d);
            }
            if (!strnicmp(optptr,"fps",3))
            {
                d = SPIT_getfield( OPTfps,optptr+3,1,0);
                return((char *)d);
            }
            if (!strnicmp(optptr,"fif",3))
            {
                d = SPIT_getfield( OPTfif,optptr+3,1,0);
                return((char *)d);
            }
            if (!strnicmp(optptr,"fill",4))
            {
                d = SPIT_getfield(OPTfillx,optptr+4,1,0);
                if (*d==',')
                {
                    d = SPIT_getfield(OPTfilly,d+1,1,0);
                    if (*d==',') d = SPIT_getcolouropt(d+1, &OPTfillc, "fill colour", 0x80000000, 0x7fffffff,1);
                }
                else
                {
                    d = SPIT_getcolouropt(optptr+4, &OPTfillc, "fill colour", 0x80000000, 0x7fffffff,1);
                    OPTfillx[0] = 0;
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"flattened",9))
            {
                return(GX_getopt( &OPTpsflattened,optptr+9, "Extract only flattened PSD image", 0, 1));
            }
            else if (!strnicmp(optptr,"flatten",7))
            {
                OPTflatten = 0;
                d = GX_getopt( &OPTflatten,optptr+7, "Flatten frames to into one", 0, 2);
                if(OPTflatten == 2)
                    OPTpaste = 1;
                return((char *)d); //GX_getopt( &OPTflatten,optptr+7, "Flatten frames to into one", 0, 2));
            }
            else if(!strnicmp(optptr,"filterdxt",9))
            {
                return(GX_getopt( &OPTfdxt,optptr+9,"DXT filter",0,6));
            }
            else if (!strnicmp(optptr,"fdxt",4))
            {
                int dxt=0;
                d = GX_getopt2( &dxt,optptr+4, "fast dxt compress", -1, 5, -1);

                if (dxt)
                {
                    strcpy(OPTpack,"4"); // for shape files
                    if (dxt==3 || dxt==2)
                    {
                        OPTdepth = 28;
                    }
                    else if (dxt==5 || dxt==4)
                    {
                        OPTdepth = 27;
                    }
                    else if (dxt==1)
                    {
                        OPTdepth = 25;
                    }
                    // else -1 let bpp autoselect format
                }
                else
                    strcpy(OPTpack,"");
                return((char *)d);
            }
            else if (!strnicmp(optptr,"ff",2))
            {
                return(SPIT_getrangeopt( OPTfilterframes,optptr+2, "filter frames",0,0x7fffffff));
            }
            else if (!strnicmp(optptr,"fu",2))
            {
                OPTquantmethod = CR_FU;
                return((char *)(optptr+2));
            }
            else if (!strnicmp(optptr,"fyi",3))
            {
                             d = SPIT_getfield( OPTFYIYScale ,optptr+3,1,0);
                if (*d==',') d = SPIT_getfield( OPTFYIYOffset,d+1,1,0);
                if (*d==',') d = SPIT_getfield( OPTFYICScale ,d+1,1,0);
                if (*d==',') d = SPIT_getfield( OPTFYICOffset,d+1,1,0);
                if (*d==',') d = SPIT_getfield( OPTFYIAScale ,d+1,1,0);
                if (*d==',') d = SPIT_getfield( OPTFYIAOffset,d+1,1,0);
                return((char *)d);
            }
            return(SPIT_getrangeoptnostrip( OPTframes,optptr+1, "frames",0,0x7fffffff));

        case 'g':

            if (!strnicmp(optptr,"gamma",5))
            {
                d = SPIT_getfield( OPTstr,SPIT_nextfield(optptr+5),1,0);
                OPTgamma = (float) atof(OPTstr);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"globalframe",11))
            {
                return(GX_getopt( &OPTglobalframe,optptr+11, "global frame counting", 0, 1));
            }
            else if ((!strnicmp(optptr,"grey",4))
                  || (!strnicmp(optptr,"gray",4)))
            {
                d = GX_getopt2( &OPTgrey,optptr+4, "make image grey scale", 0, 256, 256);
                if (OPTgrey==1)
                    OPTgrey = 256;
                return((char *)d);
            }
            else if (!strnicmp(optptr,"globalpalette",13))
            {
                return(GX_getopt( &OPTglobalpalette,optptr+13, "global palette", 0, 1));
            }
            else if (!strnicmp(optptr,"grb",3))
            {
                return(GX_getopt( &OPTgrb,optptr+3, "grb palettize", 0, 3));
            }
            else if (!strnicmp(optptr,"gshembm",7))
            {
                if(OPTsubtype < 0)
                    OPTsubtype = 0;
                OPTembm = 0;
                d = GX_getopt( &OPTembm,optptr+7, "embm from height", 0, 1);
                if(OPTembm)
                    OPTembm = 6;    // set to gsh embm type for the filter.
                OPTsubtype |= 131072;
                return((char *)d);
            }
            else if (!strnicmp(optptr,"gshunembm",9))
            {
                if(OPTsubtype < 0)
                    OPTsubtype = 0;
                OPTgshunembm = 0;
                d = GX_getopt( &OPTgshunembm,optptr+9, "embm to heightfield",0,100);
                OPTsubtype &= ~(131072);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"gop",3))
            {
                        d = GX_getopt( &OPTgopi,optptr+3, "movie gop keyframe interval", 0, 255);
                if (*d==',') d = GX_getopt( &OPTgopp,d+1, "movie gop P frame interval", 0, 255);
            }
            else if (!strnicmp(optptr,"grid",4))
            {
                             d = SPIT_getfield(OPTgridx,optptr+4,1,0);
                if (*d==',') d = SPIT_getfield(OPTgridy,d+1,1,0);
                if (*d==',') d = SPIT_getfield(OPTgridw,d+1,1,0);
                if (*d==',') d = SPIT_getfield(OPTgridh,d+1,1,0);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPTgridc, "grid colour", 0x80000000, 0x7fffffff,1);
                return((char *)d);
            }
            // This will make the FourCC one of:
            // "GST0" = gst121
            // "GST1" = gst221
            // "GST2" = gst421
            // "GST3" = gst821
            // "GST4" = gst122
            // "GST5" = gst222
            // "GST6" = gst422
            // "GST7" = gst822
            // In terms of bits:
            // 0011 0bww
            // b  - bits per pixel 0 = 1bpp  1 = 2bpp
            // ww - blockwidth 0 = 1 pixel
            //                 1 = 2 pixels
            //                 2 = 4 pixels
            //                 3 = 8 pixels
            else if(!strnicmp(optptr,"gst",3))
            {
                int bw   = 4;
                int bh   = 2;
                int dbpp = 2;
                unsigned int mask = 0x30;
                             d = GX_getopt2( &bw,optptr+3, "blockwidth", 0, 8, 4);
                if(*d ==',') d = GX_getopt2( &dbpp,d+1, "detailbpp", 1, 2, 2);
                if(bw != 1 && bw != 2 && bw != 4 && bw != 8)
                {
                    gprintf("GST block width must be one of 1,2,4,8 setting to default of 4\n");
                    bw = 4;
                }
                bw   = bw > 6? 3 : (bw>>1);  // set to 0..3 range to pack into the FourCC.
                dbpp >>= 1;                  // integer divide. Makes the value range from 0..1 for packing into FourCC.
                mask |= ((bw)|(dbpp<<2));    // last char of fourCC will range from '0'..'7' as stated above.
                strcpy(&OPTpack[0],"'GST '");  // OPTpack has a funny hack where first byte is pack number other bytes are FourCC
                OPTpack[4] = (unsigned char)mask;
                return (char *)d;
            }
            break;

        case 'h':
            if (!strnicmp(optptr,"halfsize",8))
            {
                return(GX_getopt2( &OPThalfsize,optptr+8, "halfsize", 0, 3, 3));
            }
            else if (!strnicmp(optptr,"halfalpha",9))
            {
                d = GX_getopt( &OPThalfalpha,optptr+9, "halfalpha", 0, 1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"halfpixel",9))
            {
                return(GX_getopt( &OPThalfpixel,optptr+9, "halfpixel", 0, 1));
            }
            else if (!strnicmp(optptr,"hflip",5))
            {
                return(GX_getopt( &OPThflip,optptr+5, "hflip", 0, 1));
            }
            else if (!strnicmp(optptr,"hsl",3))
            {
                        d = GX_getopt( &OPThslh,optptr+3, "adjust hsl hue", -180,360);
                if (*d==',') d = GX_getopt( &OPThsls,d+1, "adjust hsl saturation", -100,100);
                if (*d==',') d = GX_getopt( &OPThsll,d+1, "adjust hsl lightness", -100,100);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"hsv",3))
            {
                        d = GX_getopt( &OPThsvh,optptr+3, "adjust hsv hue", -180,360);
                if (*d==',') d = GX_getopt( &OPThsvs,d+1, "adjust hsv saturation", -100,100);
                if (*d==',') d = GX_getopt( &OPThsvb,d+1, "adjust hsv brightness", -100,100);
                if (*d==',') d = GX_getopt( &OPThsvv,d+1, "adjust hsv value", -100,100);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"hotadjust",9))
            {
                return(GX_getopt( &OPThotadjust,optptr+9, "hot adjust", -1, 1));
            }
            else if (!strnicmp(optptr,"hotadd",6))
            {
                        d = GX_getopt( &OPThotn,optptr+6, "hot spot to add to", 0, 32);
                if (*d==',') d = GX_getopt( &OPThotx,d+1, "hot x add", -32000, 32000);
                if (*d==',') d = GX_getopt( &OPThoty,d+1, "hot y add", -32000, 32000);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"hash",4))
            {
                return(GX_getopt( &OPThashlen,optptr+4, "hash framename to specified length", 0, 512));
            }
            else if(!strnicmp(optptr,"high",4))
            {
                return(GX_getopt( &OPTpriorityhigh,optptr+4, "high priority", 0, 1));
            }

            break;

        case 'i':
            if (!strnicmp(optptr,"if",2))
            {
                d = SPIT_getfield( OPTif,optptr+2,1,0);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"invert",6))
            {
                OPTinvert = 0x00ffffff;
                d = optptr+6;
                if (d[0]>' ')
                    d = SPIT_getcolouropt(d, &OPTinvert, "invert ARGBs", 0x80000000, 0x7fffffff,1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"italic",6))
            {
                return(GX_getopt( &OPTitalic,optptr+6, "italic font", 0, 1));
            }
            else if (!strnicmp(optptr,"info",4))
            {
                d = GX_getopt2( &OPTinfo,optptr+4, "info", -1, 6, -1);
                return((char *)d);
            }

            return(GX_getopt( &OPTinverse,optptr+1, "inverse table", 0, 2));

        case 'l':
            if (!strnicmp(optptr,"line",4))
            {
                             d = SPIT_getfield(OPTlinex,optptr+4,1,0);
                if (*d==',') d = SPIT_getfield(OPTliney,d+1,1,0);
                if (*d==',') d = SPIT_getfield(OPTlinex2,d+1,1,0);
                if (*d==',') d = SPIT_getfield(OPTliney2,d+1,1,0);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPTlinec, "line colour", 0x80000000, 0x7fffffff,1);

                return((char *)d);
            }
            else if(!strnicmp(optptr,"low",3))
            {
                return(GX_getopt( &OPTprioritylow,optptr+3, "low priority", 0, 1));
            }
            else if(!strnicmp(optptr,"layered",7))   // if mipmap treat as if layered.
            {
                return(GX_getopt( &OPTlayered,optptr+7, "layered", 0, 1));
            }
            else if(!strnicmp(optptr,"log",3))
            {
                if (optptr[3]>='0' && optptr[3]<='2')
                {
                    d = SPIT_getopt( &OPTlog,optptr+3, "log", 0, 2);
                }
                else
                {
                    d = SPIT_getfield( OPTlogfile,SPIT_nextfield(optptr+3),1,0);
                    if (gstdout==stdout)
                    {
                        if (OPTlogfile[0])
                            gstdout=fopen(OPTlogfile,"w");
                    }
                }
                return((char *)d);
            }
            // PAULDING START
            else if(!strnicmp(optptr,"layer",5))
            {
                return(SPIT_getrangeoptnostrip( OPTlayers,optptr+5, "layers",0,0x7fffffff));
            }
            // PAULDING END
            if (isdigit(optptr[1]))
                return(SPIT_getrangeopt( OPTtranslucient,optptr+1, "translucient colours",-1,255));
            break;

        case 'm':
            if(!strnicmp(optptr,"mkftexmap",9))
            {
                return(GX_getopt( &OPTMkFTexMap,optptr+9, "Make Facial Texture map from several source images.", 0, 1));
            }
            else if (!strnicmp(optptr,"mipmont",7))
            {
                return(GX_getopt( &OPTmipmont,optptr+7, "mipmap montage for ps2", 0, 1));
            }
            else if (strnicmp(optptr,"mips", 4) == 0 || strnicmp(optptr, "mipmap", 6) == 0)
            {
                int idx = (optptr[3] == 'm') ? 6 : 4;

                if (optptr[idx] == 'b')
                {
                    d = optptr + idx + 1;
                    OPTmipmapmethod = OPTMIPMAPBOX;
                }
                else if (optptr[idx] == 's')
                {
                    d = optptr + idx + 1;
                    OPTmipmapmethod = OPTMIPMAPSIZE;
                }
                else if (optptr[idx] == 'c')
                {
                    d = optptr + idx + 1;
                    OPTmipmapmethod = OPTMIPMAPSIZECOLREDUCE;
                }
                else if (optptr[idx] == 'p')
                {
                    d = optptr + idx + 1;
                    OPTmipmapmethod = OPTMIPMAPSCALE;
                }
                else
                {
                    d = optptr + idx;
                    OPTmipmapmethod = OPTMIPMAPBOX;
                }

                OPTmipmap = OPTMIPMAPDEFAULT;
                if (isdigit(*d))
                {
                    d = GX_getopt( &OPTmipmap,d, "mipmap", 0, 16);
                }

                /* Get mipmap minsize if specified. */
                if (*d==',')
                {
                    if (!OPTmipmap)
                        OPTmipmap = OPTMIPMAPDEFAULT;
                    d = GX_getopt( &OPTmipmapmin,d+1, "mipmap width", 0, 5000);
                }

                if (OPTmipmap == 1)
                    OPTmipmap = 0;      /* 1 level of mipmap makes no sense. */

                if (*d==',')
                {
                    OPTmipmapwidth = OPTmipmapmin;
                    OPTmipmapmin = 1;
                    d = GX_getopt( &OPTmipmapheight,d+1, "mipmap height", 0, 5000);

                    if (*d==',') d = GX_getopt( &OPTmipmapmin,d+1, "mipmap min size", 1, 5000);
                    if (*d==',') d = GX_getopt( &OPTmipmaptrans,d+1, "maintain sharp transparent edges", 0, 1);
                    if (*d==',') d = GX_getopt( &OPTmipmapchromakey,d+1, "transparent colour for sharp edge test", 0x80000000, 0x7fffffff);
                    if (*d==',') d = GX_getopt( &OPTmipmapclearlast,d+1, "clean last n mipmaps to background colour", 0x80000000, 0x7fffffff);
                }

                return((char *)d);
            }
            // note that both -m and -ma should update if quant methods change in crapi
            else if (!strnicmp(optptr,"ma",2))
                return(GX_getopt( &OPTquantmethodalpha,optptr+2, "quantizer method for alpha art", -1, 10));/*getnumcrapi()-1)); */
            return(GX_getopt( &OPTquantmethod,optptr+1, "quantizer method", -1, 10));/*getnumcrapi()-1)); */

            /*! \page o_quantizer_method Quantizer Method
                \ingroup g_colour_reduction
               \par Syntax:
               <i> -m<w>,<h> </i><br>
               <i> -m<n></i>          colour reduction method -1,1..9 (1 default) <br>
               \subsection o_quant_meth Methods
                    <b>-m-1</b>         -1 auto select method based on num colours or argb bits <br>
                    <b>-m1</b>          Wu (default). Best for 24 bit <br>
                    <b>-m2</b>          Fu with 3555 colour cube <br>
                    <b>-m3</b>          Wu with 777 colour cube <br>
                    <b>-m7</b>          exact simple histogram preserves alpha & order <br>
                    <b>-m8</b>          Alpha <b>-</b> WU's with alpha <br>
                    <b>-m9</b>          Wu with 555 colour cube <br>
               \par Description
               Gx allows a user to force it to use a specific quantization method when colour reducing.
               Gx will by default attempt to make an educated guess as to the best quantization method,
               but it does not know all that you do about the image in question.
               \par Example
                   gx -m9 -8 alphaTexture.tga =out.gif <i> // Generates an 8 bit image using the wu quantizer </i>
            */

        case 'n':
            if (!strnicmp(optptr,"name",4))
            {
                if (optptr[4]<=' ') // -name to turn off naming
                {
                    strcpy(OPTframename,"%s");
                    OPTframenameexpr[0] = 0;
                    d = optptr+4;
                }
                else
                {
                    d = SPIT_getfield(OPTframename,optptr+4,1,0);
                    if (*d==',') d = SPIT_getfield(OPTframenameexpr,d+1,1,0);

                    char testname[512];
                    sprintf(testname,OPTframename,"abcd","abcd","abcd","abcd");
//                    int l = strlen(testname);
//                    if (l<OPThashlen)
//                        OPThashlen = l;
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"noremap",7))
            {
                return(GX_getopt( &OPTnoremap,optptr+7, "force no auto colour remapping", 0, 1));
            }
            else if (!strnicmp(optptr,"nocomment",9))
            {
                // could be done as subtype 8 for shapes, but comments are
                // supported by many formats, and -comment sets the string
                // so -comment can not be the subtype vs -nocomment

                strcpy(OPTcomment,"");
                return((char *)optptr+9);
            }
            return(GX_getopt( &OPTcolours,optptr+1, "number of colours", 1, 256));

        case 'o':
            if (!strnicmp(optptr,"opaque",6))
            {
                return(SPIT_getrangeopt( OPTopaque,optptr+6, "opaque colours",-1,255));
            }
            else if (!strnicmp(optptr,"outline",7))
            {
                             d = SPIT_getfield(OPToutlinex,optptr+7,1,0);
                if (*d==',') d = SPIT_getfield(OPToutliney,d+1,1,0);
                if (*d==',') d = SPIT_getfield(OPToutlinew,d+1,1,0);
                if (*d==',') d = SPIT_getfield(OPToutlineh,d+1,1,0);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPToutlinec, "outline colour", 0x80000000, 0x7fffffff,1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"overwrite",9))
            {
                return(GX_getopt( &OPToverwrite,optptr+9, "allow overwriting original file", 0, 2));
            }
            else if (!strnicmp(optptr,"openas",6))
            {
                char openasstr[SPITMAXLINE];

                d = SPIT_getfield(openasstr,SPIT_nextfield(optptr+6),1,0);
                type = GIMEX_find(openasstr,NULL);
                if (type)
                {
                    OPTopenastype = type;
                }
            }
            else
            {
                *(char *)optptr = '=';
                d = optptr;
            }
            break;

        case 'p':
            if (!strnicmp(optptr,"palgrad",7))
            {
                OPTpalgradargbq = 0;
                        d = GX_getopt( &OPTpalgradstartindex,optptr+7, "palette gradient start colour index", -1, 255);
                if (*d==',') d = GX_getopt( &OPTpalgradendindex,d+1, "palette gradient end colour index", 0, 255);
                if (*d==',')
                {
                    OPTpalgradargbq = 1;
                    d = SPIT_getcolouropt(d+1, &OPTpalgradstartargb, "palette gradient start argb colour", 0x80000000, 0x7fffffff,0);
                    if (*d==',') d = SPIT_getcolouropt(d+1, &OPTpalgradendargb, "palette gradient end argb colour", 0x80000000, 0x7fffffff,0);
                    if (*d==',') d = SPIT_getcolouropt(d+1, &OPTpalgradmaskargb, "palette gradient mask argb colour", 0x80000000, 0x7fffffff,1);
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"palcopy",7))
            {
                              d = SPIT_getfield( &OPTpalcopydest[0],optptr+7, 1,0);
                if (*d==',')  d = SPIT_getrangeopt( OPTpalcopysource,d+1, "palsort source palette range",0,255);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"palmove",7))
            {
                              d = SPIT_getfield( &OPTpalmovedest[0],optptr+7, 1,0);
                if (*d==',')  d = SPIT_getrangeopt( OPTpalmovesource,d+1, "palmove source palette range",0,255);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"palswap",7))
            {
                              d = SPIT_getfield( &OPTpalswapdest[0],optptr+7, 1,0);
                if (*d==',')  d = SPIT_getrangeopt( OPTpalswapsource,d+1, "palswap source palette range",0,255);
                return((char *)d);
            }
           /*! \page o_pixel pixel
              \ingroup g_advanced
              \par Syntax:
              <i>-pixel<r>,<g>,\<b\></i> replace pixels using expressions <br>
              <i>-pixel<a>,<r>,<g>,\<b\>,\<i\>,<x>,<y></i> pixel expression <br>
              \param a alpha expression <br>
              \param r red expression <br>
              \param g green expression <br>
              \param b blue expression <br>
              \param i index expression for 8 bit image <br>
              \param x map destination x to source x with expression <br>
              \param y map destination y to source y with expression <br>
              \par Symbols available:
              a,r,g,b,x,argb,frame,framenum,width,height,bpp, <br>
              startcolour,numcolours,colours,subtype,packed, <br>
              quality,framesize,alphabits,redbits,greenbits, <br>
              bluebits,centerx,centery,defaultx,defaulty, <br>
              numhotspots,fps,dpi,ispal,framename <br>
              hot0x,hot0y,hot1x,hot1y,hot2x,hot2y,hot3x,hot3y, <br>
              hot4x,hot4y,hot5x,hot5y,hot6x,hot6y,hot7x,hot7y, <br>
              hot8x,hot8y,hot9x,hot9y,hot10x,hot10y,hot11x,hot11y, <br>
              hot12x,hot12y,hot13x,hot13y,hot14x,hot14y, <br>
              hot15x,hot15y,hot16x,hot16y,hot17x,hot17y, <br>
              hot18x,hot18y,hot19x,hot19y <br>
              a_ prefix for auxilliary file variables (see -a) <br>
              ispal is for expressions on the 8 bpp index \<i\> which are <br>
              applied to both palette index and image <br>

              \par Functions available:
              <i>
              min(...)     smallest of n values <br>
              max(...)     largest of n values <br>
              int(n)       cast to integer (truncate toward 0) <br>
              round(n)     round to nearest integer <br>
              floor(n)     truncate down to integer (negative away from 0) <br>
              ceil(n)      truncate up to integer (negative toward 0) <br>
              sin(n)       sine of radian angle <br>
              cos(n)       cosine of radian angle <br>
              tan(n)       tan of radian <br>
              asin(n)      arcsine <br>
              acos(n)      arccosine <br>
              atan(n)      arctangent <br>
              exp(n)       e raised to power of n <br>
              log(n)       log base e of n <br>
              log2(n)      log base 2 of n <br>
              log10(n)     log base 10 of n <br>
              sqrt(n)      square root of n <br>
              rand(max)    float random up to/including max <br>
              hypot(...)   hypotenous of n dimensional vertex <br>
              floorpow2(x) next lower power of 2 of x <br>
              ceilpow2(x)  next higher power of 2 of x <br>
              pow(x,y)   x to the power of y <br>
              </i>
              \par Description
              The -pixel option is probably the most complex and powerful option that Gx has. The pixel
              option allows developers to modify an image on a per pixel basis. Users specify an expression
              that is then applied to each pixel in the image. The pixel option offers a series of variables
              to access many different image components. An auxiliary file can also be used in conjunction
              with a pixel expression to combine two images.
              \par Example
                  gx -pixelg,g,g,g john.jpg =out.tga <i>// takes the green channel and copies it into all other channels</i><br>
                  gx -pixela,r,a_g,b john.jpg -aother.jpg =out.tga <i>takes other.jpg's green channel and copies it into john's green channel</i>
            */
            else if (!strnicmp(optptr,"pixel",5))
            {
                /* 3 args = rgb */
                              d = SPIT_getfield( &OPTpixelr[0],optptr+5, 1,0);
                if (*d==',')  d = SPIT_getfield( &OPTpixelg[0],d+1, 1,0);
                if (*d==',')  d = SPIT_getfield( &OPTpixelb[0],d+1, 1,0);
                if (*d==',')
                {
                    /* 4 args = argb.  5th is i for 8 bit index mapping */

                    strcpy(OPTpixela,OPTpixelr);
                    strcpy(OPTpixelr,OPTpixelg);
                    strcpy(OPTpixelg,OPTpixelb);
                    OPTpixelb[0]=0;
                                  d = SPIT_getfield( &OPTpixelb[0],d+1, 1,0);
                    if (*d==',')  d = SPIT_getfield( &OPTpixeli[0],d+1, 1,0);
                    if (OPTverbose>=5)
                        printf("-pixel i got %s.. next is %s\n",OPTpixeli,d);
                    if (*d==',')  d = SPIT_getfield( &OPTpixelx[0],d+1, 1,0);
                    if (*d==',')  d = SPIT_getfield( &OPTpixely[0],d+1, 1,0);
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"preten",6))
            {
                OPTpreten = -1;
                return(GX_getopt2( &OPTpreten,optptr+6, "preattenuate ARGBs", -1, 1, 0));
            }
            else if (!strnicmp(optptr,"palsort",7))
            {
                d = SPIT_getfield(OPTpalsortmethod,optptr+7, 1,0);
                if (*d==',')
                {
                    d = GX_getopt( &OPTpalsortblack,d+1, "force 1=black0,2=trans255,3=trans0", 0,3);
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"pass",4))
            {
                return(GX_getopt( &OPTpass,optptr+4, "passes of image process", -1, 2));
            }
            else if (!strnicmp(optptr,"paste",5))
            {
                return(GX_getopt( &OPTpaste,optptr+5, "composite paste frames", 0, 1));
            }
            else if (!strnicmp(optptr,"pack",4))
            {
                return(SPIT_getfield( &OPTpack[0],optptr+4, 0,0));
            }
            else if (!strnicmp(optptr,"psnr",4))
            {
                return(GX_getopt( &OPTpsnr,optptr+4, "calculate psnr in compositor", 0, 2));
            }
            else if (!strnicmp(optptr,"pointsize",9))
            {
                return(SPIT_getfield( &OPTpointsize[0],optptr+9, 1,0));
            }
            else if(!strnicmp(optptr,"pscolred",8))
            {
                return(GX_getopt( &OPTpscolred,optptr+8, "use photoshop7 to colour reduce", 8, 32));
            }
            else if (!strnicmp(optptr,"pq",2))
            {
                OPTquantmethod = CR_PQ;
                return((char *)(optptr+2));
            }
            // else -p<file>
            {
                char framename[SPITMAXLINE];
                d = SPIT_getfield( OPTpalfile,optptr+1,1,0);
                if (*d==',') d = SPIT_getfield(framename,d+1, 1,0);
                //d = GX_getopt(&framenumber,d+1, "palette from frame number",0,32000);
                GXLoadPalette(&OPTdepth,OPTpalfile,framename);
            }
            return((char *)d);

        case 'q':
            return(SPIT_getfield( &OPTquality[0],optptr+1, 1,0));
//            return(GX_getopt( &OPTquality,optptr+1, "quality", -2, 100));

        case 'r':
            if (!strnicmp(optptr,"rotate",6))
            {
                return(GX_getopt( &OPTrotate,optptr+6, "rotate degrees", -360, 360));
            }
            else if (!strnicmp(optptr,"rot",3))
            {
                return(GX_getopt( &OPTrot,optptr+3, "rotate amount", -4, 4));
            }
            else if (!strnicmp(optptr,"rect",4))
            {
                             d = SPIT_getfield(OPTrectx,optptr+4,1,0);
                if (*d==',') d = SPIT_getfield(OPTrecty,d+1,1,0);
                if (*d==',') d = SPIT_getfield(OPTrectw,d+1,1,0);
                if (*d==',') d = SPIT_getfield(OPTrecth,d+1,1,0);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPTrectc, "rect colour", 0x80000000, 0x7fffffff,1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"replace",7))
            {
                        d = SPIT_getcolouropt(optptr+7, &OPTreplaces,  "replace search colour", 0x80000000, 0x7fffffff, 0);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPTreplacer,  "replace colour",        0x80000000, 0x7fffffff, 0);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPTreplacesm, "search colour mask",    0x80000000, 0x7fffffff, 0);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPTreplacerm, "replace colour mask",   0x80000000, 0x7fffffff, 0);
                if (*d==',') d = GX_getopt( &OPTreplacesn,d+1, "search not",0,1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"remove",6))
            {
                return(GX_getopt( &OPTremove,optptr+6, "composite remove frame (subtract with clip to 0)", 0, 1));
            }
            else if (!strnicmp(optptr,"reset",5))
            {
                GXFILTER_reset();
                return((char *)optptr+5);
            }
            else if (!strnicmp(optptr,"return",6))
            {
                d = SPIT_getfield( OPTreturn,optptr+6,1,0);
                return((char *)d);
            }

            break;

        case 's':
            if(!strnicmp(optptr,"safe2_grey",10))
            {
                return(GX_getopt(&OPTsafe2grey,optptr+10, "grey threshold for safe2 option", 0, 254));
            }
            else if (!strnicmp(optptr,"safe",4))
            {
                d = SPIT_getfield( OPTsafe,optptr+4,1,0);
                if (!OPTsafe[0])
                    strcpy(OPTsafe,"1");
                if (!strcmp(OPTsafe,"0"))
                    OPTsafe[0] = 0;
                return((char *)d);
            }
            else if (!strnicmp(optptr,"size",4))
            {
                OPTsizex[0] = 0;
                OPTsizey[0] = 0;
                OPTsizexby = 0;
                OPTsizeyby = 0;
                OPTsizetrans = 0;
                OPTsizemethod = 0;
                OPTsizesharpx = 100;
                OPTsizesharpy = 100;
                d = GXgetscaleopt(optptr+4, OPTsizex, &OPTsizexby, OPTsizey, &OPTsizeyby, "size", 1, -32767, 32767);
                if (*d==',') d = GX_getopt( &OPTsizemethod,d+1, "resize method", 0, 7);
                if (*d==',') d = GX_getopt( &OPTsizetrans,d+1, "maintain sharp transparent edges", 0, 1);
                if (*d==',') d = GX_getopt( &OPTsizechromakey,d+1, "transparent colour for sharp edge test", 0x80000000, 0x7fffffff);
                if (*d==',') d = GX_getopt( &OPTsizesharpx,d+1, "horizontal sharpness", 1, 1000);
                if (*d==',') d = GX_getopt( &OPTsizesharpy,d+1, "vertical sharpness", 1, 1000);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"stereo",6))
            {
                d = GX_getopt( &OPTstereofocus,optptr+6, "stereo gram focus", 0, 256);
                if (*d==',') d = GX_getopt( &OPTstereorange,d+1, "stereo gram range", 0, 256);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"shell",5))
            {
                return(SPIT_getfield( OPTshell,optptr+5,1,1));
            }
            else if (!strnicmp(optptr,"scale",5))
            {
                d = GXgetscaleopt(optptr+5, OPTscalex, &OPTscalexby, OPTscaley, &OPTscaleyby, "scale", 1, -32767, 32767);
                return((char *)d);
            }
#if 0
            else if (!strnicmp(optptr,"startcolour",11))
            {
                return(GX_getopt( &OPTstartcolour,optptr+7, "sharpen image amount (16 reasonable)", 0, 256));
            }
#endif
            else if (!strnicmp(optptr,"sharpen",7))
            {
                return(GX_getopt2( &OPTsharpen,optptr+7, "sharpen image method", 0, 2,1));
            }
            else if (!strnicmp(optptr,"shrinktofit",11))
            {
                return(GX_getopt2( &OPTshrinktofit,optptr+11, "shrink to fit", 0, 3, 3));
            }
            else if (!strnicmp(optptr,"setcolour",9)
                  || !strnicmp(optptr,"setcolor",8))
            {
                if (!strnicmp(optptr,"setcolour",9))
                    d = optptr+9;
                else
                    d = optptr+8;
                d = SPIT_getfield( OPTsetcolour,SPIT_nextfield(d),1,0);
                if (*d==',')
                {
                    d = SPIT_getcolouropt(d+1, &OPTsetcolourvalue, "set colour value", 0x80000000, 0x7fffffff,0);
                    if (*d==',')
                    {
                        d = SPIT_getcolouropt(d+1, &OPTsetcolourmask, "set colour mask", 0x80000000, 0x7fffffff,1);
                    }
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"sort",4))
            {
                return(GX_getopt( &OPTsorttype,optptr+4, "sort names", 0, 5));
            }
            else if (!strnicmp(optptr,"subtype",7))
            {
                return(GX_getopt( &OPTsubtype,optptr+7, "subtype", 0x80000000, 0x7fffffff));
            }
            else if (!strnicmp(optptr,"sub",3))
            {
                return(GX_getopt( &OPTsub,optptr+3, "composite subtract frames", 0, 1));
            }
            else if (!strnicmp(optptr,"startindex",10))
            {
                return(GX_getopt( &OPTstartindex,optptr+10, "start index for export filenames", 0, 0x7fffffff));
            }
            else if (!strnicmp(optptr,"swap",4))
            {
                d = GX_getopt( &OPTswap1,optptr+4, "swap colour 1", 0, 255);
                if (*d==',')
                {
                    d = GX_getopt( &OPTswap2,d+1, "swap colour 2", 0, 255);
                }
                return((char *)d);
            }
#if 0
            else if (!strnicmp(optptr,"shadow",6))
            {
                        d = GX_getopt( &OPTshadowx1,optptr+6, "shadow x1", -200, 200);
                if (*d==',') d = GX_getopt( &OPTshadowx2,d+1, "shadow x1", -200, 200);
                if (*d==',') d = GX_getopt( &OPTshadowy,d+1,  "shadow y",  -200, 200);
                if (*d==',') d = GX_getopt( &OPTshadowdx,d+1, "shadow dx", -200, 200);
                if (*d==',') d = GX_getopt( &OPTshadowdy,d+1, "shadow dy", -200, 200);
                return((char *)d);
            }
#endif
            else if (!strnicmp(optptr,"semitrans",6))
            {
                return(GX_getopt( &OPTsemitrans,optptr+9, "semitrans", 0, 5));
            }
            else if (!strnicmp(optptr,"sethot",6))
            {
                d = SPIT_getfield( OPThotspots,SPIT_nextfield(optptr+6),1,1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"set",3))
            {
                char *x;
//                char OPTexpr[SPITMAXLINE];

                d = SPIT_getfield( OPTset,SPIT_nextfield(optptr+3),1,1);

                x = strchr(OPTset,'=');
                if (x)
                {
//                    *x++ = 0;
//                    EXPR_set(expr,OPTset,EXPR_get(expr, x));
//                    *--x = '=';

                    return((char *)d);
                }
                else
                    OPTset[0] = 0;
            }
            else if (!strnicmp(optptr,"screen",6))
            {
                OPTscreenorientation = 0;
                             d = SPIT_getcolouropt(optptr+6, &OPTscreencolour, "screen colour", 0x80000000, 0x7fffffff,1);
                if (*d==',') d = GX_getopt( &OPTscreenorientation,d+1, "screen orientation", -1,1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"style",5))
            {
                return(SPIT_getfield( &OPTstyle[0],optptr+5, 1,0));
            }
            else if (!strnicmp(optptr,"smallcaps",9))
            {
                return(SPIT_getfield( &OPTsmallcaps[0],optptr+9, 1,0));
            }

            break;


            return(GX_getopt( &OPTsubtype,optptr+1, "subtype", 0x80000000, 0x7fffffff));

        case 't':
            if (!strnicmp(optptr,"tblack",6))
            {
                type = 0;
                OPTreplaces  = OPTAUTO;
                d = GX_getopt( &type,optptr+6, "make black transparent", 0, 1);
                if (type)
                {
                    OPTreplaces  = 0x00000000;
                    OPTreplacer  = 0x00000000;
                    OPTreplacesm = 0x00f8f8f8;
                    OPTreplacerm = 0xffffffff;
                    OPTreplacesn = 0;
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"twhite",6))
            {
                type = 0;
                OPTreplaces  = OPTAUTO;
                d = GX_getopt( &type,optptr+6, "make white transparent", 0, 1);
                if (type)
                {
                    OPTreplaces  = 0x00ffffff;
                    OPTreplacer  = 0x00000000;
                    OPTreplacesm = 0x00f8f8f8;
                    OPTreplacerm = 0xffffffff;
                    OPTreplacesn = 0;
                }
                return((char *)d);
            }
            else if (!strnicmp(optptr,"time",4))
            {
                return(GX_getopt( &OPTtime,optptr+4, "enable timing diagnostics", 0, 1));
            }
            else if (!strnicmp(optptr,"tedge",5))
            {
                d = GX_getopt( &OPTtedge,optptr+5, "transparent edge extrude", 0, 1);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"thumb",5))
            {
                if (optptr[5]<=' ')
                {
                    GXgetscaleopt("128,102", OPTthumbx, &OPTthumbxby, OPTthumby, &OPTthumbyby, "thumb", 1, -32767, 32767);
                    d = optptr+5;
                }
                else
                    d = GXgetscaleopt(optptr+5, OPTthumbx, &OPTthumbxby, OPTthumby, &OPTthumbyby, "thumb", 1, -32767, 32767);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"temporalfix",11))
            {
                return(GX_getopt( &OPTtemporalfix,optptr+11, "temporalfix", 0,1));
            }
            else if (!strnicmp(optptr,"transpose",9))
            {
                d = GX_getopt( &OPTtranspose,optptr+9, "Transpose Image", 0, 1);
                return((char *)d);
            }
            else if (!strnicmp(optptr, "text", 4))
            {
                d = SPIT_getfield(OPTtextstring, optptr+4, 1, 0);
                if (*d==',') d = GX_getopt(&OPTtextx, d+1, "x output for font", 0, 10000);
                if (*d==',') d = GX_getopt(&OPTtexty, d+1, "y output for font", 0, 10000);
                if (*d==',') d = GX_getopt(&OPTtextsize, d+1,"font size", 0, 100);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPTtextcolour, "font colour", 0x80000000, 0x7fffffff,0);
                if (*d==',') d = GX_getopt(&OPTtextmode, d+1,"font mode", 0, 1);
                if (*d==',') d = SPIT_getfield(OPTtextfont, d+1, 1,0);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"typeface",8))
            {
                return(SPIT_getfield( &OPTtypeface[0],optptr+8, 1,0));
            }
            else if (!strnicmp(optptr,"tighten",7))
            {
                return(SPIT_getfield( &OPTtighten[0],optptr+7, 1,0));
            }
            else if (!strnicmp(optptr,"tracking",8))
            {
                return(SPIT_getfield( &OPTtracking[0],optptr+8, 1,0));
            }

            return(SPIT_getrangeopt( OPTtransparent,optptr+1, "transparent colours",-1,255));

        case 'u':
            if (!strnicmp(optptr,"unembm",6))
            {
                if(OPTsubtype < 0)
                    OPTsubtype = 0;
                OPTunembm = 0;
                d = GX_getopt( &OPTunembm,optptr+6, "embm to heightfield",0,100);
                OPTsubtype &= ~(131072);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"undot3",6))
            {
                if(OPTsubtype < 0)
                    OPTsubtype = 0;
                OPTunembm = 0;
                d = GX_getopt( &OPTundot3,optptr+6, "dot3 to heightfield",0,100);
                OPTsubtype &= ~(65536);
                return((char *)d);
            }
            else if (!strnicmp(optptr,"unpack",6))
            {
                strcpy(OPTpack,"0");
                return (char *)optptr+6;
            }
            else if(!strnicmp(optptr,"usersize",8))
            {
                d = SPIT_getfield(OPTUserSize,optptr+8,1,0);
                return((char *)d);
            }
            break;

        case 'v':
            if (!strnicmp(optptr,"vflip",5))
            {
                return(GX_getopt( &OPTvflip,optptr+5, "vflip", 0, 1));
            }
            else if (!strnicmp(optptr,"verbosity",9))
            {
                return(GX_getopt( &OPTverbose,optptr+9, "verbosity", 0, 6));
            }
            break;

        case 'w':
            if (!strnicmp(optptr,"warp",4))
            {
                int count = 0;
                OPTgen    = 0;
                OPTwarpy1 = 0;
                OPTwarpx2 = 0;
                OPTwarpy2 = 0;
                OPTwarpc = 0;
                OPTwarpclampx = 0;
                OPTwarpclampy = 0;
                OPTwarptype = 0;
                if(*(optptr+4) != '?')
                {
                d = SPIT_getopt( &OPTwarptype,optptr+4, "warp type", 1, 5);
                if (*d==',') d = SPIT_getopt( &OPTwarpclampx,d+1, "warp clamp x", 0, 4);
                if (*d==',') d = SPIT_getopt( &OPTwarpclampy,d+1, "warp clamp y", 0, 4);
                if (*d==',') d = SPIT_getcolouropt(d+1, &OPTwarpc, "warp colour", 0x80000000, 0x7fffffff,0);
                if (*d==',') d = SPIT_getopt(&OPTgen,d+1, "warp point", 0, 32767);
                if(OPTwarptype == 1)
                {
                    if (*d==',') d = SPIT_getopt(&OPTwarpy1,d+1, "warp y1", 0, 32767);
                    if (*d==',') d = SPIT_getopt(&OPTwarpx2,d+1, "warp x2", 0, 32767);
                    if (*d==',') d = SPIT_getopt(&OPTwarpy2,d+1, "warp y1", 0, 32767);
                }
                else if(OPTwarptype == 3 || OPTwarptype == 4)
                {
                    while(count < OPTgen*8 && *d == ',')
                    {
                        d = SPIT_getopt(&OPTwarpcontlines[count],d+1, "warp point", 0, 32767);
                        count++;
                    }
                    if(count != OPTgen*8)
                    {
                        printf("Error line information does not match line count\n");
                        OPTwarptype = 0;
                        return((char *)d);
                    }
                }
                }
                else
                {
                    char dummy[100];   // just grab the option off the options list, has been parsed earlier.
                    return(SPIT_getfield( dummy,optptr,1,1));
                }
                return((char *)d);
            }

            if (!strnicmp(optptr,"wrap",4))
            {
                d = SPIT_getpositionopt(optptr+4, &OPTwrapx[0], "wrap x", 0, 32767);
                if (*d==',')
                {
                    d = SPIT_getpositionopt(d+1, &OPTwrapy[0], "wrap y", 0, 32767);
                }
                return((char *)d);
            }
            if (!strnicmp(optptr,"while",5))
            {
                d = SPIT_getfield( OPTwhile,optptr+5,1,0);
                return((char *)d);
            }
            if (!strnicmp(optptr,"wu5",3))
            {
                OPTquantmethod = CR_WU5;
                return((char *)(optptr+3));
            }
            if (!strnicmp(optptr,"wu6",3))
            {
                OPTquantmethod = CR_WU6;
                return((char *)(optptr+3));
            }
            if (!strnicmp(optptr,"wu7",3))
            {
                OPTquantmethod = CR_WU7;
                return((char *)(optptr+3));
            }
            if (!strnicmp(optptr,"wuda",4))
            {
                OPTquantmethod = CR_WUDA;
                return((char *)(optptr+4));
            }

            d = SPIT_getfield( OPTwritex,optptr+1,1,0);
            return((char *)d);

        case 'x':   return(SPIT_getpositionopt(optptr+1, &OPTdefaultx[0], "default x coordinate", -32768, 32767));
        case 'y':
            if (!strnicmp(optptr,"yuy2",4))
            {
                OPTalphabits    =  0;
                OPTredbits      =  4;
                OPTgreenbits    =  8;
                OPTbluebits     =  4;
                return((char *)optptr+4);
            }
            return(SPIT_getpositionopt(optptr+1, &OPTdefaulty[0], "default y coordinate", -32768, 32767));

        case 'z':
            if (!strnicmp(optptr,"zoom",4))
            {
                d = GXgetscaleopt(optptr+4, OPTzoomx, &OPTzoomxby, OPTzoomy, &OPTzoomyby, "zoom", 1, -32767, 32767);
                if (*d==',') d = GX_getopt( &OPTzoomsharp,d+1, "zoom sharpness", 1, 1000);
                return((char *)d);
            }
            break;

        default: break;
    }
    return((char *) d);
}
