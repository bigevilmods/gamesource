// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                      VP6 - VP6 Movie Gimex                       */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 060516 FB based on HSI but only open/info implemented    */
/*    1.01 060620 FB skip audio chunk                               */
/*    1.02 061219 FB support old 06PV header                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* Reentrant                                                        */
/* Files: vp6read.cpp vp6write.cpp vp6gimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "vp6gimex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL VP6_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','V','P','6');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 0;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->globalpalette   = 0;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->movie           = 1;    /* movie file format */
        info->file64          = 1;    /* this format supports 64 bit file sizes */
        info->mactype[0]      = GMAKEID('.','V','P','6');   /* mac file system types used */
        strcpy(info->extensions[0], ".vp6");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.02");                /* version # */
        strcpy(info->shorttypestr,  "VP6");                 /* type */
        strcpy(info->wordtypestr,   "VP6");                 /* wordtype */
        strcpy(info->longtypestr,   "VP6 Movie");           /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* has MVhd chunk with codec type vp60 */

// bfmc.vp6
// 00000: 4d566864 20000000 76703630 0005d002  MVhd ...vp60..P.
// 00010: 40080000 dc540100 12750000 e8030000  @...\T...u..h...

// old thx.vp6
// 00000: 4d566864 20000000 30365056 8002e001  MVhd ...06PV..`.
// 00010: 84000000 843d0000 19330d00 ff7f0000  .....=...3......

int GCALL VP6_is(GSTREAM *g)
{
    unsigned char header[32];
    int ok=0;
    int code;
	GPOS p=0;
    gseek(g,p);

    if (gread(g,header,32))
    {
        code = ggetm(header,4);
        while (code == GMAKEID('S','C','H','l'))
        {
			p += ggeti(header+4,4);
			if (gseek(g, p))
			{
			    if (gread(g, header, 32))
				{
					code = ggetm(header,4);
				}
				else break;
			}
			else break;
		}

		if (ggetm(header,4)==0x4d566864) // MVhd
		{
			if (ggetm(header+8,4)==0x76703630) // vp60
				ok = 100;
			if (ggetm(header+8,4)==0x30365056) // 06PV
				ok = 95;
			if (ggetm(header+8,4)==0)
				ok = 90;
		}
	}
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL VP6_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    int numframes;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->frames = 1;
        gx->signature=GMAKEID('.','V','P','6');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;

        gx->gref = galloc(32);
        if (gx->gref)
        {
		    int code;
			GPOS p=0;

            gseek(gx->gstream,p);

            if (gread(gx->gstream,gx->gref,32))
			{
				code = ggetm(gx->gref,4);
				while (code == GMAKEID('S','C','H','l'))
				{
					p += ggeti((char *)gx->gref+4,4);
					if (gseek(gx->gstream, p))
					{
						if (gread(gx->gstream, gx->gref, 32))
						{
							code = ggetm(gx->gref,4);
						}
						else break;
					}
					else break;
				}

				numframes = ggeti(&(((ChunkedAviHeader *)gx->gref)->mTotalFrames),4);
				gx->frames = numframes;
				ok = numframes;
			}
        }
        if (gx->gref && !ok)
            gfree(gx->gref);
    }

    return(ok);
}

#if 0
struct ChunkedAviHeader
{
  char   mId[4];                    // 'AVhd' FourCC
  char   mLength[4];                // Length of 'AVhd' chunk
  char   mCompressor[4];            // Compressor FourCC 
  char   mWidth[2];
  char   mHeight[2];
  char   mTotalFrames[4];           // Total number of frames in the file. This should always be accurate
  char   mSuggestedBufferSize[4];   // Recommended decode buffer size 
  char   mFpsNumerator[4];          // FPS = Numerator/Denominator
  char   mFpsDenominator[2];        // Eg. 30000/1001=29.97
  char   mFlags[2];                 // Eg. uses B-frames
};
#endif

GINFO * GCALL VP6_info(GINSTANCE *gx, int framenum)
{
    ChunkedAviHeader *vp6header;
    GINFO *ginfo=0;

    vp6header = (ChunkedAviHeader *)gx->gref;

    if (framenum < gx->frames)
        ginfo = (GINFO *) galloc(sizeof(GINFO));
    if (ginfo)
    {
        memset(ginfo,0,sizeof(GINFO));

        /* template for ginfo */

        ginfo->signature        = GMAKEID('.','V','P','6');   /* Gimex type */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;    /* Current frame */
        ginfo->subtype          = 0;    /* Internal format sub-type 0-default */
        ginfo->packed           = 0;    /* Type of packing on original image. 0 none, 1 run, n other */
        ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
        ginfo->redbits          = 8;    /* Number of bits in red channel */
        ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
        ginfo->bluebits         = 8;    /* Number of bits in blue channel */
        ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
        ginfo->centery          = 0;
        ginfo->defaultx         = 0;    /* Default Coordinate Point */
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */

        ginfo->width  = VP6GETFIELD(vp6header->mWidth);
        ginfo->height  = VP6GETFIELD(vp6header->mHeight);
        ginfo->fps  = (float)VP6GETFIELD(vp6header->mFpsNumerator)/VP6GETFIELD(vp6header->mFpsDenominator);

        ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
        ginfo->originalbpp      = 24;   /* Bits per pixel in original image (1 to 32) */
        ginfo->numcolours  = 0;    /* Number of colours in original indexed palette */
    }

    return(ginfo);
}

int GCALL VP6_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
	return 0;
#if 0

    char *linebuf;
    char *lineptr;
    int bpp;
    int bpl;
    int x;
    int y;
    int ok=0;
    ARGB *d;
    GPOS framesize;

    framesize = ginfo->width*ginfo->height*ginfo->originalbpp>>3;
    gseek(gx->gstream,32+ginfo->numcolours*3+ginfo->framenum*framesize);
    bpp = ginfo->originalbpp/8;
    bpl = ginfo->width*bpp;

    if (bpp==1 && rowbytes==ginfo->width)
    {
        ok = gread(gx->gstream,dest,(int)framesize);
    }
    else
    {
        linebuf = (char *) galloc(bpl);

        if (linebuf)
        {
            ok = 1;

            for (y=0; y<ginfo->height && ok; ++y)
            {
                d = (ARGB *) (dest+rowbytes*y);

                if (bpp==1)
                {
                    ok = gread(gx->gstream,(unsigned char *)d,bpl);
                }
                else
                {
                    ok = gread(gx->gstream,linebuf,bpl);
                    lineptr = linebuf;

                    for (x=0; x<ginfo->width; ++x)
                    {
                        d->a = 255;
                        d->r = lineptr[0];
                        d->g = lineptr[1];
                        d->b = lineptr[2];
                        ++d;
                        lineptr += 3;
                    }
                }
            }
            gfree(linebuf);
        }
    }
    return(ok);
#endif

}

int GCALL VP6_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        if (gx->gref)
            gfree(gx->gref);

        ok = gfree((void *) gx);
    }
    return(ok);
}

