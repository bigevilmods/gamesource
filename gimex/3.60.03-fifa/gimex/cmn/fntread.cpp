// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 FNT - REAL Library Font Gimex                    */
/*                                                                  */
/*                by Frank Barchard & Gerry Shaw                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    0.90 941208 GS initial beta version                           */
/*    0.91 941214 GS ffn 1 and 4 bit read tested and working        */
/*    0.92 941215 GS added centerpoint, hotspots, framename         */
/*    0.93 950406 GS fixed bug in write, no warnings, ANSI C        */
/*    0.94 950418 GS new naming, 1bpp, write looks at frame         */
/*    1.00 950419 GS memory leak fixed, 1 & 4 bpp ffn support       */
/*    1.01 950421 GS would not write 1 bpp fonts                    */
/*    1.02 950426 GS creates 96 or 234 characters (for intl fonts)  */
/*    1.03 950428 GS understands decimal framenames                 */
/*    1.04 950529 FB ginstance signature                            */
/*    1.05 950804 GS allways creates 234 characters                 */
/*    2.00 950820 GS new file format                                */
/*    2.01 950828 GS better handling of ascent/descent/advance      */
/*    2.02 950830 GS fixed bug in x/yoffsets with old fonts         */
/*    2.03 950830 GS optimizations for PSX/PC (min width of 4)      */
/*    2.04 950905 GS removed asserts (global data), fix old read    */
/*    2.05 950908 GS better UV placement algorithm                  */
/*    2.06 950912 GS fixed bug with assumed structure alignment     */
/*    2.07 950914 FB isfnt ggetm, warnings                          */
/*    2.08 950929 GS PSX VRAM specifics, better error handling      */
/*    2.09 951004 FB while gfree loop bug fix                       */
/*    2.10 951020 FB left justify framename decimal digits          */
/*    2.11 951024 FB >=256 use hex (to keep to 4 characters)        */
/*    2.12 951030 GS ultra64 support                                */
/*    2.13 951030 GS all font shapes are now 32 bit aligned         */
/*    2.14 960320 GS fixed problem with advance calc on write       */
/*    2.15 960806 GS fixed problem with advance calc on read        */
/*    2.16 961016 GS sfn characters start on 32 boundary            */
/*    2.17 970117 GS colortbl values are opaque                     */
/*    2.18 970501 GS nfn is now motorola format                     */
/*    2.19 970723 GS extraspace fonts now supported                 */
/*    2.20 970822 KO updates header is fewer writes than expected   */
/*    2.21 980326 GS set subtype to correct value on read           */
/*    2.22 980327 KO Subtype 0x4 is outline so 0x8 adds extra space */
/*    2.23 980814 KO Byte align first pixel in row (1->2)           */
/*    2.24 981027 GS PSX 4 bit fonts are nibble swaped (version 101)*/
/*    2.25 990324 GS FFN font textures (height <= width)            */
/*    2.26 990421 GS Saturn format replaced by Sony PS2 format (sfn)*/
/*    2.27 990421 GS shadow and outline fonts set correct palette   */
/*    2.28 990521 GS large fonts >256x256 no respect boundaries     */
/*    2.29 990810 GS palettes are now unattenuated                  */
/*    2.30 990818 GS 8888 argb bits, sfn nibble swap		        */
/*    2.31 991020 SD 4444 pixel support (imports & exports 32 bit)  */
/*    2.32 991102 SD 8888 pixel support                             */
/*    2.33 000103 SD added kerning table support to be inside file  */
/*    2.34 000105 GS force sfn and pfn width to be power of 2       */
/*    2.35 000530 FB dfn/mfn added, version 200 use 12 byte index   */
/*    2.36 000607 FB import old 11 byte index types, kern 221(+3)   */
/*    2.37 000612 FB export kern table 221 format                   */
/*    2.38 000720 FB about font                                     */
/*    2.39 001121 FB export 0 frames                                */
/*    2.40 010125 SW Swizzles gfn files                             */
/*    2.41 010202 FB XFN added                                      */
/*    2.42 010213 SW Fixed alignment for GC                         */
/*    2.43 010306 FB import kern sign extend                        */
/*    2.44 010424 FB 8 bit                                          */
/*    2.45 010517 FB 128 byte align, version 201, pad with ERTS     */
/*    2.46 010523 FB version 202, 0x64 ALPHA8 shapetype for xfn/ffn */
/*    2.47 010813 ID added palette to the format                    */
/*    2.48 010815 ID all cluts padded out to 128 bytes              */
/*    2.49 010825 ID fixed pixel<<=4 issue for nonautogen cluts     */
/*    2.50 010825 ID fixed clut link when kerntable data in place   */
/*    2.51 011019 FB version 210 64 byte header and char numkern    */
/*    2.52 011104 ID most fonts now export a clut by default        */
/*    2.60 011106 FB clut export size and use code version now      */
/*    2.61 011112 FB 4 bit 0x79 no nibble swap and remove 1 bit     */
/*    2.62 011116 FB version auto, nibbles swapped                  */
/*    2.63 011116 ID fixed font export and import with nibble swap  */
/*    2.64 011122 FB attach clut if exported                        */
/*    3.00 011123 FB kern current,prev,kern,pad1                    */
/*    3.01 011128 FB char12, kern4                                  */
/*    3.02 011129 FB export metalbin attachment                     */
/*    3.03 011203 FB default public states                          */
/*    3.04 011204 FB SCALE_U/V presets for 3rash/eagl               */
/*    3.05 020124 FB unsigned & clamped ascent, descent             */
/*    3.06 020125 FB signed but clamped advance                     */
/*    3.07 020131 FB import xfn create alpha clut                   */
/*    3.08 020203 FB xfn has no clut                                */
/*    3.09 020204 FB advance clamp bug fix                          */
/*    3.10 020206 AE export using new 4 byte kern elements          */
/*	  3.11 020208 AE space char width & height=0(export),=1(import) */
/*	  3.12 020215 AE swizzle for Xbox palettized 8 fonts			*/
/*	  3.13 020218 AE arbitrary images (max. w/h: 255x255) to fonts	*/
/*	  3.14 020221 AE bug fix: A8 XFN, export image data correctly	*/
/*	  3.15 020222 AE DXT1 compression for A8 XFN/FFN (-pack3 ) 		*/
/*	  3.16 020225 FB metalbin 240 for ps2                      		*/
/*    3.17 020312 AE gfn clut (AL88) bug fix, xfn metal offset bug  */
/*	  3.18 020318 AE kerning endian on GC fix						*/
/*	  3.19 020319 AE FontWiz kerning issues							*/
/*	  3.20 020327 AE space char: xoffset = 0, yoffset=ascent-1		*/
/*	  3.21 020328 AE New Font Format,-subtype0x40000 export/import	*/
/*	  3.22 020328 FB metalbin size field filled in                	*/
/*	  3.23 020409 AE bump FFN texture width to 512					*/
/*	  3.50 020410 AE char16 relayed like char12. numkern uchar, doc */
/*	  3.51 020411 AE bug fix for gfn galloc bug						*/
/*	  3.52 020411 FB filter state on by default						*/
/*    3.53 020412 AE advance field for gfn bug fix					*/
/*    3.54 020416 FB detect bad kern tables and loss them           */
/*    3.55 020416 FB FONT_UOFFSET, FONT_VOFFSET = -0.5 (gamecube)   */
/*    3.56 020421 FB METALBIN content EAGL240 EAGL64 etc            */
/*    3.57 020605 FB FONT_UOFFSET = 0.0 (gamecube),FONT_DEPTHWRITE  */
/*    3.58 020610 FB no %c on import of unicodes                    */
/*    3.60 020613 FB advance without centerx                        */
/*    3.61 020623 FB import 16 bpp                                  */
/*    3.62 020703 FB clamp next to 0x7fffff, power of 2 subtype     */
/*    3.63 020710 FB texture page relayed out                       */
/*    3.64 020729 FB FNT_is checks signature before reading version */
/*    3.65 020807 FB 4444 xfn/ffn started                           */
/*    3.66 020814 FB square power of 2 and height is max of heights */
/*    3.67 020909 FB 4kx4k xbox, 1kx1k pc                           */
/*    4.00 021023 FB Font2 use Shape2 as well as Shape1             */
/*    4.01 021027 FB Fnt export but allow FNT import signature      */
/*    4.02 021129 FB states: Blend, undef                           */
/*    4.03 021203 FB large font next field                          */
/*    4.04 021212 FB csm1 cluts for ps2 8 bpp                       */
/*    4.05 021215 FB import large Shape2 based fonts                */
/*    4.06 021220 FB map 4/8 to alpha8 xfn via clut alpha           */
/*    4.07 030109 FB maxwidth256 subtype to limit width (for EAUK)  */
/*    4.08 030114 FB clut link for real5 metal bin fixed            */
/*    4.09 030325 FB 4444 ffn export first draft. (still has clut)  */
/*    4.10 030414 FB swizzle 8 import respect rowbytes              */
/*    4.11 030422 FB 4444 import mask channels (bug fix)            */
/*    4.12 030424 FB pass thru space position for baseline fonts    */
/*    4.13 030429 FB gfn 5a3 fonts, sfn 32 bpp fonts                */
/*    4.14 030506 FB leave space position X as well as Y, alone     */
/*																	*/
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Reentrant                                                        */
/* Files: fntread.cpp, fntwrite.cpp, fntgimex.h, fntold.c           */
/*        pfnread.c, pfnwrite.c, pfngimex.h                         */
/*        ffnread.c, ffnwrite.c, ffngimex.h                         */
/*        nfnread.c, nfnwrite.c, nfngimex.h                         */
/*        gfnread.c, gfnwrite.c, gfngimex.h                         */
/*        sfnread.c, sfnwrite.c, sfngimex.h                         */
/*        mfnread.c, mfnwrite.c, mfngimex.h                         */
/*        xfnread.c, xfnwrite.c, xfngimex.h                         */
/*                                                                  */
/* Version field indicates file format and matches the code version */
/*   100 = 11 byte character, 8 byte kern                           */
/*   200 = 12 byte (padded)                                         */
/*   210 = 128 font header (state)                                  */
/*   300 = 12 byte character and 4 byte kern                        */
/*   350 = 16 byte character                                        */
/*   400 = Shape2 (as well as Shape1)                               */
/*                                                                  */
/* Module is designed to allow code to be #included or linked       */
/* with all font formats.  If linked, local functions are           */
/* duplicated, but static, to avoid link errors.                    */
/* If included, #ifdef prevents duplicate local functions.          */
/* All defines are #undefed to allow include more than once.        */
/* Can include either headers, or code.                             */
/* Local function cannot have compile time #ifdefs for target       */
/* file format.  Uses MAKECODE as runtime check.  Read does not     */
/* care what module, since type is unique.  Write passed SIGNATURE  */
/* and MAKECODE.                                                    */
/*                                                                  */
/* There is a fntcram.c module in perforce that has a working       */
/* cram replacement using the CRAM API.  But until the CRAM API     */
/* respects vertical texture boundaries it should be used.          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* GINFO Field          Font/Character Field                        */
/* -----------          --------------------                        */
/* width                width                                       */
/* height               height                                      */
/* defaultx             u                                           */
/* defaulty             v                                           */
/* centerx              -xoffset                                    */
/* centery              -yoffset                                    */
/* hotspot[0].x         advancex                                    */
/* hotspot[0].y         -yoffset (centery)                          */
/* hotspot[1].x         -xoffset (centerx)                          */
/* hotspot[1].y         advancey (\n height)                        */
/* hotspot[2].x         ascent                                      */
/* hotspot[2].y         descent                                     */
/* hotspot[3..].x       kern previous character code (unicode)      */
/* hotspot[3..].y       kern amount                                 */
/*                                                                  */
/* Subtype Values (not supported by REAL)                           */
/* --------------                                                   */
/* 0x00000000 - normal                                              */
/* 0x00000001 - antialiased                                         */
/* 0x00000002 - dropshadow                                          */
/* 0x00000004 - outline                                             */
/* 0x00000008 - spaced extra hardware filtering (obsolete assumed)  */
/* 0x00020000 - shift-jis encoded (obsolete not supported)          */
/* 0x00040000 - char16 format, 2byte advance, kern offset           */
/* 0x00080000 - power of 2 texture pages                            */
/* 0x00100000 - limit width to 256                                  */
/* Refer to fontfile.doc for more information on this.              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/*                                                                  */
/* Maximum texture widths                                           */
/*    FFN   1024                                                    */
/*    MFN   1024                                                    */
/*    SFN   1024                                                    */
/*    XFN   4096                                                    */
/*    GFN   1024                                                    */
/*    Other 256                                                     */
/*                                                                  */
/* FONT struct                                                      */
/*  unsigned int    signature;    0 signature of font               */
/*  unsigned int    size;         4 size of entire file in bytes    */
/*  unsigned short  version;      8 format vesion (260)             */
/*  unsigned short  num;         10 number of characters in font    */
/*  int             flags;       12 font flags                      */
/*  signed char     centerx;     16 xoffset delta for all chars     */
/*  signed char     centery;     17 yoffset delta for all chars     */
/*  unsigned char   ascent;      18 distance from baseline to top   */
/*  unsigned char   descent;     19 distance baseline to bottom     */
/*  int             chartbl;     20 offset to first character info  */
/*  int             kerntbl;     24 offset to kern table            */
/*  int             shape;       28 offset to shape header          */
/*  int             states[24];  32 driver/font states              */
/*                                                                  */
/* FONTCHAR struct                                                  */
/*      unsigned short  index;                                      */
/*      unsigned char   width;                                      */
/*      unsigned char   height;                                     */
/*      unsigned short  u;                                          */
/*      unsigned short  v;                                          */
/*      unsigned char   advance;                                    */
/*      signed char     xoffset;                                    */
/*      signed char     yoffset;                                    */
/*      unsigned char   numkern;                                    */
/*      unsigned short  kernindex;                                  */
/*                                                                  */
/* FONTKERN struct                                                  */
/*      unsigned short  previouscode; char preceding to be kerned   */
/*      signed   char   kernvalue;    amount to kern left by        */
/*      unsigned char   code;         char about to be kerned       */
/*                                                                  */
/* Common states                                                    */
/* FONT_COLOUR  0 current font colour as ARGB (0xffffffff)          */
/* FONT_SCALEX  6 scale x factor (1.0f)                             */
/* FONT_SCALEY  7 scale y factor (1.0f)                             */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include <stdio.h>
#include <stdlib.h>


#include "gimex.h"
#include "fntgimex.h"

//#include "shpgimex.h"
#include "shpcmn.h"


/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

#ifndef __FNTREAD
#define __FNTREAD 1

#include "fntold.cpp"


#if FNTTYPE == FNTTYPE_XFN

//
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

static int intlog2(int val)
{
	 int t;
    *(float *)&t = (float) val;
    return((t>>23)-127);

}

//
//--------------------------------------------------------------------------------------------------------
// get the swizzled offset into an image buffer. Used for 8 bit Xbox font swizzling
// returned offset is in bits
static int Xbox_GetSwizzledPixelOffset( int x, int y, int width, int height, int depth )
{
    int xBits     = intlog2( width  );
    int yBits     = intlog2( height );
    int offset    = 0;
    int mask      = 0;
    int xExpand   = 0;
    int yExpand   = 0;
    int bitShift  = 0;


    if( xBits < yBits )
    {
        mask = (width - 1);

        xExpand = ExpandTableXbox[ (x & 0xFF) ];

        if( x & 0xFF00 )
            xExpand |= (ExpandTableXbox[x >> 8] << 16);

        yExpand = ExpandTableXbox[y & (0xFF & mask)];

        if( (y & 0xFF00) && (y & mask) )
            yExpand |= (ExpandTableXbox[y >> 8] << 16);

        offset = xExpand | (yExpand << 1);

        offset |= ((y >> xBits) << (xBits * 2));
    }
    else
    {
        mask = (height - 1);

        yExpand = ExpandTableXbox[ (y & 0xFF) ];

        if( y & 0xFF00 )
            yExpand |= (ExpandTableXbox[y >> 8] << 16);

        xExpand = ExpandTableXbox[x & (0xFF & mask)];

        if( (x & 0xFF00) && (x & mask) )
            xExpand |= (ExpandTableXbox[x >> 8] << 16);

        offset = xExpand | (yExpand << 1);

        offset |= ((x >> yBits) << (yBits * 2));
    }

    switch( depth )
    {
        case 32:  bitShift = 5;  break;
        case 16:  bitShift = 4;  break;
        case 15:  bitShift = 4;  break;
        case  8:  bitShift = 3;  break;
        case  4:  bitShift = 2;  break;
   //   default:  REAL_ASSERT(0);     break;		// Ali
    }

    // convert offset to bits
    return (offset << bitShift);
}


static void XboxUnswizzle8( unsigned char* srcBase, int shapew, int shapeh,
							unsigned char * dstBase, int dstRowBytes, int charw, int charh,
							 int u, int v)

{

	int row, col;
	int i =0;
	int j = 0;
	int srcOffset = 0;
	int dstOffset = 0;


	for(row = v; row < v+charh; row++)
	{
		for(col = u; col < u+charw; col++)
		{
			dstOffset = i+j*dstRowBytes;
			srcOffset = (Xbox_GetSwizzledPixelOffset(u+i,v+j, shapew, shapeh, 8) >> 3);

			dstBase[dstOffset]= srcBase[srcOffset];
			i++;

		}

		j++;
		i=0;
	}

}



#endif


static int unmapDXT(int value)
{
	if(value==0)
		return 3;
	if(value==1)
		return 0;
	if(value==2)
		return 2;
	if(value==3)
		return 1;

	else return 0;
}

static void CopyFromDXT1CompressedFont(unsigned char * srcBase, unsigned char * dstBase,
									   int width, int height,int charw, int charh,
									   int u, int v)
{

	// First transform the DXT data into regular linear pixel data, then
	// extract the desired character from this

	int row, col, i;
	int j=0;
	int srcOffset=0;

	int dstOffset=0;

	unsigned char *blockbuf = (unsigned char*) galloc(4);

	// allocate memory for entire texture page. 8bpp
	unsigned char* dstTexturePage = (unsigned char*) galloc(width*height);

	for(row = 0; row < height; row+=4)
	{
		for(col=0; col < width; col+=4)
		{
			// skip the colours
			srcOffset += 4;



			FNTPUTMEM(blockbuf, FNTGETMEM(&srcBase[srcOffset], 4), 4);

			dstOffset = row*width+col;


			for(i = 0; i<4; i++)
			{

			dstTexturePage[dstOffset] = unmapDXT(blockbuf[i]&3);
			dstTexturePage[dstOffset+1] = unmapDXT((blockbuf[i]&12)>>2);
			dstTexturePage[dstOffset+2] = unmapDXT((blockbuf[i]&48)>>4);
			dstTexturePage[dstOffset+3] = unmapDXT((blockbuf[i]&192)>>6);

			dstOffset += width;
			}

			// skip the pixel data we just read
			srcOffset += 4;

		}


	}

	// We now extraxt the desired part of the entire texture page

	for(row = v; row < v+charh; row++)
	{
		for(col = u; col < u+charw; col++)
		{

			srcOffset = row*width+col;

			dstBase[j] = dstTexturePage[srcOffset];
			j++;



		}



	}

	gfree(dstTexturePage);
	gfree(blockbuf);

}

//--------------------------------------------------------------------------------------------------------
// get the number of colours in the clut.
static int SHPRclutcolours(CLUTRHeaderStruct *theClut,int *startptr,int bpp,int type)
{
    int startcolour;
    int colours;
    int colours2;
    int maxcolours;

    startcolour = 0;
    if (type==FNTTYPE_FFN || type==FNTTYPE_XFN || type==FNTTYPE_GFN || type==FNTTYPE_MFN || type==FNTTYPE_SFN || type==FNTTYPE_NFN)
    {
        startcolour = FNTGETFIELD(theClut->centery);
        if (startcolour<0 || startcolour>255)
            startcolour = 0;
    }

    /* 3 conventions for palettes (mostly on VSH) */
    /* 256x1 (current) */
    /* 256x3 */
    /* 768x1 */

    colours  = FNTGETFIELD(theClut->centerx);
    colours2 = FNTGETFIELD(theClut->width)&0x0fff;

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

//--------------------------------------------------------------------------------------------------------
// Read the ClutShape and fill in the Ginfo clut.
static void FNTRGetClut(GINFO * ginfo, CLUTRHeaderStruct * theClut,int bpp,int type)
{
    /* 8 grey or colour, with 16 or 24 bpp palette */

    int startcolour = 0;
    int colours     = 0;
    int csm1        = 0;
    int i           = 0;
    int j           = 0;
    unsigned char * pal      = 0;


    if (!bpp)        bpp = 8;

    ginfo->bpp              = 8;                        /* Bits per pixel (8 or 32) */
    ginfo->originalbpp      = bpp;
    colours = (1 << bpp);

    if (theClut)
    {
        int clutx;
        int cluty;
        clutx = cluty = 0;
        if (type==FNTTYPE_PFN)
        {
            clutx = FNTGETFIELD(theClut->shapex);
            cluty = FNTGETFIELD(theClut->shapey);
        }

        colours = SHPRclutcolours(theClut,&startcolour,bpp,type);
        csm1 = FNTGETFIELD(theClut->shapex)&0x2000;
        if (type==FNTTYPE_SFN && !csm1 && bpp==8)
            ginfo->subtype |= SUBCSM2;

        ginfo->numcolours  = colours+startcolour;

        pal = (unsigned char *) &theClut->Entries[0];
        switch (theClut->type)
        {
            case 0x22:  /* VSH 24 bpp (6 bit channels) */
                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 6;
                ginfo->alphabits = 0;
                if (colours==256 && !ggeti(&pal[255*3+0],3))
                    ginfo->alphabits = 1;

                for (i=0; i<colours; ++i)
                {
                    ginfo->colourtbl[startcolour+i].a = (unsigned char)((i==255 && ginfo->alphabits)?0:255);
                    ginfo->colourtbl[startcolour+i].r = (unsigned char)((pal[i*3+0]*255+32)/63);
                    ginfo->colourtbl[startcolour+i].g = (unsigned char)((pal[i*3+1]*255+32)/63);
                    ginfo->colourtbl[startcolour+i].b = (unsigned char)((pal[i*3+2]*255+32)/63);
                }
                break;


            case 0x24:  /* FSH 24 bpp */
                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 8;
                ginfo->alphabits = 0;
                if (colours==256 && !ggeti(&pal[255*3+0],3))
                    ginfo->alphabits = 1;

                for (i=0; i<colours; ++i)
                {
                    ginfo->colourtbl[startcolour+i].a = (unsigned char)((i==255 && ginfo->alphabits)?0:255);
                    ginfo->colourtbl[startcolour+i].r = pal[i*3+0];
                    ginfo->colourtbl[startcolour+i].g = pal[i*3+1];
                    ginfo->colourtbl[startcolour+i].b = pal[i*3+2];
                }
                break;

            case 0x21:  /* SSH 32 bpp */
                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = ginfo->alphabits = 8;

                for (i=0; i<colours; ++i)
                {
                    j = (csm1 && (i&31)>=8 && (i&31)<=23) ? i ^ 0x18 : i;

                    ginfo->colourtbl[startcolour+i].r = pal[j*4+0];
                    ginfo->colourtbl[startcolour+i].g = pal[j*4+1];
                    ginfo->colourtbl[startcolour+i].b = pal[j*4+2];
                    ginfo->colourtbl[startcolour+i].a = pal[j*4+3];
                }
                break;

            case 0x32:  /* GSH 3555 bpp */
                ginfo->alphabits = 3;
                ginfo->redbits =   5;
                ginfo->greenbits = 5;
                ginfo->bluebits =  5;
                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=GSHRpixel5To8(ggetm(pal+i*2,2));
                }
                break;

            case 0x2d:  /* FSH 1555 bpp */
                ginfo->alphabits = 1;
                ginfo->redbits =   5;
                ginfo->greenbits = 5;
                ginfo->bluebits =  5;
                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=FSHRpixel5To8(ggeti(pal+i*2,2));
                }
                break;

            case 0x20:  /* SSH 1555 bpp */
            case 0x39:  /* MSH 1555 bpp */
                ginfo->alphabits = 1;
                ginfo->redbits =   5;
                ginfo->greenbits = 5;
                ginfo->bluebits =  5;
                for (i=0; i<colours; ++i)
                {
                    j = i;
                    if (csm1)
                        if ((i&31)>=8 && (i&31)<=23)
                            j ^= (8^16);

                    *(int *)&ginfo->colourtbl[startcolour+i]=SSHRpixel5To8(ggeti(pal+j*2,2));
                }
                break;

            case 0x31:  /* GSH 565 bpp */
            case 0x29:  /* FSH 565 bpp */
                ginfo->alphabits = 0;
                ginfo->redbits =   5;
                ginfo->greenbits = 6;
                ginfo->bluebits =  5;
                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=FSHRpixel565To8(ggeti(pal+i*2,2));
                }
                break;

            case 0x38:  /* MSH 565 bpp */
                ginfo->alphabits = 0;
                ginfo->redbits =   5;
                ginfo->greenbits = 6;
                ginfo->bluebits =  5;
                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=MSHRpixel565To8(ggeti(pal+i*2,2));
                }
                break;

            case 0x3a:  /* MSH 4444 bpp */
                ginfo->alphabits = 4;
                ginfo->redbits =   4;
                ginfo->greenbits = 4;
                ginfo->bluebits =  4;
                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=MSHRpixel4444To8(ggeti(pal+i*2,2));
                }
                break;

            case 0x30:  /* GSH IA8 */
                ginfo->alphabits = 8;
                ginfo->redbits =   8;
                ginfo->greenbits = 0;
                ginfo->bluebits =  0;
                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=GSHRpixelIATo8(ggetm(pal+i*2,2));
                }
                break;


            case 0x2a:  /* XSH/FSH 32 bpp */
                {
                    ginfo->alphabits = 8;
                    ginfo->redbits =   8;
                    ginfo->greenbits = 8;
                    ginfo->bluebits =  8;
                    for (i=0; i<colours; ++i)
                    {
                        ginfo->colourtbl[startcolour+i].b = pal[i*4+0];
                        ginfo->colourtbl[startcolour+i].g = pal[i*4+1];
                        ginfo->colourtbl[startcolour+i].r = pal[i*4+2];
                        ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                    }
                }
                break;

            case 0x3b:  /* MSH 32 bpp */
                ginfo->alphabits = 8;
                ginfo->redbits =   8;
                ginfo->greenbits = 8;
                ginfo->bluebits =  8;
                for (i=0; i<colours; ++i)
                {
                        ginfo->colourtbl[startcolour+i].r = pal[i*4+0];
                        ginfo->colourtbl[startcolour+i].g = pal[i*4+1];
                        ginfo->colourtbl[startcolour+i].b = pal[i*4+2];
                        ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                }
                break;

            case 0x2e:  /* FSH 8555 bpp */
                ginfo->alphabits = 8;
                ginfo->redbits =   5;
                ginfo->greenbits = 5;
                ginfo->bluebits =  5;
                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=FSHRpixel5To8(ggeti(pal+i*4,2));
                    ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                }
                break;

            case 0x2c:  /* FSH 8565 bpp */
                ginfo->alphabits = 8;
                ginfo->redbits =   5;
                ginfo->greenbits = 6;
                ginfo->bluebits =  5;
                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=FSHRpixel565To8(ggeti(pal+i*4,2));
                    ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                }
                break;

            case 0x23:  /* PSH 16 bpp */
                ginfo->alphabits = 1;
                ginfo->redbits =   5;
                ginfo->greenbits = 5;
                ginfo->bluebits =  5;

                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=PSHRpixel5To8(ggeti(pal+i*2,2));
                }
                break;

            case 0x27:  /* NSH 16 bpp RGBA */
                ginfo->alphabits = 1;
                ginfo->redbits =   5;
                ginfo->greenbits = 5;
                ginfo->bluebits =  5;

                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=NSHRpixel5To8(ggetm(pal+i*2,2));
                }
                break;

            case 0x28:  /* NSH 16 bpp intensity/alpha */
                ginfo->alphabits = 8;
                ginfo->redbits =   8;
                ginfo->greenbits = 8;
                ginfo->bluebits =  8;

                for (i=0; i<colours; ++i)
                {
                    *(int *)&ginfo->colourtbl[startcolour+i]=NSHRpixelIATo8(ggetm(pal+i*2,2));
                }
                break;

            default:
                break;
        }// end switch
    }// end if TheClut
}

#define GC_SWIZZLE 1

#if FNTTYPE == FNTTYPE_GFN && GC_SWIZZLE==1
static int GC_SwizzleGetPixelOffset32( int x, int y, int width)
{
    int NumberOfBlocksX = (3+width)>>2;
    int XBlock = x>>2;
    int YBlock = y>>2;
    int XPix   = x&3;
    int YPix   = y&3;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<6) + // the offset to the current block
            ((YPix<<3) + (XPix<<1)); // within the block the current pixel
}

static int GC_SwizzleGetPixelOffset16( int x, int y, int width)
{
    int NumberOfBlocksX = (3+width)>>2;
    int XBlock = x>>2;
    int YBlock = y>>2;
    int XPix   = x&3;
    int YPix   = y&3;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<3) + (XPix<<1)); // within the block the current pixel
}

static int GC_SwizzleGetPixelOffset8( int x, int y, int width)
{
    int NumberOfBlocksX = (7+width)>>3;
    int XBlock = x>>3;
    int YBlock = y>>2;
    int XPix   = x&7;
    int YPix   = y&3;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<3) + XPix); // within the block the current pixel
}

static int GC_SwizzleGetPixelOffset4( int x, int y, int width)
{
    int NumberOfBlocksX = (7+width)>>3;
    int XBlock = x>>3;
    int YBlock = y>>3;
    int XPix   = x&7;
    int YPix   = y&7;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<2) + (XPix>>1)); // within the block the current pixel
}


static int getpixeladr(int x,int y,int WidthInPix,int Bpp)
{
    switch(Bpp)
    {
        case 32:
            return GC_SwizzleGetPixelOffset32( x, y, WidthInPix);
        case 16:
        case 15:
            return GC_SwizzleGetPixelOffset16( x, y, WidthInPix);
        case 8:
            return GC_SwizzleGetPixelOffset8 ( x, y, WidthInPix);
        case 4:
            return GC_SwizzleGetPixelOffset4 ( x, y, WidthInPix);
    }
    return 0;
}



// copy from the 4 bit or 8 bit big swizzled font bit map to the 8 bit charater bit map
static void CopyFromGC(unsigned char* srcBase, int srcRowBytes, int srcBpp,
                       unsigned char* dstBase, int destRowBytes, int dstBpp,
                       int w, int h, int u, int v)
{
    int x;
    int y;
    int Value;
    int srcoffset;

    int WidthInPix;

    switch(srcBpp)
    {
        case 32:
            WidthInPix = srcRowBytes/4;
            break;
		case 16:
            WidthInPix = srcRowBytes/2;
            break;
		case 8:
            WidthInPix = srcRowBytes;
            break;
        case 4:
            WidthInPix = srcRowBytes<<1;
            break;
    }

    for (y = 0; y < h; y++)
    {
        unsigned char* dst = dstBase + (0+y)*destRowBytes;

        for (x = 0; x < w; x++)
        {
            srcoffset = getpixeladr(u+x,v+y,WidthInPix,srcBpp);

            if(srcBpp == 4)
            {

                Value = srcBase[srcoffset];
                if (!(x&1))
                    Value >>= 4;

                dst[0] = (unsigned char)(Value&15);
                dst++;
            }
            else if(srcBpp == 8)
            {
                dst[0] = srcBase[srcoffset];
                dst++;
            }
            else if(srcBpp == 16)
            {
                *(int *)dst = GSHRpixel5To8(ggetm(srcBase+srcoffset,2));
				dst += 4;
            }
        }
    }
}

#endif

static void CopyFrom(unsigned char *srcBase, int srcRowBytes, int srcBpp,
                     unsigned char *dstBase, int dstRowBytes, int dstBpp,
                     int w, int h, int u, int v, int type)
{
    int x;
    int y;

    for (y = 0; y < h; y++)
    {
        unsigned char *src = srcBase + (v+y)*srcRowBytes;
        unsigned char *dst = dstBase + (0+y)*dstRowBytes;

        if (srcBpp==8)
        {
            for (x=0; x<w; ++x)
            {
                dst[x] = src[u+x];
            }
        }
        else if (type==0x6d)  /* FSH 4444 */
        {
            for (x=0; x<w; ++x)
            {
                unsigned int p=ggeti(src+(u+x)*2,2);
                ((ARGB *) dst)[x].a = (unsigned char)((((p>>12)&15)*255+8)/15);
                ((ARGB *) dst)[x].r = (unsigned char)((((p>> 8)&15)*255+8)/15);
                ((ARGB *) dst)[x].g = (unsigned char)((((p>> 4)&15)*255+8)/15);
                ((ARGB *) dst)[x].b = (unsigned char)((((p>> 0)&15)*255+8)/15);
            }
        }
        else if (type==0x5a)  /* MSH 4444 */
        {
            for (x=0; x<w; ++x)
            {
                unsigned int p=ggeti(src+(u+x)*2,2);
                ((ARGB *) dst)[x].a = (unsigned char)((((p>>12)&15)*255+8)/15);
                ((ARGB *) dst)[x].b = (unsigned char)((((p>> 8)&15)*255+8)/15);
                ((ARGB *) dst)[x].g = (unsigned char)((((p>> 4)&15)*255+8)/15);
                ((ARGB *) dst)[x].r = (unsigned char)((((p>> 0)&15)*255+8)/15);
            }
        }
        else if (type==0x7d)  /* FSH 8888 */
        {
            for (x=0; x<w; ++x)
            {
                unsigned int p=ggeti(src+(u+x)*4,4);
                ((ARGB *) dst)[x].a = (unsigned char)(p>>24);
                ((ARGB *) dst)[x].r = (unsigned char)(p>>16);
                ((ARGB *) dst)[x].g = (unsigned char)(p>> 8);
                ((ARGB *) dst)[x].b = (unsigned char)(p>> 0);
            }
        }
        else if (type==0x05 || type==0x5b)  /* SSH 8888 & MSH 8888 */
        {
            for (x=0; x<w; ++x)
            {
                unsigned int p=ggeti(src+(u+x)*4,4);
                ((ARGB *) dst)[x].a = (unsigned char)(p>>24);
                ((ARGB *) dst)[x].b = (unsigned char)(p>>16);
                ((ARGB *) dst)[x].g = (unsigned char)(p>> 8);
                ((ARGB *) dst)[x].r = (unsigned char)(p>> 0);
            }
        }
        else // bpp==4
        {
            int Value;
            for (x=0; x<w; ++x)
            {
                // lo/hi
                if (type==0x01  /* SSH 4 bit */
                 || type==0x40  /* PSH 4 bit */
                 || type==0x5c  /* MSH 4 bit */
                 || type==0x79) /* FSH 4 bit */
                {
                    if ((u+x)&1)
                        Value = src[(u+x)>>1]>>4;
                    else
                        Value = src[(u+x)>>1]&0x0f;
                }
                else        /* GSH/NSH 4 bit */
                {
                    if ((u+x)&1)
                        Value = src[(u+x)>>1]&0x0f;
                    else
                        Value = src[(u+x)>>1]>>4;
                }
                dst[x] = Value;
            }
        }
    }
}

static CHARACTER12 *GetCharacter12(FONTREADINFO *info, int index)
{
    CHARACTER12 *c;

    if (FNTGETFIELD(info->header->version) < 200)
        c = (CHARACTER12 *)(((char*)info->chartbl) + index*CHARACTERSIZE11);
    else
        c = (CHARACTER12 *)(((char*)info->chartbl) + index*CHARACTERSIZE12);

	// if we have found the space character, and its height and width are
	// 0, change them to 1 for compatability with tools. (Note: font exporter
	// changes the space character's width and height to 0)
	 if( FNTGETFIELD(c->index) == 32 && c->width == 0 && c->height== 0)
	{
		c->width = 1;
		c->height = 1;
	}

	return(c);
}

static CHARACTER16 *GetCharacter16(FONTREADINFO *info, int index)
{
    CHARACTER16 *c;

	c = (CHARACTER16 *)(((char*)info->chartbl16) + index*CHARACTERSIZE16);

	// if we have found the space character, and its height and width are
	// 0, change them to 1 for compatability with tools. (Note: font exporter
	// changes the space character's width and height to 0)
	 if( FNTGETFIELD(c->index) == 32 && c->width == 0 && c->height== 0)
	{
		c->width = 1;
		c->height = 1;
	}

	return(c);
}


static int GetShapeDepth(SHAPE *shp)
{
    /* The shape type is the first byte of the shape header that */
    /* describes the type of shape.  Font shapes are a subset */
    /* of the available shape types so we can use this function */
    /* instead of the EAC library shapedepth function. */

    /* this function is directly copied from textsubs.c in the EAC library */

    char shpType = shp->type;
    int bpp;

    switch (shpType)
    {
        case 0x44:  /* PSX */
     //   case 0x60:  /* N64 */
            bpp = 1;
            break;


		case 0x60:
			bpp = 4;
			break;

		case 0x01:  /* PS2 */
        case 0x40:  /* PSX */
        case 0x63:  /* N64 */
        case 0x79:  /* PC  */
        case 0x7a:  /* old PC */
        case 0x18:  /* GC  */
        case 0x5c:  /* PSP */
            bpp = 4;
            break;

        case 0x02:  /* PS2 */
        case 0x41:  /* PSX */
        case 0x66:  /* N64 */
        case 0x64:  /* XBox/PC */
        case 0x7b:  /* XBox/PC */
        case 0x19:  /* GC  */
        case 0x5d:  /* PSP */
            bpp = 8;
            break;


        case 0x5a: // MSH_ARGB_4444
		case 0x6d: // FSH_ARGB_4444
			bpp = 16;
            break;

        case 0x5b: // MSH_ARGB_8888
		case 0x7d: // FSH_ARGB_8888
		case 0x05: // SSH_ARGB_8888
			bpp = 32;
            break;

		case 0x15: // GSH_5A3
			bpp = 16;
            break;

        default:
            bpp = 4;    /* most fonts are 4 bit */
            break;
    }
    return(bpp);
}

static ARGB BlendColour(ARGB c1, ARGB c2, float step)
{
    ARGB val;

#define FNTGIMEX_CLAMP(val, min, max)    (unsigned char)((((val) > (max)) ? (max) : ((val) < (min)) ? (min) : (val)))
    val.a = FNTGIMEX_CLAMP(c2.a*step + c1.a*(1.0 - step), 0.0, 255.0);
    val.r = FNTGIMEX_CLAMP(c2.r*step + c1.r*(1.0 - step), 0.0, 255.0);
    val.g = FNTGIMEX_CLAMP(c2.g*step + c1.g*(1.0 - step), 0.0, 255.0);
    val.b = FNTGIMEX_CLAMP(c2.b*step + c1.b*(1.0 - step), 0.0, 255.0);
#undef FNTGIMEX_CLAMP

    return val;
}

static void SetFontPaletteNormal(ARGB Palette[16], ARGB ForegroundArgb, ARGB BackgroundArgb, int numcolours)
{
    int i;
    for (i = 0; i < numcolours; i++)
    {
        Palette[i] = BlendColour(BackgroundArgb, ForegroundArgb, (float)i/(numcolours-1));
    }
}

static void SetFontPaletteShadow(ARGB Palette[16], ARGB ForegroundArgb, ARGB ShadowArgb)
{
    ARGB TransparentForegroundArgb = ForegroundArgb;
    ARGB TransparentShadowArgb     = ShadowArgb;

    TransparentForegroundArgb.a = 0;
    TransparentShadowArgb.a     = 0;

    Palette[0]  = TransparentForegroundArgb;                                      /* 0 transparent */
    Palette[1]  = ShadowArgb;                                           /* 1 ShadowArgb */
    Palette[2]  = BlendColour(ShadowArgb,TransparentForegroundArgb, 1.0f/4.0f);   /* 2..4 ShadowArgb to transparent */
    Palette[3]  = BlendColour(ShadowArgb,TransparentForegroundArgb, 2.0f/4.0f);
    Palette[4]  = BlendColour(ShadowArgb,TransparentForegroundArgb, 3.0f/4.0f);
    Palette[5]  = BlendColour(TransparentForegroundArgb,ForegroundArgb, 1.0f/5.0f);     /* 5..8 transargb to ForegroundArgb */
    Palette[6]  = BlendColour(TransparentForegroundArgb,ForegroundArgb, 2.0f/5.0f);
    Palette[7]  = BlendColour(TransparentForegroundArgb,ForegroundArgb, 3.0f/5.0f);
    Palette[8]  = BlendColour(TransparentForegroundArgb,ForegroundArgb, 4.0f/5.0f);
    Palette[9]  = ForegroundArgb;                                             /* 9 ForegroundArgb */
    Palette[10] = BlendColour(ForegroundArgb,ShadowArgb, 1.0f/5.0f);          /* 10..13 ForegroundArgb to ShadowArgb */
    Palette[11] = BlendColour(ForegroundArgb,ShadowArgb, 2.0f/5.0f);
    Palette[12] = BlendColour(ForegroundArgb,ShadowArgb, 3.0f/5.0f);
    Palette[13] = BlendColour(ForegroundArgb,ShadowArgb, 4.0f/5.0f);
    Palette[14] = TransparentForegroundArgb;                                      /* 14 not used */
    Palette[15] = TransparentForegroundArgb;                                      /* 15 not used */
}

static void SetFontPaletteOutline(ARGB Palette[16], ARGB ForegroundArgb, ARGB BorderArgb)
{
    /* 0..8  full transparent to full border color */
    /* 8..15 full border to full foreground */

    int i;

    ARGB TransparentBlackArgb;
    TransparentBlackArgb.a = 0;
    TransparentBlackArgb.r = 0;
    TransparentBlackArgb.g = 0;
    TransparentBlackArgb.b = 0;

    /* transparent to almost border */
    for (i = 0; i <= 7; i++)
        Palette[i] = BlendColour(TransparentBlackArgb,BorderArgb, i/8.0f);

    /* border to foreground */
    for (i = 0; i <= 7; i++)
        Palette[i+8] = BlendColour(BorderArgb,ForegroundArgb, i/7.0f);
}

#endif

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL FNT_about(void)
{
    GABOUT *info;

    info = (GABOUT*) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));
        info->signature        = SIGNATURE;
        info->size             = sizeof(GABOUT);
        info->version          = GIMEX_VERSION;
        info->canimport        = 1;    /* supports importing */
        info->canexport        = 1;    /* supports exporting */
        info->importpacked     = 1;    /* supports importing packed */
        info->exportpacked     = 1;    /* supports exporting packed */
        info->import8          = 1;    /* supports importing 8 bit indexed */
        info->export8          = 1;    /* supports exporting 8 bit indexed */
        info->import32         = 1;    /* supports importing 32 bit direct rgb */
        info->export32         = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe       = 1;    /* supports multiple frames */
        info->multifile        = 0;    /* format requires additional files or resource fork */
        info->multisize        = 1;    /* supports each frame different size */
        info->framebuffer      = 1;    /* module requires memory to buffer entire frame */
        info->external         = 0;    /* module uses external resources */
        info->usesfile         = 1;    /* module is file based */
#if FNTTYPE==FNTTYPE_XFN
        info->greyscale        = 1;    /* module is grey scale based file based */
#else
        info->greyscale        = 0;    /* module support colour fonts */
#endif
        info->font             = 1;    /* format is a font */
        info->maxcolours       = 0;    /* can have any number of colors */
        info->maxframename     = 0;    /* max frame name character */
        info->defaultquality   = 100;  /* default pack quality */
        info->globalpalette    = 1;

        info->mactype[0]       = MACTYPE0;      /* mac file system types used */
        info->mactype[1]       = MACTYPE1;
        info->mactype[2]       = MACTYPE2;
        info->mactype[3]       = MACTYPE3;
        strcpy(info->extensions[0], EXTENSION0); /* extensions */
        strcpy(info->extensions[1], EXTENSION1);
        strcpy(info->extensions[2], EXTENSION2);
        strcpy(info->extensions[3], EXTENSION3);
        strcpy(info->authorstr,    "Frank Barchard"); /* author */
        sprintf(info->versionstr,   "%.2f",FNT_VERSION/100.0f); /* version # */
        strcpy(info->shorttypestr, SHORTTYPE);  /* type */
        strcpy(info->wordtypestr,  WORDTYPE);   /* wordtype */
        strcpy(info->longtypestr,  LONGTYPE );  /* longtype */
    }
    return(info);
}

/* determine if file is a font file be looking at the type field and */
/* comparing to SIGNATURE and version */

int GCALL FNT_is(GSTREAM *g)
{
    FONTHEADER fntheader;
    int ok = 0;
    GPOS oldpos = gtell(g);

    gseek(g, 0);
    if (gread(g, &fntheader, 8))
    {
        if (ggetm(&fntheader.signature,4)==SIGNATURE)
        {
            gseek(g, 0);
            ok = 99;
            if (gread(g, &fntheader, sizeof(FONTHEADER)))
            {
                if (FNTGETFIELD(fntheader.version)>=FNT_VERSION)
                    ok = 100;
            }
        }
#ifdef OLDSIGNATURE
        else if (ggetm(&fntheader.signature,4)==OLDSIGNATURE)
        {
            gseek(g, 0);
            ok = 99;
            if (gread(g, &fntheader, sizeof(FONTHEADER)))
            {
                if (FNTGETFIELD(fntheader.version)>=FNT_VERSION)
                    ok = 100;
            }
        }

        else if (ggetm(&fntheader.signature,4)==OLDSIGNATURE)
            ok = 90;
#endif
    }
    gseek(g, oldpos);
    return(ok);
}


/****************************************************************/
/*  Canimport Functions                                            */
/****************************************************************/

int GCALL FNT_open(GINSTANCE* *ginstance, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *   gx;
    FONTREADINFO *info;
    FONTHEADER *  fntfile;

    /* handle old file format */
    if (FNT_is(g) == 90)
        return(OLDFNT_open(ginstance, g, pathname,framecountflag));

    /* allocate required buffers (gx,info,fntfile) */
    gx      = (GINSTANCE*)   galloc(sizeof(GINSTANCE));
    info    = (FONTREADINFO*)galloc(sizeof(FONTREADINFO));
    fntfile = (FONTHEADER*)  galloc((int)glen(g));

    /* ensure no errors */
    if (gx && info && fntfile)
    {
        /* read font file */
        gseek(g, 0);
        if (gread(g, fntfile, (int)glen(g)))
        {
            /* fill in read info fields */
            memset(info, 0, sizeof(FONTREADINFO));
            info->header  = fntfile;

			if(FNTGETFIELD(info->header->flags) & 0x40000)
				info->chartbl16 = (CHARACTER16*) ((char*)fntfile + FNTGETFIELD(fntfile->chartbl));
			else
				info->chartbl = (CHARACTER12*) ((char*)fntfile + FNTGETFIELD(fntfile->chartbl));
            const int SIZEOFOLDSHAPE=16;
            info->shape   = (SHAPE*)    ((char*)fntfile + FNTGETFIELD(fntfile->shape)+SIZEOFOLDSHAPE-sizeof(SHAPE));
            info->bpp     = GetShapeDepth(info->shape);

			// assume link is to clut, except on AL_88
            if(FNTGETFIELD(info->shape->link) && info->shape->type!=0x64 && info->shape->type !=0x60)
            {
                int link = FNTGETFIELD(info->shape->link);
                if (link==0x7fffff)
				{
                    if (FNTGETFIELD(info->header->version) >= 400) // Shape2 has full 32 bit mNext field
					{
						link = FNTGETFIELD(info->shape->mNext);
					}
					else
	                    link = ((FNTGETFIELD(info->shape->width) * GetShapeDepth(info->shape) + 7)/8)*FNTGETFIELD(info->shape->height)+16;
				}

				info->clut = (CLUTRHeaderStruct *)((unsigned char *)fntfile + FNTGETFIELD(fntfile->shape) + link +SIZEOFOLDSHAPE-sizeof(SHAPE));
                info->hasclut = 1;
            }
            else
                info->hasclut = 0;

			if (FNTGETFIELD(fntfile->kerntbl))
			{
                KERN *kern;

                info->kernpairs = FNTGETMEM((char*)fntfile + FNTGETFIELD(fntfile->kerntbl),4);
				kern = (KERN *) ((char*)fntfile + FNTGETFIELD(fntfile->kerntbl) + sizeof(int));


				info->kerntbl = (int*) galloc(info->kernpairs*3*sizeof(int));
                if (info->kerntbl)
                {
                    int i;
                    memset(info->kerntbl,0,info->kernpairs*3*sizeof(int));

                    if (FNTGETFIELD(info->header->version) >= 300)
                    {
                        for (i=0; i<info->kernpairs; ++i)
                        {
                    		info->kerntbl[i*3+0] = FNTGETFIELD(kern[i].previouscode);
                    		info->kerntbl[i*3+1] = FNTGETFIELD(kern[i].code);
                    		info->kerntbl[i*3+2] = (int) (signed char) FNTGETFIELD(kern[i].kernvalue);
                        }
                    }
                    else
                    {
                        for (i=0; i<info->kernpairs; ++i)
                        {
                    		info->kerntbl[i*3+0] = FNTGETFIELD(((KERN8 *)kern)[i].previouscode);
                    		info->kerntbl[i*3+1] = FNTGETFIELD(((KERN8 *)kern)[i].code);
                    		info->kerntbl[i*3+2] = (int) (signed char) FNTGETFIELD(((KERN8 *)kern)[i].kernvalue);
                        }
                    }

                }
			}
			else
			{
				info->kerntbl = NULL;
				info->kernpairs = 0;
			}

            /* fill in instance fields */
            memset(gx, 0, sizeof(GINSTANCE));
            gx->signature = SIGNATURE;
            gx->size      = sizeof(GINSTANCE);
            gx->frames    = FNTGETFIELD(info->header->num);
            gx->gstream   = g;
            gx->gref      = (char*)info;

            *ginstance = gx;
            return(gx->frames);
        }
    }

    /* handle errors (no memory or failed read) - notice return statment above */
    if (gx)      gfree(gx);
    if (info)    gfree(info);
    if (fntfile) gfree(fntfile);

    *ginstance = NULL;
    return(0);
}

static int FNT_getkern(int *hot, const FONTREADINFO *info,int framenum)
{
    int kerncount=0;
    int i;
    //int *kerntbl = ((int *) info->kerntbl)+1;
    bool isgoodkerntable=false;

    for (i=0; i<info->kernpairs; ++i)
    {
    	if (info->kerntbl[i*3+1])
        {
            isgoodkerntable = true;
            break;
        }
    }

    if (isgoodkerntable)
    {
        for (i=0; i<info->kernpairs && kerncount<(GIMEX_HOTSPOTTBL_SIZE-3); ++i)
        {
        	if (info->kerntbl[i*3+1]==framenum)
            {
                *hot++ = info->kerntbl[i*3+0]; // PREV
                *hot++ = info->kerntbl[i*3+2]; // kern
        	    ++kerncount;
            }
        }
    }
    return kerncount;
}

#define FONT_ISSHADOW(subtype)     (subtype & 0x2)
#define FONT_ISOUTLINE(subtype)    (subtype & 0x4)

GINFO * GCALL FNT_info(GINSTANCE *gx, int framenum)
{
    FONTREADINFO *info = (FONTREADINFO*)gx->gref;
    GINFO *ginfo=0;
    //int numcolours = 0;
    /* handle old file format */
    if (gx->signature == OLDSIGNATURE)
        return(OLDFNT_info(gx, framenum));

    if (framenum < gx->frames)
        ginfo = (GINFO*)galloc(sizeof(GINFO));

	unsigned int newFormat16 = FNTGETFIELD(info->header->flags)&0x40000;	// reading char16?

    if (ginfo)
    {
        CHARACTER16 *ch16 = 0;
        CHARACTER12 *ch12 = 0;
		unsigned char width;
		unsigned char height;
		signed char xoffset;
		signed char yoffset;
		signed char advance[2];
		advance[0] = 0;
		advance[1] = 0;
		unsigned char indexA[2];
        int advancey;

        if (FNTGETFIELD(info->header->version) >=350)
            advancey = FNTGETFIELD(info->header->states[FONT_HEIGHT]);
        else
            advancey = (FNTGETFIELD(info->header->ascent)) + (FNTGETFIELD(info->header->descent));  // should use height field for new fonts in future


		/* select coresponding character info struct */
		if(newFormat16)
		{
			ch16 = GetCharacter16(info, framenum);
			FNTPUTFIELD(width,FNTGETFIELD(ch16->width));
			FNTPUTFIELD(height,FNTGETFIELD(ch16->height));
			FNTPUTFIELD(xoffset,FNTGETFIELD(ch16->xoffset));
			FNTPUTFIELD(yoffset,FNTGETFIELD(ch16->yoffset));
			FNTPUTFIELD(advance, FNTGETFIELD(ch16->advancex));
			FNTPUTFIELD(indexA, FNTGETFIELD(ch16->index));
		}
		else
		{
			ch12 = GetCharacter12(info, framenum);
			FNTPUTFIELD(width,FNTGETFIELD(ch12->width));
			FNTPUTFIELD(height,FNTGETFIELD(ch12->height));
			FNTPUTFIELD(xoffset,FNTGETFIELD(ch12->xoffset));
			FNTPUTFIELD(yoffset,FNTGETFIELD(ch12->yoffset));
			FNTPUTMEM(advance, FNTGETFIELD(ch12->advance), 1);
			FNTPUTFIELD(indexA, FNTGETFIELD(ch12->index));
		}
        /* fill in ginfo fields */
        memset(ginfo, 0, sizeof(GINFO));

        /* frame description */
        ginfo->signature        = SIGNATURE;
        ginfo->size             = sizeof(GINFO);
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;
        ginfo->width            = width;
        ginfo->height           = height;
        ginfo->subtype          = FNTGETFIELD(info->header->flags);
        ginfo->packed           = FNTGETFIELD(info->shape->type)==0x60 ? 3 : 0;
        ginfo->quality          = 100;
		ginfo->originalbpp		= info->bpp;
		ginfo->bpp              = (info->bpp <= 8) ? 8 : 32;
        ginfo->alphabits        = 8;
        ginfo->redbits          = 8;
        ginfo->greenbits        = 8;
        ginfo->bluebits         = 8;

        /* extra information */
        ginfo->centerx          = 0 - SXB(xoffset);
        ginfo->centery          = 0 - SXB(yoffset);
        ginfo->defaultx         = 0;
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 3;

		if (newFormat16)
			ginfo->hotspottbl[0][0] = (FNTGETFIELD(advance));
        else
			ginfo->hotspottbl[0][0] = (FNTGETMEM(advance,1));
        ginfo->hotspottbl[0][1] = ginfo->centery;

        ginfo->hotspottbl[1][0] = ginfo->centerx;
        ginfo->hotspottbl[1][1] = advancey; // advance to next row
        ginfo->hotspottbl[2][0] = (FNTGETFIELD(info->header->ascent));
        ginfo->hotspottbl[2][1] = (FNTGETFIELD(info->header->descent));

        {
            int countkern = FNT_getkern(&ginfo->hotspottbl[3][0],info,FNTGETFIELD(indexA));
			if(newFormat16)
				FNTPUTFIELD(ch16->numkern,countkern);
			else
				FNTPUTFIELD(ch12->numkern, countkern);
			ginfo->numhotspots  = 3+countkern;
		}

        /* framename "0041 'A' 65" */
        {
            unsigned int index = FNTGETFIELD(indexA);
            if (index>=32 && index<=127)
                sprintf(ginfo->framename, "%04x '%c' %d", index, index,index);
            else
                sprintf(ginfo->framename, "%04x %d", index, index);
        }

		if (info->hasclut)
        {
            int startcolour = 0;
            // IAN add clut expansion here.
            ginfo->numcolours = SHPRclutcolours(info->clut,&startcolour,info->bpp,FNTTYPE);
            FNTRGetClut(ginfo, info->clut,info->bpp,FNTTYPE);
        }
        else // Generate a CLUT because file does not have one.
        {
            if (info->bpp == 1)
            {
                /* 1 bpp default colour table */
                ginfo->colourtbl[0].a = 255;
                ginfo->colourtbl[0].r = 0;
                ginfo->colourtbl[0].g = 0;
                ginfo->colourtbl[0].b = 0;

                ginfo->colourtbl[1].a = 255;
                ginfo->colourtbl[1].r = 255;
                ginfo->colourtbl[1].g = 255;
                ginfo->colourtbl[1].b = 255;

                ginfo->numcolours  = 2;
                ginfo->startcolour = 0;
            }
            else if(info->shape->type == 0x60)	// DXT1 compressed
			{
				memset(ginfo->colourtbl, 0xFF, 4*sizeof(ARGB));
				ginfo->colourtbl[0].a = 0;
				ginfo->colourtbl[1].a = 85;
				ginfo->colourtbl[2].a = 170;
				ginfo->colourtbl[3].a = 255;

				ginfo->numcolours = 4;
				ginfo->startcolour = 0;
			}
			else if (info->bpp == 4 && FONT_ISSHADOW(ginfo->subtype))
            {
                ARGB fore;
                ARGB shadow;
                fore.a = 255;
                fore.r = 255;
                fore.g = 255;
                fore.b = 255;

                shadow.a = 255;
                shadow.r = 0;
                shadow.g = 0;
                shadow.b = 0;

                SetFontPaletteShadow(ginfo->colourtbl, fore, shadow);
                ginfo->numcolours  = 16;
                ginfo->startcolour = 0;
            }
            else if (info->bpp == 4 && FONT_ISOUTLINE(ginfo->subtype))
            {
                /* use a 4bpp outline palette */
                ARGB fore;
                ARGB outline;
                fore.a = 255;
                fore.r = 255;
                fore.g = 255;
                fore.b = 255;

                outline.a = 255;
                outline.r = 0;
                outline.g = 0;
                outline.b = 0;

                SetFontPaletteOutline(ginfo->colourtbl, fore, outline);

                ginfo->numcolours  = 16;
                ginfo->startcolour = 0;
            }
            //else
            else if (info->bpp == 4)
            {
                /* 4 bpp default colour table */
                ARGB fore;
                ARGB back;
                fore.a = 255;
                fore.r = 255;
                fore.g = 255;
                fore.b = 255;

                back.a = 0;
                back.r = 255;
                back.g = 255;
                back.b = 255;

                SetFontPaletteNormal(ginfo->colourtbl, fore, back, 16);

                ginfo->numcolours  = 16;
                ginfo->startcolour = 0;
            }
            //else
            else if (info->bpp == 8)
            {

				/* 8 bpp default colour table */
                ARGB fore;
                ARGB back;
                fore.a = 255;
                fore.r = 255;
                fore.g = 255;
                fore.b = 255;

                back.a = 0;
                back.r = 255;
                back.g = 255;
                back.b = 255;

                SetFontPaletteNormal(ginfo->colourtbl, fore, back, 256);

                ginfo->numcolours  = 256;
                ginfo->startcolour = 0;
			}
        }// end if hasclut.

        // done here because REAL had bug that would output 32 bpp shapes with cluts
        if (info->shape->type==0x64 || info->shape->type==0x60) // Alpha 8 or DXT1
        {
            ginfo->redbits =
            ginfo->greenbits =
            ginfo->bluebits = 0;
        }
        else if (info->shape->type==0x6d || info->shape->type==0x5a) // 4444
        {
            ginfo->numcolours = 0;
            ginfo->alphabits =
            ginfo->redbits   =
            ginfo->greenbits =
            ginfo->bluebits  = 4;
            ginfo->bpp = 32;
        }
        else if (info->shape->type==0x7d || info->shape->type==0x05 || info->shape->type==0x5b) // 8888
        {
            ginfo->numcolours = 0;
            ginfo->alphabits =
            ginfo->redbits   =
            ginfo->greenbits =
            ginfo->bluebits  = 8;
            ginfo->bpp = 32;
        }
        else if (info->shape->type==0x15) // 5a3
        {
            ginfo->numcolours = 0;
            ginfo->alphabits = 3;
            ginfo->redbits   =
            ginfo->greenbits =
            ginfo->bluebits  = 5;
            ginfo->bpp = 32;
        }
    }
    return(ginfo);
}

int GCALL FNT_read(GINSTANCE *gx, GINFO *ginfo, char *dstBase, int dstRowBytes)
{
    FONTREADINFO *info = (FONTREADINFO*)gx->gref;

    /* handle old file format */
    if (gx->signature == OLDSIGNATURE)
        return(OLDFNT_read(gx, ginfo, dstBase, dstRowBytes));

	unsigned int newFormat16 = FNTGETFIELD(info->header->flags)&0x40000;
	CHARACTER12 *ch12;
	CHARACTER16 *ch16;
	unsigned char width;
	unsigned char height;
	unsigned char u[2];
	unsigned char v[2];

    /* copy character from shape into given buffer */
    {

		if(newFormat16)
		{
			ch16 = GetCharacter16(info, ginfo->framenum);
			FNTPUTFIELD(width, FNTGETFIELD(ch16->width));
			FNTPUTFIELD(height, FNTGETFIELD(ch16->height));
			FNTPUTFIELD(u, FNTGETFIELD(ch16->u));
			FNTPUTFIELD(v, FNTGETFIELD(ch16->v));

		}
		else
		{
			ch12 = GetCharacter12(info, ginfo->framenum);
			FNTPUTFIELD(width, FNTGETFIELD(ch12->width));
			FNTPUTFIELD(height, FNTGETFIELD(ch12->height));
			FNTPUTFIELD(u, FNTGETFIELD(ch12->u));
			FNTPUTFIELD(v, FNTGETFIELD(ch12->v));

		}
        unsigned char *srcBase     = (unsigned char *)info->shape + sizeof(SHAPE);
        int srcRowBytes = GetShapeRowBytes(FNTGETFIELD(info->shape->width), info->bpp);
        int srcBpp      = GetShapeDepth(info->shape);

        //int   dstBpp      = 8;
		int   dstBpp      = ginfo->bpp;

#if FNTTYPE == FNTTYPE_GFN && GC_SWIZZLE==1
        // If the swizzle bit is set then unswizzle else do a normal copy
        if(FNTGETFIELD(info->shape->shapex)&0x2000 /*0x4*/ )
        {
            CopyFromGC(srcBase, srcRowBytes, srcBpp,
                     (unsigned char *) dstBase, dstRowBytes, dstBpp,
                     FNTGETFIELD(width), FNTGETFIELD(height),
                     FNTGETFIELD(u),     FNTGETFIELD(v));

        }
        else
        {
            CopyFrom(srcBase, srcRowBytes, srcBpp,
                     (unsigned char *) dstBase, dstRowBytes, dstBpp,
                     FNTGETFIELD(width), FNTGETFIELD(height),
                     FNTGETFIELD(u),     FNTGETFIELD(v),
                     info->shape->type);
        }


#elif FNTTYPE == FNTTYPE_XFN
		// If the swizzle bit is set, then unswizzle, else do a normal copy
		if(FNTGETFIELD(info->shape->shapex)&0x2000)
		{
			XboxUnswizzle8( srcBase, FNTGETFIELD(info->shape->width),
							FNTGETFIELD(info->shape->height),
							(unsigned char *) dstBase, dstRowBytes,
							FNTGETFIELD(width), FNTGETFIELD(height),
							FNTGETFIELD(u), FNTGETFIELD(v));

		}
		else if(info->shape->type == 0x60)
		{
			CopyFromDXT1CompressedFont(srcBase, (unsigned char *) dstBase,
										FNTGETFIELD(info->shape->width),
										FNTGETFIELD(info->shape->height),
										FNTGETFIELD(width), FNTGETFIELD(height),
										FNTGETFIELD(u), FNTGETFIELD(v) );



		}

		else
		{
			CopyFrom(srcBase, srcRowBytes, srcBpp,
                     (unsigned char *) dstBase, dstRowBytes, dstBpp,
                     FNTGETFIELD(width), FNTGETFIELD(height),
                     FNTGETFIELD(u),     FNTGETFIELD(v),
                     info->shape->type);

		}



#else

		if(info->shape->type == 0x60)
		{
			CopyFromDXT1CompressedFont(srcBase, (unsigned char *) dstBase,
										FNTGETFIELD(info->shape->width),
										FNTGETFIELD(info->shape->height),
										FNTGETFIELD(width), FNTGETFIELD(height),
										FNTGETFIELD(u), FNTGETFIELD(v) );
		}

		else
		{
			CopyFrom(srcBase, srcRowBytes, srcBpp,
					 (unsigned char *) dstBase, dstRowBytes, dstBpp,
					 FNTGETFIELD(width), FNTGETFIELD(height),
					 FNTGETFIELD(u),     FNTGETFIELD(v),
					 info->shape->type);
		}
#endif
    }

    return(1);
}

int GCALL FNT_close(GINSTANCE *gx)
{
    FONTREADINFO *info = (FONTREADINFO*)gx->gref;

    /* handle old file format */
    if (gx->signature == OLDSIGNATURE)
        return(OLDFNT_close(gx));

    /* free allocated buffers */

	if (info->kerntbl)
		gfree(info->kerntbl);

    gfree(info->header);
    gfree(info);
    gfree(gx);

    return(1);
}

/****************************************************************/
/* cleanup macros                                               */
/****************************************************************/

#undef FNT_about
#undef FNT_is
#undef FNT_open
#undef FNT_info
#undef FNT_read
#undef FNT_close
#undef FNT_wopen
#undef FNT_write
#undef FNT_wclose

#undef SIGNATURE
#undef OLDSIGNATURE
#undef EXTENSION0
#undef EXTENSION1
#undef EXTENSION2
#undef EXTENSION3
#undef MACTYPE0
#undef MACTYPE1
#undef MACTYPE2
#undef MACTYPE3
#undef SHORTTYPE
#undef WORDTYPE
#undef LONGTYPE

#undef FNTGETFIELD
#undef FNTPUTMEM
#undef FNTGETFIELD
#undef FNTPUTFIELD

#undef SHAPECODE4
#undef SHAPECODE8
#undef SHAPECODE16

#undef FNTTYPE




