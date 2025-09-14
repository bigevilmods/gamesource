// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                     OLDSHP - Shape File Gimex                    */
/*                                                                  */
/*           by Frank Barchard and Laurent Ancessi, EAC             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940822 LA read/write psh based on 3sh                    */
/*    1.01 940828 FB converted to PC                                */
/*    1.02 940901 FB generic shape gimex, for use as PSH and FSH    */
/*    1.03 940902 FB FSH write works.  Working on VSH write         */
/*    1.04 940906 FB VSH write, PSH transluciency                   */
/*    1.05 940910 FB isvsh                                          */
/*    1.06 940912 FB vshread, shpwrite multiframe                   */
/*    1.07 940919 FB vsh understand old hotspot format, pals        */
/*    1.08 940922 FB subtype to say no palette or no shape          */
/*    1.09 941017 FB FSH understand global palettes, 1 bit          */
/*    1.10 941019 FB pad ids                                        */
/*    1.11 941019 FB vsh palettes                                   */
/*    1.12 941027 FB write alignment for VSH/FSH                    */
/*    1.13 941212 FB ignore !x00,!cg0,!eg0, palette start/num col   */
/*    1.14 950307 FB refpacked shapes                               */
/*    1.15 950317 FB read return ok, read ref bug fix               */
/*    1.16 950331 FB info on 24 bpp red bits etc is wrong           */
/*    1.17 950403 FB clut position in extended GINFO for infoshp    */
/*    1.18 950417 FB original colours fix (started)                 */
/*    1.19 950418 FB vsh palettes ignore header is width!=centerx   */
/*    1.20 950419 FB !pal dont kill type for fsh/psh, range check   */
/*    1.21 950420 FB shape align on export                          */
/*    1.22 950515 FB SSH, FSH portable                              */
/*    1.23 950529 FB ginstance signature                            */
/*    1.24 950531 FB getx/putx for SSH/SSH use 32 bit 0bgr          */
/*    1.25 950531 FB 3sh import                                     */
/*    1.26 950601 FB ssh transparent                                */
/*    1.27 950601 FB 3sh ginfos, 4 and 8 bit importing              */
/*    1.28 950601 FB 3sh 8 bit palettes in                          */
/*    1.29 950628 FB export 8 bit 3sh                               */
/*    1.30 950629 FB 3sh 8 bit palette importer bug fix/16 bit 3sh  */
/*    1.31 950706 FB 3sh 8 export bug fix                           */
/*    1.32 950711 FB fsh 16,24 bit                                  */
/*    1.33 950712 FB fsh export pal and shape for each shape        */
/*    1.34 950825 FB -s8 to put named palette for shapes            */
/*    1.35 950901 FB VSH sort.  -s16 to disable sort                */
/*    1.36 950928 FB grey scale set numcolours                      */
/*    1.37 951026 FB setlink, USH                                   */
/*    1.38 951116 FB kpack shapes (if better than refpack)          */
/*    1.39 951128 FB exporter clear aligned columns to 0            */
/*    1.40 960201 FB exporter 8 bit shapes, name clut               */
/*    1.41 960207 FB bits per channel 888 for FSH now               */
/*    1.42 960312 FB align shplen after refpack, not before         */
/*    1.43 960314 FB FSH 8 transparent colours map to 255           */
/*    1.44 960321 FB PSH writer unattenuate argbs                   */
/*    1.45 960403 FB FSH support 565 16 bpp                         */
/*    1.46 960501 YK NSH support (no intensity/alpha or YUV)        */
/*    1.47 960604 YK SSH 15, 32 fixes                               */
/*    1.48 960614 YK Fixed bug in packed shape reading              */
/*    1.49 960621 GS FSH supports 32 bit palettes (bgra)            */
/*    1.50 960622 FB FSH 565 palettes                               */
/*    1.51 960625 FB FSH 4 bit low/high                             */
/*    1.52 960709 FB mipmap height and dont import mip on type!=NSH */
/*    1.53 960712 FB PSH attentuate on read                         */
/*    1.54 960726 FB import palettes                                */
/*    1.55 960726 FB SHPFSH, vsh pal, 666 fsh palettes              */
/*    1.56 960730 FB ssh 8 pixel alignment                          */
/*    1.57 960820 FB vsh determine number of colours in palette     */
/*    1.58 960822 FB FSH 8 dont remap to 255 if 32 bit clut         */
/*    1.59 960825 FB SSH 15 bit clut                                */
/*    1.60 960828 FB SSH 15 bit clut size calc in write             */
/*    1.61 960916 FB Comment attachments                            */
/*    1.62 960930 FB SSH use pure black not blue for opaque black   */
/*    1.63 961001 FB import set pack to 2, not 1. ssh transparent   */
/*    1.64 961015 FB 2 pass import to find palette                  */
/*    1.65 961107 FB palette anim support                           */
/*    1.66 961117 FB clut->shapey=0 for non PSH                     */
/*    1.67 961118 FB FSH 8565 clut                                  */
/*    1.68 961206 YK set shape_SizeClut; copy mipmap cluts          */
/*    1.69 970207 FB subtypes for rotation instead of defaultx      */
/*    1.70 970209 FB allow writing 0 shapes                         */
/*    1.71 970218 GS check gread/gwrite for errors                  */
/*    1.72 970227 FB subtype pal clear if 24 bit                    */
/*    1.73 970317 FB restrict import palette to 1<<bpp              */
/*    1.74 970407 FB FSH 4444,YUV,555 cluts                         */
/*    1.75 970506 FB PSH semitransparency support                   */
/*    1.76 970509 JF SSH Alpha channel transparency                 */
/*    1.77 970515 JF SSH Alpha channel transparency threshold       */
/*    1.78 970527 FB SSH transparent same as PSH                    */
/*    1.79 970529 FB FSH 1555 high bit alpha 1=opaque,0=transparent */
/*    1.80 970730 FB align sizes to 8 for benifit of NSH            */
/*    1.81 970819 FB alignment for mipmaps too                      */
/*    1.82 970918 KO mipmaps work for all formats not just NSH      */
/*    1.83 971016 KO import fix if shapey is neg (mipmaplevels=15)  */
/*    1.84 971111 KO Imports fsh 4444 correctly                     */
/*    1.85 971113 KO added tpage(0x6b) shape type                   */
/*    1.86 971117 FB redefined tpage to 3 shorts to allow clutid    */
/*    1.87 980107 KO width of 1 gets correct rowbytes               */
/*    1.88 980212 KO ginfo corrected for 4444 fsh                   */
/*    1.89 980512 FB removed clamp in 32 bpp FSH clut export        */
/*    1.90 980602 KO Added tex encoding.                            */
/*    1.91 980609 KO Hacked shape size for tex (size/4)             */
/*    1.92 980618 FB VSH read dont try to do attachments            */
/*    1.93 980624 KO S3T textures include width in s3t header (+6)  */
/*    1.94 980629 FB mipmap levels -1 to match 3rash log2(width)    */
/*    1.95 980828 FB DSH Dreamcast. Removed 3sh, dsh                */
/*    1.96 980929 FB DSH swizzle                                    */
/*    1.97 981001 FB swizzle as bit field for all formats           */
/*    1.98 990215 FB disable swizzle on non power of 2 export       */
/*    1.99 990303 FB import strip trailing spaces from name         */
/*    2.00 990323 FB swizzle bit was overwritten by -defaultx.      */
/*    2.01 990330 FB SSH Sony shape files for psx2                  */
/*    2.02 990401 KO Decode tex allocs enough memory                */
/*    2.03 990405 FB clut size use calcrowbytes (bumps 15 to 16)    */
/*    2.04 990604 FB attachment & swizzle import restrictions       */
/*    2.05 990703 FB 16 byte aligned                                */
/*    2.06 990720 FB YCbCr is considered 282                        */
/*    2.07 990809 FB 1555 alpha and 8 bit no remap to 255           */
/*    2.08 991005 FB packed shape import 4 bit correction           */
/*    2.09 991029 FB import mipmapcollapse subtype                  */
/*    2.10 991111 FB packtype 3 mpeg keyframe                       */
/*    2.11 991123 FB 4565 packtype 3 DXT3                           */
/*    2.12 991206 FB MG texture (mpeg keyframe) pack=4              */
/*    2.13 000117 FB csm1 clut ssh                                  */
/*    2.14 000203 FB csm1 clut fix                                  */
/*    2.15 000216 FB csm1 off on 4 bit SSH                          */
/*    2.16 000221 FB nomipmap so collapse is default                */
/*    2.17 000223 FB SSH 8 bit clut export bump size for twiddling  */
/*    2.18 000228 FB SSH csm1 swizzle fixed. DXT transparent        */
/*    2.19 000307 FB csm2 subtype (linear) for ps2 cluts            */
/*    2.20 000329 FB mpeg export buffer size increased              */
/*    2.21 000403 FB msh for mac                                    */
/*    2.22 000410 FB 0x70 is long shape name                        */
/*    2.23 000424 FB ft first draft                                 */
/*    2.24 000512 FB csm2 subtype import                            */
/*    2.25 000529 FB dsh recycled for gamecube                      */
/*    2.26 000609 FB cripple csm2 subtype for all but 8 bit         */
/*    2.27 000614 FB collapse dxt1,dxt3                             */
/*    2.28 000623 FB check framenum is ok on import                 */
/*    2.29 000825 FB renamed dsh to gsh for gamecube, added xsh     */
/*    2.30 000913 FB msg 1555 clut 0x39                             */
/*    2.31 000922 FB mpeg 4 bit low/high and file version number    */
/*    2.32 001110 FB off by in pad fill with 0                      */
/*    2.33 001120 FB imbed mipmap in dxt header, pad with 0 fix     */
/*    2.34 001126 FB compressed data alignment, XSH                 */
/*    2.35 001202 FB dxt/refpack allow buffer space to compress     */
/*    2.36 000104 LC check shpout !null before padding for write    */
/*    2.37 010119 SW export GSH swizzle (rectalinuear)              */
/*    2.38 010129 FB xsh info 4444                                  */
/*    2.39 010205 FB swizzle xsh/fsh                                */
/*    2.40 010206 FB metal meta link info dot3,embm,cubemap         */
/*    2.41 010219 ID dxt1..5 mipmap with collapse                   */
/*    2.42 010312 FB FSH 24 bit clut index 255 opaque if not 0      */
/*    2.43 010313 FB FSH 666 cluts transparency like 24 bit         */
/*    2.44 010331 FB clamp link field to 24 bits                    */
/*    2.45 010403 FB align cluts same as textures                   */
/*    2.46 010406 FB remove width constraint, fix shapey overflow   */
/*    2.47 010410 FB no collapse dxt                                */
/*    2.48 010423 ID GSH dxt swizzle                                */
/*    2.49 010424 ID added XSH/FSHswizzle read.                     */
/*    2.50 010426 FB collapsedxt                                    */
/*    2.51 010426 ID added dxt shapetype read and write.            */
/*    2.52 010427 ID fixed up dxt shapetype for GSH                 */
/*    2.53 010501 ID crippled PS2 dxt shapes to work only with 565  */
/*    2.54 010502 FB mipmont metal & MetalBin                       */
/*    2.55 010503 FB metalbin 86 for ps2                            */
/*    2.56 010507 ID ps2swizzle 8bpp                                */
/*    2.57 010508 ID ps2swizzle 4bpp                                */
/*    2.58 010509 ID capable of handleing DXT compressed world.tif  */
/*    2.59 010516 FB xsh 128 byte aligned                           */
/*    2.60 010518 FB dxt use 565 type always                        */
/*    2.61 010522 FB alpha8 type defined for xsh/fsh                */
/*    2.62 010530 ID added transparency check for min alpha formats */
/*    2.63 010531 ID made ssh bump 16bpp alphabits>2 up to 32bpp    */
/*    2.64 010531 FB gsh 5a3 force alpha=1 if alpha>0               */
/*    2.65 010531 ID alignment fix on xsh mipmaps.                  */
/*    2.66 010531 ID added the new IA embm format gsh               */
/*    2.67 010605 ID fix of DXT decode for mipmaps                  */
/*    2.68 010621 FB removed palette alignment short term           */
/*    2.69 010621 FB argb passthru and replicate channels           */
/*    2.70 010720 ID yuv support for all formats                    */
/*    2.71 010725 ID restructure of module and unswizzle support yuv*/
/*    2.72 010726 ID YUV swizzle support completed                  */
/*    2.73 010801 FB metal/tpage size 0,mipcollapse leak, oldxss export*/
/*    2.74 010803 FB info beyond end fail gracefully                */
/*    2.75 010807 FB shp_is allow for audio chunks                  */
/*    2.76 010807 FB always output a long name                      */
/*    2.77 010809 ID fixed bug with DXT and 8bpp images             */
/*    2.78 010810 FB shift 5a3 bits on export                       */
/*    2.79 010814 FB LVU655 xsh                                     */
/*    2.80 010815 ID strip out common swizzle code for use in fonts */
/*    2.81 010822 FB export set mipmaps field                       */
/*    2.82 010822 ID added gsh IA format                            */
/*    2.83 010824 ID added gsh IA4 and IA8 clut formats             */
/*    2.84 010824 ID added range checks to gsh clut type.           */
/*    2.85 010825 FB 0x33 gsh advanced 32 bit cluts, no nomipcoll.. */
/*    2.86 010826 ID 0x10 0x11 I4 and I8 gsh types                  */
/*    2.87 010907 ID fixed issues with -advanced IA and gsh         */
/*    2.88 010924 FB import dxt set pack type ignore pixel data     */
/*    2.89 010929 FB ASH for Gameboy Advance                        */
/*    2.90 011001 FB -nobitmap, GSH IA8 channels swapped            */
/*    2.91 011001 ID swizzle16 for SSH                              */
/*    2.92 011015 FB align mipmaps                                  */
/*    2.93 011018 ID fix align mipmaps                              */
/*    2.94 011022 FB shpread/shpwrite common code                   */
/*    2.95 011029 FB align export check for 0                       */
/*    2.96 011107 FB import font shape, new 4 bit for fsh/ffn       */
/*    2.97 011109 FB alpha/intensity not advanced now               */
/*    2.98 011109 ID fixed up alpha8 format for xsh fsh             */
/*    2.99 011117 ID gsh can now have odd sized swizzled texture    */
/*    3.00 011123 ID gsh fix for mipmapping now handles 2x2 & 1x1   */
/*    3.01 011123 ID ssh 24bit behind -advanced                     */
/*    3.02 011123 ID gsh I8 hidden behind -advanced                 */
/*    3.03 011123 ID streaming shape now works again                */
/*    3.04 011123 ID slight fix for gsh mipmaps                     */
/*    3.05 011129 ID IA8 formats no longer do Intensity check       */
/*    3.06 011214 ID gsh now handles dxt1 transparency              */
/*    3.07 011215 FB comment size not aligned                       */
/*    3.08 011214 ID gsh now will default to 5a3 with any alpha info*/
/*    3.09 011218 ID turned on swizzle but for gsh dxt textures     */
/*    3.10 011219 ID metalbin no longer available for gsh           */
/*    3.11 011228 PM switch dxt encode to new version               */
/*    3.12 011228 FB pack5 is like pack3, but using s3tc not dxt    */
/*    3.13 020205 ID 5a3 is now the default for gsh cluts           */
/*    3.14 020206 FB gsh clut types respect bits per channel        */
/*    3.15 020212 ID -8000 -pack2 now gives 8000 if it can          */
/*    3.16 020218 FB no short name on streaming shapes (fonts)      */
/*    3.17 020219 ID added YUY2 doesn't work correctly yet.         */
/*    3.18 020220 FB YUY2 info 484                                  */
/*    3.19 020221 ID YUY2 clamp fix, still brightness problem       */
/*    3.20 020221 ID first draft bigshapes.                         */
/*    3.21 020222 ID UMA friendly version of bigshapes              */
/*    3.22 020223 FB decode old refpack with extra 1k               */
/*    3.30 020225 FB metalbin 240 for ps2                           */
/*    3.31 020227 FB 0x66 6666 argb for fsh                         */
/*    3.32 020228 FB disable PC swizzle on 4 & 24 bpp               */
/*    3.33 020301 FB disable PC swizzle on YUV                      */
/*    3.34 020303 FB disable PSX swizzle on 24 bpp                  */
/*    3.35 020304 FB import palette as Nx1 not 256x1                */
/*    3.36 020326 FB export 32 bpp gsh clut with swizzle bit on     */
/*    3.37 020402 FB 0x6a 2101010 argb for fsh                      */
/*    3.38 020403 FB _is use type which allows meta attachments     */
/*    3.39 020416 ID expand memory buffer size to avoid overflow    */
/*    3.40 020417 FB csm clut size only on csm1 cluts               */
/*    3.41 020424 ID fixed ssh swizzle 16 offset error              */
/*    3.42 020427 ID swizzle ssh now valid.                         */
/*    3.43 020522 ID mipmaps now have basetexture name.             */
/*    3.44 020526 FB isopaque/istransparent metal hints             */
/*    3.45 020606 FB ishalfalpha metal hints                        */
/*    3.46 020610 ID cubemap support should now work fairly well    */
/*    3.47 020707 FB shape file (ie fsh) ability to open fonts      */
/*    3.48 020708 FB gsh 5a3 round without bump to 1 on alpha       */
/*    3.49 020709 FB xsh YUY2 Y scaled subsampler                   */
/*    3.50 020722 ID xsh 8000 -> gsh 8800 CLUT format now           */
/*    3.51 020723 FB YUY2 redone                                    */
/*    3.52 020725 FB YUY2 odd widths & remove luma scale            */
/*    3.53 020731 ID mipmap collapse can now detect wrong bpp       */
/*    3.54 020926 FB 0xec (not 0xe0) is threshold for opaque on gsh */
/*    3.55 021009 FB branched oldshp* from new shape                */
/*    3.56 030303 ID gsh didn't respect mipmap alignment            */
/*    3.57 030306 FB ssh dxt advanced                               */
/*    3.58 031209 FB ssh swizzle mipmaps multiple 16 or power2      */
/*    3.59 040115 FB is function detect specific platform for raw   */
/*    3.60 040213 DH msh now represents PSP shape types             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Reentrant                                                        */
/* Files: shpread.cpp, shpwrite.cpp, shpgimex.h                     */
/*        pshread.c, pshwrite.c, pshgimex.h                         */
/*        fshread.c, fshwrite.c, fshgimex.h                         */
/*        vshread.c, vshwrite.c, vshgimex.h                         */
/*        nshread.c, nshwrite.c, nshgimex.h                         */
/*        gshread.c, gshwrite.c, gshgimex.h                         */
/*        sshread.c, sshwrite.c, sshgimex.h                         */
/*        mshread.c, mshwrite.c, mshgimex.h                         */
/*        xshread.c, xshwrite.c, xshgimex.h                         */
/*                                                                  */
/* Pack types:                                                      */
/*   0 No Compression                                               */
/*   1 Run length (not supported yet)                               */
/*   2 Refpack                                                      */
/*   3 DXT (dxt1,dxt3,dxt5)                                         */
/*   4 MPEG2 Intraframe                                             */
/*   5 S3TC same as dxt, but different codec                        */
/*                                                                  */
/* Subtypes:                                                        */
/*          1       0x1 No Palette Attachment                       */
/*          2       0x2 No Bitmap (palette and hotspots only)       */
/*          4       0x4 No HotSpots                                 */
/*          8       0x8 No Comments                                 */
/*         16      0x10 No Palette Anims                            */
/*         32      0x20 No TPage                                    */
/*         64      0x40 reserved                                    */
/*        128      0x80 reserved                                    */
/*        256     0x100 Named palette                               */
/*        512     0x200 Dont collapsed mip maps						*/
/*       1024     0x400 No sorting (VSH only)                       */
/*       2048     0x800 Rotated PSH                                 */
/*       4096    0x1000 Transposed PSH                              */
/*       8192    0x2000 Swizzle (aka swizzle)                       */
/*      16384    0x4000 CSM2                                        */
/*      32768    0x8000 No Name attachment                          */
/*      65536   0x10000 isdot3                                      */
/*     131072   0x20000 isembm                                      */
/*     262144   0x40000 iscubemap                                   */
/*     524288   0x80000 ismipmont                                   */
/*    1048576  0x100000 Dont collapsed dxt header   				*/
/*    2097152  0x200000 bin data in metal               			*/
/*    4194304  0x400000 enable advanced pixel formats               */
/*    8388608  0x800000 reserved                                    */
/*   16777216 0x1000000 isopaque                                    */
/*   33554432 0x2000000 istransparent (1 bpp alpha)                 */
/*   67108864 0x4000000 ishalfalpha (alpha 128 = opaque)            */
/*                                                                  */
/* SHPMAKECODE set by ?shread.c/?shwrite.c                          */
/*   1 psh  Sony PSX                                                */
/*   2 fsh  Windows                                                 */
/*   3 vsh  VGA (legacy)                                            */
/*   4 nsh  Nintendo 64                                             */
/*   5 gsh  GameCube                                                */
/*   6 ssh  Sony PSX 2                                              */
/*   7 msh  Sony PSP                                                */
/*   8 xsh  XBox                                                    */
/*   9 ash  Gameboy Advance                                         */
/*  12 fss  Windows                                                 */
/*  15 gss  GameCube                                                */
/*  16 sss  Sony PSX 2                                              */
/*  17 mss  Sony PSP                                                */
/*  18 xss  XBox                                                    */
/*                                                                  */
/* SHPMULTIFRAME set by ?sxgimex.h to 0 to build single shape       */
/*   gimex.  Also supports fonts.                                   */
/*                                                                  */
/* Module is designed to allow code to be #included or linked       */
/* with all shape formats.  If linked, local functions are          */
/* duplicated, but static, to avoid link errors.                    */
/* If included, #ifdef prevents duplicate local functions.          */
/* All defines are #undefed to allow include more than once.        */
/* Can include either headers, or code.                             */
/* Local function cannot have compile time #ifdefs for target       */
/* file format.  Uses SHPMAKECODE as runtime check.  Read doesnt    */
/* care what module, since type is unique.  Write passed            */
/* SHPSIGNATURE and SHPMAKECODE.                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* Shape files contain multiple unrelated shapes (images) and       */
/* other information, such as palettes and geometry information.    */
/* An individual shape has a link to additional shapes, which are   */
/* typically a palette and hot spots.                               */
/*                                                                  */
/* The current shape files supported are:                           */
/*   FSH  PC        4,8,1555,565,4444,888,8888 intel byte           */
/*   SSH  Sony      4,8,1555,888,8888          intel word           */
/*   XSH  PC        8,1555,565,4444,8888       intel byte           */
/*   GSH  GameCube  4,8,4555(5a3),565,8888     moto  byte           */
/*   PSH  PSX       4,8,1555,888               intel word           */
/*   VSH  PC        8                          intel byte           */
/*   NSH  N64       1,4,8,1555,8888            moto  byte           */
/*   MSH  PSP       4,8,1555,565,4444,8888     intel byte           */
/*   ASH  GBA       4,8,0555                   intel byte           */
/*                                                                  */
/* Semi transparency for PSH 16 bit from Sony 4.1 documentation:    */
/* (Table 3-1: STP Bit Function in Combination with R, G, B Data)   */
/* STP/R,G,B Translucent processing on Translucent processing off   */
/* 0,0,0,0     Transparent                   Transparent            */
/* 0,X,X,X     Not transparent               Not transparent        */
/* 1,X,X,X     Semi-transparent              Not transparent        */
/* 1,0,0,0     Non-transparent black         Non-transparent black  */
/*                                                                  */
/* How semitransparency maps to Gimex ARGB values:                  */
/* STP/R,G,B   ARGB        Description                              */
/* 0,0,0,0     0x00xxxxxx  Transparent                              */
/* 0,X,X,X     0xFFxxxxxx  Not transparent                          */
/* 1,X,X,X     0x80xxxxxx  Semi-transparent                         */
/* 1,0,0,0     0xFF000000  Non-transparent black                    */
/* 1,0,0,1     0x80000000  Semi-transparent black (maps to blue)    */
/*                                                                  */
/* YUY2                                                             */
/* http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnwmt/html/YUVFormats.asp */
/*                                                                  */
/* Y = round( 0.256788 * R + 0.504129 * G + 0.097906 * B) +  16     */
/* U = round(-0.148223 * R - 0.290993 * G + 0.439216 * B) + 128     */
/* V = round( 0.439216 * R - 0.367788 * G - 0.071427 * B) + 128     */
/*                                                                  */
/* C = Y - 16                                                       */
/* D = U - 128                                                      */
/* E = V - 128                                                      */
/*                                                                  */
/* R = clip( round( 1.164383 * C                   + 1.596027 * E  ) ) */
/* G = clip( round( 1.164383 * C - (0.391762 * D) - (0.812968 * E) ) ) */
/* B = clip( round( 1.164383 * C +  2.017232 * D                   ) ) */
/*                                                                  */
/* Patricks coefficient                                             */
/*  0.25678843   0.50412965   0.09790586                            */
/* -0.14822295  -0.29099280   0.43921576                            */
/*  0.43921557  -0.36778826  -0.07142731                            */
/*                                                                  */
/* Color Faq                                                        */
/* http://www.inforamp.net/~poynton/PDFs/ColorFAQ.pdf               */
/*                                                                  */
/* Reference for fourcc (4 character codes)                         */
/* http://www.webartz.com/fourcc/                                   */
/*                                                                  */
/* Most common packed format: YUY2 (Microsoft is pushing UYVY?)     */
/* Most common planar format: YVU9                                  */
/*                                                                  */
/*                                                                  */
/* XBox August 2001 XDK texture formats (D3DFORMAT)                 */
/*                                                                  */
/* typedef enum _D3DFORMAT                                          */
/* {                                                                */
/*     D3DFMT_UNKNOWN              = 0xFFFFFFFF,                    */
/*                                                                  */
/*      * Swizzled formats *                                        */
/*                                                                  */
/*     D3DFMT_A8R8G8B8             = 0x00000006,                    */
/*     D3DFMT_X8R8G8B8             = 0x00000007,                    */
/*     D3DFMT_R5G6B5               = 0x00000005,                    */
/*     D3DFMT_X1R5G5B5             = 0x00000003,                    */
/*     D3DFMT_A1R5G5B5             = 0x00000002,                    */
/*     D3DFMT_A4R4G4B4             = 0x00000004,                    */
/*     D3DFMT_A8                   = 0x00000019,                    */
/*     D3DFMT_A8B8G8R8             = 0x0000003A,                    */
/*     D3DFMT_B8G8R8A8             = 0x0000003B,                    */
/*     D3DFMT_R4G4B4A4             = 0x00000039,                    */
/*     D3DFMT_R5G5B5A1             = 0x00000038,                    */
/*     D3DFMT_R8G8B8A8             = 0x0000003C,                    */
/*     D3DFMT_R8B8                 = 0x00000029,                    */
/*     D3DFMT_G8B8                 = 0x00000028,                    */
/*                                                                  */
/*     D3DFMT_P8                   = 0x0000000B,                    */
/*                                                                  */
/*     D3DFMT_L8                   = 0x00000000,                    */
/*     D3DFMT_A8L8                 = 0x0000001A,                    */
/*     D3DFMT_AL8                  = 0x00000001,                    */
/*     D3DFMT_L16                  = 0x00000032,                    */
/*                                                                  */
/*     D3DFMT_V8U8                 = 0x00000028,                    */
/*     D3DFMT_L6V5U5               = 0x00000027,                    */
/*     D3DFMT_X8L8V8U8             = 0x00000007,                    */
/*     D3DFMT_Q8W8V8U8             = 0x0000003A,                    */
/*     D3DFMT_V16U16               = 0x00000033,                    */
/*                                                                  */
/*     D3DFMT_D16_LOCKABLE         = 0x0000002C,                    */
/*     D3DFMT_D16                  = 0x0000002C,                    */
/*     D3DFMT_D24S8                = 0x0000002A,                    */
/*     D3DFMT_F16                  = 0x0000002D,                    */
/*     D3DFMT_F24S8                = 0x0000002B,                    */
/*                                                                  */
/*      * YUV formats *                                             */
/*                                                                  */
/*     D3DFMT_UYVY                 = 0x00000024,                    */
/*     D3DFMT_YUY2                 = 0x00000025,                    */
/*                                                                  */
/*      * Compressed formats *                                      */
/*                                                                  */
/*     D3DFMT_DXT1                 = 0x0000000C,                    */
/*     D3DFMT_DXT2                 = 0x0000000E,                    */
/*     D3DFMT_DXT3                 = 0x0000000E,                    */
/*     D3DFMT_DXT4                 = 0x0000000F,                    */
/*     D3DFMT_DXT5                 = 0x0000000F,                    */
/*                                                                  */
/*      * Linear formats *                                          */
/*                                                                  */
/*     D3DFMT_LIN_A1R5G5B5         = 0x00000010,                    */
/*     D3DFMT_LIN_A4R4G4B4         = 0x0000001D,                    */
/*     D3DFMT_LIN_A8               = 0x0000001F,                    */
/*     D3DFMT_LIN_A8B8G8R8         = 0x0000003F,                    */
/*     D3DFMT_LIN_A8R8G8B8         = 0x00000012,                    */
/*     D3DFMT_LIN_B8G8R8A8         = 0x00000040,                    */
/*     D3DFMT_LIN_G8B8             = 0x00000017,                    */
/*     D3DFMT_LIN_R4G4B4A4         = 0x0000003E,                    */
/*     D3DFMT_LIN_R5G5B5A1         = 0x0000003D,                    */
/*     D3DFMT_LIN_R5G6B5           = 0x00000011,                    */
/*     D3DFMT_LIN_R6G5B5           = 0x00000037,                    */
/*     D3DFMT_LIN_R8B8             = 0x00000016,                    */
/*     D3DFMT_LIN_R8G8B8A8         = 0x00000041,                    */
/*     D3DFMT_LIN_X1R5G5B5         = 0x0000001C,                    */
/*     D3DFMT_LIN_X8R8G8B8         = 0x0000001E,                    */
/*                                                                  */
/*     D3DFMT_LIN_A8L8             = 0x00000020,                    */
/*     D3DFMT_LIN_AL8              = 0x0000001B,                    */
/*     D3DFMT_LIN_L16              = 0x00000035,                    */
/*     D3DFMT_LIN_L8               = 0x00000013,                    */
/*                                                                  */
/*     D3DFMT_LIN_D24S8            = 0x0000002E,                    */
/*     D3DFMT_LIN_F24S8            = 0x0000002F,                    */
/*     D3DFMT_LIN_D16              = 0x00000030,                    */
/*     D3DFMT_LIN_F16              = 0x00000031,                    */
/*                                                                  */
/*     D3DFMT_VERTEXDATA           = 100,                           */
/*     D3DFMT_INDEX16              = 101,                           */
/*                                                                  */
/*     D3DFMT_FORCE_DWORD          =0x7fffffff                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <stdio.h>

#include "gimex.h"
#include "oldshpgimex.h"
#include "oldshpread.h"
#include "refcodex.h"
#include "idec_decode.h"
#include "idec_encode.h"

#include "gst.h"
#include "gstglue.h"
#include "oldshpgstglue.h"
using Gst::OLDSHPDecompressGstShape;
using Gst::OLDSHPIsGst;
using Gst::GetGstFourCC;
using Gst::GSTEncodeSize;

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

void OLDgputx(void *memptr, unsigned int val, int numbytes, int type);
unsigned int OLDggetx(const void *memptr, int numbytes, int type);

#define OLDSHPGETFIELD(field) OLDggetx((void *)&field, sizeof(field), type)
#define OLDSHPPUTFIELD(field,data) OLDgputx((void *)&field,(unsigned int)(data),sizeof(field), type)

#define OLDSHPMIPMAPALIGN(x) (((x)+oldshpmipmapdataalignment)&~oldshpmipmapdataalignment)



//--------------------------------------------------------------------------------------------------------
// Video for Windows codec

#if defined(_MSC_VER) && !defined(_XBOX)
#  define __MSWINDOWS__
#endif


#ifdef __MSWINDOWS__
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <vfw.h>
#endif

static int OLDMVCODEC_decodergb(void *dest, const void *source, int Width, int Height, int Bpp, int rowbytes)
{
    int ok=1;
    int i;
    int j;
    int bpl;
    unsigned char *dstp;
    const unsigned char *srcp;

    {
        if (rowbytes < Width) return(0);

        bpl = ((Width*Bpp+31)&~31)/8;

        for (i = 0; i < Height; i++)
        {
            srcp = (unsigned char *)(source)+i*bpl;
            dstp = (unsigned char *)(dest)+(Height-1-i)*rowbytes; // upside down
            switch(Bpp)
            {
                case 1:
                    for(j = 0; j < Width; j++)
                    {
                        *dstp++ = (unsigned char)((*srcp>>(7-(j&7)))&1);
                        if ((j&7)==7)
                            srcp++;
                    }
                    break;
                case 4:
                    for(j = 0; j < Width; j++)
                    {
                        if (j&1)
                        {
                            *dstp = (unsigned char)(*srcp&15);
                            srcp++;
                        }
                        else
                        {
                            *dstp = (unsigned char)((*srcp>>4)&15);
                        }
                        dstp++;
                    }
                    break;
                case 8:
                    for(j = 0; j < Width; j++)
                    {
                        *dstp = *srcp;
                        dstp++;
                        srcp++;
                    }
                    break;
                case 16:
                    for(j = 0; j < Width; j++)
                    {
                        /* 555 */
                        ((ARGB *)dstp)->b = (unsigned char)((((*srcp)&31) * 255 + 16)/31);
                        ((ARGB *)dstp)->g = (unsigned char)((((((*(srcp+1))&3)<<3) | (((*srcp)>>5)&7)) * 255 + 16)/31);
                        ((ARGB *)dstp)->r = (unsigned char)(((((*(srcp+1))>>2)&31) * 255 + 16)/31);
                        ((ARGB *)dstp)->a = (unsigned char)255;
                        dstp+=4;
                        srcp+=2;
                    }
                    break;
                case 24:
                    for(j = 0; j < Width; j++)
                    {
                        ((ARGB *)dstp)->b = srcp[0];
                        ((ARGB *)dstp)->g = srcp[1];
                        ((ARGB *)dstp)->r = srcp[2];
                        ((ARGB *)dstp)->a = (unsigned char)255;
                        srcp += 3;
                        dstp += 4;
                    }
                    break;
                case 32:
                    for(j = 0; j < Width; j++)
                    {
                        ((ARGB *)dstp)->b = srcp[0];
                        ((ARGB *)dstp)->g = srcp[1];
                        ((ARGB *)dstp)->r = srcp[2];
                        ((ARGB *)dstp)->a = srcp[3]; // 255?
                        srcp += 4;
                        dstp += 4;
                        dstp+=4;
                        srcp++;
                    }
                    break;
                default:
                    ok = 0;
                    break;
            }
            dstp-=2*rowbytes;
        }
    }
    return(ok);
}








// should check with windows for codecs

bool OLDMVCODEC_is(const void *data)
{
#ifdef __MSWINDOWS__
    if (ggetm(data,4)==GMAKEID('M','V','L','Z'))
        return true;
#endif
    return false;
}

unsigned int OLDMVCODEC_fourcc(const void *data)
{
    return ggetm(data,4);
}


int OLDMVCODEC_decode(void *output, const void *source, int /* flags */)
{
#ifdef __MSWINDOWS__

    BITMAPINFOHEADER bmiHeader;
    BITMAPINFOHEADER to;
    DWORD ok=0;
    int Width=256;
    int Height=256;
    int Bpp=24;
    int rowbytes = Width*4;

    bmiHeader.biSize = 40;
    bmiHeader.biWidth = Width;
    bmiHeader.biHeight = Height;
    bmiHeader.biPlanes = 1;
    bmiHeader.biBitCount = (unsigned short) Bpp;
    bmiHeader.biCompression = ggeti(source,4);// avi->Compression;
    bmiHeader.biSizeImage = Width*Height*Bpp/8;
    bmiHeader.biXPelsPerMeter= 0;
    bmiHeader.biYPelsPerMeter= 0;
    bmiHeader.biClrUsed= 0;//ginfo->numcolours;
    bmiHeader.biClrImportant = 0;

    to = bmiHeader;
    to.biCompression = 0;

    HIC hic = ICLocate(ICTYPE_VIDEO, 0L,
                                    &bmiHeader,
                                    &to,
                                    ICMODE_DECOMPRESS);

    ok = (ICDecompressBegin(hic,
                               &bmiHeader,
                               &to)
              == ICERR_OK);


    HANDLE dtemp = GlobalAlloc(GHND | GMEM_SHARE, Width*Height*4);
    if (dtemp)
    {
        void *pDst = GlobalLock(dtemp);
        if (pDst)
        {

            if (ICDecompress(hic, 0, &bmiHeader, (void *) source,
                &to, pDst) == ICERR_OK)
                /*
                 * WARNING - Docs say that 'output' should be GlobalAlloc()ed and then GlobalLock()ed
                 * but avi->CurrentFrame seemed so perfect for the job.
                 */
            {
                ok = OLDMVCODEC_decodergb(output, pDst, Width, Height, Bpp, rowbytes);
            }
            else
            {
                ok = 0;
            }

            GlobalUnlock(dtemp);
        }

        GlobalFree(dtemp);
    }


    ICDecompressEnd(hic);

    return ok;

#else
    return 0;
#endif
}

//--------------------------------------------------------------------------------------------------------
// Test shape, get information about what type of link is the shapetype.
int OLDSHPRGetShapetype(OLDSHPHeaderStruct *TestShape, int type)
{
    int oldshptype = kUnknowShape;

	switch (type)
	{
        case TYPEOLDVSH:
			switch (TestShape->type&0x7f)
			{
				case 0x22:  oldshptype = kClutShape;      break;
				case 0x6f:  oldshptype = kCommentShape;   break;
				case 0x70:  oldshptype = kNameShape;      break;
				case 0x7c:  oldshptype = kHotSpotShape;   break;
				default:    oldshptype = kPictureShape;
			}
			break;

		case TYPEOLDPSH:
	        switch (TestShape->type&0x7f)
	        {
				case 0x23:  oldshptype = kClutShape;      break;   /* OLDPSH */
				case 0x40:  oldshptype = kPictureShape;   break;
				case 0x41:  oldshptype = kPictureShape;   break;
				case 0x42:  oldshptype = kPictureShape;   break;
				case 0x43:  oldshptype = kPictureShape;   break;
				case 0x44:  oldshptype = kPictureShape;   break;
				default: break;
			}
			break;


		case TYPEOLDASH:
		case TYPEOLDSSS:
        case TYPEOLDSSH:
	        switch (TestShape->type&0x7f)
	        {

				case 0x20:  oldshptype = kClutShape;      break;   /* OLDSSH */
				case 0x21:  oldshptype = kClutShape;      break;
				case 0x01:  oldshptype = kPictureShape;   break;
				case 0x02:  oldshptype = kPictureShape;   break;
				case 0x03:  oldshptype = kPictureShape;   break;
				case 0x04:  oldshptype = kPictureShape;   break;
				case 0x05:  oldshptype = kPictureShape;   break;
				case 0x08:  oldshptype = kPictureShape;   break;   // GST types
				case 0x09:  oldshptype = kPictureShape;   break;
				case 0x0a:  oldshptype = kPictureShape;   break;
				case 0x0b:  oldshptype = kPictureShape;   break;
				case 0x0c:  oldshptype = kPictureShape;   break;
				case 0x0d:  oldshptype = kPictureShape;   break;
				case 0x0e:  oldshptype = kPictureShape;   break;
				case 0x0f:  oldshptype = kPictureShape;   break;

				case 0x60:  oldshptype = kPictureShape;   break;
				case 0x61:  oldshptype = kPictureShape;   break;
				case 0x62:  oldshptype = kPictureShape;   break;
			    case 0x6c:  oldshptype = kPictureShape;   break;   // YV12 format

				default: break;
			}
			break;

        case TYPEOLDGSH:
		case TYPEOLDGSS:
			switch (TestShape->type&0x7f)
			{
				case 0x30:  oldshptype = kClutShape;      break;   /* OLDGSH */
				case 0x31:  oldshptype = kClutShape;      break;
				case 0x32:  oldshptype = kClutShape;      break;
				case 0x33:  oldshptype = kClutShape;      break;
				case 0x10:  oldshptype = kPictureShape;   break;
				case 0x11:  oldshptype = kPictureShape;   break;
				case 0x12:  oldshptype = kPictureShape;   break;
				case 0x13:  oldshptype = kPictureShape;   break;
				case 0x14:  oldshptype = kPictureShape;   break;
				case 0x15:  oldshptype = kPictureShape;   break;
				case 0x16:  oldshptype = kPictureShape;   break;
				case 0x18:  oldshptype = kPictureShape;   break;
				case 0x19:  oldshptype = kPictureShape;   break;
				case 0x1a:  oldshptype = kPictureShape;   break;
				case 0x1e:  oldshptype = kPictureShape;   break;
				case 0x1c:  oldshptype = kPictureShape;   break;
				case 0x08:  oldshptype = kPictureShape;   break;   // GST types
				case 0x09:  oldshptype = kPictureShape;   break;
				case 0x0a:  oldshptype = kPictureShape;   break;
				case 0x0b:  oldshptype = kPictureShape;   break;
				case 0x0c:  oldshptype = kPictureShape;   break;
				case 0x0d:  oldshptype = kPictureShape;   break;
				case 0x0e:  oldshptype = kPictureShape;   break;
				case 0x0f:  oldshptype = kPictureShape;   break;

				default: break;
			}
			break;

		case TYPEOLDXSS:
		case TYPEOLDFSS:
        case TYPEOLDFSH:
		case TYPEOLDXSH:
			switch (TestShape->type&0x7f)
			{
				case 0x08:  oldshptype = kPictureShape;   break;   // GST types
				case 0x09:  oldshptype = kPictureShape;   break;
				case 0x0a:  oldshptype = kPictureShape;   break;
				case 0x0b:  oldshptype = kPictureShape;   break;
				case 0x0c:  oldshptype = kPictureShape;   break;
				case 0x0d:  oldshptype = kPictureShape;   break;
				case 0x0e:  oldshptype = kPictureShape;   break;
				case 0x0f:  oldshptype = kPictureShape;   break;
				case 0x22:  oldshptype = kClutShape;      break;   /* OLDFSH/OLDXSH */
				case 0x24:  oldshptype = kClutShape;      break;
				case 0x29:  oldshptype = kClutShape;      break;
				case 0x2a:  oldshptype = kClutShape;      break;
				case 0x2c:  oldshptype = kClutShape;      break;
				case 0x2d:  oldshptype = kClutShape;      break;
				case 0x2e:  oldshptype = kClutShape;      break;
				case 0x67:  oldshptype = kPictureShape;   break;
				case 0x6c:  oldshptype = kPictureShape;   break;
				case 0x6d:  oldshptype = kPictureShape;   break;
				case 0x78:  oldshptype = kPictureShape;   break;
				case 0x79:  oldshptype = kPictureShape;   break;
				case 0x7a:  oldshptype = kPictureShape;   break;
				case 0x7b:  oldshptype = kPictureShape;   break;
				case 0x7d:  oldshptype = kPictureShape;   break;
				case 0x7e:  oldshptype = kPictureShape;   break;
				case 0x7f:  oldshptype = kPictureShape;   break;
				case 0x60:  oldshptype = kPictureShape;   break;
				case 0x61:  oldshptype = kPictureShape;   break;
				case 0x62:  oldshptype = kPictureShape;   break;
				case 0x63:  oldshptype = kPictureShape;   break;  // madcow shapetype... short term hopefully
				case 0x64:  oldshptype = kPictureShape;   break;
				case 0x65:  oldshptype = kPictureShape;   break;
				case 0x66:  oldshptype = kPictureShape;   break;
				case 0x6a:  oldshptype = kPictureShape;   break;
				default: break;
			}
			break;

		case TYPEOLDMSS:
		case TYPEOLDMSH:
			switch (TestShape->type&0x7f)
			{

				case 0x38:  oldshptype = kClutShape;      break;   /* OLDMSH */
				case 0x39:  oldshptype = kClutShape;      break;
				case 0x3a:  oldshptype = kClutShape;      break;
				case 0x3b:  oldshptype = kClutShape;      break;
				case 0x58:  oldshptype = kPictureShape;   break;
				case 0x59:  oldshptype = kPictureShape;   break;
				case 0x5a:  oldshptype = kPictureShape;   break;
				case 0x5b:  oldshptype = kPictureShape;   break;
				case 0x5c:  oldshptype = kPictureShape;   break;
				case 0x5d:  oldshptype = kPictureShape;   break;
				default: break;
			}
			break;

        case TYPEOLDNSH:
			switch (TestShape->type&0x7f)
			{
				case 0x27:  oldshptype = kClutShape;      break;   /* OLDNSH */
				case 0x28:  oldshptype = kClutShape;      break;
				//case 0x60:
				//case 0x61:
				//case 0x62:
	//            case 0x63:
	//          case 0x64:
	//          case 0x65:
	//          case 0x66:
	//          case 0x67:
	//          case 0x68:
	//            case 0x6a:
							oldshptype = kPictureShape;   break;

				default: break;
			}
			break;

        default: break;
	}

	if (oldshptype == kUnknowShape)
	{
		switch (TestShape->type&0x7f)
		{
			case 0x7c:  oldshptype = kHotSpotShape;   break;
			case 0x69:  oldshptype = kMetalShape;     break;
			case 0x68:  oldshptype = kPictureShape;   break;
			case 0x6b:  oldshptype = kTpageShape;     break;
			case 0x6e:  oldshptype = kAnimShape;   break;
			case 0x6f:  oldshptype = kCommentShape;   break;
			case 0x70:  oldshptype = kNameShape;      break;
	        default: break;
		}
	}

    return(oldshptype);
}

//--------------------------------------------------------------------------------------------------------
// align to proper byte alignment for the shape.
static int OLDSHPRCalcRowBytes(int width, int bpp, int type)
{
    int SizeShape=0L;

    if (bpp == 15)
        bpp = 16;

    switch (type)
    {
        default:
        case TYPEOLDPSH:
        case TYPEOLDASH:
		case TYPEOLDSSS:
        case TYPEOLDSSH: SizeShape=((width*bpp+15)&~15)/8; break;
        case TYPEOLDGSH:
		case TYPEOLDGSS:
		case TYPEOLDXSS:
		case TYPEOLDFSS:
        case TYPEOLDXSH:
        case TYPEOLDVSH:
        case TYPEOLDNSH:
        case TYPEOLDFSH: SizeShape=((width*bpp+7)&~7)/8; break;
		case TYPEOLDMSS:
        case TYPEOLDMSH: SizeShape=((width*bpp+127)&~127)/8; break; // PSP shape rows must be 16-byte aligned.
    }

    return(SizeShape);
}


//--------------------------------------------------------------------------------------------------------
// watch out playing with shape alignment is a bit tricky
static int OLDSHPRCalcSizeShape(int width,int height,int bpp, int type)
{
    int    SizeShape=0L;

    SizeShape = OLDSHPRCalcRowBytes(width, bpp, type);
    SizeShape *= height;

//    /* For ALL, make sure total size is aligned. */
//    SizeShape = OLDSHPALIGN(SizeShape);

    return(SizeShape);
}

//--------------------------------------------------------------------------------------------------------
// Delete a list of Shapes
void OLDSHPRDeleteShape(OLDShapeListPtr theShape)
{
    int    i;

    for (i=0; i<theShape->NuoLdmshape; ++i)
    {
        if (theShape->ListShape[i].shape_Data)          gfree(theShape->ListShape[i].shape_Data);
        if (theShape->ListShape[i].shape_Clut)          gfree(theShape->ListShape[i].shape_Clut);
        if (theShape->ListShape[i].shape_HotSpot)       gfree(theShape->ListShape[i].shape_HotSpot);
        if (theShape->ListShape[i].shape_Tpage)         gfree(theShape->ListShape[i].shape_Tpage);
        if (theShape->ListShape[i].shape_Metal)         gfree(theShape->ListShape[i].shape_Metal);
        if (theShape->ListShape[i].shape_Name)          gfree(theShape->ListShape[i].shape_Name);
        if (theShape->ListShape[i].shape_Comment)       gfree(theShape->ListShape[i].shape_Comment);
        if (theShape->ListShape[i].shape_Anim)          gfree(theShape->ListShape[i].shape_Anim);
        theShape->ListShape[i].shape_Data       = 0;
        theShape->ListShape[i].shape_Clut       = 0;
        theShape->ListShape[i].shape_HotSpot    = 0;
        theShape->ListShape[i].shape_Tpage      = 0;
        theShape->ListShape[i].shape_Metal      = 0;
        theShape->ListShape[i].shape_Name       = 0;
        theShape->ListShape[i].shape_Comment    = 0;
        theShape->ListShape[i].shape_Anim       = 0;
    }
    gfree((char *)theShape);
}

//--------------------------------------------------------------------------------------------------------
// ultimate source of information about shapes. tells the bit depth of each shape.
int OLDSHPRGetOldshpbpp(OLDSHPHeaderStruct *theShape,int type)
{
    int     bpp=0;

    if (type==TYPEOLDVSH)
    {
        switch (theShape->type)
        {
            case 0x22: bpp=24; break;
            case 0x24: bpp=24; break;
            default:   bpp=8;  break;
        }
    }
    else
    {
        switch (theShape->type&0x7f)
        {
            case 0x23: bpp=16; break;   /* OLDPSH types */
            case 0x40: bpp=4;  break;
            case 0x41: bpp=8;  break;
            case 0x42: bpp=16; break;
            case 0x43: bpp=24; break;
            case 0x44: bpp=1;  break;

            case 0x30: bpp=16; break;   /* OLDGSH */
            case 0x31: bpp=16; break;
            case 0x32: bpp=15; break;
            case 0x33: bpp=32; break;
            case 0x10: bpp=4;  break;
            case 0x11: bpp=8;  break;
            case 0x12: bpp=8;  break;
            case 0x13: bpp=16; break;
            case 0x14: bpp=16; break;
            case 0x15: bpp=15; break;
            case 0x16: bpp=32; break;
            case 0x18: bpp=4;  break;
            case 0x19: bpp=8;  break;
            case 0x1a: bpp=16; break;
            case 0x1e: bpp=16; break;  /* CMPR */
            case 0x1c: bpp=12; break;  /* YUV YUM type */

            case 0x38: bpp=16; break;   /* OLDMSH types */
            case 0x39: bpp=15; break;
            case 0x3a: bpp=16; break;
            case 0x3b: bpp=32; break;
            case 0x58: bpp=16; break;
            case 0x59: bpp=15; break;
            case 0x5a: bpp=16; break;
            case 0x5b: bpp=32; break;
            case 0x5c: bpp=4;  break;
            case 0x5d: bpp=8;  break;

            case 0x20: bpp=15; break;   /* OLDSSH types */
            case 0x21: bpp=32; break;
            case 0x01: bpp=4;  break;
            case 0x02: bpp=8;  break;
            case 0x03: bpp=15; break;
            case 0x04: bpp=24; break;
            case 0x05: bpp=32; break;
            case 0x08: bpp=8;  break;   // GST 121
            case 0x09: bpp=8;  break;   // GST 221
            case 0x0a: bpp=8;  break;   // GST 421
            case 0x0b: bpp=8;  break;   // GST 821
            case 0x0c: bpp=8;  break;   // GST 122
            case 0x0d: bpp=8;  break;   // GST 222
            case 0x0e: bpp=8;  break;   // GST 422
            case 0x0f: bpp=8;  break;   // GST 822


            case 0x22: bpp=24; break;   /* OLDFSH OLDXSH OLDVSH types */
            case 0x24: bpp=24; break;
            case 0x29: bpp=16; break;
            case 0x2a: bpp=32; break;
            case 0x2c: bpp=32; break;
            case 0x2d: bpp=16; break;
            case 0x2e: bpp=32; break;
            case 0x63: bpp=16; break;   /* MADYUV */
            case 0x68: bpp=16; break;   /* YUY2 */
			case 0x6c: bpp=12; break;   /* (YUI) YUV type */
            case 0x6d: bpp=16; break;
            case 0x67: bpp=16; break;
            case 0x78: bpp=16; break;
            case 0x79: bpp=4;  break;
            case 0x7a: bpp=4;  break;
            case 0x7b: bpp=8;  break;
            case 0x7d: bpp=32; break;
            case 0x7e: bpp=15; break;
            case 0x7f: bpp=24; break;
            case 0x60: bpp=16; break;   /* DXT1   */
            case 0x61: bpp=32; break;   /* DXT3   */
            case 0x62: bpp=32; break;   /* DXT5   */
            case 0x64: bpp=8;  break;   /* Alpha8 */
            case 0x65: bpp=16; break;   /* IA8    */
            case 0x66: bpp=24; break;   /* ARGB 6666 */
            case 0x6a: bpp=32; break;   /* ARGB 2101010 */

                                        /* OLDNSH */
            case 0x27: bpp=16; break;   /* RGBA clut */
            case 0x28: bpp=16; break;   /* IA clut */
            //case 0x60: bpp=1;  break;   /* CI */
            //case 0x61: bpp=4;  break;   /* I */
            //case 0x62: bpp=4;  break;   /* IA */
//            case 0x63: bpp=4;  break;   /* CI */
//            case 0x64: bpp=8;  break;   /* I */
//            case 0x65: bpp=8;  break;   /* IA */
//            case 0x66: bpp=8;  break;   /* CI */
//            case 0x67: bpp=16; break;   /* IA */
//            case 0x68: bpp=15; break;   /* RGBA */
//            case 0x69: bpp=16; break;   /* YUV */
//            case 0x6a: bpp=32; break;   /* RGBA */

            default:   bpp=8;  break;
        }
    }
    return(bpp);
}


//--------------------------------------------------------------------------------------------------------
//   Mipmap levels are stored in upper 4 bits of
//   shapey. If 0 then shape has no mipmap levels;
//   non-zero records total number of levels,
//   including level 0.
static int OLDSHPRGetMipmapLevels(OLDSHPHeaderStruct *theShape, int newlevels, int type)
{
    int shapey;
    int levels;
    int width;

    if (!theShape)
        return 0;
    width  = OLDSHPGETFIELD(theShape->width);
    shapey = OLDSHPGETFIELD(theShape->shapey);
    levels = shapey >> 12;
    if ( (levels == 0) | (levels == 15) )
        levels = 1;
    else
        levels += 1;

    if (newlevels)
    {
        OLDSHPPUTFIELD(theShape->shapey, (shapey & 0xfff) + ((newlevels-1) << 12));
    }
    return(levels);
}

//--------------------------------------------------------------------------------------------------------
//   Mipmap levels are stored in upper 4 bits of
//   shapey. If 0 then shape has no mipmap levels;
//   non-zero records total number of levels,
//   including level 0.
static int  OLDSHPRRetMipLev(OLDSHPHeaderStruct *theShape,int type)
{
    int shapey;
    int levels;
    int width;

    if (!theShape)
        return 0;
    width  = OLDSHPGETFIELD(theShape->width);
    shapey = OLDSHPGETFIELD(theShape->shapey);
    levels = shapey >> 12;
    if ( (levels == 0) | (levels == 15) )
        levels = 1;
    else
        levels += 1;

    if((1<<(levels-1)) > width) // simple error checking for mipmap level information.
        levels = 1;
    return(levels);
}


//--------------------------------------------------------------------------------------------------------
// get the number of colours in the clut.
int OLDSHPRclutcolours(OLDClutHeaderStruct *theClut,int *startptr,int bpp,int type)
{
    int startcolour;
    int colours;
    int colours2;
    int maxcolours;

    startcolour = 0;
    if (type==TYPEOLDVSH || type==TYPEOLDFSH || type == TYPEOLDFSS || type==TYPEOLDXSH || type==TYPEOLDXSS || type==TYPEOLDGSS || type==TYPEOLDGSH || type==TYPEOLDMSH || type == TYPEOLDMSS || type==TYPEOLDSSH || type==TYPEOLDSSS || type==TYPEOLDASH || type==TYPEOLDNSH)
    {
        startcolour = OLDSHPGETFIELD(theClut->centery);
        if (startcolour<0 || startcolour>255)
            startcolour = 0;
    }

    /* 3 conventions for palettes (mostly on OLDVSH) */
    /* 256x1 (current) */
    /* 256x3 */
    /* 768x1 */

    colours = OLDSHPGETFIELD(theClut->centerx);
    colours2 = OLDSHPGETFIELD(theClut->width)&0x0fff;

    maxcolours = 255;
    if (bpp>0 && bpp<=8)
        maxcolours = (1 << bpp);

    if (!colours)    colours = (1 << bpp);
    if (colours>256) colours = 256;

    if (colours < 0
     || colours > maxcolours
     || startcolour < 0
     || startcolour > (maxcolours-1)
     || (startcolour+colours) < 0
     || (startcolour+colours) > maxcolours
     || ((colours != colours2) && (colours*3 != colours2)))
    {
        startcolour = 0;
        colours = (1 << bpp);
    }
    *startptr = startcolour;
    return(colours);
}

static bool OLDSHPRisswizzled(OLDSHPHeaderStruct *Header, int type)
{
    return (OLDSHPGETFIELD(Header->shapex)&0x2000) ? true:false;
}


//--------------------------------------------------------------------------------------------------------
// calculate the size of the shape taking into accound compression etc.
static int OLDSHPRGetSizeShape(OLDSHPHeaderStruct *Header,int type,int oldshpmipmapdataalignment)
{
    int    SizeShape=0;
    int     width;
    int     height;
    int     oldshptype;
    int     bpp;
    int     levels;
	int     collapse = 0;
	int     isDXT    = 0;
    oldshptype = OLDSHPRGetShapetype(Header,type);

    switch (oldshptype)
    {
        case kClutShape:
        {
            int startcolour;
            int colours;

            colours = OLDSHPRclutcolours((OLDClutHeaderStruct *)Header,&startcolour,8,type);
            if ((type==TYPEOLDSSH || type==TYPEOLDSSS) && OLDSHPRisswizzled(Header,type))
                colours = OLDSHPRcsmsize(colours);
            bpp = OLDSHPRGetOldshpbpp(Header,type);
            SizeShape=sizeof(OLDSHPHeaderStruct)+OLDSHPRCalcRowBytes(colours,bpp,type);
            if (Header->type==0x33)
                SizeShape = sizeof(OLDSHPHeaderStruct)+((colours*2+31)&~31)+colours*2;  /* 32 bit palette of 2 IAs, with 32 byte alignment for 2nd clut */

            break;
        }

        case kPictureShape:
            width =  OLDSHPGETFIELD(Header->width);//&0x0fff;
            height = OLDSHPGETFIELD(Header->height);//&0x0fff;
            if (oldshptype!=kPictureShape)
            {
                height = 1;
                if (type==TYPEOLDVSH)
                    width = 256;
                else if (width>256)
                    width = 256;
            }
            bpp = OLDSHPRGetOldshpbpp(Header,type);
			collapse = ((((Header->type)&0x7f) == 0x1e) || ((((Header->type)&0x7f) >= 0x60) && (((Header->type)&0x7f) <= 0x62)));
            if ((Header->type&0x80) && DXT_is((char *)Header+sizeof(OLDSHPHeaderStruct)) || collapse)
            {
                if ((collapse && ((((Header->type)&0x7f) == 0x61) || (((Header->type)&0x7f) == 0x62))) ||                 // DXT3 & DXT5 shapetypes
					(DXT3_is((char *)Header+sizeof(OLDSHPHeaderStruct))) || (DXT5_is((char *)Header+sizeof(OLDSHPHeaderStruct))))
                    bpp = 8;
                else
                    bpp = 4;
			    isDXT = 1;
            }
			// Gst should never have mipmaps. They should
			// be seperate mipmaps.
			if(OLDSHPIsGst(Header))
			{
			  SizeShape = GSTEncodeSize((Gst::GstType)Header->type,width,height) + sizeof(OLDSHPHeaderStruct);
			  break;
			}

            levels = OLDSHPRGetMipmapLevels(Header, 0, type);
            SizeShape=sizeof(OLDSHPHeaderStruct);

            /* Add size of data for each mipmap level (normal shape has 1 "level"). */
            while (levels--)
            {
                /* Account for possibly dxt compressed data (4bpp) */
                if ((Header->type&0x80) && DXT_is((char *)Header+sizeof(OLDSHPHeaderStruct))||
					(collapse))
                {
                    if(type==TYPEOLDGSH)
						SizeShape += OLDSHPALIGN(gmax(4,(width+7)&~7) * gmax(4,(height+7)&~7) * bpp/8);
					    //SizeShape += OLDSHPALIGN(gmax(4,(width+15)&~15) * gmax(4,(height+15)&~15) * bpp/8);
					else
					    SizeShape += OLDSHPALIGN(gmax(4,(width+3)&~3) * gmax(4,(height+3)&~3) * bpp/8);
                    if (!levels && !collapse)
                        SizeShape += sizeof(DXTHEADER);
					isDXT = 1;
                }
                else if ((Header->type&0x80) && IDEC_is_idec((char *)Header+sizeof(OLDSHPHeaderStruct)))
                {
                    int mwidth,mheight,malpha_depth,mquant,ok;
                    ok = IDEC_decodeinfo((char *)Header+sizeof(OLDSHPHeaderStruct), &mwidth, &mheight, &malpha_depth, &mquant, &SizeShape);
                    if (ok)
                        SizeShape += sizeof(OLDSHPHeaderStruct);
                }
                else
                {
                    if((type==TYPEOLDGSH || type == TYPEOLDSSH || type == TYPEOLDMSH) && OLDSHPGETFIELD(Header->shapex)&0x2000 && !isDXT)   // are we OLDGSH and swizzled then we have to have larger width and height.
					{
						int realwidth;
						int realheight;
			            if(type == TYPEOLDGSH)
                            RetOLDGSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
			            if(type == TYPEOLDSSH)
                            RetOLDSSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
			            if(type == TYPEOLDMSH)
                            RetOLDMSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
                        SizeShape = OLDSHPMIPMAPALIGN(SizeShape-sizeof(OLDSHPHeaderStruct))+sizeof(OLDSHPHeaderStruct) + OLDSHPRCalcSizeShape(realwidth,realheight,bpp,type);
					}
					else
                        SizeShape = OLDSHPMIPMAPALIGN(SizeShape-sizeof(OLDSHPHeaderStruct))+sizeof(OLDSHPHeaderStruct) + OLDSHPRCalcSizeShape(width,height,bpp,type);
                }

                if (width > 1)
                    width >>= 1;
                if (height > 1)
                    height >>= 1;
            }
            break;

        case kHotSpotShape:
            SizeShape=8+OLDSHPGETFIELD(((OLDHOTHeaderStruct*)Header)->NumHotSpot)*8;
            break;

        case kCommentShape:
            SizeShape=8+OLDSHPGETFIELD(((OLDCOMMEntheaderStruct*)Header)->SizeComment);
            break;

        case kNameShape:
            SizeShape=4+strlen((const char *)&(((OLDnameheaderStruct*)Header)->ListName[0]))+1;
            break;

        case kAnimShape:
            SizeShape=8+OLDSHPGETFIELD(((OLDAnimHeaderStruct*)Header)->NumAnim)*4;
            break;

        case kTpageShape:
            SizeShape=sizeof(OLDTpageHeaderStruct);
            break;

        case kMetalShape:
            SizeShape=sizeof(OLDMetalHeaderStruct); // 16: does not include metalbin data
            break;

    }
    return(SizeShape);
}

//--------------------------------------------------------------------------------------------------------
// get the offset value of a shape within the databuffer.
static int OLDSHPRGetLinkValue(char *Header, int type)
{
    int    Offset;

    Offset=OLDggetx((void*)&(((OLDSHPHeaderStruct*)Header)->LinkShape),3,type);
    Offset = (Offset<<8)>>8;    /* sign extend */

    return(Offset);
}

//--------------------------------------------------------------------------------------------------------
//   Search through list of shapes for collapsed mipmaps
//   and expand into multiple shapes if present.
static OLDShapeListPtr OLDSHPRExpandMipmaps(OLDShapeListPtr OldshpList, int type, int oldshpmipmapdataalignment)
{
    OLDShapeListPtr    OldshpListOut = 0;
    int             in, out;
    OLDgshapeInfo      *sinfo, *sinfoout;
    OLDSHPHeaderStruct *oldshpin, *oldshpout;
    char            *datain;
    char            *name = 0;
    int             levels, l;
    int             width, height, bpp, size;//,rwidth,rheight;
    int             isDXT = 0;             // if I am expanding a DXT mipmap
                                           // then I have to copy over the
                                           // DXT Header into the lower
                                           // mipmaps.
    int             isDXTOLDSHP= 0;           // if this is a DXT shape type and not a packtype
    // then I don't want to copy over the DXT header but I
    // do want to not align to the nonexistent clut.
    DXTHEADER       DXTCopyHeader;         // my copy of the original Header
                                           // to copy into the mipmaps. Only if
    // DXT packtype NOT DXT shapetype.
    int				collapse = 0;          // whether or not I am expanding a dxt shapetype.
    /* First pass counts number of output shapes. */

    out = 0;
    for (in = 0; in < OldshpList->NuoLdmshape; ++in)
    {
        sinfo = &OldshpList->ListShape[in];
        out += OLDSHPRGetMipmapLevels((OLDSHPHeaderStruct *)sinfo->shape_Data, 0,type);
    }

    /* If no additional levels to expand then just return the input shape list. */
    if (out == OldshpList->NuoLdmshape)
        return OldshpList;

    /* Otherwise allocate and fill in the new shape list. */

    OldshpListOut = (OLDShapeListPtr)galloc(sizeof(OLDgshapeInfo)*out+sizeof(int));
    if (!OldshpListOut)
        return OldshpList;

    OldshpListOut->NuoLdmshape = out;
    out = 0;
    for (in = 0; in < OldshpList->NuoLdmshape; ++in)
    {
        sinfo  = &OldshpList->ListShape[in];
        oldshpin  = (OLDSHPHeaderStruct *)sinfo->shape_Data;
        datain = (char *)oldshpin + sizeof(OLDSHPHeaderStruct);

        width  = OLDSHPGETFIELD(oldshpin->width);
        height = OLDSHPGETFIELD(oldshpin->height);
        bpp    = OLDSHPRGetOldshpbpp(oldshpin, type);
        // now check for shape type and set the bpp count correctly for DXT shape types.
        collapse = (oldshpin->type&0x7f) == 0x1e || (oldshpin->type&0x7f) == 0x60 || (oldshpin->type&0x7f) == 0x61 || (oldshpin->type&0x7f) == 0x62;
        if((oldshpin->type&0x7f) == 0x1e || (oldshpin->type&0x7f) == 0x60)       // DXT1 or OLDGSH DXT1
        {
            isDXTOLDSHP = 1;   // store info that this is indeed a shapetype shape for later.
            bpp = 4;        // we really don't have a bpp of 4 but treat as such anyways.
        }
        else if((oldshpin->type&0x7f) == 0x61 || (oldshpin->type&0x7f) == 0x62)  // DXT3 or DXT5
        {
            isDXTOLDSHP = 1;  // store info that this is indeed a shapetype shape for later.
            bpp = 8;       // we really don't have an 8bpp image but treat as such anyways.
        }
        // NOTE: for DXT shape types we do not need to copy the DXTHEADER in since we construct that
        //       at decode time.

        /* Get number of levels. */

        levels = OLDSHPRGetMipmapLevels(oldshpin, 0, type);

        /* Extract all levels. */

        for (l = 0; l < levels; ++l)
        {
            sinfoout = &OldshpListOut->ListShape[out];

            /* Copy the header. */

            memcpy(sinfoout, sinfo, sizeof(OLDgshapeInfo));

            if(l == 0)  // copy the DXT header if needed.
            {
                // copy the shape name pointer for later reinsertion into the shapename.
                if(name)
                {
                    gfree(name); name = 0;
                }
                if(sinfoout->shape_Name)
                {
                    name = (char *)galloc(strlen((char *)((OLDnameheaderStruct*)(sinfoout->shape_Name))->ListName)+1);
                    strcpy(name,(char *)((OLDnameheaderStruct*)(sinfoout->shape_Name))->ListName);
                }
                else
                {
                    name = (char *)galloc(5);
                    memcpy(name,sinfo->shape_ID,4);
                    name[4] = '\0';
                }


                if(oldshpin->type&0x80 && DXT_is(datain))                           // copy over the DXT
                {                                            // header located at beg.
                    if(DXT3_is(datain) || DXT5_is(datain))
                        bpp = 8;                             // treat as a lower bit depth even though 16bit.
                    else
                        bpp = 4;
                    isDXT = 1;                               // of top DXT image.
                    DXTCopyHeader = *((DXTHEADER *)datain);  // to insert into lower images.
                }
            }

            /* If not top level then signature is "mm<level>",
               clut is copied from top level, hot spots get lost. */

            if (l > 0)
            {
                //sprintf(sinfoout->shape_ID,"%s_mm%02d",name,l);
                sinfoout->shape_ID[0] = 'm';
                sinfoout->shape_ID[1] = 'm';
//                if (l < 10)
//                {
//                    sinfoout->shape_ID[2] = '0'+l;
//                    sinfoout->shape_ID[3] = ' ';
//                }
//                else
                {
                    sinfoout->shape_ID[2] = (char)('0'+(l/10));
                    sinfoout->shape_ID[3] = (char)('0'+(l%10));
                }
                sinfoout->shape_SizeName = (sizeof(OLDnameheaderStruct)+strlen(name)+10);
                sinfoout->shape_Name = (char *)galloc(sizeof(OLDnameheaderStruct)+strlen(name)+10);//(char *)galloc(sinfoout->shape_SizeName);
                memset(sinfoout->shape_Name,0,sizeof(OLDnameheaderStruct)+strlen(name)+10);
                ((OLDnameheaderStruct*)(sinfoout->shape_Name))->type = type;
                //((char *)(((OLDnameheaderStruct*)(sinfoout->shape_Name))->ListName)) = galloc(strlen(name)+10);
                sprintf((char *)((OLDnameheaderStruct*)(sinfoout->shape_Name))->ListName,"%s_mm%02d",name,l);

                /* Copy clut. (We could use the same clut, but then it
                   would be freed twice when the shape is deleted.) */
                if (sinfo->shape_Clut && sinfo->shape_SizeClut)
                {
                    sinfoout->shape_Clut = (char *)galloc(sinfo->shape_SizeClut);
                    memcpy(sinfoout->shape_Clut, sinfo->shape_Clut, sinfo->shape_SizeClut);
                }

                /* Toss other attachments. */
                sinfoout->shape_HotSpot = 0;
                sinfoout->shape_SizeHotSpot = 0;
                //sinfoout->shape_Name = 0;
                //sinfoout->shape_SizeName = 0;
                sinfoout->shape_Comment = 0;
                sinfoout->shape_SizeComment = 0;
                sinfoout->shape_Tpage = 0;
                sinfoout->shape_SizeTpage = 0;
                sinfoout->shape_Metal = 0;
                sinfoout->shape_SizeMetal = 0;
                sinfoout->shape_Anim = 0;
                sinfoout->shape_SizeAnim = 0;

            }

            /* Allocate new shape data. */
            {
                int tw = width;
                int th = height;
                if((isDXT||isDXTOLDSHP) && tw < 4)
                    tw = 4;
                if((isDXT||isDXTOLDSHP) && th < 4)
                    th = 4;

                if((type==TYPEOLDGSH || type==TYPEOLDSSH || type==TYPEOLDMSH) && (OLDSHPGETFIELD(oldshpin->shapex)&0x2000) && !isDXT && !isDXTOLDSHP)  // swizzled OLDGSH logically bigger perhaps...
                {
                    int realwidth;
                    int realheight;
                    if(type == TYPEOLDGSH)
                        RetOLDGSHSwizzleWidthHeight(&realwidth,&realheight,tw,th,bpp);
                    if(type == TYPEOLDSSH)
                        RetOLDSSHSwizzleWidthHeight(&realwidth,&realheight,tw,th,bpp);
                    if(type == TYPEOLDMSH)
                        RetOLDMSHSwizzleWidthHeight(&realwidth,&realheight,width,height,bpp);
                    size = OLDSHPMIPMAPALIGN(OLDSHPRCalcSizeShape(realwidth, realheight, bpp, type));
                }
                else if(type==TYPEOLDGSH && (isDXT || isDXTOLDSHP))
                {
                    tw = (tw+7)&~7;
                    th = (th+7)&~7;
                    size = OLDSHPMIPMAPALIGN(OLDSHPRCalcSizeShape(tw, th, bpp, type));
                }
                else
                    size = OLDSHPMIPMAPALIGN(OLDSHPRCalcSizeShape(tw, th, bpp, type));
            }
            // Gst should never have mipmaps. They should
            // be seperate mipmaps.
            if(OLDSHPIsGst(oldshpin))
                size = GSTEncodeSize((Gst::GstType)oldshpin->type,width,height) + sizeof(SHPHeaderStruct);

            if(isDXT)
            {
                //size /= 4;          // convert from bit count to byte count.
                if(l>0)
                    oldshpout = (OLDSHPHeaderStruct *)galloc(sizeof(OLDSHPHeaderStruct) + size + sizeof(DXTHEADER)); // leave space for DXT header.
                else
                    oldshpout = (OLDSHPHeaderStruct *)galloc(sizeof(OLDSHPHeaderStruct) + size);
            }
            else
                oldshpout = (OLDSHPHeaderStruct *)galloc(sizeof(OLDSHPHeaderStruct) + size);
            sinfoout->shape_Data = (char *)oldshpout;
            sinfoout->shape_SizeData = sizeof(OLDSHPHeaderStruct) + size;

            /* Set shape header fields for this level. */

            memcpy(oldshpout, oldshpin, sizeof(OLDSHPHeaderStruct));
            OLDSHPPUTFIELD(oldshpout->width,  width);
            OLDSHPPUTFIELD(oldshpout->height, height);

            /* Copy data for this level.. */
            if(l>0 && isDXT)  // do I need to copy in the DXT header?
            {
                memcpy((char *)oldshpout + sizeof(OLDSHPHeaderStruct), (char *)(&DXTCopyHeader),sizeof(DXTHEADER));
                memcpy((char *)oldshpout + sizeof(OLDSHPHeaderStruct) + sizeof(DXTHEADER), datain, size);
            }
            else
                memcpy((char *)oldshpout + sizeof(OLDSHPHeaderStruct), datain, size);

            /* Update for next level. */
            if(l == 0 && isDXT)  // do I need to skip the extra bytes for the DXT Header?
                datain += (size + sizeof(DXTHEADER));
            else
                datain += size;
            if (width > 1)
                width >>= 1;
            if (height > 1)
                height >>= 1;
            ++out;
        }

        /* Free input data block. (Cluts and hotspots
           not freed, since they are now attached to
           level 0 output shape.) */

        gfree(sinfo->shape_Data);
    }
    if(name)
    {
        gfree(name); name = 0;
    }
    /* Free old list and return new one. */

    gfree(OldshpList);
    return OldshpListOut;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// includes header size.
static char * RCreateCubemapBuffer(int width,int height,int bpp,int * newsize, int levels,int oldshpmipmapdataalignment)
{
    int csize = sizeof(OLDSHPHeaderStruct);
    // assume width is right and that height is divided by 6 to get real height.
    int rsize = height/6;
    int onesize = 0;
    int i = 0;
    int j = 0;
    int w = width;
    int h = rsize;
    char * newbuf = 0;
    // for each mipmaplevel.
    for(i = 0; i < levels; i++)
    {
        onesize += (int)(OLDSHPMIPMAPALIGN(onesize) + ((float)bpp/8.0)*w*h);
        w = w>>1;
        h = h>>1;
        h = h<1?1:h;
        w = w<1?1:w;
    }
    // align that face.
    //onesize += OLDSHPFACEALIGN(onesize); we will need to align to the face alignment in the future.
    onesize = onesize*6;
    csize += onesize;

    *newsize = csize;
    newbuf   = (char *)galloc(csize);
    return (newbuf);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
static int RgetCubeOffset(int l,int s,int oldshpmipmapdataalignment,int levels,int * lsize,int width,int height,int bpp)
{
    int w = width;
    int h = height;
    int levelsize = 0;
    int i = 0;
    int tsize = 0;
    // go down blocks of mipmaps till I get to my mipmap level.
    for(i = 0 ; i < l; i++)
    {
        levelsize += w*bpp/8*h;
        if(i != (levels-1))
            levelsize = OLDSHPMIPMAPALIGN(levelsize);
        w = w>>1;
        h = h>>1;
    }
    // which face am I on?
    // jump to that face.
    h = h/6;
    *lsize = (int)(((float)bpp/8.0)*h*w);
    tsize = (*lsize)*s + levelsize;
    return(tsize);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
static int RlocNextCube(int l,int current,int oldshpmipmapdataalignment,int levels,int lsize)
{
    // add the written size to the current position.
    current = (current + lsize);
    // align, in the future we will have to do the face alignment but currently we can't as
    // we do not know what to align to,
    if(l < (levels-1))
        current = OLDSHPMIPMAPALIGN(current);
    //else
    //    current = OLDSHPFACEALIGN(current);
    return(current);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Set the relative bit depth if oldshpout is DXT compressed.
static void RGetDXTBpp(OLDSHPHeaderStruct *oldshpout,OLDSHPHeaderStruct *oldshp,int collapse,int * bpp,int type)
{
    // if the shape is packed, it's a DXT shape or DXT pack type then we will check further.
    if ((oldshpout->type&0x80) && DXT_is((char *)oldshpout+sizeof(OLDSHPHeaderStruct)) || collapse)
    {
        if (DXT3_is((const char *)oldshpout+sizeof(OLDSHPHeaderStruct)) || OLDSHPGETFIELD((oldshp->type)) == 0x61)
        {
            *bpp = 8; // DXT3 treated as 8 bpp
        }
        else if (DXT5_is((const char *)oldshpout+sizeof(OLDSHPHeaderStruct))|| OLDSHPGETFIELD(oldshp->type) == 0x62)
        {
            *bpp = 8; // DXT5 treated as 8 bpp
        }
        else  // note this encompasses DXT_1B as well.
        {
            *bpp = 4; // DXT1 treated as 4 bpp
        }
    }
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//   Rewrite mipmapped shapes into single shape. The shape
//   header has the width and height of the top level; data
//   for following levels is concatenated to top level data.
static int OLDSHPRUnorderCubemaps(OLDShapeListPtr theShape,int type,int oldshpmipmapdataalignment)
{
    int                in        = 0;
    OLDgshapeInfo         *sinfo    = 0;
    OLDSHPHeaderStruct    *oldshp      = 0;
    OLDgshapeInfo         *sinfoout;
    OLDSHPHeaderStruct    *oldshpout   = 0;
    OLDMetalHeaderStruct  *smet     = 0;
    int levels                   = 0;
    int collapse                 = 0;
    int size                     = 0;
    int metflag                  = 0;

    for(in = 0; in < theShape->NuoLdmshape;in++)
    {
        sinfo    = &theShape->ListShape[in];
        oldshp      = (OLDSHPHeaderStruct *)sinfo->shape_Data;
        sinfoout = sinfo;             // just to make relevant code look like the collapse mipmaps code.
        oldshpout   = oldshp;
        levels   = OLDSHPRRetMipLev(oldshp,type);
        smet     = (OLDMetalHeaderStruct *)sinfo->shape_Metal;
        if(smet && smet->metal)
            metflag  = OLDSHPGETFIELD(smet->metal);
        else
            metflag = 0;
        // cubemaps are already collapsed. They will look like a single shape with all 6 faces followed by stips of mipmaps. Each strip will contain
        // all the mipmaps for all the feaces for a specific level.
        // so if we effectively have some mipmap levels then we will have to reorder them here.
        if (metflag&4  && (levels > 1))   // 4 means cubemap in the metal information.
        {
            int width            = 0;
            int height           = 0;
            int bpp              = 0;
            int s                = 0;
            int l                = 0;
            int lsize            = 0;
            int offset           = 0;
            unsigned int current = sizeof(OLDSHPHeaderStruct);
            char * tempbuf       = 0;
            char * data          = (char *)oldshp; //+ sizeof(OLDSHPHeaderStruct);
            char * t             = 0;
            width  = OLDSHPGETFIELD(oldshp->width);
            height = OLDSHPGETFIELD(oldshp->height);
            bpp    = OLDSHPRGetOldshpbpp(oldshp, type);
            // figure out if this is a DXT shape type or a packtype dxt shape.. either way set bpp correctly.
            collapse = ((OLDSHPGETFIELD(oldshpout->type) >= 0x60) && (OLDSHPGETFIELD(oldshpout->type) <= 0x62) || (OLDSHPGETFIELD(oldshpout->type) == 0x1e)); // 1e oldgsh comparable to dxt1.
            RGetDXTBpp(oldshpout,oldshp,collapse,&bpp,type);
            // get size of new buffer and create.
            tempbuf = RCreateCubemapBuffer(width,height,bpp,&size,levels,oldshpmipmapdataalignment);
            if(!tempbuf || size <= 0)
                return 0;
            memset(tempbuf,0,size);
            memcpy(tempbuf,oldshp,sizeof(OLDSHPHeaderStruct));
            t = tempbuf + sizeof(OLDSHPHeaderStruct);
            // for side of the cube.
            for(s = 0; s < 6; s++)
            {
                // for mipmap level.
                for(l = 0; l < levels;l++)
                {
                    // get offset of level from top.
                    // get size of level.
                    offset = RgetCubeOffset(l,s,oldshpmipmapdataalignment,levels,&lsize,width,height,bpp);
                    // copy over bitmap.
                    memcpy(t+offset,data+current,lsize);
                    // increment current position.
                    current = RlocNextCube(l,current,oldshpmipmapdataalignment,levels,lsize);
                }
            }
            // cleanup
            if(sinfo->shape_Data)
                gfree(sinfo->shape_Data);
            sinfo->shape_Data     = tempbuf;
            sinfo->shape_SizeData = size;
        }

    }  // for (in) over nuoldmshapes.
    return 1;
}


//-------------------------------------------------------------------------------------------------------
// these are DXT block offsets. The x and y specify the DXT block in a linear scanline ordering.
// the width is the number of DXT blocks that I have in a scanline.
// returns a block ordering so multiply by 8 to get the byte offset.
static int SwizzleDXT1GetIndex( int x, int y, int width)
{
    int NumberOfBlocksX = (1+width)>>1;
    int XBlock = x>>1;
    int YBlock = y>>1;
    int XPix   = x&1;
    int YPix   = y&1;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<2) + // the offset to the current tile
            ((YPix<<1) + (XPix));                    // within the tile the current block (note * by 8 to get address)
}


//--------------------------------------------------------------------------------------------------------
// assumes that DXT blocks have also been swizzled.
static void OLDSHPROLDGSHUnorderBlocks(int width, int height, int rowbytes, unsigned char *source, unsigned char *dest)
{
    int i;
    int j;
    int k;
    unsigned char *s = source;
    unsigned char *d;
	int blockwidth;
	int blockheight;
    for (i=0; i<height; i+=4)
    {
        blockheight = gmin(4, height-i);
        d = dest + i*rowbytes;
        for (j=0; j<width; j+=4)
        {
            blockwidth = gmin(4, width-j);
            s = (source + (SwizzleDXT1GetIndex(j>>2,i>>2,width>>2)*64));
			for (k=0; k<blockheight; k++)
            {
                memcpy(d + k*rowbytes, s, 4 * blockwidth);
                s += 4 * 4;
            }
            d += 4 * 4;
        }
    }
}


//--------------------------------------------------------------------------------------------------------
// for DXT shapes, unorders the DXT blocks after a decode.
static void OLDSHPRUnorderBlocks(int width, int height, int rowbytes, unsigned char *source, unsigned char *dest)
{
    int i;
    int j;
    int k;
    unsigned char *s = source;
    unsigned char *d;
	int blockwidth;
	int blockheight;

    for (i=0; i<height; i+=4)
    {
        blockheight = gmin(4, height-i);
        d = dest + i*rowbytes;
        for (j=0; j<width; j+=4)
        {
            blockwidth = gmin(4, width-j);
            for (k=0; k<blockheight; k++)
            {
                memcpy(d + k*rowbytes, s, 4 * blockwidth);
                s += 4 * 4;
            }
            s += (4-blockheight)*16; // skip over extra padding for odd dimensioned images.
            d += 4 * 4;
        }
    }
}


//--------------------------------------------------------------------------------------------------------
// decode the shape and stuff the shape buffer with decoded information.
static char  *OLDSHPRReadShape(OLDSHPHeaderStruct *AdrShape,char *EndShape,int *size,int type,int oldshpmipmapdataalignment)
{
    char *ResultShape=0L;
    int SizeShape;
    char *s = (char *)AdrShape+sizeof(OLDSHPHeaderStruct);

    SizeShape=OLDSHPRGetSizeShape(AdrShape,type,oldshpmipmapdataalignment);

    if (AdrShape->type&0x80)
    {
        if (REF_is(s))
        {
            SizeShape += 1024;  // kludge to old refpack decode overrun at end of buffer.  not needed for new shape files
        }
        else if (OLDMVCODEC_is(s))
        {
            int MaxSizeShape = EndShape-(char *)AdrShape;

            SizeShape = SizeShape * 5 + 1024; // need a better size estimate

            if (SizeShape > MaxSizeShape)
                SizeShape = MaxSizeShape;
        }
        else
        {
            int MaxSizeShape = EndShape-(char *)AdrShape;

            SizeShape = SizeShape * 3/2 + 1024;

            if (SizeShape > MaxSizeShape)
                SizeShape = MaxSizeShape;
        }
    }
    *size = SizeShape;

    if (SizeShape>0)
    {
        ResultShape=(char *)galloc(SizeShape);
        if (ResultShape)
        {
            char *s = (char *)AdrShape+sizeof(OLDSHPHeaderStruct);
            /* If this is a packed shape then unpack it,
               otherwise just copy the data. */

            if ((AdrShape->type&0x80) && REF_is(s))
            {
                memcpy(ResultShape, AdrShape, sizeof(OLDSHPHeaderStruct));
                REF_decode(ResultShape+sizeof(OLDSHPHeaderStruct),s,0);
            }
            else if ((AdrShape->type&0x80) && OLDMVCODEC_is(s))
            {
                memcpy(ResultShape, AdrShape, sizeof(OLDSHPHeaderStruct));
                OLDMVCODEC_decode(ResultShape+sizeof(OLDSHPHeaderStruct),s,0);
            }
            else
            {
                memcpy(ResultShape,(char *)AdrShape,SizeShape);
            }
        }
    }
    return(ResultShape);
}

// define these so I can check if more than one comment/picture has been loaded per shape.
#define OLDSHPFILLED_PICTURE 1
#define OLDSHPFILLED_COMMENT 2
#define OLDSHPFILLED_CLUT    4
#define OLDSHPFILLED_HOTSPOT 8
#define OLDSHPFILLED_TPAGE   16
#define OLDSHPFILLED_ANIM    32
#define OLDSHPFILLED_METAL   64
#define OLDSHPFILLED_NAME    128


//--------------------------------------------------------------------------------------------------------
// grab each shape, check it's type and extract the information out of the shape.
static int OLDSHPRReadBlocShape(OLDSHPHeaderStruct *AdrShape,char *base, char *EndShape, int flen,char *Pal, OLDgshapeInfo *ResultShape,int type,int oldshpmipmapdataalignment)
{
    int OffsetLink;
    OLDSHPHeaderStruct *CurrentShape;
    OLDSHPHeaderStruct *NextShape;
    OLDHOTHeaderStruct *OLDVSHHotSpots;
    int framesize=0;
    int filled = 0;
    int ok=0;

    ResultShape->shape_Data       = 0;
    ResultShape->shape_Clut       = 0;
    ResultShape->shape_SizeClut   = 0;
    ResultShape->shape_HotSpot    = 0;
    ResultShape->shape_Name       = 0;
    ResultShape->shape_Comment    = 0;
    ResultShape->shape_Tpage      = 0;
    ResultShape->shape_Metal      = 0;
    ResultShape->shape_Anim       = 0;
    ResultShape->subtype          = 0;
    ResultShape->packed           = 0;
    ResultShape->framesize        = 0;

    CurrentShape = AdrShape;
    do
    {
        NextShape = (OLDSHPHeaderStruct *) OLDSHPRReadShape(CurrentShape,EndShape,&framesize,type,oldshpmipmapdataalignment);
        if (NextShape)
        {
            ResultShape->framesize += framesize;
            switch (OLDSHPRGetShapetype((OLDSHPHeaderStruct *)NextShape,type))
            {
                case kClutShape:    if (!(filled&OLDSHPFILLED_CLUT))
                                    {   ResultShape->shape_Clut=(char *)NextShape;
                                        ResultShape->shape_SizeClut=framesize;
                                        filled |= OLDSHPFILLED_CLUT;
                                        break;
                                    }
                                    else
                                    {
                                        gfree(NextShape);
                                        break;
                                    }
                case kHotSpotShape: if(!(filled&OLDSHPFILLED_HOTSPOT)){ResultShape->shape_HotSpot=(char *)NextShape; filled |= OLDSHPFILLED_HOTSPOT;break;}else{gfree(NextShape);break;}
                case kTpageShape:   if(!(filled&OLDSHPFILLED_TPAGE)){ResultShape->shape_Tpage=(char *)NextShape;filled |= OLDSHPFILLED_TPAGE;break;}else{gfree(NextShape);break;}
                case kMetalShape:   if(!(filled&OLDSHPFILLED_METAL)){ResultShape->shape_Metal=(char *)NextShape;filled |= OLDSHPFILLED_METAL;break;}else{gfree(NextShape);break;}
                case kNameShape:    if(!(filled&OLDSHPFILLED_NAME)){ResultShape->shape_Name=(char *)NextShape;filled |= OLDSHPFILLED_NAME;break;}else{gfree(NextShape);break;}
                case kCommentShape: if(!(filled&OLDSHPFILLED_COMMENT)){ResultShape->shape_Comment=(char *)NextShape;filled |= OLDSHPFILLED_COMMENT;break;}else{gfree(NextShape);break;}
                case kAnimShape:    if(!(filled&OLDSHPFILLED_ANIM)){ResultShape->shape_Anim=(char *)NextShape;filled |= OLDSHPFILLED_ANIM;break;}else{gfree(NextShape);break;}
                case kPictureShape: if(!(filled&OLDSHPFILLED_PICTURE))
                    {
                        ResultShape->shape_Data=(char *)NextShape;
                        ResultShape->packed = 0;
                        if (CurrentShape->type&0x80)
                        {
                            ResultShape->packed = REF_is((char *)CurrentShape+16) ? 2
                                                : DXT_is((char *)CurrentShape+16) ? 3
                                                : IDEC_is_idec((char *)CurrentShape+16) ? 4
                                                : OLDMVCODEC_is((char *)CurrentShape+16) ? OLDMVCODEC_fourcc((char *)CurrentShape+16)
                                                : 0;
                        }
                        // GST compression. NB: Block Width and detail bpp encoded in a pack FourCC
                        else if(OLDSHPIsGst(CurrentShape))
                        {
                            ResultShape->packed = GetGstFourCC(CurrentShape->type);
                        }
                        else
                        {
                            if (CurrentShape->type == 0x60
                             || CurrentShape->type == 0x61
                             || CurrentShape->type == 0x62
                             || CurrentShape->type == 0x1e)
                                ResultShape->packed = 3;
                        }
                        filled |= OLDSHPFILLED_PICTURE;
                        break;
                    }
                    else
                    {
                        gfree(NextShape);
                        break;
                    }
                default:
                    gfree(NextShape); break;
            }
        }

        OffsetLink=OLDSHPRGetLinkValue((char *)CurrentShape, type);
        NextShape=(OLDSHPHeaderStruct *)((char *)CurrentShape+OffsetLink);

        /* check next shape type and range and first shape type */

        ok = 0;

        if (OffsetLink && ((char *)NextShape)>base && ((char *)NextShape)<(base+flen)
            && (OLDSHPRGetShapetype(CurrentShape,type))
            && type!=TYPEOLDVSH)
        {
            int nexttype=OLDSHPRGetShapetype(NextShape,type);

            if (nexttype /*&& nexttype!=kPictureShape*/)
            {
                int NextOffsetLink;
                OLDSHPHeaderStruct *NextNextShape;

                NextOffsetLink=OLDSHPRGetLinkValue((char *)NextShape, type);
                NextNextShape=(OLDSHPHeaderStruct *)((char *)NextShape+NextOffsetLink);

                if (!NextOffsetLink || (NextOffsetLink && ((char *)NextNextShape)>base && ((char *)NextNextShape)<(base+flen)))
                {
                    ok = 1;
                    CurrentShape = NextShape;
                }
            }
        }

    } while (ok);

    if (type==TYPEOLDVSH && !ResultShape->shape_HotSpot && ResultShape->shape_Data)
    {
        OLDVSHHotSpots = (OLDHOTHeaderStruct *) galloc(8+4*4);
        if (OLDVSHHotSpots)
        {
            OLDSHPPUTFIELD(OLDVSHHotSpots->type,0x7d);
            OLDSHPPUTFIELD(OLDVSHHotSpots->Link,0);
            OLDSHPPUTFIELD(OLDVSHHotSpots->NumHotSpot,2);
            OLDSHPPUTFIELD(OLDVSHHotSpots->ListHotSpot[0],(int)*(((signed char *)AdrShape)+0));
            OLDSHPPUTFIELD(OLDVSHHotSpots->ListHotSpot[1],(int)*(((signed char *)AdrShape)+1));
            OLDSHPPUTFIELD(OLDVSHHotSpots->ListHotSpot[2],(int)*(((signed char *)AdrShape)+2));
            OLDSHPPUTFIELD(OLDVSHHotSpots->ListHotSpot[3],(int)*(((signed char *)AdrShape)+3));

            ResultShape->shape_HotSpot=(char *)OLDVSHHotSpots;
        }
    }

    if (ResultShape->shape_Data
     || ResultShape->shape_Clut
     || ResultShape->shape_HotSpot
     || ResultShape->shape_Tpage
     || ResultShape->shape_Metal
     || ResultShape->shape_Anim
     || ResultShape->shape_Name
     || ResultShape->shape_Comment)
    {
        if (!ResultShape->shape_Clut)
        {
            // OLDXSH A8 textures should not set the no palette flag
            // since they are a texture with an implied palette.
            if(ResultShape->shape_Data && ((OLDSHPHeaderStruct *)(ResultShape->shape_Data))->type != 0x64)
                ResultShape->subtype |= SUBNOPAL;
            if (Pal)
            {
                ResultShape->shape_Clut=OLDSHPRReadShape((OLDSHPHeaderStruct*)Pal,EndShape,&framesize,type,oldshpmipmapdataalignment);
                ResultShape->shape_SizeClut=framesize;
            }
        }

        // generate a 256x1x8 texture for the clut
        if (!ResultShape->shape_Data)
        {
            unsigned char *p;
            int i;

            int colours = 256;
            int startcolour = 0;
            if (ResultShape->shape_Clut)
                colours = OLDSHPRclutcolours((OLDClutHeaderStruct *)(ResultShape->shape_Clut),&startcolour,8,type);

            ResultShape->subtype |= SUBNOOLDSHP;

            AdrShape = (OLDSHPHeaderStruct *) galloc(16+colours);
            switch (type)
            {
                case TYPEOLDPSH: OLDSHPPUTFIELD(AdrShape->type,0x41); break;
				case TYPEOLDXSS: OLDSHPPUTFIELD(AdrShape->type,0x7b); break;
                case TYPEOLDXSH: OLDSHPPUTFIELD(AdrShape->type,0x7b); break;
                case TYPEOLDFSS: OLDSHPPUTFIELD(AdrShape->type,0x7b); break;
				case TYPEOLDFSH: OLDSHPPUTFIELD(AdrShape->type,0x7b); break;
                case TYPEOLDVSH: OLDSHPPUTFIELD(AdrShape->type,0x7b); break;
                case TYPEOLDGSH: OLDSHPPUTFIELD(AdrShape->type,0x19); break;
				case TYPEOLDGSS: OLDSHPPUTFIELD(AdrShape->type,0x19); break;
                case TYPEOLDMSH: OLDSHPPUTFIELD(AdrShape->type,0x5d); break;
				case TYPEOLDMSS: OLDSHPPUTFIELD(AdrShape->type,0x5d); break;
//                case TYPEOLDNSH: OLDSHPPUTFIELD(AdrShape->type,0x66); break;
                case TYPEOLDSSH: OLDSHPPUTFIELD(AdrShape->type,0x02); break;
				case TYPEOLDSSS: OLDSHPPUTFIELD(AdrShape->type,0x02); break;
                case TYPEOLDASH: OLDSHPPUTFIELD(AdrShape->type,0x02); break;
            }
            OLDSHPPUTFIELD(AdrShape->width,colours);
            OLDSHPPUTFIELD(AdrShape->height,1);
            OLDSHPPUTFIELD(AdrShape->centerx,0);
            OLDSHPPUTFIELD(AdrShape->centery,0);
            OLDSHPPUTFIELD(AdrShape->shapex,0);
            OLDSHPPUTFIELD(AdrShape->shapey,0);
            p = (unsigned char *) AdrShape + 16;
            for (i=0; i<colours; ++i)
                p[i] = (unsigned char)(i);

            ResultShape->shape_Data = (char *)AdrShape;
        }
        if (!ResultShape->shape_Tpage && type==TYPEOLDPSH)
        {
            ResultShape->subtype |= SUBNOTPAGE;
        }
#if 0
        // disable until we can determine a better way to look ahead and detect mipmaps that are not collapsed
        if (!(OLDSHPRGetMipmapLevels((OLDSHPHeaderStruct *)(ResultShape->shape_Data), 0, type) > 1))
        {
            ResultShape->subtype |= SUBNOMIPMAPCOLLAPSE;
        }
#endif
		// if DXT compressed and NOT a shapetype then do this.
		if(  !(((OLDSHPHeaderStruct *)(ResultShape->shape_Data))->type >= 0x60 && ((OLDSHPHeaderStruct *)(ResultShape->shape_Data))->type <= 0x62) &&
			 !(((OLDSHPHeaderStruct *)(ResultShape->shape_Data))->type == 0x1e) && ResultShape->packed == 3)
		{
			ResultShape->subtype |= SUBNODXTCOLLAPSE;
		}


        ok = 1;
    }
    else
    {
        ok = 0;
    }


    return(ok);
}
// only specific flags for loading in a shape block.
#undef OLDSHPFILLED_COMMENT
#undef OLDSHPFILLED_PICTURE
#undef OLDSHPFILLED_METAL
#undef OLDSHPFILLED_TPAGE
#undef OLDSHPFILLED_NAME
#undef OLDSHPFILLED_HOTSPOT
#undef OLDSHPFILLED_ANIM
#undef OLDSHPFILLED_CLUT

static bool OLDSSF_isvideochunk(const char *buf, unsigned int id)
{
    bool ok=false;
    if (id && ggetm(buf,4)==id)
        ok = true;
    return ok;
}

int OLDSSF_numchunks(GSTREAM *g, unsigned int oldshpid, unsigned int oldfntid, int maxcount)
{
    char buf[8];
    int newpos=0;
    int frames=0;
    int chunks=0;
    int ok=1;

    gseek(g,newpos);
    while(ok && (gread(g, buf, 8) == 8) && chunks<maxcount)
    {
        if (OLDSSF_isvideochunk(buf,oldshpid) || OLDSSF_isvideochunk(buf,oldfntid))
        {
            ++frames;
        }
        ++chunks;

        newpos += ggeti(buf+4, 4);
        if ( (newpos >= 0) && (newpos <= glen(g)-8) )
        {
            gseek(g, newpos);
        }
        else
        {
            ok = 0;
        }
    }

    return(frames);
}

//--------------------------------------------------------------------------------------------------------
// returns non zero if the shape type is valid, must be updated each time a new shape type
// is added.

bool OLDSHPRisvalidtype(unsigned char *buf,int type)
{
    return OLDSHPRGetShapetype((OLDSHPHeaderStruct *)buf,type)?true:false;
}

//--------------------------------------------------------------------------------------------------------
// read the entire shape file in and expand all the shape information, also calls expand mipmaps
// and rebuilds the mipmaps from the collapsed data.
int OLDSHPRReadShapeFile(GSTREAM *g,OLDShapeListPtr *theShape,int multi,int type,unsigned int signature, unsigned int oldfntsignature, int oldshpmipmapdataalignment)
{
    OLDSHPFILEHeaderStruct HeaderFile;
    OLDShapeTbl        *TableShape=0L;
    char            *EndShape;
    OLDShapeListPtr    ReturoLdnshape=0L;
    int             SizeReturn;
    OLDgshapeInfo      theOLdgshapeInfo;
    char            *AdrShape;
    char            *Pal=0;
    int             i;
    int             j;
    int             ok=0;
    int             nuoldmshapes;
    int             flen;
    int             hlen;
    int             pass;
    unsigned int    oldshpid;
    char            oldvshheader[6];
    bool isfont=false;


    // check for fonts
    {
        char headerbuf[16];

        gseek(g,(GPOS)0);

        if (gread(g,headerbuf,16) == 16)
        {
            if (oldfntsignature && OLDSSF_isvideochunk(headerbuf,oldfntsignature))
                isfont = true;
        }
    }

	gseek(g,(GPOS)0);

    if (!isfont && multi && (type/TYPEOLDMAX)==SHAPEFILE)
    {
        if (type==TYPEOLDVSH)    /* OLDVSH header */
        {
            ok = (gread(g,oldvshheader,6) == 6);
            hlen = 6;
            nuoldmshapes = ggeti(oldvshheader+4,2);
        }
        else            /* OLDXSH/OLDFSH/OLDSSH/OLDGSH/OLDMSH header */
        {
            ok = (gread(g,(char *)&HeaderFile,sizeof(OLDSHPFILEHeaderStruct)) == sizeof(OLDSHPFILEHeaderStruct));
            hlen = sizeof(OLDSHPFILEHeaderStruct);
            nuoldmshapes = OLDSHPGETFIELD(HeaderFile.NuoLdmshape);
        }

        if (ok)
        {
            ok = 0;
            flen = (int)glen(g);
            TableShape = (OLDShapeTbl*) galloc(flen-hlen);

            if (TableShape)
            {
                /* read in entire file while parsing */

                EndShape = (char *)TableShape+flen-hlen;
                if (gread(g,TableShape,flen-hlen) == flen-hlen)
                {
                    SizeReturn=(sizeof(OLDgshapeInfo)*nuoldmshapes)+sizeof(int);
                    ReturoLdnshape=(OLDShapeListPtr) galloc(SizeReturn);
                    if (ReturoLdnshape)
                    {
                        ReturoLdnshape->NuoLdmshape=0;

                        for (pass=0; pass<2; ++pass)
                        {
                            for (i=0; i<nuoldmshapes; ++i)
                            {
                                if (type==TYPEOLDVSH)
                                {
                                    AdrShape = (char *) TableShape + (ggeti((char *) TableShape + (i+nuoldmshapes)*4, 4)+nuoldmshapes*8+6)-hlen;
                                    oldshpid = ggetm((char *) TableShape + i*4, 4);
                                }
                                else
                                {
                                    AdrShape = (char *) TableShape + OLDSHPGETFIELD(TableShape->ShapeTable[i].OffsetShape)-hlen;
                                    oldshpid = ggetm(TableShape->ShapeTable[i].IDShape,4);
                                }

                                /* fix OLDVSH header now */

                                if (type==TYPEOLDVSH && pass==0)
                                {
                                    char *oldvshheader;
                                    char oldfshheader[16];

                                    oldvshheader = (char *) AdrShape;

                                    memcpy(oldfshheader+4,oldvshheader,   12);
                                    memcpy(oldfshheader  ,oldvshheader+12,4);
                                    memcpy(oldvshheader  ,oldfshheader   ,16);
                                }


                                if ((oldshpid>>24)=='!')
                                {
                                    Pal = AdrShape;
                                    if (!(oldshpid==GMAKEID('!','x','0','0')
                                       || oldshpid==GMAKEID('!','e','g','0')
                                       || oldshpid==GMAKEID('!','c','g','0')))
                                    {
                                        if (type==TYPEOLDVSH)
                                        {
                                            *(Pal) = 0x22;
                                            *(Pal+1) = 0;
                                            *(Pal+2) = 0;
                                            *(Pal+3) = 0;
                                        }
                                    }
                                }

                                if (pass==1)
                                {
                                    if (OLDSHPRReadBlocShape((OLDSHPHeaderStruct*)AdrShape,((char *) TableShape)-hlen,EndShape,flen,Pal,&theOLdgshapeInfo,type,oldshpmipmapdataalignment))
                                    {
                                        j = ReturoLdnshape->NuoLdmshape;
                                        ReturoLdnshape->ListShape[j]=theOLdgshapeInfo;
                                        gputm(ReturoLdnshape->ListShape[j].shape_ID,oldshpid,4);
                                        ++ReturoLdnshape->NuoLdmshape;
                                    }
                                }
                            }
                        }
                    }
                }
                gfree((char *)TableShape);
            }
        }
    }

    if (isfont || (multi && (type/10)==STREAMINOLDGSHAPE))
    {
        TableShape=(OLDShapeTbl*)galloc(((int)glen(g)));
        if (TableShape)
        {
            /* read in entire file while parsing */

            flen = (int)glen(g);
            EndShape = (char *)TableShape+flen;
            if (gread(g,TableShape,flen) == flen)
            {
                // first count number of frames

                int newpos=0;
                char *buf =  (char *)TableShape + newpos;
                nuoldmshapes = 0;
                ok = 1;

                while (ok && newpos<(flen-8))
                {
                    if (OLDSSF_isvideochunk(buf,signature)
                     || OLDSSF_isvideochunk(buf,oldfntsignature))
                    {
                        ++nuoldmshapes;
                    }
                    else // could be audio chunks, or could be a shape
                    {
                        if (!newpos && !nuoldmshapes && OLDSHPRisvalidtype((unsigned char *)buf,type))
                        {
                            ++nuoldmshapes;
                            break;
                        }
                    }

                    newpos += ggeti(buf+4, 4);
                    if ( (newpos >= 0) && (newpos <= glen(g)-8) )
                    {
                        buf =  (char *)TableShape + newpos;
                    }
                    else
                    {
                        ok = 0;
                    }
                }

                if (nuoldmshapes)
                {
                    SizeReturn=(sizeof(OLDgshapeInfo)*nuoldmshapes)+sizeof(int);
                    ReturoLdnshape=(OLDShapeListPtr) galloc(SizeReturn);
                    if (ReturoLdnshape)
                    {
                        ReturoLdnshape->NuoLdmshape=nuoldmshapes;

                        newpos=0;
                        buf =  (char *)TableShape;

                        for (i=0; i<nuoldmshapes; ++i)
                        {
                            if (OLDSSF_isvideochunk(buf,signature))
                            {
                                AdrShape = (char *) buf+8;
                                if (OLDSHPRReadBlocShape((OLDSHPHeaderStruct*)AdrShape,(char *) TableShape, EndShape, flen,(char *)0,&theOLdgshapeInfo,type,oldshpmipmapdataalignment))
                                {
                                    ReturoLdnshape->ListShape[i]=theOLdgshapeInfo;
                                }
                            }
                            else if (OLDSSF_isvideochunk(buf,oldfntsignature))
                            {
                                AdrShape = (char *) buf+OLDggetx(buf+28,4,type);
                                if (OLDSHPRReadBlocShape((OLDSHPHeaderStruct*)AdrShape,(char *) TableShape, EndShape, flen,(char *)0,&theOLdgshapeInfo,type,oldshpmipmapdataalignment))
                                {
                                    ReturoLdnshape->ListShape[i]=theOLdgshapeInfo;
                                }
                            }
                            else if (OLDSHPRisvalidtype((unsigned char *)buf,type))
                            {
                                AdrShape = (char *) buf;
                                if (OLDSHPRReadBlocShape((OLDSHPHeaderStruct*)AdrShape,(char *) TableShape, EndShape, flen,(char *)0,&theOLdgshapeInfo,type,oldshpmipmapdataalignment))
                                {
                                    ReturoLdnshape->ListShape[i]=theOLdgshapeInfo;
                                }
                            }

                            newpos += ggeti(buf+4, 4);
                            if ( (newpos >= 0) && (newpos <= glen(g)-8) )
                            {
                                buf =  (char *)TableShape + newpos;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }

                }
            }
            gfree((char *)TableShape);
        }
    }
#if 0
    else
    {
        TableShape=(OLDShapeTbl*)galloc(((int)glen(g)));
        if (TableShape)
        {
            /* read in entire file while parsing */

            flen = (int)glen(g);
            EndShape = (char *)TableShape+flen;
            if (gread(g,TableShape,flen) == flen)
            {
                SizeReturn=(sizeof(OLDgshapeInfo)*(1))+sizeof(int);
                ReturoLdnshape=(OLDShapeListPtr) galloc(SizeReturn);
                if (ReturoLdnshape)
                {
                    ReturoLdnshape->NuoLdmshape=0;

                    AdrShape = (char *) TableShape;
                    if (ggetm(AdrShape,4)==signature)   /* Font */
                    {
                        AdrShape = OLDggetx(AdrShape+28,4,type)+AdrShape;
                    }

                    if (OLDSHPRReadBlocShape((OLDSHPHeaderStruct*)AdrShape,(char *) TableShape, EndShape, flen,(char *)0,&theOLdgshapeInfo,type,oldshpmipmapdataalignment))
                    {
                        j = 0;
                        ReturoLdnshape->ListShape[j]=theOLdgshapeInfo;
                        gputm(ReturoLdnshape->ListShape[j].shape_ID,0,4);
                        (ReturoLdnshape->NuoLdmshape)++;
                    }
                }
            }
            gfree((char *)TableShape);
        }
    }
#endif

        // Check for and reorder cubemaps.
    ok = OLDSHPRUnorderCubemaps(ReturoLdnshape,type,oldshpmipmapdataalignment);
        // Check for and expand mipmaps if present. These are stored contiguous with
        // the image itself.
    ReturoLdnshape = OLDSHPRExpandMipmaps(ReturoLdnshape, type, oldshpmipmapdataalignment);
    *theShape=ReturoLdnshape;
    if (ReturoLdnshape)
        ok = 1;

    return(ok);
}

//--------------------------------------------------------------------------------------------------------
// Hot Spot shape, get the hot spot information and stuff into ginfo.
int OLDSHPRGetShapeHotSpot(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type)
{
    int                i;
    OLDHOTHeaderStruct*    theHotSpot;

    ginfo->numhotspots=0;
    theHotSpot=(OLDHOTHeaderStruct*)theShape->ListShape[IndexFrame].shape_HotSpot;
    if (theHotSpot)
    {
        ginfo->numhotspots = OLDSHPGETFIELD(theHotSpot->NumHotSpot);
        if(ginfo->numhotspots > GIMEX_HOTSPOTTBL_SIZE)
            return 0;
        for (i=0; i<ginfo->numhotspots; ++i)
        {
            ginfo->hotspottbl[i][0]=OLDSHPGETFIELD(theHotSpot->ListHotSpot[i*2]);
            ginfo->hotspottbl[i][1]=OLDSHPGETFIELD(theHotSpot->ListHotSpot[(i*2)+1]);
        }
    }
    return 1;
}

//--------------------------------------------------------------------------------------------------------
// Anim shape, get any palette animation information, uses extended ginfo structure.
void OLDSHPRGetShapeAnim(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type)
{
    int i;
    OLDAnimHeaderStruct *theAnim;
    GINFOOLDSHP *ginfooldshp = (GINFOOLDSHP *)ginfo;

    ginfooldshp->numanims=0;
    theAnim=(OLDAnimHeaderStruct*)theShape->ListShape[IndexFrame].shape_Anim;
    if (theAnim)
    {
        ginfooldshp->numanims = gmin(MAXPALANIMS,OLDSHPGETFIELD(theAnim->NumAnim));

        for (i=0; i<ginfooldshp->numanims; ++i)
        {
            ginfooldshp->palanim[i].dir   =   ((signed char *)(theAnim->ListAnim+i))[0];
            ginfooldshp->palanim[i].speed =   ((signed char *)(theAnim->ListAnim+i))[1];
            ginfooldshp->palanim[i].start = ((unsigned char *)(theAnim->ListAnim+i))[2];
            ginfooldshp->palanim[i].end   = ((unsigned char *)(theAnim->ListAnim+i))[3];
        }
    }
}

//--------------------------------------------------------------------------------------------------------
// Name shape, get the name of the current shape.
void OLDSHPRGetShapeName(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type)
{
    OLDnameheaderStruct *theName;

    theName=(OLDnameheaderStruct*)theShape->ListShape[IndexFrame].shape_Name;
    if (theName)
    {
        strcpy(ginfo->framename,(char *)theName->ListName);
    }
}

//--------------------------------------------------------------------------------------------------------
// Comment shape, get the comment and stuff into the ginfo structure.
void OLDSHPRGetShapeComment(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type)
{
    OLDCOMMEntheaderStruct *theComment;

    ginfo->comment[0]=0;
    theComment=(OLDCOMMEntheaderStruct*)theShape->ListShape[IndexFrame].shape_Comment;
    if (theComment)
    {
        strcpy(ginfo->comment,(char *)theComment->ListComment);
    }
}


//--------------------------------------------------------------------------------------------------------
// Tpage shape, get the TPage information.
void OLDSHPRGetShapeTpage(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type)
{
    OLDTpageHeaderStruct *theTpage;
    GINFOOLDSHP *ginfooldshp = (GINFOOLDSHP *)ginfo;

    theTpage=(OLDTpageHeaderStruct*)theShape->ListShape[IndexFrame].shape_Tpage;
    if (theTpage)
    {
        ginfooldshp->tpage = OLDSHPGETFIELD(theTpage->tpage);
        ginfooldshp->clutid = (char) OLDSHPGETFIELD(theTpage->clutid);
        ginfooldshp->code =   (char) OLDSHPGETFIELD(theTpage->code);
    }
}

//--------------------------------------------------------------------------------------------------------
// Metal shape, Get Meta Information from a metal.
void OLDSHPRGetShapeMetal(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type)
{
    OLDMetalHeaderStruct *theMetal;

    theMetal=(OLDMetalHeaderStruct*)theShape->ListShape[IndexFrame].shape_Metal;
    if (theMetal)
    {
        unsigned int metalflags = OLDSHPGETFIELD(theMetal->metal);
        if (metalflags & 1)
            ginfo->subtype |= SUBDOT3;
        if (metalflags & 2)
            ginfo->subtype |= SUBEMBM;
        if (metalflags & 4)
            ginfo->subtype |= SUBCUBEMAP;
        if (metalflags & 8)
            ginfo->subtype |= SUBMIPMONT;
        if (metalflags & 16)
            ginfo->subtype |= SUBMETALBIN;
        if (metalflags & 32)
            ginfo->subtype |= SUBOPAQUE;
        if (metalflags & 64)
            ginfo->subtype |= SUBTRANSPARENT;
        if (metalflags & 128)
            ginfo->subtype |= SUBHALFALPHA;
    }
}

//----------------------------------------------------------------------------------------
// checks if a bitmap can or cannot be swizzled. returns non zero if the bitmap is indeed
// swizzled and needs to be unswizzled.
static int Risswizzled(OLDSHPHeaderStruct *HeaderPict,int type,int Width, int Height,int Shapebpp)
{
    int isswizzled = 0;
    int rotatebits=OLDSHPGETFIELD(HeaderPict->shapex);
    if (rotatebits&0x2000)
    {
        if (!(rotatebits&0x1000)) /* reserved, should be off */
        {
            if(type==TYPEOLDSSH || type == TYPEOLDSSS)
            {
                #define ISPOWER2(v) (!((v)&((v)-1)))

                /* some error checking, because previous versions had this bit on for negative shapex */

                if (OLDCanSwizzleSsh(Width,Height,Shapebpp))
                {
                    isswizzled = 1;
                }
            }
            else if(type==TYPEOLDMSH || type == TYPEOLDMSS)
            {
                switch(Shapebpp)
                {
					case 4:
						if(((Width&~31)==Width) && ((Height&~7)==Height))
							isswizzled = 1;
						break;
					case 8:
						if(((Width&~15)==Width) && ((Height&~7)==Height))
							isswizzled = 1;
						break;
					case 16:
						if(((Width&~7)==Width) && ((Height&~7)==Height))
							isswizzled = 1;
						break;
					case 32:
						if(((Width&~3)==Width) && ((Height&~7)==Height))
							isswizzled = 1;
						break;
				}
			}
            else if(type==TYPEOLDGSH || type == TYPEOLDGSS)
            {
                switch(Shapebpp)
                {
                    case 4:// size need to be div by 8x8 for 4 bit else can not swizzle
                        //if(((Width&~7)==Width) && ((Height&~7)==Height))
                            isswizzled = 1;
                        break;
                    case 8:// size needs to be div by 8x4 for 8 bit else can not swizzle
                        //if(((Width&~7)==Width) && ((Height&~3)==Height))
                            isswizzled = 1;
                        break;
                    case 12:                   // YUV support.
					case 15:
                    case 16:
                    case 32:
                        // size  need to be div by 4x4 for alll others else can not swizzle
                        //if(((Width&~3)==Width) && ((Height&~3)==Height))
                            isswizzled = 1;
                        break;
                }
            }
			else if(type ==TYPEOLDXSH || type == TYPEOLDFSH || type == TYPEOLDFSS || type == TYPEOLDXSS)  // OLDXSH and OLDFSH use the same swizzle format.
			{
				isswizzled = 1;
			}
        }
    }
    return(isswizzled);
}

//----------------------------------------------------------------------------------------
// function to handle unswizzleing of all data. Combines all swizzle code into one
// place. Returns a buffer with the unswizzled data with size MemRowBytes*Height.
static char * UnSwizzleAll(char * ShapePtr, int MemRowBytes, int ShapeRowBytes, int Shapebpp, int ShapeType, int type, int Width, int Height)
{
    unsigned int Value = 0;
	int maxrowbytes    = (MemRowBytes>ShapeRowBytes)?MemRowBytes:ShapeRowBytes;
	char * MemPtr      = (char *) galloc(maxrowbytes*Height);
    int Membpp         = Shapebpp<=8?1:4;
    int MemRowbase     = 0;
    int ShapeRowbase   = 0;
	int MemOffset      = 0;
	int ShapeOffset    = 0;
	int x              = 0;
	int y              = 0;
	int bypp           = 0;


    if(MemPtr)
	{
		memset(MemPtr,0,MemRowBytes*Height);
		// do a special check for S shapes 4bpp as it cannot be done inside the normal loop
		if(type == TYPEOLDSSH || type==TYPEOLDSSS)
		{
			if (Shapebpp == 4)                                         // special conversion for PS2 shapes.
			{                                                     // must do this here so that we can expand
				OLDSHAPE_unswizzle4((unsigned char *)ShapePtr,(unsigned char *)MemPtr,Width,Height);
			}
			else if (Shapebpp == 8)
			{
				OLDSHAPE_unswizzle8((unsigned char *)ShapePtr,(unsigned char *)MemPtr,Width,Height);
			}
			else if (Shapebpp == 16 || Shapebpp == 15)
			{
				OLDSHAPE_unswizzle16((unsigned char *)ShapePtr,(unsigned char *)MemPtr,Width,Height);
			}
			else                                                  // 16 bpp unswizzleing unsupported at the moment.
				return(NULL);
		}

		// now do the normal case.
		// expand the internal format 4bpp etc to 8bpp or 32bpp internal GIMEX format.

		if(type == TYPEOLDGSH || type == TYPEOLDXSH || type == TYPEOLDFSH || type == TYPEOLDGSS || type == TYPEOLDFSS || type == TYPEOLDXSS || type == TYPEOLDMSS || type == TYPEOLDMSH)
		{
			// have to set the byte per pixel amount. Most of the 4bpp formats can
			// be treated the same as the 8bpp format in the unswizzle, only
			// oldfsh and oldpsh need special care.
			if(Shapebpp <= 8)       bypp = 1;
			else if(Shapebpp <= 16) bypp = 2;
			else if(Shapebpp <= 24) bypp = 3;
			else if(Shapebpp <= 32) bypp = 4;

			// loop over the entire image and lets unswizzle it into our new temp buffer.
			for (y=0; y<Height; ++y)
			{
				MemOffset=MemRowbase;                                        // on each iteration set the row start position.
				ShapeOffset=ShapeRowbase;
				for (x=0;x<Width;++x)
				{
					if (type==TYPEOLDGSH || type==TYPEOLDGSS)                      // do OLDGSH specific stuff
					{
						ShapeOffset = OLDSHPRswizzleGC(x,y,Width,Shapebpp);     // this swizzles relative to source pixels not destination pixels.
					}
					else if (type==TYPEOLDFSH || type==TYPEOLDXSH || type==TYPEOLDXSS || type==TYPEOLDFSS)    // both OLDXSH and OLDFSH use the same swizzle format.
					{
						int PixOffset;
						int tx,ty;

						PixOffset = OLDSHPRswizzleOLDXSH(x,y,Width,Height);        // do the pixel lookup

						ty = PixOffset/Width;
						tx = PixOffset%Width;
    				    // KLUDGE to index 4 bit
						if(ShapeType == 0x40 || ShapeType == 0x79 || ShapeType == 0x7a)
                            MemOffset = ty*ShapeRowBytes+tx/2;
						else
						    MemOffset = ty*ShapeRowBytes+tx*bypp;            // do a regular pixel lookup
					}
					else if (type==TYPEOLDMSH || type==TYPEOLDMSH)
					{
						int PixOffset;
						int tx,ty;

						PixOffset = OLDSHPWswizzleOLDMSH(x,y,Width,Height,Shapebpp);        // do the pixel lookup

						ty = PixOffset/Width;
						tx = PixOffset%Width;
    				    // KLUDGE to index 4 bit
						if(ShapeType == 0x5c)
                            ShapeOffset = ty*ShapeRowBytes+tx/2;
						else
						    ShapeOffset = ty*ShapeRowBytes+tx*bypp;            // do a regular pixel lookup
					}

					// OLDGSH 32 bpp and
		            // OLDGSH uncompressed DXT have a funny swizzle type for greater cache locality.
					if((type == TYPEOLDGSH || type == TYPEOLDGSS) && (ShapeType == 0x16 || ShapeType == 0x1e))
					{
				        (*(MemPtr+MemOffset+0)) = *(ShapePtr+ShapeOffset+0);
				        (*(MemPtr+MemOffset+1)) = *(ShapePtr+ShapeOffset+1);
				        (*(MemPtr+MemOffset+2)) = *(ShapePtr+ShapeOffset+32);
				        (*(MemPtr+MemOffset+3)) = *(ShapePtr+ShapeOffset+33);
					}
                    // 4 bit low/high
					else if(ShapeType == 0x40 || ShapeType == 0x79 || ShapeType == 0x5c)          // oldfsh's (and oldpsh) have to have each nibble extracted
					{                                                        // and reinserted. Rather ugly but it's the way it
						Value=ShapePtr[ShapeOffset];                         // has to be done.
						if (x&1)
    						MemPtr[MemOffset] |= (char)(Value&0xF0);
						else
							MemPtr[MemOffset] |= (char)(Value&0x0F);
					}
					else if (ShapeType == 0x7a) // 4 bit high/low for old ffn
					{                                                        // and reinserted. Rather ugly but it's the way it
						Value=ShapePtr[ShapeOffset];                         // has to be done.
						if (x&1)
							MemPtr[MemOffset] |= (char)(Value&0x0F);
						else
    						MemPtr[MemOffset] |= (char)(Value&0xF0);
					}
					else                                                           // all other formats can just use a memcpy.
					    memcpy(MemPtr + MemOffset, ShapePtr + ShapeOffset, bypp);  // can only do 8,16,24,32 bit swizzling.

				    if(Shapebpp != 4 || (Shapebpp == 4 && x&1))   // only incerment every second pass for the bpp types otherwise always increment.
					{
					    ShapeOffset += bypp;
					    MemOffset   += bypp;
					}

				}

				ShapeRowbase += ShapeRowBytes; // Increment the pointer with a line align on 16 bits
				MemRowbase   += ShapeRowBytes; // Used to be MemRowBytes;
			}
		} // if OLDXSH,OLDGSH,OLDFSH
	    return(MemPtr);                        // use is responsible for deleting the new membuf.
	}// if MemPtr
	return(NULL);                              // didn't unswizzle so return NULL.
}

#define CLAMP255(a) ( (a) > 255? 255: ( (a) < 0 ? 0: (a) ) )

static unsigned char ROUNDCLAMPINT(float f)
{
    int i = (int) (f+0.5f);
    if (i<0) i=0;
    if (i>255) i=255;
    return (unsigned char) i;
}

static void YUVtoRGB(float &R, float &G, float &B, float Y, float U, float V)
{
    float C = Y - 16.0f;
    float D = U - 128.0f;
    float E = V - 128.0f;

    R = 1.164383f * C                   + 1.596027f * E  ;
    G = 1.164383f * C - (0.391762f * D) - (0.812968f * E) ;
    B = 1.164383f * C +  2.017232f * D                   ;
}


//------------------------------------------------------------------------------------
// convert one 32-bit yuy2 value to two 32-bit source pixels
void OLDyuy2toargb32( unsigned char* pDst, unsigned char* pSrc )
{
	float Y0 = (float)(*(pSrc + 0));
	float U  = (float)(*(pSrc + 1));
	float Y1 = (float)(*(pSrc + 2));
	float V  = (float)(*(pSrc + 3));

	// rescale from (0-240) to (0-255)
//	Y0 *= (255.0f / 240.0f);
//	Y1 *= (255.0f / 240.0f);

	// undo the +128 bias
//	U  -= 128.0f;
//	V  -= 128.0f;

	float R,G,B;

    YUVtoRGB(R,G,B,Y0,U,V);

	((ARGB *) pDst)->a = 255;  // alpha
	((ARGB *) pDst)->r = ROUNDCLAMPINT(R);
	((ARGB *) pDst)->g = ROUNDCLAMPINT(G);
	((ARGB *) pDst)->b = ROUNDCLAMPINT(B);

	// pixel two

    YUVtoRGB(R,G,B,Y1,U,V);

	((ARGB *) (pDst+4))->a = 255;  // alpha
	((ARGB *) (pDst+4))->r = ROUNDCLAMPINT(R);
	((ARGB *) (pDst+4))->g = ROUNDCLAMPINT(G);
	((ARGB *) (pDst+4))->b = ROUNDCLAMPINT(B);
}

#undef CLAMP255

//----------------------------------------------------------------------------------------
//  handles conversion of data from YUY2 format to RGB 32bpp format. (opaque no transparency)
static void YUY2toRGBData(char *dest, int rowbytes,char *source ,int width, int height)
{
    int x = 0;
    int y = 0;
	unsigned char  *d = (unsigned char *)dest;
	unsigned char  *s = (unsigned char *)source;
    bool isodd = (width&1)?true:false;

    for (y=0; y<height; y++)
    {
        for (x=0; x<width; x+=2)
        {
			OLDyuy2toargb32( d, s );
            // if the width and height of the image are odd then
            // then we will have to do some fancy footwork to ensure that
            // the last pixel of each line is not counted.
            if(isodd && x == width-1)
            {
                d+=4;
                s+=4;
            }
            else
            {
			    d += 8;  // advance by two 32-bit argb pixels
			    s += 4;  // advance by one 32-bit yuy2 pixel
            }
        }
    }
}


//----------------------------------------------------------------------------------------
// convert YUV to RGB structure using standard conversion. convert to char and cap
// at 255
static void yumtorgb(YUMOLDCOORD *yum, YUMOLDCOORD *rgb)
{
    int Y;
    int Cb;
    int Cr;
    int Red;
    int Green;
    int Blue;

    Y =  ((yum->x-16)*255+(235-16)/2)/(235-16);
    Cb = (yum->y-128);
    Cr = (yum->z-128);

    Red    = (int)gmax(0,gmin(255,Y+0.0000*Cb+1.4020*Cr));
    Green  = (int)gmax(0,gmin(255,Y-0.3441*Cb-0.7141*Cr));
    Blue   = (int)gmax(0,gmin(255,Y+1.7720*Cb+0.0000*Cr));

    rgb->x = Red;
    rgb->y = Green;
    rgb->z = Blue;
}

//----------------------------------------------------------------------------------------
// pack data into YUV struct and decode data then stuff d structure with
// correct value.
static void YUMdecode(ARGB *d,unsigned int y,unsigned int cb,unsigned int cr)
{
    float C = y - 16.0f;
    float D = cb - 128.0f;
    float E = cr - 128.0f;

    float R = 1.164383f * C                   + 1.596027f * E  ;
    float G = 1.164383f * C - (0.391762f * D) - (0.812968f * E) ;
    float B = 1.164383f * C +  2.017232f * D                   ;

    //yumtorgb(&yum,&rgb);
    d->a = (unsigned char) 255;
    d->r = (unsigned char) ROUNDCLAMPINT(R);     /* r */
    d->g = (unsigned char) ROUNDCLAMPINT(G);     /* g */
    d->b = (unsigned char) ROUNDCLAMPINT(B);     /* b */
}

//----------------------------------------------------------------------------------------
// unswizzle each plane of the source. If unswizzleing fails then return NULL
// otherwise return a pointer to the unswizzled data.
static unsigned char * GetYUVUnswizzled(char * source, int rowbytes, int ShapeType, int type, int width, int height)
{
	unsigned char * yumbuf;
	unsigned char * buf2;
	unsigned char * buf3;
	unsigned char * fin;
	yumbuf = (unsigned char *)UnSwizzleAll(source, rowbytes, width, 8, ShapeType, type, width, height);
    if(yumbuf)
	{

        buf2 = (unsigned char *)UnSwizzleAll(source+(width*height), rowbytes/2, width/2, 8, ShapeType, type, width/2, height/2);
		if(buf2)
		{
			buf3 = (unsigned char *)UnSwizzleAll(source+(width*height)+(width*height/4), rowbytes/2, width/2, 8, ShapeType, type, width/2, height/2);
			if(buf3)
			{
				fin  = (unsigned char *)galloc(width*height*3/2);
				if(fin)
				{
					memcpy(fin,yumbuf,width*height);
					memcpy(fin+(width*height),buf2,width*height/4);
					memcpy(fin+(width*height)+(width*height/4),buf3,width*height/4);
					if(yumbuf) gfree(yumbuf);
					if(buf2)   gfree(buf2);
					if(buf3)   gfree(buf3);
					yumbuf = fin;
				}
				else
				{
    				gfree(yumbuf);
	    			gfree(buf2);
					gfree(buf3);
					yumbuf = 0;
				}
			}
			else
			{
    			gfree(yumbuf);
	    		gfree(buf2);
				yumbuf = 0;
			}
		}
		else
		{
			gfree(yumbuf);
			yumbuf = 0;
		}
	}
	return (yumbuf);
}


//----------------------------------------------------------------------------------------
//  handles conversion of data from YUV format to RGB 32bpp format. (opaque no transparency)
//  also handles unswizzleing, treats each plane as if it were 8bpp.
static void YUVtoRGBData(char *dest, int rowbytes,char *source ,int width, int height, int ShapeType, int type, int isswizzled)
{
    int x = 0;
    int y = 0;
	int f = 0;
    unsigned char *Yptr   = 0;
    unsigned char *Cbptr  = 0;
    unsigned char *Crptr  = 0;
    unsigned char *yumbuf = 0;
    ARGB *d;

    if(isswizzled)
	{
		yumbuf = GetYUVUnswizzled(source,rowbytes, ShapeType, type, width, height);
		if(yumbuf)
			f = 1;
	}
    if(!yumbuf)
        yumbuf = (unsigned char *)source;


    for (y=0; y<height; y+=2)
    {
        d = (ARGB *) (dest+rowbytes*y);
        Yptr = yumbuf+width*y;
        Cbptr = yumbuf+width*height    +width/2*y/2;
        Crptr = yumbuf+width*height*5/4+width/2*y/2;

        for (x=0; x<width; x+=2)
        {
            YUMdecode(d+0,Yptr[0],Cbptr[0],Crptr[0]);
            YUMdecode(d+1,Yptr[1],Cbptr[0],Crptr[0]);
            YUMdecode(d+width+0,Yptr[width+1],Cbptr[0],Crptr[0]);
            YUMdecode(d+width+1,Yptr[width+1],Cbptr[0],Crptr[0]);

            d+=2;
            Yptr += 2;
            Cbptr += 1;
            Crptr += 1;
        }
    }
	if(isswizzled && f && yumbuf) gfree(yumbuf);    // if I unswizzled and it was successful then delete extra memory.
}


static void CompactOLDGSHDXTTexture(unsigned char * source,unsigned char * MemPtr,int pwidth,int pheight,int Width,int Height)
{
    int y   = 0;
	int rowbytes       = 4*pwidth;
	int nonpadrowbytes = 4*Width;
	for(y = 0; y < Height;y++)
	{
	    memcpy(MemPtr,source,nonpadrowbytes);
		MemPtr += nonpadrowbytes;
		source += rowbytes;
	}
}

//----------------------------------------------------------------------------------------
// handle DXT shapes and non collapsed DXT compressed shapes.
// Fills MemPtr with the decompressed shape data.
static void DoDXTUnencode(char * ShapePtr, int ShapeType, char * MemPtr, int collapse, int MemRowBytes, int Width, int Height)
{
    char *temp    = 0;					    	 // to hold my DXT decoded data before I unorder the blocks.
	char *temp2   = 0;                           // to hold my ShapePtr data with the new added DXTHEADER.
    int pwidth    = (Width+3)&~3;                // standard DXT only requires image to be multiple of 4
    int pheight   = (Height+3)&~3;
    int isBig     = 0;
    int TotalSize = 0;                           // find final DXT image size.
	int mode      = 0;                           // figure out which compression mode I am in.
	int dxtb      = 0;

	if(DXT1B_is(ShapePtr) || ShapeType == 0x1e)  // Swizzled OLDGSH bigendian format needs multiple of 16
	{
//		pwidth  = (Width+15)&~15;
//		pheight = (Height+15)&~15;
		pwidth  = (Width+7)&~7;
		pheight = (Height+7)&~7;

		dxtb    = 1;
	}

    TotalSize = pwidth*pheight*sizeof(ARGB);
    isBig = (TotalSize > 16000000)?1:0;          // any images over 16MB have to be handled with the new DXTBIG header.
    if(collapse)                                 // do I have to reconstruct the DXT header?
	{
		int bytes = (ShapeType == 0x60 || ShapeType == 0x1e)?8:16;                          // bytes for each block.
		temp = (char *)galloc(((pwidth>>2)*(pheight>>2)*bytes) + sizeof(DXTHEADER));        // make temp buffer to tack on DXTHEADER.
		memcpy(temp+sizeof(DXTHEADER),ShapePtr,((pwidth>>2)*(pheight>>2)*bytes));           // copy over image data skipping DXTHEADER.
	    //gfree(temp);
		//temp = 0;
	}

	temp2 = (char *)galloc(pwidth*pheight*4);   // create working buffer to decompress into.
	//temp  = (char *)galloc(pwidth*pheight*4);   // create working buffer to unblock    into.
	if (temp2)
    {
		memset(temp2,0,pwidth*pheight*4);

		if(collapse)                            //setup DXTHEADER for DXT shape type.
		{
            if(isBig)                           // big flag forces us to use the BIGDXTHEADER which has 4bytes for size.
                gputm(((BIGDXTHEADER*)temp)->size, pwidth*pheight*sizeof(ARGB), sizeof(((BIGDXTHEADER*)temp)->size));  // put the size
            else
                gputm(((DXTHEADER*)temp)->size, pwidth*pheight*sizeof(ARGB), sizeof(((DXTHEADER*)temp)->size));        // put the size

			// save this code, used to use 555 on PS2.
			//if(type == TYPEOLDSSH)
            //    mode = DXT_ENCODE_RGB_555;

			// for each DXT shapetype rebuild my DXT HEADER
			if(ShapeType == 0x61)       // DXT3 type
			{
                if(isBig)
                {
                    gputm(((BIGDXTHEADER*)temp)->code, DXT_BIGDXT3_CODE, sizeof(((BIGDXTHEADER*)temp)->code));
                    gputm(((BIGDXTHEADER*)temp)->mode, mode | DXT_ENCODE_RGB_FULL | DXT_ENCODE_ALPHA_EXPLICIT, sizeof(((BIGDXTHEADER*)temp)->mode));
                    ((BIGDXTHEADER*)temp)->reserved[0]  = (char)(pwidth  >> 2);
                }
                else
                {
                    gputm(((DXTHEADER*)temp)->code, DXT_DXT3_CODE, sizeof(((DXTHEADER*)temp)->code));
                    gputm(((DXTHEADER*)temp)->mode, mode | DXT_ENCODE_RGB_FULL | DXT_ENCODE_ALPHA_EXPLICIT, sizeof(((DXTHEADER*)temp)->mode));
                    ((DXTHEADER*)temp)->reserved[0]  = (char)(pwidth  >> 2);
                    ((DXTHEADER*)temp)->reserved[1]  = (char)(pheight >> 2);
                }
			}
			else if(ShapeType == 0x62)  // DXT 5 type
			{
                if(isBig)
                {
                    gputm(((BIGDXTHEADER*)temp)->code, DXT_BIGDXT5_CODE, sizeof(((BIGDXTHEADER*)temp)->code));
                    gputm(((BIGDXTHEADER*)temp)->mode, mode | DXT_ENCODE_RGB_FULL | DXT_ENCODE_ALPHA_INTERPOLATED, sizeof(((BIGDXTHEADER*)temp)->mode));
                    ((BIGDXTHEADER*)temp)->reserved[0]  = (char)(pwidth  >> 2);
                }
                else
                {
                    gputm(((DXTHEADER*)temp)->code, DXT_DXT5_CODE, sizeof(((DXTHEADER*)temp)->code));
                    gputm(((DXTHEADER*)temp)->mode, mode | DXT_ENCODE_RGB_FULL | DXT_ENCODE_ALPHA_INTERPOLATED, sizeof(((DXTHEADER*)temp)->mode));
                    ((DXTHEADER*)temp)->reserved[0]  = (char)(pwidth  >> 2);
                    ((DXTHEADER*)temp)->reserved[1]  = (char)(pheight >> 2);
                }
			}
			else if(ShapeType == 0x1e )  // we are GOING to collapse for OLDGSH.
			{
                if(isBig)
                {
                    gputm(((BIGDXTHEADER*)temp)->code, DXT_BIGDXT1B_CODE, sizeof(((BIGDXTHEADER*)temp)->code));
                    gputm(((BIGDXTHEADER*)temp)->mode, mode | DXT_ENCODE_RGB_FULL, sizeof(((BIGDXTHEADER*)temp)->mode));
                    ((BIGDXTHEADER*)temp)->reserved[0]  = (char)(pwidth  >> 2);
                }
                else
                {
                    gputm(((DXTHEADER*)temp)->code, DXT_DXT1B_CODE, sizeof(((DXTHEADER*)temp)->code));
                    gputm(((DXTHEADER*)temp)->mode, mode | DXT_ENCODE_RGB_FULL, sizeof(((DXTHEADER*)temp)->mode));
                    ((DXTHEADER*)temp)->reserved[0]  = (char)(pwidth  >> 2);
                    ((DXTHEADER*)temp)->reserved[1]  = (char)(pheight >> 2);
                }
			}
			else  // assume DXT1 by default.
			{
                if(isBig)
                {
                    gputm(((BIGDXTHEADER*)temp)->code, DXT_BIGDXT1_CODE, sizeof(((BIGDXTHEADER*)temp)->code));
                    gputm(((BIGDXTHEADER*)temp)->mode, mode | DXT_ENCODE_RGB_FULL, sizeof(((BIGDXTHEADER*)temp)->mode));
                    ((BIGDXTHEADER*)temp)->reserved[0]  = (char)(pwidth  >> 2);
                }
                else
                {
                    gputm(((DXTHEADER*)temp)->code, DXT_DXT1_CODE, sizeof(((DXTHEADER*)temp)->code));
                    gputm(((DXTHEADER*)temp)->mode, mode | DXT_ENCODE_RGB_FULL, sizeof(((DXTHEADER*)temp)->mode));
                    ((DXTHEADER*)temp)->reserved[0]  = (char)(pwidth  >> 2);
                    ((DXTHEADER*)temp)->reserved[1]  = (char)(pheight >> 2);
                }
			}
		}

		// Now do the actual decode.
		                                                           // if this is the new DXT shapetype then we get to decode from the temp buffer built above.
		if(collapse)                                               // have to use prepared buffer.because we had to tack on DXTHEADER.
			DXT_decode(temp2,temp,0);
		else                                                       // otherwise we use the DXTHEADER already embeded in the shape and continue.
		{
            if(isBig)
                gputm(((BIGDXTHEADER*)ShapePtr)->size, pwidth*pheight*sizeof(ARGB), sizeof(((BIGDXTHEADER*)ShapePtr)->size));      // put the size
            else
                gputm(((DXTHEADER*)ShapePtr)->size, pwidth*pheight*sizeof(ARGB), sizeof(((DXTHEADER*)ShapePtr)->size));            // put the size
			DXT_decode(temp2,ShapePtr,0);
		}

        // unorder my DXT blocks into linear left to right top down format.
		if(dxtb)  // OLDGSH swizzle must unorder blocks specifically.
		{
		    if(temp)
			    gfree(temp);
			temp = (char *)galloc(pwidth*pheight*4);
			OLDSHPROLDGSHUnorderBlocks(pwidth, pheight, (pwidth*4), (unsigned char *)temp2, (unsigned char *)temp);
			CompactOLDGSHDXTTexture((unsigned char *)temp,(unsigned char *) MemPtr,pwidth,pheight,Width,Height);    // Swizzled OLDGSH bigendian format needs multiple of 16

		}
        else
			OLDSHPRUnorderBlocks(Width, Height, MemRowBytes, (unsigned char *)temp2, (unsigned char *)MemPtr);

    }// if temp2

    // cleanup.
	if(temp2)	gfree(temp2);
	if(temp)    gfree(temp);
}


//----------------------------------------------------------------------------------------
// Read Pixel
// handles reading of a single pixel out of the shape into the memory buffer.
inline static void OLDSHPRReadPixel(int ShapeType,unsigned int *pMemOffset,unsigned int *oLdpshapeOffset,unsigned char *MemPtr, unsigned char *ShapePtr,int *px)
{
    unsigned int Value;
    int MemOffset   = *pMemOffset;
    int ShapeOffset = *oLdpshapeOffset;
	int x = *px;
    switch (ShapeType&0x7F)     /* Ignore the high bit since it has been unpacked */
    {
        case 0x44:        /* OLDPSH 1 bpp */
        //case 0x60:      /* OLDNSH 1 bpp */
            Value=ShapePtr[ShapeOffset];
            if ((x&7)==7)
                ++ShapeOffset;
            else
                Value >>= (7-(x&7));
            MemPtr[MemOffset++] = (char)(Value&1);
            break;

        case 0x01:        /* OLDSSH 4 bpp */
        case 0x40:        /* OLDPSH 4 bpp */
        case 0x5c:        /* OLDMSH 4 bpp indexed */
        case 0x79:        /* OLDFSH 4 bpp */
            Value=ShapePtr[ShapeOffset];
            if (x&1)
            {
                Value >>= 4;
                ++ShapeOffset;
            }
            MemPtr[MemOffset++] = (char)(Value&15);
            break;

        case 0x7a:        /* OLDFSH 4 bpp hi/lo */
            Value=ShapePtr[ShapeOffset];
            if (x&1)
                ++ShapeOffset;
            else
                Value >>= 4;
            MemPtr[MemOffset++] = (char)(Value&15);
            break;


        case 0x10:        /* OLDGSH 4 bpp I4 */
        case 0x18:        /* OLDGSH 4 bpp */
        //case 0x61:      /* OLDNSH 4 bpp intensity--look up grey scale. */
        //case 0x62:      /* OLDNSH 4 bpp intensity/alpha */
//        case 0x63:        /* OLDNSH 4 bpp colour-indexed */
            Value=ShapePtr[ShapeOffset];
            if (x&1)
                ++ShapeOffset;
            else
                Value >>= 4;

            MemPtr[MemOffset++] = (char)(Value&15);
            break;

        case 0x11:        /* OLDGSH 8 bpp I8 */
        case 0x12:        /* OLDGSH 8 bpp IA4 */
        case 0x02:        /* OLDSSH 8 bpp */
        case 0x19:        /* OLDGSH 8 bpp */
        case 0x41:        /* OLDPSH 8 bpp */
//        case 0x64:      /* OLDNSH 8 bpp intensity--look up grey level. */
//        case 0x65:      /* OLDNSH 8 bpp intensity/alpha */
//        case 0x66:        /* OLDNSH 8 bpp colour-indexed */
        case 0x7b:        /* OLDFSH/OLDVSH 8 bpp */
        case 0x5d:        /* OLDMSH 8 bpp */
        case 0x64:        /* OLDXSH 8 bpp Alpha8 */
            MemPtr[MemOffset++] = ShapePtr[ShapeOffset++];
            break;

        case 0x7e:      /* OLDFSH 15 bpp */
            *(int *)(MemPtr+MemOffset) = OLDFSHRpixel5To8(ggeti(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x15:      /* OLDGSH 15 bpp (5a3) */
            *(int *)(MemPtr+MemOffset) = OLDGSHRpixel5To8(ggetm(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x03:      /* OLDSSH 15 bpp */
        case 0x59:      /* OLDMSH 15 bpp */
            *(int *)(MemPtr+MemOffset) = OLDSSHRpixel5To8(ggeti(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

//        case 0x68:      /* OLDNSH 15 bpp RGBA */
//            *(int *)(MemPtr+MemOffset) = OLDNSHRpixel5To8(ggetm(ShapePtr+ShapeOffset,2));
//            ShapeOffset+=2;
//            MemOffset+=4;
//            break;

        case 0x14:      /* OLDGSH 16 bpp */
            *(int *)(MemPtr+MemOffset) = OLDFSHRpixel565To8(ggetm(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x58:      /* OLDMSH 16 bpp */
            *(int *)(MemPtr+MemOffset) = OLDMSHRpixel565To8(ggeti(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x78:      /* OLDFSH 16 bpp */
            *(int *)(MemPtr+MemOffset) = OLDFSHRpixel565To8(ggeti(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x65:      /* OLDXSH IA8     */
            ((ARGB *) (MemPtr+MemOffset))->a = *(ShapePtr+ShapeOffset+1);
            ((ARGB *) (MemPtr+MemOffset))->r = *(ShapePtr+ShapeOffset+0);
            ((ARGB *) (MemPtr+MemOffset))->g = *(ShapePtr+ShapeOffset+0);
            ((ARGB *) (MemPtr+MemOffset))->b = *(ShapePtr+ShapeOffset+0);
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x67:      /* OLDFSH 655 bpp */
            *(int *)(MemPtr+MemOffset) = OLDFSHRpixel655To8(ggeti(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;


        case 0x13:      /* OLDNSH 16 bpp intensity/alpha */
            *(int *)(MemPtr+MemOffset) = OLDGSHRpixelIATo8(ggetm(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x42:      /* OLDPSH 16 bpp */
            *(int *)(MemPtr+MemOffset) = OLDPSHRpixel5To8(ggeti(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

#if 0
        case 0x67:      /* OLDNSH 16 bpp intensity/alpha */
            /* RGBA format reports 15 bpp, so this must be intensity/alpha (YUV not supported). */
            *(int *)(MemPtr+MemOffset) = OLDNSHRpixelIATo8(ggetm(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;
#endif

//                case 0x69:      /* OLDNSH 16 bpp YUV not supported. */
        case 0x6c:      /* OLDFSH 16 bpp YUV not supported. */
            break;

        case 0x63:      /* MADYUV 16 bpp not supported. */
        case 0x68:      /* OLDXSH/OLDFSH 16 bpp YUY2 not supported. */
			break;      // conversion already done

        case 0x5a:      /* OLDMSH 16 bpp argb (4444) */
            *(int *)(MemPtr+MemOffset) = OLDMSHRpixel4444To8(ggeti(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x6d:      /* OLDFSH 16 bpp argb (4444) */
            *(int *)(MemPtr+MemOffset) = OLDFSHRpixel4444To8(ggeti(ShapePtr+ShapeOffset,2));
            ShapeOffset+=2;
            MemOffset+=4;
            break;

        case 0x66:      /* OLDFSH 24 bpp argb (6666) */
            *(int *)(MemPtr+MemOffset) = OLDFSHRpixel6666To8(ggeti(ShapePtr+ShapeOffset,3));
            ShapeOffset+=3;
            MemOffset+=4;
            break;

        case 0x6a:      /* OLDFSH 32 bpp argb (2101010) */
            *(int *)(MemPtr+MemOffset) = OLDFSHRpixel2101010To8(ggeti(ShapePtr+ShapeOffset,3));
            ShapeOffset+=4;
            MemOffset+=4;
            break;


        case 0x04:      /* OLDSSH 24 bpp */
        case 0x43:      /* OLDPSH 24 bpp */
            ((ARGB *) (MemPtr+MemOffset))->a = 255;
            ((ARGB *) (MemPtr+MemOffset))->r = *(ShapePtr+ShapeOffset+0);
            ((ARGB *) (MemPtr+MemOffset))->g = *(ShapePtr+ShapeOffset+1);
            ((ARGB *) (MemPtr+MemOffset))->b = *(ShapePtr+ShapeOffset+2);
            ShapeOffset+=3;
            MemOffset+=4;
            break;

        case 0x7f:      /* OLDFSH 24 bpp */
            ((ARGB *) (MemPtr+MemOffset))->a = 255;
            ((ARGB *) (MemPtr+MemOffset))->r = *(ShapePtr+ShapeOffset+2);
            ((ARGB *) (MemPtr+MemOffset))->g = *(ShapePtr+ShapeOffset+1);
            ((ARGB *) (MemPtr+MemOffset))->b = *(ShapePtr+ShapeOffset+0);
            ShapeOffset+=3;
            MemOffset+=4;
            break;

        case 0x7d:      /* OLDFSH 32 bpp */
		case 0x60:      /* DXT1 type */
		case 0x61:      /* DXT3 type */
		case 0x62:      /* DXT5 type */
            ((ARGB *) (MemPtr+MemOffset))->a = *(ShapePtr+ShapeOffset+3);
            ((ARGB *) (MemPtr+MemOffset))->r = *(ShapePtr+ShapeOffset+2);
            ((ARGB *) (MemPtr+MemOffset))->g = *(ShapePtr+ShapeOffset+1);
            ((ARGB *) (MemPtr+MemOffset))->b = *(ShapePtr+ShapeOffset+0);
            ShapeOffset += 4;
            MemOffset   += 4;
            break;

        case 0x16:      /* OLDGSH 32 bpp */
		case 0x1e:      /* OLDGSH uncompressed DXT */

			((ARGB *) (MemPtr+MemOffset))->a = *(ShapePtr+ShapeOffset+0);
			((ARGB *) (MemPtr+MemOffset))->r = *(ShapePtr+ShapeOffset+1);
			((ARGB *) (MemPtr+MemOffset))->g = *(ShapePtr+ShapeOffset+2);
			((ARGB *) (MemPtr+MemOffset))->b = *(ShapePtr+ShapeOffset+3);
            ShapeOffset += 4;
            MemOffset   += 4;
            break;


        case 0x05:      /* OLDSSH 32 bpp */
        case 0x5b:      /* OLDMSH 32 bpp */
            ((ARGB *) (MemPtr+MemOffset))->r = *(ShapePtr+ShapeOffset+0);
            ((ARGB *) (MemPtr+MemOffset))->g = *(ShapePtr+ShapeOffset+1);
            ((ARGB *) (MemPtr+MemOffset))->b = *(ShapePtr+ShapeOffset+2);
            ((ARGB *) (MemPtr+MemOffset))->a = *(ShapePtr+ShapeOffset+3);
            ShapeOffset += 4;
            MemOffset   += 4;
            break;


//        case 0x6a:      /* OLDNSH 32 bpp RGBA */
//            ((ARGB *) (MemPtr+MemOffset))->r = *(ShapePtr+ShapeOffset+0);
//            ((ARGB *) (MemPtr+MemOffset))->g = *(ShapePtr+ShapeOffset+1);
//            ((ARGB *) (MemPtr+MemOffset))->b = *(ShapePtr+ShapeOffset+2);
//            ((ARGB *) (MemPtr+MemOffset))->a = *(ShapePtr+ShapeOffset+3);
//            ShapeOffset+=4;
//            MemOffset+=4;
//            break;

        default:
            break;
    }
    *pMemOffset   = MemOffset;                   // have to notify the caller as per which
    *oLdpshapeOffset = ShapeOffset;                 // byte I have moved to.
    *px = x;
}


//----------------------------------------------------------------------------------------
// reads the entire bitmap, handles swizzling and DXT compression issues.
int OLDSHPRReadBitmap(OLDSHPHeaderStruct *HeaderPict,char *MemPtr,int MemRowBytes,int type)
{
    int     Width,Height;
    int     x,y;
    int     ShapeRowBytes,Shapebpp;
    unsigned int    MemOffset,ShapeOffset;
    unsigned int    MemRowbase,ShapeRowbase;
    char    *ShapePtr     = 0;
    char    *temoLdpshapePtr = 0;
	int     ShapeType;
    int     Membpp;
    int     isswizzled=0;
	int     collapse = 0;
	int     mode = 0;

	// get information about the shape itself.
    Shapebpp = OLDSHPRGetOldshpbpp(HeaderPict,type);
    Width =    OLDSHPGETFIELD(HeaderPict->width);//&0x0fff;
    Height =   OLDSHPGETFIELD(HeaderPict->height);//&0x0fff;
    ShapeType = OLDSHPGETFIELD(HeaderPict->type)&0x7f;                             // bit depth information is contained here.
    ShapePtr=((char *)HeaderPict)+sizeof(OLDSHPHeaderStruct);                      // skip over the SHAPE header and grab the actual shape data.
	collapse = ((ShapeType >= 0x60 && ShapeType <= 0x62) || ShapeType == 0x1e); // DXT shape type flag meaning we should insert the DXT header and decompress.
    ShapeRowBytes=OLDSHPRCalcRowBytes(Width,Shapebpp,type);

    // DXT Compression
    if (((HeaderPict->type&0x80) && DXT_is(ShapePtr)) || collapse)  // dxt shape types.
    {
		DoDXTUnencode(ShapePtr, ShapeType, MemPtr, collapse, MemRowBytes, Width, Height);
        return(1);
    }
	// GST Compression
	if(OLDSHPIsGst(HeaderPict))
	{
	  OLDSHPDecompressGstShape(HeaderPict,MemPtr,type);
	  return(1);
	}
	// IDEC Compression
    else if ((HeaderPict->type&0x80) && IDEC_is_idec(ShapePtr))
    {
        int pwidth = (Width+15)&~15;
        int pheight = (Height+15)&~15;
		int i = 0;

		void * tempbuf = galloc(pwidth*pheight*sizeof(ARGB));
        ((IDEC_HEADER *)ShapePtr)->size = pwidth*pheight*sizeof(ARGB);

		IDEC_decode( tempbuf, pwidth*sizeof(ARGB), ShapePtr, Width, Height, 0);
		for(i = 0; i < Height; i++)
			memcpy((char *)MemPtr+i*MemRowBytes,(char *)tempbuf+i*pwidth*sizeof(ARGB),MemRowBytes);

		gfree(tempbuf);

        return(1);
    }
	// YUV Page Format
    else if (((HeaderPict->type&0x7F) == 0x1c) || ((HeaderPict->type&0x7F) == 0x6c))
    {
		isswizzled = Risswizzled(HeaderPict,type,Width,Height,Shapebpp);
        YUVtoRGBData(MemPtr,MemRowBytes,ShapePtr,Width,Height,ShapeType,type,isswizzled);
        return(1);
    }
	else if ((HeaderPict->type&0x7F) == 0x68  // YUY2 format
         ||  (HeaderPict->type&0x7F) == 0x63) // MADYUV format
	{
	    YUY2toRGBData(MemPtr, MemRowBytes,ShapePtr ,Width,Height);
	    return(1);
	}

    // check if I need to unswizzle or not. If I need to unswizzle then I better do it
	// while I'm expanding the file into 8bpp or 32bpp common GIMEX format.
	isswizzled = Risswizzled(HeaderPict,type,Width,Height,Shapebpp);


    Membpp = Shapebpp<=8?1:4;
    MemRowbase=0;
    ShapeRowbase=0;

                                           // do unswizzleing if necessary.
	if (isswizzled)
	{
        if(type==TYPEOLDGSH || type==TYPEOLDSSH || type==TYPEOLDMSH)
		{
			int realwidth;
			int realheight;
			if(type == TYPEOLDGSH)
                RetOLDGSHSwizzleWidthHeight(&realwidth,&realheight,Width,Height,Shapebpp);
			if(type == TYPEOLDSSH)
                RetOLDSSHSwizzleWidthHeight(&realwidth,&realheight,Width,Height,Shapebpp);
			if(type == TYPEOLDMSH)
				RetOLDMSHSwizzleWidthHeight(&realwidth,&realheight,Width,Height,Shapebpp);

            ShapeRowBytes= OLDSHPRCalcRowBytes(realwidth,Shapebpp,type);
            temoLdpshapePtr = UnSwizzleAll(ShapePtr, MemRowBytes, ShapeRowBytes, Shapebpp, ShapeType, type, realwidth, realheight);
	    }
		else
		    temoLdpshapePtr = UnSwizzleAll(ShapePtr, MemRowBytes, ShapeRowBytes, Shapebpp, ShapeType, type, Width, Height);
    }
	if(!temoLdpshapePtr)                      // if UnSwizzleAll failed then we are not swizzled.
		isswizzled = 0;

	// now do the normal case.
	// expand the internal format 4bpp etc to 8bpp or 32bpp internal GIMEX format.
    for (y=0; y<Height; ++y)
    {
        MemOffset=MemRowbase;
        ShapeOffset=ShapeRowbase;
        for (x=0;x<Width;++x)
        {
            if(isswizzled)
			    OLDSHPRReadPixel(ShapeType,(unsigned int *)&MemOffset,(unsigned int *)&ShapeOffset,(unsigned char *)MemPtr,(unsigned char *)temoLdpshapePtr,&x);
			else
				OLDSHPRReadPixel(ShapeType,(unsigned int *)&MemOffset,(unsigned int *)&ShapeOffset,(unsigned char *)MemPtr,(unsigned char *)ShapePtr,&x);
        }
        ShapeRowbase += ShapeRowBytes;     // Increment the pointer with a line align on 16 bits
        MemRowbase   += MemRowBytes;
    }

	if(temoLdpshapePtr) gfree(temoLdpshapePtr);  // if I unswizzled then I need to delete the extra memory buffer.
    return(1);
}

