// (c) 1998-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               DDS - S3 Compressed Texture Gimex                  */
/*                                                                  */
/*                   by FrANK G. Barchard, EAC                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 980317 FB Based on BMP                                   */
/*    1.01 980515 FB Data follows data size immediately now (was+4) */
/*    1.02 980529 KO Export done. uses S3 codex module              */
/*    1.03 980601 KO Import uses S3 codex.  Rounds export up to 4   */
/*    1.04 990422 FB Signature 'S3TC'                               */
/*    1.05 991027 FB DDS import, read leak fix                      */
/*    1.06 991116 KO Supports 1565 and 4565                         */
/*    1.07 000720 FB about mipmap                                   */
/*    1.08 001207 FB import framesize calc fix                      */
/*    1.09 010206 FB spun off s3t module                            */
/*    1.10 010208 ID added support for uncomrpressed images         */
/*    1.11 010209 ID added support for mipmaps and DXT1..5          */
/*    1.12 010223 ID added CubeMap support                          */
/*    1.13 010227 ID added Uncompressed Volume Texture Support      */
/*    1.14 010510 ID added big dxt support                          */
/*    1.15 011114 FB unused/uninitialized variable fixup for gcc    */
/*    1.16 030310 FB framename mm00, '.dds' signature               */
/*    1.17 070213 FB XBGR                                           */
/*    1.18 070402 FB YUY2 import                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 16 bit                                                           */
/* Reentrant                                                        */
/*                                                                  */
/* Files: ddsread.cpp ddswrite.cpp ddsgimex.h                       */
/*                                                                  */
/* export subtype                                                   */
/*      0 means insert explicit alpha block                         */
/*      1 means insert interpolated alpha block                     */
/*                                                                  */
/* Rounds up width and height to nearest multiple of 4              */
/*                                                                  */
/* import alpha not supported.                                      */
/*                                                                  */
/* CUBEMAP ORDERING:                                                */
/*    Face #   SideInfo                                             */
/*       0     PositiveX                                            */
/*       1     NegativeX                                            */
/*       2     PositiveY                                            */
/*       3     NegativeY                                            */
/*       4     PositiveZ                                            */
/*       5     NegativeZ                                            */
/*                                                                  */
/*  SUBTYPES: Complex CubeMap:            777                       */
/*            Simple  CubeMap:             42                       */
/*            Uncompressed VolumeTexture: 256                       */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* DDS files are 4x4 tiles with 2 colours and shades of 1/3 and     */
/* 2/3 between.  16x16 tiles are built on top of the 4x4            */
/* Basic file format is RIFF                                        */
/*                                                                  */
/* DDS header                                                       */
/* 00000: 44445320 7c000000 07100800 00010000  DDS ............     */
/* 00010: 00010000 00800000 00000000 00000000  ................     */
/* 00020: 00000000 00000000 00000000 00000000  ................     */
/* 00030: 00000000 00000000 00000000 00000000  ................     */
/* 00040: 00000000 00000000 00000000 20000000  ............ ...     */
/* 00050: 04000000 44585431 00000000 00000000  ....DXT1........     */
/* 00060: 00000000 00000000 00000000 00100000  ................     */
/* 00070: 00000000 00000000 00000000 00000000  ................     */
/* 00080: 0000ffff ffffffff 0000ffff ffffffff  ................     */
/* 00090: 0000ffff ffffffff 0000ffff ffffffff  ................     */
/* 000a0: 0000ffff ffffffff 0000ffff ffffffff  ................     */
/* 000b0: 0000ffff ffffffff 0000ffff ffffffff  ................     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <stdio.h>
#include <string.h>
#include "gimex.h"
#include "ddsgimex.h"
#include "codex.h"
#include "dxtcodex.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/




//-----------------------------------------------------------------
// retrieve which face this item belongs to. (CUBEMAP Specific)
static int DDSGetFace(unsigned long DdsCapsField)
{
    int face;
    face = (DdsCapsField & GDDSCAPS_CUBEMAP_POSITIVEX)?0:
          ((DdsCapsField & GDDSCAPS_CUBEMAP_NEGATIVEX)?1:
         (((DdsCapsField & GDDSCAPS_CUBEMAP_POSITIVEY)?2:
        ((((DdsCapsField & GDDSCAPS_CUBEMAP_NEGATIVEY)?3:
       (((((DdsCapsField & GDDSCAPS_CUBEMAP_POSITIVEZ)?4:
      ((((((DdsCapsField & GDDSCAPS_CUBEMAP_NEGATIVEZ)?5:0)))))))))))))));

    return face;
}

//-----------------------------------------------------------------
// Helper function to reduce code overhead.
// the DX docs indicate that it is possible to have cubemaps
// with only one face/frame inside them, however DxTex seems
// to be a bit sloppy with the DDSCAPS field and so it
// is necessary to check the complex flag. Later if
// it is decided upon we should change this to allow for
// single frame cube maps.
static int DDSisCubeMap(unsigned long DdsCapsField)
{
   return( ((DdsCapsField & GDDSCAPS_CUBEMAP_POSITIVEX)|
            (DdsCapsField & GDDSCAPS_CUBEMAP_NEGATIVEX)|
            (DdsCapsField & GDDSCAPS_CUBEMAP_POSITIVEY)|
            (DdsCapsField & GDDSCAPS_CUBEMAP_NEGATIVEY)|
            (DdsCapsField & GDDSCAPS_CUBEMAP_POSITIVEZ)|
            (DdsCapsField & GDDSCAPS_CUBEMAP_NEGATIVEZ)) &&
            (DdsCapsField & GDDSCAPS2_CUBEMAP));

}

// width*height*4/3 is quick, but this is more accurate

static int DDSsizemipmaps(int width,int height,int sizer)
{
    int s=0;
    do
    {
        s += ((width+3)/4) * ((height+3)/4) * sizer;
        width = gmax(1,width>>1);
        height = gmax(1,height>>1);
    } while (width>1 || height>1);
    return(s);
}

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



static int readUncompressedImage(GINSTANCE *gx,GINFO *ginfo,DDSHeaderStruct * ddsheader,char *dest,
	int isComplex,int isMipMap,int isVolumeTexture)
{
   int fieldsize = DDSGETFIELD(ddsheader->dwSize);

   int bpp = DDSGETFIELD(ddsheader->ddsPixelFormat.dwRGBBitCount);
   char *t = ddsheader->ddsPixelFormat.dwFourCC;
   int FourCC = GDDSMAKEFOURCC(t[0],t[1],t[2],t[3]);
    if (FourCC==0x52474247  // RGBG
     || FourCC==0x47524742  // GRBG
     || FourCC==0x59555932) // YUY2
        bpp = 16;

   int mul = bpp/8;
   int i  = 0;
   int m  = 0;
   int ok = 1;
   unsigned char *d = (unsigned char *)dest;
   unsigned char *r = NULL;
   int T;
   int width,height,mipoffset;
   unsigned char *rowbuf = (unsigned char *) galloc(ginfo->width*mul);
   // get the bit masks so I can test for 16 bit formats.
   unsigned long RM = DDSGETFIELD(ddsheader->ddsPixelFormat.dwRBitMask);
   unsigned long GM = DDSGETFIELD(ddsheader->ddsPixelFormat.dwGBitMask);
   unsigned long BM = DDSGETFIELD(ddsheader->ddsPixelFormat.dwBBitMask);
   gseek(gx->gstream,(GPOS)0);                        // move to top of stream
   gseek(gx->gstream,(fieldsize+4));                  // move to enf of header.

   // figure out mipmap information.
   width   = DDSGETFIELD(ddsheader->dwWidth);         // Width of bitmap in pixels
   height  = DDSGETFIELD(ddsheader->dwHeight);        // Height of bitmap in pixels
   if(isMipMap)
   {
        mipoffset = 0;
        for(i = 0; i < ginfo->framenum;i++)
        {
            mipoffset += width*height*(bpp/8);            // add previous mipmap sizes.
            width  /= 2;
            height /= 2;
        }
        if (mipoffset)
            ok = gseek(gx->gstream,gtell(gx->gstream)+mipoffset);
   }
   if(isComplex||isVolumeTexture) // jump to the cube/volumetexture I want to read.
   {
       gseek(gx->gstream,(width)*(height)*ginfo->framenum*mul+sizeof(DDSHeaderStruct));
   }
   if(ok)
   {
   ok = 0;
   for(i = 0; i < ginfo->height;i++)
   {
         ok = gread(gx->gstream,rowbuf,ginfo->width*mul);
         if(!ok)break;
         r = rowbuf;
         for(m = 0; m < ginfo->width;m++)
         {
             T=ggeti(r,2);

             if (FourCC==0x52474247)  // RGBG
             {
                 if ((m&1)==0)
                 {
                    ((ARGB *)d)->a = (unsigned char)255;
                    ((ARGB *)d)->b = (unsigned char)r[3];
                    ((ARGB *)d)->g = (unsigned char)r[0];
                    ((ARGB *)d)->r = (unsigned char)r[1];
                 }
                 else
                 {
                    ((ARGB *)d)->a = (unsigned char)255;
                    ((ARGB *)d)->b = (unsigned char)r[1];
                    ((ARGB *)d)->g = (unsigned char)r[0];
                    ((ARGB *)d)->r = (unsigned char)r[-1];
                 }
             }
             else if (FourCC==0x47524742)  // GRBG
             {
                 if ((m&1)==0)
                 {
                    ((ARGB *)d)->a = (unsigned char)255;
                    ((ARGB *)d)->b = (unsigned char)r[2];
                    ((ARGB *)d)->g = (unsigned char)r[1];
                    ((ARGB *)d)->r = (unsigned char)r[0];
                 }
                 else
                 {
                    ((ARGB *)d)->a = (unsigned char)255;
                    ((ARGB *)d)->b = (unsigned char)r[0];
                    ((ARGB *)d)->g = (unsigned char)r[1];
                    ((ARGB *)d)->r = (unsigned char)r[-2];
                 }
             }
             else if (FourCC==0x59555932)  // YUY2
             {
                 float R,G,B,Y,U,V;

                 if ((m&1)==0)
                 {
                     Y = (float)r[0];
                     U = (float)r[1];
                     V = (float)r[3];
                 }
                 else
                 {
                     Y = (float)r[0];
                     U = (float)r[-1];
                     V = (float)r[1];
                 }

                YUVtoRGB(R,G,B,Y,U,V);

                ((ARGB *) d)->a = 255;  // alpha
                ((ARGB *) d)->r = ROUNDCLAMPINT(R);
                ((ARGB *) d)->g = ROUNDCLAMPINT(G);
                ((ARGB *) d)->b = ROUNDCLAMPINT(B);
             }
             else if(bpp == 8 && (RM == 0x0f) && (GM == 0) && (BM == 0)) // a4l4
             {
                     T=ggeti(r,1);
                     ((ARGB *)d)->a = (unsigned char)((((T>> 4)&15)*255+8)/15);
                     ((ARGB *)d)->r = (unsigned char)((((T>> 0)&15)*255+8)/15);
                     ((ARGB *)d)->g = (unsigned char)((((T>> 0)&15)*255+8)/15);
                     ((ARGB *)d)->b = (unsigned char)((((T>> 0)&15)*255+8)/15);
             }
             else if(bpp == 8 && (RM == 0xff) && (GM == 0) && (BM == 0)) // l8
             {
                    ((ARGB *)d)->b = (unsigned char)r[0];
                    ((ARGB *)d)->g = (unsigned char)r[0];
                    ((ARGB *)d)->r = (unsigned char)r[0];
                    ((ARGB *)d)->a = 0xFF;
             }
             else if(bpp == 8 && (RM == 0xe0) && (GM == 0x1c) && (BM == 0x03))
             {
                     T=ggeti(r,1);
                     ((ARGB *)d)->a = (unsigned char)255;
                     ((ARGB *)d)->r = (unsigned char)((((T>>5)&7)*255+ 4)/7);
                     ((ARGB *)d)->g = (unsigned char)((((T>>2)&7)*255+ 4)/7);
                     ((ARGB *)d)->b = (unsigned char)((((T>>0)&3)*255+ 2)/3);
             }
             else if(bpp == 8)
             {
                    ((ARGB *)d)->r = (unsigned char)255;
                    ((ARGB *)d)->g = (unsigned char)255;
                    ((ARGB *)d)->b = (unsigned char)255;
                    ((ARGB *)d)->a = (unsigned char)r[0];
             }
             else if(bpp == 16 && (RM == 0x00ff) && (GM == 0) && (BM == 0))  // a8l8
             {
                    ((ARGB *)d)->b = (unsigned char)r[0];
                    ((ARGB *)d)->g = (unsigned char)r[0];
                    ((ARGB *)d)->r = (unsigned char)r[0];
                    ((ARGB *)d)->a = (unsigned char)r[1];
             }
             else if(bpp == 16 && (RM == 0xffff) && (GM == 0) && (BM == 0))  // l16
             {
                    ((ARGB *)d)->b = (unsigned char)r[1]; // ignore low 8 bits
                    ((ARGB *)d)->g = (unsigned char)r[1];
                    ((ARGB *)d)->r = (unsigned char)r[1];
                    ((ARGB *)d)->a = (unsigned char)255;
             }
             else if(bpp == 16 && ( ((RM == GR4444BITMASK) && (GM == GG4444BITMASK) && (BM == GB4444BITMASK))  // argb
                            || ((RM == 0x0f00) && (GM == 0x00f0) && (BM == 0x000f)) )) // xrgb
             {
                     ((ARGB *)d)->a = (unsigned char)((((T>>12)&15)*255+8)/15);
                     ((ARGB *)d)->r = (unsigned char)((((T>> 8)&15)*255+8)/15);
                     ((ARGB *)d)->g = (unsigned char)((((T>> 4)&15)*255+8)/15);
                     ((ARGB *)d)->b = (unsigned char)((((T>> 0)&15)*255+8)/15);
             }
             else if(bpp == 16 && ((RM == GR555BITMASK) && (GM == GG555BITMASK) && (BM == GB555BITMASK)))
             {
                     ((ARGB *)d)->a = (unsigned char)((((T>>15)&1)*255 + 0)/1);
                     ((ARGB *)d)->r = (unsigned char)((((T>>10)&31)*255+ 16)/31);
                     ((ARGB *)d)->g = (unsigned char)((((T>> 5)&31)*255+ 16)/31);
                     ((ARGB *)d)->b = (unsigned char)((((T>> 0)&31)*255+ 16)/31);
             }
             else if(bpp == 16 && ((RM == GR565BITMASK) && (GM == GG565BITMASK) && (BM == GB565BITMASK)))
             {
                     ((ARGB *)d)->a = 0xFF;
                     ((ARGB *)d)->r = (unsigned char)((((T>>11)&31)*255+ 16)/31);
                     ((ARGB *)d)->g = (unsigned char)((((T>> 5)&63)*255+ 32)/63);
                     ((ARGB *)d)->b = (unsigned char)((((T>> 0)&31)*255+ 16)/31);
             }
             else if(bpp == 24)
             {
                    ((ARGB *)d)->b = (unsigned char)r[0];
                    ((ARGB *)d)->g = (unsigned char)r[1];
                    ((ARGB *)d)->r = (unsigned char)r[2];
                    ((ARGB *)d)->a = 0xFF;
             }
             else if(bpp == 32 && BM==0x000003ff)  // a2r10g10b10
             {
                     T=ggeti(r,4);
                     ((ARGB *)d)->a = (unsigned char)((((T>>30)&3)*255 + 2)/3);
                     ((ARGB *)d)->b = (unsigned char)((((T>>20)&1023)*255+ 512)/1023);
                     ((ARGB *)d)->g = (unsigned char)((((T>>10)&1023)*255+ 512)/1023);
                     ((ARGB *)d)->r = (unsigned char)((((T>> 0)&1023)*255+ 512)/1023);
             }
             else if(bpp == 32 && RM==0x000003ff)  // a2b10g10r10
             {
                     T=ggeti(r,4);
                     ((ARGB *)d)->a = (unsigned char)((((T>>30)&3)*255 + 2)/3);
                     ((ARGB *)d)->r = (unsigned char)((((T>>20)&1023)*255+ 512)/1023);
                     ((ARGB *)d)->g = (unsigned char)((((T>>10)&1023)*255+ 512)/1023);
                     ((ARGB *)d)->b = (unsigned char)((((T>> 0)&1023)*255+ 512)/1023);
             }
             else if(bpp == 32 && BM==0x000000ff)  // argb
             {
                    ((ARGB *)d)->b = (unsigned char)r[0];
                    ((ARGB *)d)->g = (unsigned char)r[1];
                    ((ARGB *)d)->r = (unsigned char)r[2];
                    ((ARGB *)d)->a = (unsigned char)r[3];
             }
             else if(bpp == 32) // abgr
             {
                    ((ARGB *)d)->r = (unsigned char)r[0];
                    ((ARGB *)d)->g = (unsigned char)r[1];
                    ((ARGB *)d)->b = (unsigned char)r[2];
                    ((ARGB *)d)->a = (unsigned char)r[3];
             }
             else
                return(0);     // report error if not one of the above formats.
             r+=mul;
             d+=4;
         }
    }
    } //if
    gfree(rowbuf);
    return(ok);
}


static void DDSunorderblocks(int width, int height, int rowbytes, const void *source, void *dest)
{
    int x,y,bx,by;
    const unsigned char *s = (const unsigned char *)source;

    for (y=0; y<height; y+=4)
    {
        for (x=0; x<width; x+=4)
        {
            for (by=0; by<4; ++by) // rows of block
            {
                for (bx=0; bx<4; ++bx) // columns of block
                {
                    if ((y+by)<height && (x+bx)<width)
                        *(int *) ((char *)dest + (y+by)*rowbytes + (x+bx)*4) = *(int *) (s + by*16 + bx*4);
                }
            }
            s += 64;
        }
    }
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL DDS_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','D','D','S');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->mipmap          = 1;    /* module supports mipmaps */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 70;   /* default pack quality (for DXT) */
        info->mactype[0]      = GMAKEID('.','D','D','S');
        strcpy(info->extensions[0], ".dds");                /* extensions */
        strcpy(info->authorstr,     "Ian Davids and FrANK Barchard");   /* author */
        strcpy(info->versionstr,    "1.18");                /* version # */
        strcpy(info->shorttypestr,  "DDS");                 /* type */
        strcpy(info->wordtypestr,   "DDS");                 /* wordtype */
        strcpy(info->longtypestr,   "DirectDraw Surface");  /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* id dds=0x91910000 dds=0x44445320*/

int GCALL DDS_is(GSTREAM *g)
{
    char ddsbuf[4];
    int ok=0;

    gseek(g, 0);
    if (gread(g, ddsbuf, 4))
    {
        if (ggetm(ddsbuf,4)==0x91910000
         || ggetm(ddsbuf,4)==0x44445320)
            ok = 100;    /* it's an dds/dds */
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

static int hasMipmapFlags(DDSHeaderStruct *ddsheader)
{
	return (DDSGETFIELD(ddsheader->dwFlags) & GDDSD_MIPMAPCOUNT) || ( DDSGETFIELD(ddsheader->dwCaps) & GDDSCAPS_MIPMAP);
}

int GCALL DDS_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    DDSHeaderStruct ddsheader;
    GINSTANCE *gx;
    int ok=0;
    int mul = 0;
    int isCubeMap = 0;
    int isComplex = 0;
    int isVolumeTexture = 0;
    int isDdsCapsValid = 0;


    gseek(g,(GPOS)0);
    memset(&ddsheader, 0, sizeof(DDSHeaderStruct));
    ok = gread(g, (char *) &ddsheader, 4+0x6c); /* read header */
    if (ok)
    {
        ok = 0;
        gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
		if (gx)
		{
			memset(gx, 0, sizeof(GINSTANCE));
			isDdsCapsValid = ((DDSGETFIELD(ddsheader.dwFlags))& GDDSD_CAPS);  // do I have a valid caps entry.
			isVolumeTexture = ((DDSGETFIELD(ddsheader.dwCaps2))& GDDSCAPS2_VOLUME);
			if (isDdsCapsValid) // do I have a cubemap?
			{
				int DdsCapsField = DDSGETFIELD(ddsheader.dwCaps);
				int DdsCapsField2 = 0;
				
				DdsCapsField2 = DDSGETFIELD(ddsheader.dwCaps2);
				isCubeMap = DDSisCubeMap(DdsCapsField2);
				isComplex = (DdsCapsField & GDDSCAPS_COMPLEX);
				int mipcount = DDSGETFIELD(ddsheader.dwMipMapCount);
				
				if (isCubeMap && isComplex) // if this is a cubemap and it is complex.
				{
					ok = 6;
				}
				else if (mipcount > 1)
                {
					ok = mipcount;    /* number of mipmaps */
                    if (!ok) ok = 1;
                }
                else if(isVolumeTexture)
                {
                    ok = DDSGETFIELD(ddsheader.dwDepth);
                    if(ok&(ok-1))  // if not a power of two then make it so.
                    {
                       int temp = ok;
                       for(mul = 2;mul <= 1024;mul *=2)
                       {
                           if((ok>>1) < mul) // round down..
                           {    temp= mul;
                                break;
                           }
                           temp = mul;
                       }
                       ok = temp;
                    }

                }
                else
                   ok = 1;

            }
            else // not a cubemap or a VTC so set to mipmap count or 1.
            {
                if(isVolumeTexture)
                {
                    ok = DDSGETFIELD(ddsheader.dwDepth);
                    if(ok&(ok-1))  // if not a power of two then make it so.
                    {
                       int temp = ok;
                       for(mul = 2;mul <= 1024;mul *=2)
                       {
                           if((ok>>1) < mul) // round down..
                           {    temp= mul;
                                break;
                           }
                           temp = mul;
                       }
                       ok = temp;
                    }

                }
                else
                    ok = DDSGETFIELD(ddsheader.dwMipMapCount);      /* number of mipmaps */
                if (!ok) ok = 1;
            }
            gx->frames = ok;
            gx->signature=GMAKEID('.','D','D','S');
            gx->size=sizeof(GINSTANCE);
            gx->gstream = g;
            *gxptr = gx;
        }
    }
    return(ok);
}

GINFO * GCALL DDS_info(GINSTANCE *gx, int framenum)
{
    int ok=0;
    int FourCC = 0;
    DDSHeaderStruct ddsheader;
    GINFO *ginfo=0;
    int width;
    int height;
    int redb=0,blueb=0,greenb=0,alphab=0,origbpp,packed,flags;
    unsigned long RM             = 0;   // RGB dept mask holders.
    unsigned long GM             = 0;
    unsigned long BM             = 0;
    int centery                  = 0;   // cubemap offset holders.
    int defaulty                 = 0;
    int isCubeMap                = 0;
    int isComplex                = 0;
    int isVolumeTexture          = 0;
    unsigned long isDdsCapsValid = 0;   // is my dwCaps field valid?
    int face                     = 0;   // which face of the cubemap is this?
    int mipcount = 0;

    gseek(gx->gstream,(GPOS)0);
    memset(&ddsheader, 0, sizeof(DDSHeaderStruct));

    ok = gread(gx->gstream, (char *) &ddsheader, sizeof(DDSHeaderStruct)); /* read header */
    isDdsCapsValid = ((DDSGETFIELD(ddsheader.dwFlags))& GDDSD_CAPS);  // do I have a valid caps entry.
    isVolumeTexture= ((DDSGETFIELD(ddsheader.dwCaps2))& GDDSCAPS2_VOLUME);
	mipcount = DDSGETFIELD(ddsheader.dwMipMapCount);
	    
    if (ok)
    {
		if(mipcount > 1 || hasMipmapFlags(&ddsheader))
        {
            width   = gmax(1,DDSGETFIELD(ddsheader.dwWidth) >> framenum);      /* Width of bitmap in pixels */
            height  = gmax(1,DDSGETFIELD(ddsheader.dwHeight) >> framenum);     /* Height of bitmap in pixels */
        }
        else // otherwise this is a complex image with multi frame each of the same size.
        {
            width   = DDSGETFIELD(ddsheader.dwWidth);
            height  = DDSGETFIELD(ddsheader.dwHeight);
        }
        flags = DDSGETFIELD(ddsheader.dwFlags);
        if(flags & GDDSD_PIXELFORMAT)
        {
                packed  = (DDSGETFIELD(ddsheader.ddsPixelFormat.dwFlags) & GDDPF_FOURCC)?3:0;
                origbpp = DDSGETFIELD(ddsheader.ddsPixelFormat.dwRGBBitCount);
                if (!packed)
                {
                    RM = DDSGETFIELD(ddsheader.ddsPixelFormat.dwRBitMask);
                    GM = DDSGETFIELD(ddsheader.ddsPixelFormat.dwGBitMask);
                    BM = DDSGETFIELD(ddsheader.ddsPixelFormat.dwBBitMask);

                    if(origbpp == 8) // setup bit information for each image type.
                    {
                        if ((RM == 0x0f) && (GM == 0) && (BM == 0))  // A4L4
                        {
                            redb = alphab = 4;
                            blueb = greenb = 0;
                        }
                        else if ((RM == 0xff) && (GM == 0) && (BM == 0))  // L8
                        {
                            redb = 8;
                            blueb = greenb = alphab = 0;
                        }
                        else if((RM == 0xe0) && (GM == 0x1c) && (BM == 0x03)) // r3g3b2
                        {
                            redb = greenb = 3;
                            blueb = 2;
                            alphab = 0;
                        }
                        else // A8
                        {
                            alphab = 8;
                            redb = blueb = greenb = 0;
                        }
                    }
                    else if(origbpp == 16) // setup bit information for each image type.
                    {
                        if ((RM == 0x00ff) && (GM == 0) && (BM == 0))  // a8l8
                        {
                            redb = alphab = 8;
                            blueb = greenb = 0;
                        }
                        else if ((RM == 0xffff) && (GM == 0) && (BM == 0))  // l16
                        {
                            redb = 16;
                            blueb = greenb = alphab = 0;
                        }
                        else if (((RM == GR4444BITMASK) && (GM == GG4444BITMASK) && (BM == GB4444BITMASK))  // argb
                         || ((RM == 0x0f00) && (GM == 0x00f0) && (BM == 0x000f)) ) // xrgb
                        {
                            redb = blueb = greenb = alphab = 4;
                        }
                        else if((RM == GR555BITMASK) && (GM == GG555BITMASK) && (BM == GB555BITMASK))
                        {
                            redb = blueb = greenb  = 5;
                            alphab = 1;
                        }
                        else if((RM == GR565BITMASK) && (GM == GG565BITMASK) && (BM == GB565BITMASK))
                        {
                           redb = blueb = 5;
                           greenb = 6;
                           alphab = 0;
                        }
                    }
                    else if(origbpp == 24)
                    {
                           redb = blueb = greenb = 8;
                           alphab = 0;
                    }
                    else if(origbpp == 32)
                    {
                        if(RM==0x000003ff || BM==0x000003ff)  // a2b10g10r10
                        {
                            redb = blueb = greenb = 10;
                            alphab = 2;
                        }
                        else
                            redb = blueb = greenb = alphab = 8;
                    }
                }// if !packed.
                else // have to find out what the fourCC type is so I can set info.
                {
                  char *t;
                  t =   ddsheader.ddsPixelFormat.dwFourCC;
                  FourCC = GDDSMAKEFOURCC(t[0],t[1],t[2],t[3]);

                  if (FourCC==0x52474247  // RGBG
                   || FourCC==0x47524742  // GRBG
                   || FourCC==0x59555932) // YUY2
                  {
                      packed = 1;             // 1 to indicate simple compression
                      redb = blueb = 4;       // all the formats are 565 only alpha changes.
                      greenb = 8;
                      alphab = 0;
                      origbpp = 16;
                  }
                  else
                  {
                    redb    = 5;       // all the formats are 565 only alpha changes.
                    greenb  = 6;
                    blueb   = 5;
                    origbpp = 16;
                    if(FourCC == GDXT1FORMAT)
                        alphab  = 1;
                    else if(FourCC == GDXT2FORMAT || FourCC == GDXT3FORMAT)
                        alphab  = 4;
                    else if(FourCC == GDXT4FORMAT || FourCC == GDXT5FORMAT)
                        alphab  = 3;
                  }
                }

        }    // if the ddsPixelFormat structure is valid.
        else // else setup some good guess defaults.
        {
                origbpp = 16;
                packed  = 3;
                redb    = 5;
                greenb  = 6;
                blueb   = 5;
                alphab  = 1;

        }
        // check the DDSCaps field for cubemap information.
        if (isDdsCapsValid)
        {

            // get cubemap information..
            int DdsCapsField  = DDSGETFIELD(ddsheader.dwCaps);
            int DdsCapsField2 = DDSGETFIELD(ddsheader.dwCaps2);
            isCubeMap = DDSisCubeMap(DdsCapsField2);
            isComplex = (DdsCapsField & GDDSCAPS_COMPLEX);
            if(isCubeMap) // if this is a cubemap.
            {
               if(isComplex)
                    face = framenum;
               else
                    face = DDSGetFace(DdsCapsField);
              defaulty = face*height;
              centery  = defaulty + (int)((float)height/2.0);
            }
        }

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','D','D','S'); // Type of file
            ginfo->size             = sizeof(GINFO);            // Length of GINFO
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                 // Number of frames (if more than 1, openframeblt required)
            ginfo->width            = width;                    // Width of bitmap in pixels
            ginfo->height           = height;                   // Height of bitmap in pixels
            ginfo->subtype          = (isCubeMap)?((isComplex)?GDDS_SUBTYPE_CUBECOMPlEX:GDDS_SUBTYPE_CUBESIMPLE):(isVolumeTexture?GDDS_SUBTYPE_VOLUMETEX:0);
            ginfo->packed           = packed;                   // 3 is DXT 0 means uncompressed, 1 if FOURCC type (R8G8B8G8)
            ginfo->quality          = 90;                       // Quality of lossy packing 0..100
            ginfo->alphabits        = alphab;                   // Number of bits in alpha channel
            ginfo->redbits          = redb;                     // Number of bits in red channel
            ginfo->greenbits        = greenb;                   // Number of bits in grreen channel
            ginfo->bluebits         = blueb;                    // Number of bits in blue channel
            ginfo->centerx          = 0;                        // Center Point relative to upper left corner
            ginfo->centery          = centery;                  // if cubemap this will reflect vertical height in ordering.
            ginfo->defaultx         = 0;                        // Default Coordinate Point
            ginfo->defaulty         = defaulty;
            ginfo->numhotspots      = 0;                        // Number of Hot Spots defined
            if (framenum>0)
                sprintf(ginfo->framename,"mm%02d",framenum);        // Null terminated name of frame/image
            ginfo->bpp              = 32;                       // Bits per pixel (8 or 32)
            ginfo->originalbpp      = origbpp;
            ginfo->numcolours       = 0;                        // Number of colours in original indexed palette
            ginfo->framesize        = ((width+3)/4) * ((height+3)/4) * 8;
        }
    }

    return(ginfo);
}

int GCALL DDS_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=0;
    DDSHeaderStruct ddsheader;
    unsigned char blockbuf[8];
    int width;
    int height;
    int framenum=ginfo->framenum;
    int mipoffset;
    int headersize;
    void *source;
    char *temp;
    char *t;
    int compressedsize;
    int FourCC;
    int sizer = 8;
    unsigned long checker = 0;
    DXTHEADER *header;
    BIGDXTHEADER *Bigheader;
    int code = DXT_DXT1_CODE;   // default decompression type.
    int moder =0;
    int isCubeMap = 0;
    int isComplex = 0;
    int mipcount = 0;
    int mipmapflags = 0;
    // setup for header read.
    gseek(gx->gstream,(GPOS)0);
    memset(&ddsheader, 0, sizeof(DDSHeaderStruct));

    ok = gread(gx->gstream, (char *) &ddsheader, sizeof(DDSHeaderStruct)); /* read header */

    // these are stored in order in memory as bytes so create the fourCC to figure out encoding type.
    t =   ddsheader.ddsPixelFormat.dwFourCC;
    FourCC = GDDSMAKEFOURCC(t[0],t[1],t[2],t[3]);
    if((DDSGETFIELD(ddsheader.dwFlags))& GDDSD_CAPS)  // do I have a valid caps entry.
    {
        int DdsCapsField = DDSGETFIELD(ddsheader.dwCaps);
        int DdsCapsField2 = DDSGETFIELD(ddsheader.dwCaps2);
        isCubeMap = DDSisCubeMap(DdsCapsField2);
        isComplex = (DdsCapsField & GDDSCAPS_COMPLEX);
    }


    if (ok)
    {
        headersize = DDSGETFIELD(ddsheader.dwSize);

        int mipmapflags = hasMipmapFlags(&ddsheader);
        // is this an uncompressed image then read it otherwise setup for other read.

        if (ginfo->packed <= 1)
        {
            ok = readUncompressedImage(gx, ginfo, &ddsheader, dest, (isCubeMap&&isComplex),  mipmapflags, (DDSGETFIELD(ddsheader.dwCaps2)) & GDDSCAPS2_VOLUME);
            return(ok);
        }

        // seek to end of header.
        if (headersize<0x7c)
        {
            ok = gseek(gx->gstream,4+headersize+4);        /* 4 for id,+header,+4 for datasize */
            ok = gread(gx->gstream, blockbuf, 4);
            if ((ggetm(blockbuf,4)&0xff00ffff)!=0x00000000) /* consume old format constant */
                ok = gseek(gx->gstream,4+headersize+4);
        }
        else
            ok = gseek(gx->gstream,4+headersize);        /* 4 for id,+header */


        // first figure out the compression type and setup accordingly.
        if(FourCC == GDXT1FORMAT)
        {
            sizer = 8;
        }
        else if(FourCC == GDXT3FORMAT || FourCC == GDXT2FORMAT)
        {
            code = DXT_DXT3_CODE;
            sizer = 16;            // note there is both an explicit alpha block
                                   // AND a colour block THIS IS 16!
        }
        else if(FourCC == GDXT5FORMAT || FourCC == GDXT4FORMAT)
        {
            code = DXT_DXT5_CODE;
            sizer = 16;
        }
        if((FourCC == GDXT4FORMAT) || (FourCC == GDXT2FORMAT))
           moder = DXT_ENCODE_RGB_ATTENUATE;
        else
           moder = 0;

        // figure out mipmap information.
        width   = DDSGETFIELD(ddsheader.dwWidth);      /* Width of bitmap in pixels */
        height  = DDSGETFIELD(ddsheader.dwHeight);     /* Height of bitmap in pixels */
        mipcount = DDSGETFIELD(ddsheader.dwMipMapCount); /* Number of mipmaps */
        mipmapflags = hasMipmapFlags(&ddsheader);
        if (mipcount > 1 || mipmapflags) // if this is a mipmap then.
        {
            mipoffset = (DDSsizemipmaps(width,height,sizer)
                        -DDSsizemipmaps(gmax(1,width>>framenum),gmax(1,height>>framenum),sizer));
            width = gmax(1,width >> framenum);
            height = gmax(1,height >> framenum);
        }
        else
           mipoffset = 0;
        if (mipoffset)
            ok = gseek(gx->gstream,gtell(gx->gstream)+mipoffset);

        if (ok)
        {
            int dxtsize = ((width+3)/4) * ((height+3)/4) * 64;
            int isBig   = (dxtsize > 16000000)?1:0;
            ok = 0;
            // Read the compressed image
            // first alloc my termporary image buffer for read of file data.
            source = galloc(((width+3)/4) * ((height+3)/4) * sizer + sizeof(DXTHEADER));
            if (source)
            {
                if(isCubeMap && isComplex) // jump to the cube I want to read.
                   gseek(gx->gstream,((width+3)/4)*((height+3)/4)*sizer*framenum+sizeof(DDSHeaderStruct));
                // read in the file data making sure to take into account the block size.
                gread(gx->gstream, (char *)source + sizeof(DXTHEADER), ((width+3)/4) * ((height+3)/4) * sizer); // bug (fb) - should use BIGDXTHEADER

                // Prepend the CODEX header to the compressed data
                if(!isBig)
                {
                    header = (DXTHEADER*)source;
                    gputm(header->code, code, sizeof(header->code));
                    gputm(header->size, ((width+3)/4) * ((height+3)/4) * 64, sizeof(header->size));
                    gputm(header->mode, (DXT_MODE_RGB|moder), sizeof(header->mode));
                }
                else
                {
                    Bigheader = (BIGDXTHEADER*)source;
                    code |= 0x8000;                       // set the high bit, we're useing big format.
                    gputm(Bigheader->code, code, sizeof(Bigheader->code));
                    gputm(Bigheader->size, ((width+3)/4) * ((height+3)/4) * 64, sizeof(Bigheader->size));
                    gputm(Bigheader->mode, (DXT_MODE_RGB|moder), sizeof(Bigheader->mode));

                }

                // Decode and reorder the blocks for the destination image
                temp = (char *) galloc(DXT_size(source));
                if (temp)
                {
                    memset(temp, 0, DXT_size(source));
                    DXT_decode((void*)temp,source, &compressedsize);
                    DDSunorderblocks(width, height, rowbytes, (void*)(temp), dest);
                    gfree(temp);
                    ok = 1;
                }
                gfree(source);
            }
        }
    }
    return(ok);
}

int GCALL DDS_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


#undef GDDSD_VOLUMETEXTURE
#undef GDDS_SUBTYPE_VOLUMETEX
#undef GDDS_SUBTYPE_CUBESIMPLE
#undef GDDS_SUBTYPE_CUBECOMPLEX
#undef GDDSMAKEFOURCC
#undef GDXT1FORMAT
#undef GDXT2FORMAT
#undef GDXT3FORMAT
#undef GDXT4FORMAT
#undef GDXT5FORMAT
#undef GR4444BITMASK
#undef GG4444BITMASK
#undef GB4444BITMASK
#undef GA4444BITMASK
#undef GA555BITMASK
#undef GR555BITMASK
#undef GG555BITMASK
#undef GB555BITMASK
#undef GR565BITMASK
#undef GG565BITMASK
#undef GB565BITMASK
#undef GDDPF_ALPHAPIXELS
#undef GDDPF_RGB
#undef GDDPF_FOURCC
#undef GDDSD_HEIGHT
#undef GDDSD_WIDTH
#undef GDDSD_PIXELFORMAT
#undef GDDSD_MIPMAPCOUNT
#undef GDDSD_LINEARSIZE
#undef GDDSD_CAPS
#undef GDDCAPS_TEXTURE
#undef GDDSCAPS_MIPMAP
#undef GDDSCAPS2_CUBEMAP
#undef GDDSCAPS_CUBEMAP_POSITIVEX
#undef GDDSCAPS_CUBEMAP_NEGATIVEX
#undef GDDSCAPS_CUBEMAP_POSITIVEY
#undef GDDSCAPS_CUBEMAP_NEGATIVEY
#undef GDDSCAPS_CUBEMAP_POSITIVEZ
#undef GDDSCAPS_CUBEMAP_NEGATIVEZ
#undef GDDSCAPS_COMPLEX

