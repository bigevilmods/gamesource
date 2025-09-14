// (c) 1994-2007 Electronic Arts Inc.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "gimex.h"
#include "fntgimex.h"
#include "shpcmn.h"
#undef SHPDATAALIGNMENT

#if FNTTYPE == FNTTYPE_GFN
/* GC needs 32 and a header of 16 */
/* for now it is just easier to read in a hex dump when it is on a 16 byte boundary */
#define SHPDATAALIGNMENT    31
#define SHPCLUTALIGN        31
#define SHAPEHEADER         48
#define METALSIZE           (32+64)
#define METALSTR            "EAGL64"
#define FONT_UVOFFSET       0.0f
#elif FNTTYPE == FNTTYPE_XFN
#define SHPDATAALIGNMENT    127
#define SHPCLUTALIGN        63
#define SHAPEHEADER         48
#define METALSIZE           (32+64)
#define METALSTR            "EAGL64"
#define FONT_UVOFFSET       0.0f
#elif FNTTYPE == FNTTYPE_FFN
#define SHPDATAALIGNMENT    127
#define SHPCLUTALIGN        63
#define SHAPEHEADER         48
#define METALSIZE           (32+64)
#define METALSTR            "EAGL64"
#define FONT_UVOFFSET       0.5f
#elif FNTTYPE == FNTTYPE_SFN
#define SHPDATAALIGNMENT    127
#define SHPCLUTALIGN        15
#define SHAPEHEADER         48
#define METALSIZE           (32+240)
#define METALSTR            "EAGL240"
#define FONT_UVOFFSET       0.5f
#else
/* if the shape ever has to align to a specific byte boundary set it here */
/* for now it is just easier to read in a hex dump when it is on a 16 byte boundary */
#define SHPDATAALIGNMENT    127
#define SHPCLUTALIGN        15
#define SHAPEHEADER         48
#define METALSIZE           (32+64)
#define METALSTR            "EAGL64"
#define FONT_UVOFFSET       0.0f
#endif

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

#ifndef __FNTWRITE
#define __FNTWRITE 1

/* specifies the minimum and maximum shape width when using the goodfit algorithm */
#define MINSHAPEWIDTH   32
#if FNTTYPE == FNTTYPE_FFN
#define MAXSHAPEWIDTH   1024
#elif FNTTYPE == FNTTYPE_MFN
#define MAXSHAPEWIDTH   512
#elif FNTTYPE == FNTTYPE_SFN
#define MAXSHAPEWIDTH   1024
#elif FNTTYPE == FNTTYPE_XFN
#define MAXSHAPEWIDTH   4096
#elif FNTTYPE == FNTTYPE_GFN
#define MAXSHAPEWIDTH   1024
#else
/* otherwise set to some arbitrary value that won't break anything */
#define MAXSHAPEWIDTH   256
#endif

#define FNTisalnum(x) ((x>='0' && x<='9') || (x>='a' && x<='z') || (x>='A' && x<='Z'))


//--------------------------------------------------------------------------------------------------------------------------------------------------------------

/*	This function maps the values we read in the font to values that DXT1 associates with
	the right colours.

	In our interpretation: black is 00, 1/3 is 01, 2/3 is 10 and
						   white is 11.

	For DXT :	Colour 00 is White, Colour 01 is Black, Colour 10 is 2/3 white, and
				colour 11 is 1/3 white.
*/
static int DXTmap1(int alpha)
{
	int b=1;
	if (alpha>=43) b=3;
	if (alpha>=128) b=2;
	if (alpha>=213) b=0;
	return b;
}

// This function maps pixel values in a DXT font
static int DXTmapFromDXT(int value)
{

	if (value==0)
		return 1;
	else if (value==1)
		return 3;
	else if (value==2)
		return 2;
	else if (value==3)
		return 0;
	else
		return 0;

}

// FNTWintlog2: returns integer log to the base 2 of a number.
static int FNTWintlog2(unsigned int n)
{
    int msb=0;

    if (n & 0xffff0000)
    {
        msb += 16;
        n &= 0xffff0000;
    }
    if (n & 0xff00ff00)
    {
        msb += 8;
        n &= 0xff00ff00;
    }
    if (n & 0xf0f0f0f0)
    {
        msb += 4;
        n &= 0xf0f0f0f0;
    }
    if (n & 0xcccccccc)
    {
        msb += 2;
        n &= 0xcccccccc;
    }
    if (n & 0xaaaaaaaa)
    {
        msb += 1;
    }

    return(msb);
}


// help with PS2 compiler warnings.
#if FNTTYPE == FNTTYPE_XFN
//--------------------------------------------------------------------------------------------------------
// get the swizzled offset into an image buffer. Used for 8 bit Xbox font swizzling
// returned offset is in bits
static int Xbox_GetSwizzledPixelOffset( int x, int y, int width, int height, int depth )
{
    int xBits     = FNTWintlog2( width  );
    int yBits     = FNTWintlog2( height );
    int offset    = 0;
    int mask      = 0;
    int xExpand   = 0;
    int yExpand   = 0;
    int bitShift  = 0;


    if ( xBits < yBits )
    {
        mask = (width - 1);

        xExpand = ExpandTableXbox[ (x & 0xFF) ];

        if ( x & 0xFF00 )
            xExpand |= (ExpandTableXbox[x >> 8] << 16);

        yExpand = ExpandTableXbox[y & (0xFF & mask)];

        if ( (y & 0xFF00) && (y & mask) )
            yExpand |= (ExpandTableXbox[y >> 8] << 16);

        offset = xExpand | (yExpand << 1);

        offset |= ((y >> xBits) << (xBits * 2));
    }
    else
    {
        mask = (height - 1);

        yExpand = ExpandTableXbox[ (y & 0xFF) ];

        if ( y & 0xFF00 )
            yExpand |= (ExpandTableXbox[y >> 8] << 16);

        xExpand = ExpandTableXbox[x & (0xFF & mask)];

        if ( (x & 0xFF00) && (x & mask) )
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
 //     default:  REAL_ASSERT(0);     break;		// Ali
    }

    // convert offset to bits
    return (offset << bitShift);
}
#endif

//--------------------------------------------------------------------------------------------------------


static void DXT1CompressFont(unsigned char* src, unsigned char** dst, int width, int height, CLUTRHeaderStruct *clutin)
{
	if (!(height%4) && !(width%4) )			// ensure multiples of 4
	{

		ARGB *clut = (ARGB*) clutin->Entries;

		int col, row, i;
		int srcOffset = 0;
		int numblocks = (width/4) * (height/4);


		*dst = (unsigned char *) galloc(width*height*2/8 + numblocks*4);	// #pixels * 2bpp / 8 (pixel data) + (4 bytes for 2 colours per block)
		unsigned char * dstStart = *dst;

/*
	Code below is DXT1 compression code. We break the image into 4x4 blocks. In each
	4x4 block we write the colours white as 16 bits, and black as 16 bits, followed
	by 32 bits (16 pixels * 2bpp) of pixel references to the colours. Two intermediate
	colours are interpolated from white and black.
*/

		for (row = 0 ; row < height; row+=4)
		{
			for (col = 0; col < width ; col+=4)
			{

				srcOffset = row*width + col;

				FNTPUTMEM(*dst,0xFFFF, 2);		// colour 0 : White
				*dst+=2;
				FNTPUTMEM(*dst,0x0000, 2);		// colour 1 : Black
				*dst+=2;

				for (i=0; i<4; i++)
				{
					**dst = (DXTmap1(clut[src[srcOffset+0]].a)<<0)
						  | (DXTmap1(clut[src[srcOffset+1]].a)<<2)
						  | (DXTmap1(clut[src[srcOffset+2]].a)<<4)
						  | (DXTmap1(clut[src[srcOffset+3]].a)<<6);

					(*dst)++;
					srcOffset += width;
				}
			}
		}
		*dst = dstStart;
	}
}

static void DXTCompressFontFromDxt(unsigned char* src,unsigned char** dst,int width,int height)
{

	if (!(height%4) && !(width%4) )
	{

		int col, row, i;
		int srcOffset = 0;
		int numblocks = (width/4) * (height/4);


		*dst = (unsigned char *) galloc(width*height*2/8 + numblocks*4);	// #pixels * 2bpp / 8 (pixel data) + (4 bytes for 2 colours per block)
		unsigned char * dstStart = *dst;

		for (row = 0 ; row < height; row+=4)
		{
			for (col = 0; col < width ; col+=4)
			{

				srcOffset = (row*width + col)*4/8;

				FNTPUTMEM(*dst,0xFFFF, 2);		// colour 0 : White
				*dst+=2;
				FNTPUTMEM(*dst,0x0000, 2);		// colour 1 : Black
				*dst+=2;


				for (i=0; i<4; i++)
				{
					**dst =		 ( DXTmapFromDXT( ((src[srcOffset])&15) ) << 0 )
						    |	((DXTmapFromDXT(   (((src[srcOffset])&240)>>4) ) ) << 2 )
							|	( DXTmapFromDXT( ((src[srcOffset+1])&15) ) << 4 )
							|	((DXTmapFromDXT(  (((src[srcOffset+1])&240)>>4) ) ) << 6 );



					(*dst)++;
					srcOffset += width*4/8;


				}


			}

		}

		*dst = dstStart;

	}



}

#if 0
// handle all swizzling actions for all formats. returns a pointer to the swizzled data on
// success NULL on failure.
static char * FNTSwizzleAll(unsigned char * MemPtr,int Width, int Height,int Fontbpp,int FontType,int type,int FontRowBytes)
{

    int x,y;
    int bypp         = 1;
    int FontOffset   = 0;
    int MemOffset    = 0;
    int MemRowbase   = 0;
    int FontRowbase  = 0;
    char * FontPtr   = (char *)galloc(FontRowBytes*Height);         // create a temp buffer to swizzle into.

    if (FontPtr)
	{
        // clear the buffer for ffn 4bpp
        memset(FontPtr,0,FontRowBytes*Height);

        // special case stuff for SFN  Functions Swizzle 8 or 4 and then unswizzle 32 for speed increase.
        if (type == FNTTYPE_SFN)
		{

            if (Fontbpp == 8)
                SHAPE_swizzle8((unsigned char *)MemPtr,(unsigned char *)FontPtr,Width,Height);

            else if (Fontbpp == 4)
                SHAPE_swizzle4((unsigned char *)MemPtr,(unsigned char *)FontPtr,Width,Height);

            else if (Fontbpp == 32)
                memcpy(FontPtr,MemPtr,FontRowBytes*Height);

            else if (Fontbpp == 16 || Fontbpp == 24)   // not supported
			{
                if (FontPtr) gfree(FontPtr);
				return (NULL);
			}

            return(FontPtr);
		}

		// other formats.
        if (type == FNTTYPE_GFN || type == FNTTYPE_FFN || type == FNTTYPE_XFN)
		{
			// have to set the byte per pixel amount. Most of the 4bpp formats can
			// be treated the same as the 8bpp format in the unswizzle, only
			// fsh and psh need special care.
            if (Fontbpp <= 8)       bypp = 1;
            else if (Fontbpp <= 16) bypp = 2;
            else if (Fontbpp <= 24) bypp = 3;
            else if (Fontbpp <= 32) bypp = 4;

			for (y=0;y<Height;++y)
			{
				MemOffset=MemRowbase;
                FontOffset=FontRowbase;

				for (x=0; x<Width; ++x)
				{
                    if (type==FNTTYPE_GFN)
					{
                        FontOffset = SHPWswizzleGC(x,y,Width,Fontbpp);// this swizzles the dest not the src like PS2 swizzle
					}
                    else if (type==FNTTYPE_FFN || type==FNTTYPE_XFN)
					{
						int PixOffset;
						int tx,ty;

						PixOffset = SHPWswizzleXSH(x,y,Width,Height);

						ty = PixOffset/Width;
						tx = PixOffset%Width;

                        // kludge for 4 bit indexing
                        if (FontType == 0x40 || FontType == 0x79)
                            MemOffset = ty*FontRowBytes+tx/2;
						else
                            MemOffset = ty*FontRowBytes+tx*bypp;            // do a regular pixel lookup
					}


					// GSH 32 bpp and
		            // GSH uncompressed DXT have a funny swizzle type for greater cache locality.
                    if (type == FNTTYPE_GFN && (FontType == 0x16 || FontType == 0x1e))
					{
                        (*(FontPtr+FontOffset+0))  = *(MemPtr+MemOffset+0);
                        (*(FontPtr+FontOffset+1))  = *(MemPtr+MemOffset+1);
                        (*(FontPtr+FontOffset+32)) = *(MemPtr+MemOffset+2);
                        (*(FontPtr+FontOffset+33)) = *(MemPtr+MemOffset+3);
					}
#if 0
                    else if (FontType == 0x40 || FontType == 0x79)                // fsh's (and psh) have to have each nibble extracted
					{                                                              // and reinserted. Rather ugly but it's the way it
						Value=MemPtr[MemOffset];                                   // has to be done.
						if (x&1)
                            FontPtr[FontOffset] |= (char)(Value&0xF0);
						else
                            FontPtr[FontOffset] |= (char)(Value&0x0F);
					}
#endif

					else                                                           // all other formats can just use a memcpy.
                        memcpy(FontPtr+FontOffset,MemPtr+MemOffset,bypp);

                    if (Fontbpp != 4 || (Fontbpp == 4 && x&1))                    // only incerment every second pass for the bpp types otherwise always increment.
					{
					    MemOffset     += bypp;
                        FontOffset    += bypp;
					}

				}
                FontRowbase += FontRowBytes;
                MemRowbase  += FontRowBytes;

			}
        }// end if type == GFN,XFN,FFN
        return(FontPtr);
    }// end if FontPtr
	return(NULL);
}
#endif

static bool IsAlphaClut(ARGB* clut, int numcol)
{
	bool isalpha = true;

	for (int i=0; i<numcol; ++i)
	{
		if (clut[i].a && (clut[i].r != (unsigned char) 255
 					  ||  clut[i].g != (unsigned char) 255
					  ||  clut[i].b != (unsigned char) 255))
		{
			isalpha = false;
			break;
		}

	}

	return isalpha;

}

//--------------------------------------------------------------------------
// create my clut no matter what the size. Assume it's a normal type
// clut.
static void GenerateNormalClut(ARGB *Palette, int numcolours)
{
    int i;

    for (i=0; i<numcolours; ++i)
    {
        Palette[i].a = (unsigned char) ((i*255)/(numcolours-1));
        Palette[i].r = (unsigned char) 0xff;
        Palette[i].g = (unsigned char) 0xff;
        Palette[i].b = (unsigned char) 0xff;
    }
}

//--------------------------------------------------------------------------
// encode the clut in the correct format and stuff the buffer with the clut.
static void FNTWSetShapeClut(int originalbpp,int alphabits,int redbits,int greenbits,int bluebits,int subtype,int NbColours,const ARGB *colourtbl,CLUTRHeaderStruct **theClut,int *SizeClut,int type)
{
    int  i;
    int  j;
    int  csm1=0;
    int  clutx;
    int  cluty;
    int  cluttype=0;
    char *pal;
    int SizeData=0;
    CLUTRHeaderStruct *DataClut=0L;\

    if (NbColours>0)
    {
        SizeData = NbColours;
        switch (type)
        {
            case FNTTYPE_PFN:     /* PSH */
                cluttype = 0x23;
                SizeData *= 2;      /* 16-bit palette */
                break;

            case FNTTYPE_GFN:     /* GSH */
            {
                if (IsAlphaClut((ARGB*)colourtbl, NbColours))
				{
					cluttype = 0x30;
				}
				else
				{
					cluttype=gshcluttype(alphabits,redbits,greenbits,bluebits);
                }
				SizeData *= 2;  /* 16 bit palette */
				break;
            }

            case FNTTYPE_MFN:     /* MSH */
            {
                cluttype=mshcluttype(alphabits,redbits,greenbits,bluebits);
                if (cluttype==0x3b)
                    SizeData *= 4;  /* 32 bit palette */
                else /* type 38,39 & 3a */
                    SizeData *= 2;  /* 16 bit palette */
                break;
            }

            case FNTTYPE_SFN:     /* SSH */
            {
                cluttype=sshcluttype(alphabits,redbits,greenbits,bluebits);
                if (originalbpp>4)
                {
                    if (!(subtype&SUBCSM2))
                        csm1 = 1;
                    SizeData = SHPWcsmsize(SizeData);
                }
                if (cluttype==0x20)
                    SizeData *= 2;  /* 16 bit palette */
                else /* type 21 */
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case FNTTYPE_XFN:     /* XSH */
            {
                cluttype=xshcluttype(alphabits,redbits,greenbits,bluebits);
                if (cluttype==0x29 || cluttype==0x2d)
                    SizeData *= 2;  /* 16 bit palette */
                else
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case FNTTYPE_FFN:     /* FSH */
            {
                cluttype=fshcluttype(alphabits,redbits,greenbits,bluebits);

                if (cluttype==0x29 || cluttype==0x2d)
                    SizeData *= 2;  /* 16 bit palette */
                else if (cluttype==0x22 || cluttype==0x24)
                    SizeData *= 3;  /* 24 bit palette */
                else
                    SizeData *= 4;  /* 32 bit palette */
                break;
            }

            case FNTTYPE_NFN:     /* NSH */
            {
                cluttype = 0x27;

                /* Intensity or intensity/alpha shapes have
                   no CLUT. If all colours in the palette are
                   grey scale then assume this is the case. */

                if (NSHWIsGreyPalette(colourtbl, NbColours, subtype&SUBNOPAL))
                {
                    SizeData = 0;
                }
                else
                {
                        /* Otherwise use 16-bit RGBA palette. */
                    SizeData *= 2;
                }
                break;
            }
        }

        if (SizeData)
        {
            SizeData = sizeof(SHAPE)+SizeData;
            DataClut = (CLUTRHeaderStruct*) galloc(SizeData);
        }

        if (DataClut)
        {
            clutx = 0;
            cluty = 0;
            if (type==FNTTYPE_PFN)
                cluty = 240;

            memset(DataClut,0,SizeData);

            FNTPUTFIELD(DataClut->type,     cluttype);
            FNTPUTFIELD(DataClut->width,    NbColours);
            FNTPUTFIELD(DataClut->height,   1);
            FNTPUTFIELD(DataClut->centerx,  NbColours);
            FNTPUTFIELD(DataClut->centery,  0);
            FNTPUTFIELD(DataClut->shapex,   clutx | (csm1?0x2000:0)); // swizzle bit for csm1 cluts
            FNTPUTFIELD(DataClut->shapey,   cluty);

            FNTPUTFIELD(DataClut->mtype,     cluttype);
            FNTPUTFIELD(DataClut->mFlags,    FONT_TEXTURE_FLAG_NEW | (csm1?FONT_TEXTURE_FLAG_SWIZZLED:0)); // swizzle bit for csm1 cluts);
            FNTPUTFIELD(DataClut->mwidth,    NbColours);
            FNTPUTFIELD(DataClut->mheight,   1);
            FNTPUTFIELD(DataClut->mshapex,   clutx);
            FNTPUTFIELD(DataClut->mshapey,   cluty);
            FNTPUTFIELD(DataClut->mDataOffset, sizeof(SHAPE));
            FNTPUTFIELD(DataClut->mDataSize,  SizeData-sizeof(SHAPE));


            pal = (char *) &DataClut->Entries[0];

            for (j=0; j<NbColours; ++j)
            {
                i = (csm1 && (j&31)>=8 && (j&31)<=23) ? j ^ 0x18 : j;
                switch (type)
                {
                    case FNTTYPE_PFN: /* PSH */
                        FNTPUTMEM(pal+i*2,PSHWpixel8To5(&colourtbl[j]),2);
                        break;

                    case FNTTYPE_FFN: /* FSH */
                    case FNTTYPE_XFN: /* XSH */
                    case FNTTYPE_GFN: /* GSH */
                    case FNTTYPE_SFN: /* SSH */
                    case FNTTYPE_MFN: /* MSH */
                        switch (cluttype)
                        {
                            case 0x20: /* SSH 15 bit palette (1555) */
                            case 0x39: /* MSH 15 bit palette (1555) */
                                FNTPUTMEM(pal+i*2,SSHWpixel8To5(&colourtbl[j]),2);
                                break;

                            case 0x32: /* GSH 15 bpp */
                                FNTPUTMEM(pal+i*2,GSHWpixel8To5(&colourtbl[j]),2);
                                break;

                            case 0x2d: /* FSH 15 bit palette (1555) */
                                FNTPUTMEM(pal+i*2,FSHWpixel8To5(&colourtbl[j]),2);
                                break;

                            case 0x31: /* GSH 565 */
                            case 0x29: /* FSH 16 bit palette (565) */
                                FNTPUTMEM(pal+i*2,FSHWpixel8To565(&colourtbl[j]),2);
                                break;

                            case 0x38: /* MSH 16 bit palette (565) */
                                FNTPUTMEM(pal+i*2,MSHWpixel8To565(&colourtbl[j]),2);
                                break;

                            case 0x30: /* GSH IA8 */
                                gputm(pal+i*2,(int)GSHWpixel8ToIA(&colourtbl[j]),2);
                                break;

                            case 0x3a: /* MSH 16 bit palette (4444) */
                                FNTPUTMEM(pal+i*2,MSHWpixel8To4444(&colourtbl[j]),2);
                                break;

                            case 0x22: /* FSH 24 bit palette (666) */
                                pal[i*3+0] = colourtbl[j].r>>2;
                                pal[i*3+1] = colourtbl[j].g>>2;
                                pal[i*3+2] = colourtbl[j].b>>2;
                                break;

                            case 0x24: /* FSH 24 bit palette (888) */
                                pal[i*3+0] = colourtbl[j].r;
                                pal[i*3+1] = colourtbl[j].g;
                                pal[i*3+2] = colourtbl[j].b;
                                break;

                            case 0x2e: /* FSH 32 bit palette (8555) */
                                FNTPUTMEM(pal+i*4,FSHWpixel8To5(&colourtbl[j]),2);
                                pal[i*4+2] = 0;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x2c: /* FSH 32 bit palette (8565) */
                                FNTPUTMEM(pal+i*4,FSHWpixel8To565(&colourtbl[j]),2);
                                pal[i*4+2] = 0;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x2a: /* FSH 32 bit palette (8888) */
                                pal[i*4+0] = colourtbl[j].b;
                                pal[i*4+1] = colourtbl[j].g;
                                pal[i*4+2] = colourtbl[j].r;
                                pal[i*4+3] = colourtbl[j].a;
                                break;

                            case 0x21: /* SSH 8888 */
                            case 0x3b: /* MSH 32 bit palette (8888) */
                                pal[i*4+0] = colourtbl[j].r;
                                pal[i*4+1] = colourtbl[j].g;
                                pal[i*4+2] = colourtbl[j].b;
                                pal[i*4+3] = colourtbl[j].a;
                                break;
                        }
                        break;

                    case FNTTYPE_NFN: /* NSH */
                        FNTPUTMEM(pal+i*2,NSHWpixel8To5(&colourtbl[j]),2);
                        break;
                }
            }
        }
    }
    *SizeClut=SizeData;
    *theClut=DataClut;
}


//--------------------------------------------------------------------------
// If I don't have a clut, save one in the file. Also if it's a GSH save
// the clut as IA8 clut type
static void FNTWCreateAutoClut(int originalbpp,int subtype,CLUTRHeaderStruct ** theClut,int * SizeClut,int type)
{
    ARGB TempClut[256];
    int  numcols = 16;
    if (originalbpp <= 4)
        numcols = 16;
    else
        numcols = 256;
    GenerateNormalClut(TempClut,numcols);
    if (type == FNTTYPE_GFN)   // set clut type to IA88 if it's
                              // a GSH font with not clut.
        FNTWSetShapeClut(originalbpp,8,8,0,0,subtype,numcols,TempClut,theClut,SizeClut,type);
    else
        FNTWSetShapeClut(originalbpp,8,8,8,8,subtype,numcols,TempClut,theClut,SizeClut,type);
}

// to get rid of compiler warnings on PS2
#if FNTTYPE == FNTTYPE_XFN
// figure out if the clut is an alpha ramp
static bool IsAlphaClut(const GINFO * ginfo)
{
    int i = 0;

    bool isalpha = true;
    int numcolours = ginfo->numcolours;
    if (!numcolours) numcolours = 1<<ginfo->originalbpp;

    // are all the colours white?
    for (i=0; i<numcolours; ++i)
    {
        if (ginfo->colourtbl[i].a &&
         (
            ginfo->colourtbl[i].r != (unsigned char) 255
         || ginfo->colourtbl[i].g != (unsigned char) 255
         || ginfo->colourtbl[i].b != (unsigned char) 255))
        {
            isalpha = false;
            break;
        }
    }

    return(isalpha);
}
#endif

static bool IsAlphaClut(CLUTRHeaderStruct *clutin, int sizeclut)
{
	ARGB *clut = (ARGB*) clutin->Entries;
	int i;
	int numcolours = (sizeclut - sizeof(SHAPE))/sizeof(ARGB);

	bool isalpha = true;

	for (i=0; i<numcolours && isalpha==true; i++)
	{
		if (clut[i].a && (clut[i].r != (unsigned char) 255
					 ||  clut[i].g != (unsigned char) 255
					 ||  clut[i].b != (unsigned char) 255))
		{
			isalpha = false;
		}

	}

	return isalpha;

}




/* GX used this function for consistant font naming */
/* normal framename "0041 'A' 65" */

static unsigned int FNTFramenameToIndex(const char* framename, int framenum)
{
    unsigned int index;

    if (!strncmp(framename,"chr",3))                    /* chrA */
        index = (unsigned char)framename[3];
    else if (framename[0]=='\'' && framename[2]=='\'')  /* 'A' */
        index = (unsigned char)framename[1];
    else if (framename[0]=='$')                         /* $0040 */
        sscanf(framename+1, "%x", &index);
    else if (framename[0]=='0' && framename[1]=='x')    /* 0x0040 */
        sscanf(framename+2, "%x", &index);
    else if (FNTisalnum(framename[0])                   /* 0000 */
        && FNTisalnum(framename[1])
        && FNTisalnum(framename[2])
        && FNTisalnum(framename[3])
        && (framename[4]<=' '))
    {
        sscanf(framename, "%x", &index);
    }
    else if (framename[0])                              /* 65 */
        sscanf(framename, "%d", &index);
    else
        index = 32 + framenum;   /* default */

    return(index);
}

#undef FNTisalnum



#if FNTTYPE == FNTTYPE_GFN


#define ALINGNPOW2(currentx,pow2align) (((currentx) + ((pow2align)-1)) & (~((pow2align)-1)))

static void AlignToCell(int *currentx, int *currenty, int bpp)
{
    switch(bpp)
    {
        case 4:// size need to be div by 8x8 for 4 bit else can not swizzle or 4 bytes by 8
        {
			currentx[0]=ALINGNPOW2(currentx[0],8);
			currenty[0]=ALINGNPOW2(currenty[0],8);
            break;
        }
        case 8:// size need to be div by 8x4 for 4 bit else can not swizzle or 8 bytes by 4
        {
			currentx[0]=ALINGNPOW2(currentx[0],8);
			currenty[0]=ALINGNPOW2(currenty[0],4);
            break;
        }
        case 15:
        case 16:
        {
            // size  need to be div by 4x4 for all others else can not swizzle or 8 bytes by 4
			currentx[0]=ALINGNPOW2(currentx[0],4);
			currenty[0]=ALINGNPOW2(currenty[0],4);
            break;
        }
        case 32:
        {
            // size  need to be div by 4x4 for all others else can not swizzle or 16 bytes by 4
			currentx[0]=ALINGNPOW2(currentx[0],4);
			currenty[0]=ALINGNPOW2(currenty[0],4);
            break;
        }
    }
}
#endif

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

static void CopyToGC(unsigned char * srcBase, int srcRowBytes, int srcBpp,
                   unsigned char * dstBase, int destRowBytes, int dstBpp,
                   int w, int h, int u, int v)
{
    int x;
    int y;
    int Value;
    int dstoffset;

    int WidthInPix;

    switch(dstBpp)
    {
        case 32:
            WidthInPix = destRowBytes/4;
            break;
		case 16:
            WidthInPix = destRowBytes/2;
            break;
		case 8:
            WidthInPix = destRowBytes;
            break;
        case 4:
            WidthInPix = destRowBytes<<1;
            break;
    }

    for (y = 0; y < h; y++)
    {
        unsigned char * src = srcBase + (0+y)*srcRowBytes;

        for (x = 0; x < w; x++)
        {
            dstoffset = getpixeladr(u+x,v+y,WidthInPix,dstBpp);
            //memset(&dstBase[dstoffset],0,destRowBytes);

            if (dstBpp == 4)
            {
                Value = src[0]<<4;
                src++;
                if (++x<w) Value |= (src[0]&15);
                src++;

                dstBase[dstoffset] = (char) (unsigned char) Value;
            }
            else if (dstBpp == 8)
            {
                dstBase[dstoffset] = src[0];
                src++;
            }
            else if (dstBpp == 16) // 5A3
			{
				FNTPUTMEM(dstBase+dstoffset,GSHWpixel8To5((ARGB *)src),2);
				src += 4;
			}
#if 0
			else if (dstBpp == 32)
			{
				FNTPUTMEM(dstBase+dstoffset,GSHWpixel8To5((ARGB *)src),2);
				src += 4;
			}
#endif
		}
    }
}

#else

static void CopyTo(unsigned char * srcBase, int srcRowBytes, int srcBpp,
                   unsigned char * dstBase, int dstRowBytes, int dstBpp,
                   int w, int h, int u, int v,
				   ARGB *clut,
				   int type) //isnotAutoGenClut)
{
    int x;

    for (int y = 0; y < h; ++y)
    {
        unsigned char * src = srcBase + (0+y)*srcRowBytes;
        unsigned char * dst = dstBase + (v+y)*dstRowBytes;


        if (dstBpp==32)
        {
			if (type==0x05 || type==0x5b) // ssh or msh
			{
				for (x=0; x<w; ++x)
				{
					unsigned char *p = dst+(u+x)*4;
		            *(p+0) = ((ARGB *)src)[x].r;
					*(p+1) = ((ARGB *)src)[x].g;
		            *(p+2) = ((ARGB *)src)[x].b;
			        *(p+3) = ((ARGB *)src)[x].a;
				}
			}
			else // fsh/xsh
			{
				for (x=0; x<w; ++x)
				{
					unsigned char *p = dst+(u+x)*4;
		            *(p+0) = ((ARGB *)src)[x].b;
					*(p+1) = ((ARGB *)src)[x].g;
		            *(p+2) = ((ARGB *)src)[x].r;
			        *(p+3) = ((ARGB *)src)[x].a;
				}
			}
        }
        else if (dstBpp==16) // 4444
        {
			if (type==0x5a) // msh
			{
				for (x=0; x<w; ++x)
				{
					((unsigned short *)dst)[(u+x)] = (((((ARGB *)src)[x]).a>>4)<<12) | (((((ARGB *)src)[x]).b>>4)<<8) | (((((ARGB *)src)[x]).g>>4)<<4) | (((((ARGB *)src)[x]).r>>4));
				}
			}
			else
			{
				for (x=0; x<w; ++x)
				{
					((unsigned short *)dst)[(u+x)] = (((((ARGB *)src)[x]).a>>4)<<12) | (((((ARGB *)src)[x]).r>>4)<<8) | (((((ARGB *)src)[x]).g>>4)<<4) | (((((ARGB *)src)[x]).b>>4));
				}
			}
        }
        else if (dstBpp==8)
        {
			if ((type==0x64) && clut) // map to Alpha8 via clut alpha
			{
				for (x=0; x<w; ++x)
				{
					dst[(u+x)] = clut[src[x]].a;
				}
			}
			else
			{
				for (x=0; x<w; ++x)
				{
					dst[(u+x)] = src[x];
				}
			}
        }
        else if (dstBpp==4)
        {
            for (x=0; x<w; ++x)
            {
                int Value = 0;

                Value=src[x]&15;

                // lo/hi
                if (type==0x01  /* SSH 4 bit */
                 || type==0x40  /* PSH 4 bit */
                 || type==0x5c  /* MSH 4 bit */
                 || type==0x79) /* FSH 4 bit */
                {
                    if ((u+x)&1)
                        dst[(u+x)>>1] = (char)((dst[(u+x)>>1]&0x0f) | (Value<<4));
                    else
                        dst[(u+x)>>1] = (char)((dst[(u+x)>>1]&0xf0) | (Value&0x0f));
                }
                else        /* GSH/NSH 4 bit */
                {
                    if ((u+x)&1)
                        dst[(u+x)>>1] = (char)((dst[(u+x)>>1]&0xf0) | (Value&0x0f));
                    else
                        dst[(u+x)>>1] = (char)((dst[(u+x)>>1]&0x0f) | (Value<<4));
                }
            }
        }
    }
}
#endif

static int WGetShapeRowBytes(int width, int bpp)
{
    /* align rows to 32 bit and minimum of 8 bytes */

    return  gmax((width*bpp + 31)/32 * 4, 8);
}

static CHARACTER12 *WGetCharacter12(FONTWRITEINFO* info, int index)
{
    return (CHARACTER12 *) (((char*)info->chartbl) + index*CHARACTERSIZE12);
}

static CHARACTER16 *WGetCharacter16(FONTWRITEINFO* info, int index)
{
	return (CHARACTER16 *) (((char*)info->chartbl16) + index*CHARACTERSIZE16);
}

static const int MAXINDEXVAL=65536;

static int reindex(int /*index*/, int height)
{
    return height;
#if 0
    const char sortstr[]="BUYERTS";
    int strindex=0;
    if ((index>='A' && index<='Z') || (index>='0' && index<='9'))
    {
        const char *indexptr = strchr(sortstr,index);
        if (indexptr)
            strindex = indexptr-sortstr+1;
        else if (index>='A')
            strindex = 16+index-'A';
        else // 0..9
            strindex = 16+index-'0'+'Z'-'A'+1;
    }
    if (!strindex)
        strindex = height+64;
    return strindex;
#endif
}

#define fntabs(x) (x>0?x:-x)
static int CompareHeight(const void* elem1, const void* elem2)
{
    CHARACTER12* c1 = *((CHARACTER12**)elem1);
    CHARACTER12* c2 = *((CHARACTER12**)elem2);

    int result = reindex(FNTGETFIELD(c1->index),FNTGETFIELD(c1->height))
               - reindex(FNTGETFIELD(c2->index),FNTGETFIELD(c2->height));

    return result;
}

static int CompareHeight16(const void* elem1, const void* elem2)
{
	CHARACTER16* c1 = *((CHARACTER16**)elem1);
    CHARACTER16* c2 = *((CHARACTER16**)elem2);

    int result = reindex(FNTGETFIELD(c1->index),FNTGETFIELD(c1->height))
               - reindex(FNTGETFIELD(c2->index),FNTGETFIELD(c2->height));

    return result;
}

static int CompareIndex(const void* elem1, const void* elem2)
{
    CHARACTER12* c1 = (CHARACTER12*)elem1;
    CHARACTER12* c2 = (CHARACTER12*)elem2;

    int result = FNTGETFIELD(c1->index) - FNTGETFIELD(c2->index);

    return result;
}

static int CompareIndex16(const void* elem1, const void* elem2)
{
    CHARACTER16* c1 = (CHARACTER16*)elem1;
    CHARACTER16* c2 = (CHARACTER16*)elem2;

    int result = FNTGETFIELD(c1->index) - FNTGETFIELD(c2->index);

    return result;
}

static int CompareKernCode(const void* elem1, const void* elem2)
{

	KERN8* k1 = (KERN8 *) elem1;
	KERN8* k2 = (KERN8 *) elem2;

	int result = FNTGETFIELD(k1->code) - FNTGETFIELD(k2->code);
	return result;


}

static int byte_align(int currentx, int bpp)
{
    if (bpp == 4)
    {
        currentx = (currentx + 1)/2 * 2;
    }
    else if (bpp == 1)
    {
        currentx = (currentx + 7)/8 * 8;
    }
	else /* bpp >= 8 */
	{
		currentx = currentx;
	}
    return currentx;
}

static unsigned int FLOATASINT(float f)
{
    return *(unsigned int *) &f;
}

static void UpdateUVFields(FONTWRITEINFO* info, int* shapeWidth, int* shapeHeight)
{
    /* The UV field in each character indicates where it will be */
    /* copied into the shape.  This has a drastic influence on */
    /* the resulting size of the shape. */

    /* This method tries to build a square.  Characters are sorted by */
    /* height to minimize wasted space by disimilar height characters. */

    CHARACTER12** sortedCharacterList12 = NULL;        /* contains a list of the pointers in the info->chartbl list */
	CHARACTER16** sortedCharacterList16 = NULL;
	int charactersize = info->characterinfosize;

    int i;
    int frames = FNTGETFIELD(info->header->num);

    int maxWidth  = 0; /* maximum width of the shape */
    bool goodfit = false; /* indicates if the resulting cram is a good fit or not */

    /* Number of pixels to add around characters (for propert uv extraction with bilinear filtering) */
    int extraspace = 0;

    if (FNTGETFIELD(info->header->flags) & 0x08)
    {
        extraspace = 1;
    }

#if FNTTYPE == FNTTYPE_PFN || FNTTYPE == FNTTYPE_XFN || FNTTYPE == FNTTYPE_FFN || FNTTYPE == FNTTYPE_MFN
	/* PSX and PC fonts always need this value to be at least 1. */
	if (extraspace <= 0)
	{
		extraspace = 1;
	}
#endif

    /* sort characters by height - this keeps characters of the same height on the same row, which helps get a better fit */
    if (charactersize==16)
	{
		sortedCharacterList16 = (CHARACTER16**)(galloc(frames * sizeof(CHARACTER16*)));
		for (i = 0; i < frames; i++)
		{
			CHARACTER16* ch = (CHARACTER16*)((char*)(info->chartbl16) + i*CHARACTERSIZE16);
			sortedCharacterList16[i] = ch;
		}
		qsort(sortedCharacterList16, frames, sizeof(CHARACTER16*), CompareHeight16);
	}
	else
	{
		sortedCharacterList12 = (CHARACTER12**)(galloc(frames * sizeof(CHARACTER12*)));
		for (i = 0; i < frames; i++)
		{
			CHARACTER12* ch = (CHARACTER12*)((char*)(info->chartbl) + i*CHARACTERSIZE12);
			sortedCharacterList12[i] = ch;
		}
		qsort(sortedCharacterList12, frames, sizeof(CHARACTER12*), CompareHeight);
	}


    /* Calculate maximum shape width by dividing the width of all the characters */
    /* by the approiximate number of rows.  Rows are calculated by taking the */
    /* square root of the number of characters in to get a rectangle/square. */
    /* ie: 64 characters = 8 rows of 8 characters. */
    {
        int rows = (int)(sqrt((double)(frames)));
        int totalWidth = 0;

        for (i = 0; i < frames; i++)
		{
            if (charactersize==16)
				totalWidth += FNTGETFIELD(sortedCharacterList16[i]->width) + extraspace;
			else
				totalWidth += FNTGETFIELD(sortedCharacterList12[i]->width) + extraspace;

		}
        maxWidth = totalWidth/rows;
        maxWidth = (1<<(FNTWintlog2(maxWidth-1)+1)); // bump out to next power of 2
    }

#if FNTTYPE == FNTTYPE_PFN
    /* force to fit in one texture page width */
    maxWidth = 255;

#elif (FNTTYPE == FNTTYPE_FFN) || (FNTTYPE == FNTTYPE_XFN) || (FNTTYPE == FNTTYPE_SFN) || (FNTTYPE == FNTTYPE_MFN)
    /* using the goodfit variable we will set maxWidth to a power of 2. */
    /* the good fit algorithm will keep iterating until a maxWidth is 256 or */
    /* maxWidth is a power of 2 AND the height of the shape is less than the width */
    /* this in effect gives us a square shape but with out the extra padding at the end of */
    /* of the shape.  Copying the shape data into a square power of 2 texture is trivial */
    maxWidth = MINSHAPEWIDTH;
#endif

	int maxShapeWidth=MAXSHAPEWIDTH;
    bool maxWidth256=(info->subtype&0x100000)?true:false;
	if (maxWidth256)
		maxShapeWidth = 256;

    goodfit = false;
    while (!goodfit)
    {
        int currentx = 0;  /* current UV location */
        int currenty = 0;

        int shpWidth  = 0; /* dimensions of shape */
        int shpHeight = 0;

	    int rowHeight = 0; /* height of current row */

		bool newRow = false; /* if true we should start a new row of characters */

        currentx = extraspace;
		currenty = extraspace;

        CHARACTER12* ch12      = 0;
        CHARACTER16* ch16      = 0;
        unsigned char chwidth  = 0;  // char width
        unsigned char chheight = 0;  // char height

        /* update each UV field for all characters */
        for (i = 0; i < frames; i++)
        {
            /* upadate character UV */

			if (charactersize==16)
			{
				ch16 = sortedCharacterList16[i];
				FNTPUTFIELD(chwidth, FNTGETFIELD(ch16->width));
				FNTPUTFIELD(chheight, FNTGETFIELD(ch16->height));
			}
			else
			{
			    ch12 = sortedCharacterList12[i];
				FNTPUTFIELD(chwidth, FNTGETFIELD(ch12->width));
				FNTPUTFIELD(chheight, FNTGETFIELD(ch12->height));

			}
            /* determine if we need a new row - check if the character is too wide for this row */
			{
				int potentialNextX = currentx + (int)(FNTGETFIELD(chwidth)) + extraspace;
				if (potentialNextX > maxWidth)
				{
					currenty += rowHeight;
					newRow = true;
				}
			}

            /* determine if we need a new row - check for a crossed vertical texture page */
			{
				int maxHeight = maxWidth;
				int top = currenty % maxHeight;

				int characterHeight    = (int)(FNTGETFIELD(chheight)) + extraspace;
				int potentialRowHeight = gmax(rowHeight, characterHeight);

				int bottom = top + potentialRowHeight;

				if (bottom > maxHeight)
				{
					newRow = true;

					/* round currenty to the next vertical texture boundary */
					while (currenty % maxHeight != 0)
						++currenty;

					currenty += extraspace;
				}
			}

			if (newRow)
            {
				/* Start of a new row */

                /* Add border around font for bilinear interpolation. */
                /* ensure U value is on a byte boundary */
                currentx = byte_align(extraspace, info->bpp);

                //currenty += rowHeight;
                rowHeight = 0;

				/* reset newRow flag */
				newRow = false;
            }

			if (charactersize == 16)
			{
				FNTPUTFIELD(ch16->u, currentx);
				FNTPUTFIELD(ch16->v, currenty);
			}
			else
			{
				FNTPUTFIELD(ch12->u, currentx);
				FNTPUTFIELD(ch12->v, currenty);

			}

            /* update current location */
            /* add extraspace for VRAM fonts so they bilinear interpolate correctly */
            currentx += FNTGETFIELD(chwidth) + extraspace;

            /* ensure U value is on a byte boundary as default for all other platforms */
            currentx = byte_align(currentx, info->bpp);

            /* track row height so that we know how far to advance to the next row */
            /* add extraspace for VRAM fonts so they bilinear interpolate correctly */
            rowHeight = gmax(rowHeight, ((int)(FNTGETFIELD(chheight)) + extraspace));

            /* track shape dimensions */
            shpWidth  = gmax(shpWidth, currentx);
            shpHeight = gmax(shpHeight, currenty + rowHeight);

        }

#if FNTTYPE == FNTTYPE_GFN
    	AlignToCell(&shpWidth,&shpHeight,info->bpp); // shape width and height must be on cell bondary
#endif

        /* keep cramming until shpHeight <= shpWidth (or we hit are max width) */
        if (shpHeight <= shpWidth || maxWidth >= maxShapeWidth)
        {
			/* force width to be a power of 2 */

			if (shpWidth<=maxWidth)		// for large glyphs(ie. images, the shpWidth
				shpWidth = maxWidth;	// may be larger than the maxWidth for the current pass
			else						// if this is the case, knock shpWidth up to
			{							// next power of 2 greater than or equal to shpWidth
    			// need to go up to next largest power of 2 for width
                shpWidth = (1<<(FNTWintlog2(shpWidth-1)+1));
			}
            goodfit = true;
        }
        else
        {
            maxWidth = gmin(maxShapeWidth, maxWidth*2);
        }

        *shapeWidth  = shpWidth;
        *shapeHeight = shpHeight;
    }

    /* clean up */
	if (sortedCharacterList16)
		gfree(sortedCharacterList16);
	if (sortedCharacterList12)
		gfree(sortedCharacterList12);
}

/****************************************************************/
/*  export Functions                                            */
/****************************************************************/

int GCALL FNT_wopen(GINSTANCE** ginstance, GSTREAM* g, const char *, int frames)
{
    /* allocate required buffers */
    GINSTANCE *    gx           = (GINSTANCE*)    galloc(sizeof(GINSTANCE));
    FONTWRITEINFO *info         = (FONTWRITEINFO*)galloc(sizeof(FONTWRITEINFO));
    FONTHEADER *   header       = (FONTHEADER*)   galloc(sizeof(FONTHEADER));

	// CHARACTER12 * chartbl		= (CHARACTER12*)  galloc(CHARACTERSIZE12 * frames);


	unsigned char ** frameList    = (unsigned char**)        galloc(sizeof(char*) * frames);
    int*           rowBytesList = (int*)          galloc(sizeof(int)   * frames);

    /* ensure no errors */
    if (gx && info && header && frameList && rowBytesList)
    {
        /* fill in header fields */
        memset(header, 0, sizeof(FONTHEADER));

        gputm(header->signature, SIGNATURE, sizeof(header->signature));
        FNTPUTFIELD(header->version,   FNT_VERSION); /* format version, not gimex version */
        FNTPUTFIELD(header->num,       frames);
        FNTPUTFIELD(header->chartbl,   sizeof(FONTHEADER));

        /* fill in write info fields */
        memset(info, 0, sizeof(FONTWRITEINFO));
        info->header       = header;
        //info->chartbl      = chartbl;
        info->frameList    = (char **)frameList;
        info->rowBytesList = rowBytesList;

        info->kerntbl   = NULL;
		info->kernpairs = 0;
        info->clut      = 0;
        info->numcolours= 0;

        /* clear lists */
       // memset(chartbl,      0, CHARACTERSIZE12 * frames);
        memset(frameList,    0, sizeof(char*) * frames);
        memset(rowBytesList, 0, sizeof(int) * frames);

        /* fill in instance fields */
        memset(gx, 0, sizeof(GINSTANCE));
        gx->signature = SIGNATURE;
        gx->size      = sizeof(GINSTANCE);
        gx->frames    = FNTGETFIELD(info->header->num);
        gx->framenum  = 0;
        gx->gstream   = g;
        gx->gref      = (char*)info;

        *ginstance = gx;
        return gx->frames;
    }

    /* handle errors (no memory) - notice return statment above */
    if (gx)           gfree(gx);
    if (info)         gfree(info);
    if (header)       gfree(header);
    if (frameList)    gfree(frameList);
    if (rowBytesList) gfree(rowBytesList);

    *ginstance = NULL;
    return 0;
}

int GCALL FNT_write(GINSTANCE* gx, const GINFO* ginfo, char* src, int rowBytes)
{
	static int kernpairs;	// FNT_Write gets called for all glyphs.
							// Use static integer to count up kernpairs.

	static KERN8* kernstart;	//beginning of kern data, need since using realloc and
								// address may change.

	static KERN8* kerntblQsort; // holds kern data for quicksort
	// Note about static variables: compiler initializes numeric values to zero
	// and pointers to NULL automatically


    FONTWRITEINFO* info      = (FONTWRITEINFO*)gx->gref;

	unsigned int newFormat16 = ginfo->subtype&0x40000;	// are we outputting the Char16 ?
    CHARACTER16* ch16 = 0;
    CHARACTER12* ch12 = 0;
    int charactersize = info->characterinfosize;

	/* On the first frane allocate the proper table and
	   get character info struct currently being written to */
    if (gx->framenum == 0)
	{
		if (newFormat16)
		{
			CHARACTER16* chartbl16	= (CHARACTER16*)  galloc(CHARACTERSIZE16 * gx->frames);

			if (chartbl16)
			{
				memset(chartbl16, 0, sizeof(CHARACTER16) * gx->frames);
				info->chartbl16 = chartbl16;
				charactersize = sizeof(CHARACTER16);
			}
		}
		else	// outputting char12
		{
			CHARACTER12* chartbl12 = (CHARACTER12*) galloc(CHARACTERSIZE12 * gx->frames);

			if (chartbl12)
			{
				memset(chartbl12, 0, sizeof(CHARACTER12) * gx->frames);
				info->chartbl = chartbl12;
				charactersize = sizeof(CHARACTER12);
			}
		}

		// pad shape data for alignment
		{
			int shapeoffset=sizeof(FONTHEADER)+charactersize*gx->frames;
			int padlen;

			padlen = ((shapeoffset+sizeof(SHAPE)+SHPDATAALIGNMENT)&~SHPDATAALIGNMENT)-sizeof(SHAPE)-shapeoffset;
			if (padlen>0)
			{
				shapeoffset += padlen;
			}
			FNTPUTFIELD(info->header->shape, shapeoffset+32); // 32 to skip SHAPE2 portion
            FNTPUTFIELD(info->header->states[FONT_SHAPE], shapeoffset);  // Shape2
		}

		info->characterinfosize = charactersize;

	} // end if gx->framenum = 0


	if (charactersize==16)
		ch16 = WGetCharacter16(info, gx->framenum);
	else
		ch12 = WGetCharacter12(info, gx->framenum);



    /* compute often used values */
    int xoffset = 0 - ginfo->centerx;
    int yoffset = 0 - ginfo->centery;

    /* allocate frame buffer and copy src into it */

    char* frameBuffer;
    int ok=0;

	info->packed	  = ginfo->packed;

    {
        if (!info->originalclut)
        {
            info->originalclut = (ARGB *)galloc(sizeof(ARGB)*256);
            memset(info->originalclut,0,sizeof(ARGB)*256);
        }

        info->numcolours = gmax(16,gmin(256,ginfo->numcolours));
        memcpy(info->originalclut,ginfo->colourtbl,sizeof(ARGB)*ginfo->numcolours);
    }

    // if image has colour, remember it (last frame) especially
#if FNTTYPE == FNTTYPE_XFN
    if (!IsAlphaClut(ginfo))
#endif
    {
		if (ginfo->originalbpp<=8)
		{
			if (!info->clut)
			{
				info->clut = (ARGB *)galloc(sizeof(ARGB)*256);
				memset(info->clut,0,sizeof(ARGB)*256);
			}

			info->numcolours = gmax(16,gmin(256,ginfo->numcolours));
			memcpy(info->clut,ginfo->colourtbl,sizeof(ARGB)*ginfo->numcolours);
		}

        info->alphabits	  = ginfo->alphabits;
        info->redbits     = ginfo->redbits;
        info->greenbits	  = ginfo->greenbits;
        info->bluebits    = ginfo->greenbits;
        info->subtype     = ginfo->subtype;
        info->originalbpp = ginfo->originalbpp;
    }



	info->originalbpp = ginfo->originalbpp;

    if (gx->framenum < gx->frames)
    {
        frameBuffer = (char*)galloc(ginfo->height * rowBytes);
        if (frameBuffer)
        {
            // fb - kludge this should be respecting rowbytes better
            memcpy(frameBuffer, src, ginfo->height * rowBytes);

            /* store frame buffer pointer in frameList */
            info->frameList[gx->framenum] = frameBuffer;

            /* store rowBytes in rowBytesList */
            info->rowBytesList[gx->framenum] = rowBytes;

            /* fill in known character info, for char16 */
            if (newFormat16)
			{
                FNTPUTFIELD(ch16->index,   FNTFramenameToIndex(ginfo->framename,gx->framenum));
                FNTPUTFIELD(ch16->width,   ginfo->width);
                FNTPUTFIELD(ch16->height,  ginfo->height);

				if (ginfo->numhotspots >= 1)
					FNTPUTFIELD(ch16->advancex, gmax(-32768,gmin(32767,ginfo->hotspottbl[0][0])));
				else /* hotspot not present, take our best guess */
					FNTPUTFIELD(ch16->advancex, gmax(-32768,gmin(32767,xoffset + ginfo->width)));

                FNTPUTFIELD(ch16->xoffset, xoffset);
                FNTPUTFIELD(ch16->yoffset, yoffset);

				FNTPUTFIELD(ch16->numkern, gmax(0,ginfo->numhotspots - 3));
			}
			/* fill in known character info, for char12 */
            else
			{
                FNTPUTFIELD(ch12->index,   FNTFramenameToIndex(ginfo->framename,gx->framenum));
                FNTPUTFIELD(ch12->width,   ginfo->width);
                FNTPUTFIELD(ch12->height,  ginfo->height);


				if (ginfo->numhotspots >= 1)
					FNTPUTFIELD(ch12->advance, gmax(-128,gmin(127,ginfo->hotspottbl[0][0])));
                else /* hotspot not present, take our best guess */
					FNTPUTFIELD(ch12->advance, gmax(-128,gmin(127,xoffset + ginfo->width)));

                FNTPUTFIELD(ch12->xoffset, xoffset);
                FNTPUTFIELD(ch12->yoffset, yoffset);

				FNTPUTFIELD(ch12->numkern, gmax(0,ginfo->numhotspots - 3));
			}


            /* update font header */
            FNTPUTFIELD(info->header->flags, ginfo->subtype);

#if SHAPECODE4
        	if (ginfo->originalbpp <= 4)
        		info->bpp = 4;
        	else
#endif
        		info->bpp = 8;

#if SHAPECODE16
        	if (ginfo->originalbpp > 8)
	    		info->bpp = 16;
#endif

#if SHAPECODE32
#if SHAPECODE16
			if (ginfo->originalbpp > 16)
        		info->bpp = 32;
#else
			if (ginfo->originalbpp > 8)
        		info->bpp = 32;
#endif
#endif

            {
                int ascent = ginfo->hotspottbl[2][0];
                int descent = ginfo->hotspottbl[2][1];

				if (ascent==0 && descent==0)			// both are zero, this means possibly we're trying to insert an image into the font
				{
					if (newFormat16)
					{
						ascent = gmax(1,(ch16->height)/2);			// default to half the height
						descent = ch16->height;

					}
					else
					{
						ascent = gmax(1,(ch12->height)/2);			// default to half the height
						descent = ch12->height;
					}

					FNTPUTFIELD(ginfo->hotspottbl[2][0], ascent);
					FNTPUTFIELD(ginfo->hotspottbl[2][1], descent);

					if (ginfo->hotspottbl[1][1]==0)
						FNTPUTFIELD(ginfo->hotspottbl[1][1],ginfo->centery+ascent+descent);
				}

                /* update font height (advancey) */
                if (ginfo->numhotspots >= 2)
                    info->fontheight = gmax(info->fontheight, ginfo->hotspottbl[1][1]);
                else    /* hotspot not present, take our best guess */
                    info->fontheight = gmax(info->fontheight, gmax(ascent+descent,yoffset + ginfo->height));


                info->fontascent = gmax(info->fontascent, ascent);
                info->fontdescent = gmax(info->fontdescent, descent);

                FNTPUTFIELD(info->header->ascent,  gmax(0,gmin(255,info->fontascent)));
            	FNTPUTFIELD(info->header->descent, gmax(0,gmin(255,info->fontdescent)));

                FNTPUTFIELD(info->header->states[FONT_HEIGHT], info->fontheight); // was ascent+descent

                ok = 1;
            }

			signed char numkern;
			unsigned char index[2];
			if (newFormat16)
			{
				FNTPUTMEM(&numkern,FNTGETFIELD(ch16->numkern),1);
				FNTPUTMEM(index,FNTGETFIELD(ch16->index),2);
			}
			else
			{
				FNTPUTMEM(&numkern,FNTGETFIELD(ch12->numkern),1);
				FNTPUTMEM(index,FNTGETFIELD(ch12->index),2);
			}

			/* Ali */
			if (numkern>0)		// we have kerning info
			{
				int i;

			//	FNTPUTFIELD(ch->kernindex, (short) kernpairs);
				kernpairs += numkern;

				// have some kerning info
				if ( ginfo->numhotspots > 3 )
				{

					//reallocate size of kerntblQsort to fit incoming KERN entries
					kerntblQsort = (KERN8 *) realloc(kerntblQsort, kernpairs * sizeof(KERN8) );

					if (kerntblQsort)
					{
						kernstart = kerntblQsort;
						kerntblQsort += (kernpairs - numkern); // move to end of mem block

						for (i=0; i<numkern; i++)
						{

							FNTPUTFIELD(kerntblQsort->previouscode, ginfo->hotspottbl[3+i][0]);
							//clamp kernvalue between -127 and 128, since its a char
							FNTPUTFIELD(kerntblQsort->kernvalue, ginfo->hotspottbl[3+i][1]>=0 ? gmin(128, ginfo->hotspottbl[3+i][1]) : gmax(-127, ginfo->hotspottbl[3+i][1]) );
							FNTPUTFIELD(kerntblQsort->code, FNTGETFIELD(index) );
							kerntblQsort++;
						}

						kerntblQsort = kernstart; // point to beginning of block for realloc

					}
				}	// end if numhotspots >3
			}


			// last frame
			if ( (unsigned int) gx->framenum == (FNTGETFIELD(info->header->num))-1)
			{
				KERN * kerntbl = NULL;
				if (kernpairs)
				{
					kerntbl = (KERN *) galloc(kernpairs * sizeof(KERN));

					if (kerntbl)
					{
						int i;

						// sort the table based on 'current' kern codes (see CompareKernCode)
						qsort(kerntblQsort, kernpairs, sizeof(KERN8), CompareKernCode);

						// now convert KERN8 table to KERN table for export
						for (i=0; i<kernpairs; i++)
						{
							FNTPUTFIELD(kerntbl[i].previouscode, FNTGETFIELD(kerntblQsort[i].previouscode));
							FNTPUTFIELD(kerntbl[i].kernvalue, FNTGETFIELD(kerntblQsort[i].kernvalue));
							FNTPUTFIELD(kerntbl[i].code, FNTGETFIELD(kerntblQsort[i].code));
						}
					} // end if kerntbl

					if (kerntblQsort != NULL)
					{
						free(kerntblQsort);
						kerntblQsort = NULL;
					}

					info->kernpairs = kernpairs;
				} // end if kernpairs

				if (kerntbl != NULL)
					info->kerntbl = (int *) kerntbl;	// if no kerning, this will simply be NULL

				kernpairs = 0;
				kernstart = NULL;
			}

			/* Ali End */


			++gx->framenum;
        }
    }
    return(ok);
}

#define KERNHEADERSIZE   4 // for clut link. there is no structure defined for the kern header
                           // so define kernheadersize here so I can have clut link and
						   // the actual memory buffer in sync.

int GCALL FNT_wclose(GINSTANCE* gx)
{
    FONTWRITEINFO* info = (FONTWRITEINFO*)gx->gref;
    SHAPE* shp;
    CLUTRHeaderStruct * Clutshp = 0;
    int i;
    int frames = gmin(gx->framenum, gx->frames);
    int ok=1;
    int shpWidth;
    int shpHeight;
    int shapeoffset;
    int SizeClut;
    int PreClutPad=0;
    int shapepad = 0;
    int kernsize = 0;
	int shpBytes = 0;
    bool iskern = (info->kerntbl)?true:false;
    bool isclut = (info->clut)?true:false;
	int charactersize = info->characterinfosize;

    if (frames)
    {

        /* Update header to reflect the actual number of frames */
        FNTPUTFIELD(info->header->num, frames);

        // pad shape data for alignment
        {
            if (iskern)
    	        shapeoffset=(((sizeof(FONTHEADER)+charactersize*frames)+3)&~3) + info->kernpairs*sizeof(KERN) + (iskern)*KERNHEADERSIZE;
	        else
        		shapeoffset= sizeof(FONTHEADER)+charactersize*frames;

            int padlen = ((shapeoffset+SHAPEHEADER+SHPDATAALIGNMENT)&~SHPDATAALIGNMENT)-SHAPEHEADER-shapeoffset;
            shapepad = padlen; // length of padding that I will need on my
	                       // shape, store for later use.
    	    if (padlen>0)
            {
                shapeoffset += padlen;
            }
            FNTPUTFIELD(info->header->shape, shapeoffset+32);  // OldShape follows Shape2
            FNTPUTFIELD(info->header->states[FONT_SHAPE], shapeoffset);  // Shape2
        }

        /* update uv's in character info structs */
        UpdateUVFields(info, &shpWidth, &shpHeight);

        /* align width to shpRowBytes */
		int shpRowBytes = WGetShapeRowBytes(shpWidth, info->bpp);
		shpWidth = (shpRowBytes*8)/info->bpp;

		// don't do for A8 type since there should be an A8 type and not a clut format.
		if (isclut)
        {
            /* allocate shape and fill in */
            FNTWSetShapeClut(info->originalbpp,info->alphabits,info->redbits,info->greenbits,info->bluebits,info->subtype,info->numcolours,&info->clut[0],&Clutshp,&SizeClut,FNTTYPE);
        }
		else // numcolours 0
        {
    	    isclut = 0;
			FNTWCreateAutoClut(info->originalbpp,info->subtype,&Clutshp,&SizeClut,FNTTYPE);
        }

#if FNTTYPE == FNTTYPE_XFN || FNTTYPE == FNTTYPE_FFN

		if (IsAlphaClut(Clutshp, SizeClut) && info->packed==3 )
		{
			// for DXT compression, we need a width/height that are multiples of 4
			// The following bumps width/height to the next largest multiple of 4, if
			// it isnt one already
			if (shpWidth%4)
				shpWidth = shpWidth + (4-(shpWidth%4));
			if (shpHeight%4)
				shpHeight = shpHeight + (4-(shpHeight%4));

		}

#endif

        bool forcepower2=(info->subtype&0x80000)?true:false;

// Palettized 8 XFNs are always swizzled. Need to set height/width to power of 2
#if FNTTYPE == FNTTYPE_XFN

		if (!IsAlphaClut(Clutshp, SizeClut))
		{
            forcepower2 = true;
		}

#endif
#if FNTTYPE == FNTTYPE_FFN
        forcepower2 = true;
#endif


        if (forcepower2)
        {
			if (shpWidth > 0 && shpHeight >0 )
			{
                if (shpWidth!=(1<<FNTWintlog2(shpWidth)))
				{
					// need to go up to next largest power of 2 for width
                    shpWidth = (1<<(FNTWintlog2(shpWidth-1)+1));
				}

                if (shpHeight!=(1<<FNTWintlog2(shpHeight)))
				{
					// need to go up to next largest power of 2 for width
                    shpHeight = (1<<(FNTWintlog2(shpHeight-1)+1));
				}
			}
//            if (shpHeight < shpWidth)
//                shpHeight = shpWidth;   // make it square as well
        }

		shpRowBytes = WGetShapeRowBytes(shpWidth, info->bpp);


        /* calc size of file */
        {
            int size = sizeof(FONTHEADER) + charactersize*frames;
            if (iskern)
    	        size = ((size+3)&~3);// ensure kerntable is aligned to 4 bytes.

    	    size += sizeof(SHAPE) + shpRowBytes*shpHeight + shapepad;
    	    kernsize = info->kernpairs*sizeof(KERN) + (iskern)*KERNHEADERSIZE;
    	                             // a bit of bookeeping for later.
    	    PreClutPad = (((size + kernsize + SHPCLUTALIGN + sizeof(SHAPE))&~SHPCLUTALIGN) - sizeof(SHAPE))  - (size + kernsize);
    	                             // now the question is, do a have a clut.
    	    if ((info->clut && info->numcolours) && (!((FNTTYPE == FNTTYPE_XFN || FNTTYPE == FNTTYPE_FFN) && info->originalbpp == 8)))
    	        size = (((size + kernsize + SHPCLUTALIGN + sizeof(SHAPE))&~SHPCLUTALIGN)- sizeof(SHAPE)) + SizeClut;
    	    else                  // nope no clut so don't pad.
    	        size = (size + kernsize);
            size += METALSIZE;

            gputi(info->header->size, size, 4);
        }

        /* allocate shape and fill in fields */
        shp = (SHAPE *) galloc(sizeof(SHAPE) + shpRowBytes*shpHeight);
        memset(shp, 0, sizeof(SHAPE) + shpRowBytes*shpHeight);

#if SHAPECODE4
    	if (info->bpp <= 4)
        {
    		FNTPUTFIELD(shp->type, SHAPECODE4);
    		FNTPUTFIELD(shp->mtype, SHAPECODE4);
        }
    	else
#endif
#if SHAPECODE32
    	if (info->bpp == 32)
        {
    		FNTPUTFIELD(shp->type, SHAPECODE32);
    		FNTPUTFIELD(shp->mtype, SHAPECODE32);
        }
    	else
#endif
#if SHAPECODE16
    	if (info->bpp >= 16)
        {
    		FNTPUTFIELD(shp->type, SHAPECODE16);
    		FNTPUTFIELD(shp->mtype, SHAPECODE16);
        }
    	else
#endif
		//if (info->bpp == 8)
		{
    		FNTPUTFIELD(shp->type, SHAPECODE8);
    		FNTPUTFIELD(shp->mtype, SHAPECODE8);
        }

#if FNTTYPE==FNTTYPE_XFN
		if (!IsAlphaClut(Clutshp, SizeClut))
        {
			FNTPUTFIELD(shp->type, SHAPECODEP8);
			FNTPUTFIELD(shp->mtype, SHAPECODEP8);
        }
#endif

        FNTPUTFIELD(shp->width, shpWidth);
        FNTPUTFIELD(shp->height, shpHeight);

        FNTPUTFIELD(shp->mFlags, FONT_TEXTURE_FLAG_NEW);
        FNTPUTFIELD(shp->mwidth, shpWidth);
        FNTPUTFIELD(shp->mheight, shpHeight);
        FNTPUTFIELD(shp->mDataOffset, sizeof(SHAPE));
        FNTPUTFIELD(shp->mDataSize, shpRowBytes*shpHeight);

        {
            int mnext = sizeof(SHAPE) + shpRowBytes*shpHeight + (isclut?PreClutPad:0);
            int next = mnext-(isclut?0:16);

      	    FNTPUTFIELD(shp->mNext,mnext);

            if (next>0x7fffff)  // clamp to max, which is way to say use size+alignment
            {
                next=0x7fffff;
//                PreClutPad = 0;
            }

      	    FNTPUTFIELD(shp->link,next);
		}


//IAN have to update shape link with clut if exist.

#if FNTTYPE == FNTTYPE_GFN && GC_SWIZZLE==1
        // output the swizzle bit
        FNTPUTFIELD(shp->shapex,0x2000);
		FNTPUTFIELD(shp->mFlags, FONT_TEXTURE_FLAG_NEW | FONT_TEXTURE_FLAG_SWIZZLED);
#endif

    #if FNTTYPE == FNTTYPE_PFN
        if (info->bpp == 4)
        {
            /* position shape in bottom right corner of VRAM */
            FNTPUTFIELD(shp->shapex, 1024 - shpWidth);
            FNTPUTFIELD(shp->shapey,  512 - shpHeight);
        }
    #endif

		CHARACTER16* ch16;
		CHARACTER12* ch12;
		unsigned char width;
		unsigned char height;
		unsigned char u[2];
		unsigned char v[2];

        /* copy characters in given buffer into shape */
        {
            for (i = 0; i < frames; i++)
            {

				if (charactersize==16)
				{
					ch16 = WGetCharacter16(info, i);
					FNTPUTFIELD(width,FNTGETFIELD(ch16->width));
					FNTPUTFIELD(height,FNTGETFIELD(ch16->height));
					FNTPUTFIELD(u, FNTGETFIELD(ch16->u));
					FNTPUTFIELD(v, FNTGETFIELD(ch16->v));

				}
				else if (charactersize==12)
				{
					ch12 = WGetCharacter12(info, i);
					FNTPUTFIELD(width,FNTGETFIELD(ch12->width));
					FNTPUTFIELD(height,FNTGETFIELD(ch12->height));
					FNTPUTFIELD(u, FNTGETFIELD(ch12->u));
					FNTPUTFIELD(v, FNTGETFIELD(ch12->v));
				}

                unsigned char* srcBase     = (unsigned char *)info->frameList[i];
                int   srcRowBytes = info->rowBytesList[i];

                unsigned char* dstBase     = (unsigned char *)shp + sizeof(SHAPE);
                int   dstRowBytes = shpRowBytes;
                int   dstBpp      = info->bpp;

                int   srcBpp      = 8;


#if FNTTYPE == FNTTYPE_GFN && GC_SWIZZLE==1
                CopyToGC(srcBase, srcRowBytes, srcBpp,
                    dstBase, dstRowBytes, dstBpp,
                    FNTGETFIELD(width), FNTGETFIELD(height),
                    FNTGETFIELD(u),     FNTGETFIELD(v));

#else
                srcBpp = info->originalbpp;
				if (info->bpp == 4 && info->packed == 3)
				{
				//	dstBpp = 8;
				//	dstRowBytes = shpWidth;
				}

				CopyTo(srcBase, srcRowBytes, srcBpp,
                    dstBase, dstRowBytes, dstBpp,
                    FNTGETFIELD(width), FNTGETFIELD(height),
                    FNTGETFIELD(u),     FNTGETFIELD(v),
					info->originalclut,
                    shp->type);

//                    (info->clut && info->numcolours) || (!(FNTTYPE == FNTTYPE_XFN && info->originalbpp == 8) ||
//	             !(FNTTYPE == FNTTYPE_FFN && info->originalbpp == 8) ));
#endif
            }
        }

#if	FNTTYPE == FNTTYPE_XFN
{
	    // if we have a P8 XFN file, we want to swizzle it

    	if (info->bpp == 8 && !IsAlphaClut(Clutshp, SizeClut) )
		{
			unsigned char *shpSwizzle = (unsigned char *) galloc(shpWidth*shpHeight);
			int row, col = 0;
			int srcOffset, dstOffset = 0;
			unsigned char * shpStart = (unsigned char *)shp + sizeof(SHAPE);


				for (row=0; row<shpHeight; row++)
				{
					for (col=0; col<shpWidth; col++)
					{
						srcOffset = row*shpWidth + col;
						dstOffset = (Xbox_GetSwizzledPixelOffset(col, row, shpWidth, shpHeight, info->bpp)) >> 3 ;

						shpSwizzle[dstOffset] = shpStart[srcOffset];
					}
				}

			memcpy(shp + 1, shpSwizzle, shpHeight*shpWidth);


			FNTPUTFIELD(shp->shapex, 0x2000);	// it is swizzled
			FNTPUTFIELD(shp->mFlags,  FONT_TEXTURE_FLAG_NEW | FONT_TEXTURE_FLAG_SWIZZLED);	// it is swizzled

			gfree(shpSwizzle);

		}	// end if bpp = 8, and !alphaclut
		else	// it is not Palettized 8, so we dont swizzle
		{

		}
}
#endif

#if FNTTYPE == FNTTYPE_XFN || FNTTYPE == FNTTYPE_FFN


		// this means we read from a DXT compressed font and we're writting to
		// one as well
		if (info->packed == 3 && info->bpp == 4)
		{

			unsigned char* dst;
			int numblocks = (shpWidth/4) * (shpHeight/4);
			shpBytes = shpWidth*shpHeight*2/8 + numblocks*4;

			DXTCompressFontFromDxt( (unsigned char *) (shp + 1), &dst, shpWidth, shpHeight);

			gfree(shp);
			shp=NULL;
			shp = (SHAPE *) galloc(sizeof(SHAPE) + shpBytes);
			memcpy(shp+1, dst, shpBytes);
			gfree(dst);
		}

		// pack3 is DXT1 compression.
		// DXT compress 8bpp Alpha 8 PC/Xbox fonts
		if (info->packed == 3 && info->bpp==8 && IsAlphaClut(Clutshp, SizeClut) )
		{
			unsigned char* dst;
			int numblocks = (shpWidth/4) * (shpHeight/4);
			shpBytes = shpWidth*shpHeight*2/8 + numblocks*4;

			isclut = 0;

			// skip the shp header, send image data
			// dst comes back as DXT compressed 2bpp pixel data
			DXT1CompressFont( (unsigned char *) (shp + 1), &dst, shpWidth, shpHeight, Clutshp);


			/* calc new size of file */
			{
				int size = sizeof(FONTHEADER) + charactersize*frames;
				if (iskern)
    				size = ((size+3)&~3);// ensure kerntable is aligned to 4 bytes.

    			size += sizeof(SHAPE) + shpBytes + shapepad;


				kernsize = info->kernpairs*sizeof(KERN) + (iskern)*KERNHEADERSIZE;
    									 // a bit of bookeeping for later.
    			PreClutPad = (((size + kernsize + SHPCLUTALIGN + sizeof(SHAPE))&~SHPCLUTALIGN) - sizeof(SHAPE))  - (size + kernsize);
    									 // now the question is, do a have a clut.


				if ((info->clut && info->numcolours) && (!((FNTTYPE == FNTTYPE_XFN || FNTTYPE == FNTTYPE_FFN) && info->originalbpp == 8)))
    				size = (((size + kernsize + SHPCLUTALIGN + sizeof(SHAPE))&~SHPCLUTALIGN)- sizeof(SHAPE)) + SizeClut;
    			else                  // nope no clut so don't pad.
    				size = (size + kernsize);
				size += METALSIZE;

				gputi(info->header->size, size, 4);
			}


			/* allocate shape and fill in fields */
			gfree(shp);
			shp = NULL;
			shp = (SHAPE *) galloc(sizeof(SHAPE) + shpBytes);

			memset(shp, 0, sizeof(SHAPE) + shpBytes);

			memcpy(shp+1, dst, shpBytes);

			FNTPUTFIELD(shp->type, SHAPECODEDXT);
			FNTPUTFIELD(shp->width, shpWidth);
			FNTPUTFIELD(shp->height, shpHeight);

			FNTPUTFIELD(shp->mtype, SHAPECODEDXT);
			FNTPUTFIELD(shp->mFlags, FONT_TEXTURE_FLAG_NEW);
			FNTPUTFIELD(shp->mwidth, shpWidth);
			FNTPUTFIELD(shp->mheight, shpHeight);

			int next = sizeof(SHAPE) + shpBytes;
  			FNTPUTFIELD(shp->mNext,next);
            if (next >= 0x007fffff)
	            next = 0x007fffff;
   			FNTPUTFIELD(shp->link,next);
			info->bpp = 2;

			gfree(dst);

		}	// end if packed and alpha clut

		else
		{
			shpBytes = shpRowBytes * shpHeight;
			if (info->packed == 3)
			{
				isclut = false;

				int next = sizeof(SHAPE) + shpRowBytes*shpHeight;

				FNTPUTFIELD(shp->link,next);
				FNTPUTFIELD(shp->type, SHAPECODEDXT);

				FNTPUTFIELD(shp->mNext,next);
				FNTPUTFIELD(shp->mtype, SHAPECODEDXT);
				FNTPUTFIELD(shp->mFlags, FONT_TEXTURE_FLAG_NEW);
			}
		}
#else
			shpBytes = shpRowBytes *shpHeight;


#endif

        /* sort character info structures on index */
        /* these must be sorted (by ascending index) for the binary search to work in the fontlib */
        if (charactersize==16)
			qsort(info->chartbl16, frames, charactersize, CompareIndex16);
		else
			qsort(info->chartbl, frames, charactersize, CompareIndex);


		/* we now have the character table and the kern table in order of index */
		/* we now fill in the kernindex field for CHARACTER16s */

		if (iskern && charactersize==16)
		{
			int k;
			int kernpairs=0;

			for (k=0; k<frames; k++)
			{
				CHARACTER16 * ch16 = WGetCharacter16(info, k);
				if (FNTGETFIELD(ch16->numkern) >0)
					FNTPUTFIELD(ch16->kernindex,kernpairs);

				kernpairs += FNTGETFIELD(ch16->numkern);

			}
		}

    	/* store position of the kerning table in the file */
    	if (iskern)
    	{
    		FNTPUTFIELD(info->header->kerntbl, (((sizeof(FONTHEADER)+charactersize*frames)+3)&~3));    //shapeoffset + sizeof(SHAPE) +
    	}
    	else
    	{
    		FNTPUTFIELD(info->header->kerntbl, 0);
    	}

        // fill in default states
        FNTPUTFIELD(info->header->states[FONT_COLOUR],    0xffffffff); // white
        FNTPUTFIELD(info->header->states[FONT_Z],         FLOATASINT(FONT_ZNEAR));
        FNTPUTFIELD(info->header->states[FONT_W],         FLOATASINT(FONT_WNEAR));
        FNTPUTFIELD(info->header->states[FONT_UNDEF],     0x7f);
		FNTPUTFIELD(info->header->states[FONT_SCALE_X],   FLOATASINT(1.0f));
        FNTPUTFIELD(info->header->states[FONT_SCALE_Y],   FLOATASINT(1.0f));
        FNTPUTFIELD(info->header->states[FONT_FILTER],    1);
        FNTPUTFIELD(info->header->states[FONT_DEPTHWRITE],0);
        FNTPUTFIELD(info->header->states[FONT_ANGLE],     FLOATASINT(0.0f));
        FNTPUTFIELD(info->header->states[FONT_DANGLE],    FLOATASINT(0.0f));
		FNTPUTFIELD(info->header->states[FONT_BLEND],     1);
#if FNTTYPE == FNTTYPE_XFN
        if (!isclut)
        {
            FNTPUTFIELD(info->header->states[FONT_SCALE_U],   FLOATASINT(1.0f));
            FNTPUTFIELD(info->header->states[FONT_SCALE_V],   FLOATASINT(1.0f));
        }
        else
#endif
        {
            FNTPUTFIELD(info->header->states[FONT_SCALE_U],   FLOATASINT(1.0f/shpWidth));
            FNTPUTFIELD(info->header->states[FONT_SCALE_V],   FLOATASINT(1.0f/shpHeight));
        }

#if 0
    if (!shp->swizzled)
    {
        *(float *)&font->states[FONT_USCALE] = 1.0f;
        *(float *)&font->states[FONT_VSCALE] = 1.0f;
    }
    else
    {
        *(float *)&font->states[FONT_USCALE] = 1.0f/shpWidth;
        *(float *)&font->states[FONT_VSCALE] = 1.0f/shpWidth;
    }
#endif
        /* write data to disk */
        {
			int k;
            GSTREAM* g = gx->gstream;
			gwrite(g, info->header, sizeof(FONTHEADER));            /* write font header */


			bool foundSpaceChar = false;

			if (charactersize==16)
			{
				CHARACTER16* charstart = info->chartbl16;

				// look for the space character and if we find it, change its
				// height and width to 0. This improves font redering performance.
				// On import we change the width and height of the space character
				// back to 1, to ensure compatability with tools using gimex.
				// Also, when find the space character, set xoffset to 0 and yoffset to ascent-1
				for (k=0; k<frames && foundSpaceChar==false; k++)
				{
					if (FNTGETFIELD(info->chartbl16->index) ==32) //found space
					{
// when doing baseline oriented fonts, this messes up.  Move this logic to import level
//						FNTPUTFIELD(info->chartbl16->xoffset, 0);
//						FNTPUTFIELD(info->chartbl16->yoffset, FNTGETFIELD(info->header->ascent)-1);

						if ( FNTGETFIELD(info->chartbl16->width) != 0 && FNTGETFIELD(info->chartbl16->height) != 0)
						{
							FNTPUTFIELD(info->chartbl16->width, 0);
							FNTPUTFIELD(info->chartbl16->height,0);
						}


						foundSpaceChar = true;
					}
					(info->chartbl16)++;
				}

				info->chartbl16 = charstart;

			}

			else if (charactersize==12)
			{
				CHARACTER12* charstart = info->chartbl;

				// look for the space character and if we find it, change its
				// height and width to 0. This improves font redering performance.
				// On import we change the width and height of the space character
				// back to 1, to ensure compatability with tools using gimex.
				for (k=0; k<frames && foundSpaceChar==false; k++)
				{
					if ( FNTGETFIELD(info->chartbl->index)==32)
					{
						//FNTPUTFIELD(info->chartbl->xoffset, 0);
						//FNTPUTFIELD(info->chartbl->yoffset, FNTGETFIELD(info->header->ascent)-1);
						if (FNTGETFIELD(info->chartbl->width) != 0 && FNTGETFIELD(info->chartbl->height) != 0)
						{
							FNTPUTFIELD(info->chartbl->width,0);
							FNTPUTFIELD(info->chartbl->height,0);

						}

						foundSpaceChar = true;
					}

					(info->chartbl)++;
				}

				info->chartbl = charstart;
			}


			/* write character info structs */
			if (charactersize==16)
				gwrite(g, info->chartbl16, charactersize*frames);
			else if (charactersize==12)
				gwrite(g, info->chartbl, charactersize*frames);


       	    /* write the kerning table */
    	    if (info->kerntbl)
    	    {
                char kernheader[KERNHEADERSIZE];
                char c[4];
        		int pad = (((sizeof(FONTHEADER)+charactersize*frames)+3)&~3) - (sizeof(FONTHEADER)+charactersize*frames);
        		KERN *kern;

                kern = (KERN *) galloc(info->kernpairs*sizeof(KERN));
                if (kern)
                {
        		    memset(c,0,4);
    	    	    gwrite(g,&c,pad);

              	    FNTPUTMEM(kernheader,info->kernpairs,4);

					/* no need to sort here, already sorted in FNT_write */

					//qsort(info->kerntbl, info->kernpairs, CHARACTERSIZE12, CompareIndex);



					gwrite(g, kernheader, sizeof(kernheader));
                    memset(kern,0,info->kernpairs*sizeof(KERN));
                    for (i=0; i<info->kernpairs; ++i)
                    {
                    	FNTPUTFIELD(kern[i].previouscode, FNTGETFIELD( ((KERN *) info->kerntbl)[i].previouscode));
                    	FNTPUTFIELD(kern[i].kernvalue   , FNTGETFIELD( ((KERN *) info->kerntbl)[i].kernvalue));
                    	FNTPUTFIELD(kern[i].code,         FNTGETFIELD( ((KERN *) info->kerntbl)[i].code));
					}

            	    ok = gwrite(g, kern, info->kernpairs*sizeof(KERN));
                    gfree(kern);
                 }
                 else
                     ok = 0;
    	    }

            // pad shape data for alignment
            {
                int shapeoffset=(((sizeof(FONTHEADER)+charactersize*frames) + 3)&~3) + info->kernpairs*sizeof(KERN) + (iskern)*KERNHEADERSIZE;
                int padlen;

				if (!iskern)
                    shapeoffset = sizeof(FONTHEADER)+charactersize*frames;

                padlen = ((shapeoffset+sizeof(SHAPE)+SHPDATAALIGNMENT)&~SHPDATAALIGNMENT)-sizeof(SHAPE)-shapeoffset;
                if (padlen>0)
                {
                    char padstr[SHPDATAALIGNMENT+1];
                    memset(padstr,0,SHPDATAALIGNMENT+1);
#ifdef DEBUG
                    memcpy(padstr,"SellERTS",8); /* a way to spot debug lib :-) */
#else
                    memcpy(padstr,"Buy ERTS",8); /* pad with a promotional message */
#endif
                    ok = (gwrite(g,padstr,padlen) == padlen);
                    shapeoffset += padlen;
                }
            }

			/* write shape header and shape frame buffer */
           // gwrite(g, shp, sizeof(SHAPE) + shpRowBytes*shpHeight);
			  gwrite(g, shp, sizeof(SHAPE) + shpBytes);

            /* write shape Clut */

	        if (isclut)
            {
                int ind = 0;
                char c  = 0;

                FNTPUTFIELD(Clutshp->LinkShape,SizeClut-16);     // point next to metal
                FNTPUTFIELD(Clutshp->mNext,SizeClut);
                for (ind = 0; ind < PreClutPad; ind++)
                    gwrite(g,&c,1);
                gwrite(g, Clutshp, SizeClut);
            }

            /* write metal */
            {
                FONTMetalHeaderStruct *metal = (FONTMetalHeaderStruct *) galloc(METALSIZE);
                memset(metal,0,METALSIZE);

                FNTPUTFIELD(metal->type,0x69);  // metalbinsize
                FNTPUTFIELD(metal->metalsize,METALSIZE-sizeof(FONTMetalHeaderStruct));  // metalbinsize
                FNTPUTFIELD(metal->metalbits,0x10);  // metalbin flag

                FNTPUTFIELD(metal->mtype,0x69);  // metalbinsize
                FNTPUTFIELD(metal->mFlags,FONT_TEXTURE_FLAG_NEW);
                FNTPUTFIELD(metal->mDataOffset,sizeof(FONTMetalHeaderStruct));  // metalbin flag
                FNTPUTFIELD(metal->mDataSize,METALSIZE-sizeof(FONTMetalHeaderStruct));  // metalbin flag

                strcpy((char *)metal+sizeof(FONTMetalHeaderStruct),METALSTR);  // ie "EAGL64"
                gwrite(g, metal, METALSIZE);

                gfree(metal);

            }

            if (Clutshp)
            {
                gfree(Clutshp);
                Clutshp = 0;
            }
        }

        /* free all allocated data */

        for (i = 0; i < frames; i++)
        {
            if (info->frameList[i])
                gfree(info->frameList[i]);
        }
        gfree(shp);
    } // end if frames.

    gfree(info->header);
    if (info->chartbl)
		gfree(info->chartbl);
	if (info->chartbl16)
		gfree(info->chartbl16);
	if (info->kerntbl)
		gfree(info->kerntbl);
    gfree(info->frameList);
    gfree(info->rowBytesList);
    if (info->clut) gfree(info->clut);
    if (info->originalclut) gfree(info->originalclut);
    gfree(info);

    gfree(gx);

    return(ok);
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

#undef FNTGETMEM
#undef FNTPUTMEM
#undef FNTGETFIELD
#undef FNTPUTFIELD

#undef SHAPECODE4
#undef SHAPECODE8
#undef SHAPECODE16
#undef SHAPECODE32

#undef SHPDATAALIGNMENT
#undef SHAPEHEADER
#undef MINSHAPEWIDTH
#undef MAXSHAPEWIDTH

#undef FNTTYPE

#endif
