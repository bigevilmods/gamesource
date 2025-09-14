// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "fligimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL FLI_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->frames = numframes;
        gx->framenum = 0;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL FLI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int width;
    int height;
    int bpp;
    int colours;
    int y;
    int ok=0;
    int i;
    int bitmapsize;
    int flen;
    FLIHeaderStruct fliheader;
    unsigned char chunk[10+256*3];

    if (ginfo->bpp<=8)
    {
        colours = ginfo->numcolours;
        width = ginfo->width;
        height = ginfo->height;
        bpp = ginfo->bpp;

        bitmapsize = 16+6+2+1+1+256*3+6+width*height;
        flen = 128+bitmapsize*gx->frames;

        if (gx->framenum==0)
        {
            memset(&fliheader,0,sizeof(fliheader));


            /* FLI FILE HEADER structure 128 bytes */

            FLIPUTFIELD(fliheader.size,flen);               /* Size of flic including this header. */
            FLIPUTFIELD(fliheader.type,0xaf12);         /* Either FLI_TYPE or FLC_TYPE below. */
            FLIPUTFIELD(fliheader.frames,gx->frames);       /* Number of frames in flic. */
            FLIPUTFIELD(fliheader.width,width);             /* Flic width in pixels. */
            FLIPUTFIELD(fliheader.height,height);       /* Flic height in pixels. */
            FLIPUTFIELD(fliheader.depth,bpp);               /* Bits per pixel.  (Always 8 now.) */
            FLIPUTFIELD(fliheader.flags,3);                 /* FLI_FINISHED | FLI_LOOPED ideally. */
            FLIPUTFIELD(fliheader.speed,5);                 /* Delay between frames. */
            FLIPUTFIELD(fliheader.reserved1,0);             /* Set to zero. */
            FLIPUTFIELD(fliheader.created,0);               /* Date of flic creation. (FLC only.) */
            FLIPUTFIELD(fliheader.creator,0);               /* Serial # of flic creator. (FLC only.) */
            FLIPUTFIELD(fliheader.updated,0);               /* Date of flic update. (FLC only.) */
            FLIPUTFIELD(fliheader.updater,0);               /* Serial # of flic updater. (FLC only.) */
            FLIPUTFIELD(fliheader.aspect_dx,640);       /* Width of square rectangle. (FLC only.) */
            FLIPUTFIELD(fliheader.aspect_dy,480);       /* Height of square rectangle. (FLC only.) */
            FLIPUTFIELD(fliheader.oframe1,0);               /* Offset to frame 1. (FLC only.) */
            FLIPUTFIELD(fliheader.oframe2,0);               /* Offset to frame 2. (FLC only.) */

            ok = gwrite(gx->gstream, (char *) &fliheader, sizeof(FLIHeaderStruct));
        }

        memset(&chunk,0,16);                    /* image chunk */
        gputi(chunk+0,bitmapsize,4);
        gputi(chunk+4,0xf1fa,2);
        gputi(chunk+6,2,2);                     /* 2 subchunks */
        ok = gwrite(gx->gstream, chunk, 16);

        gputi(chunk+0,6+2+1+1+256*3,4);         /* FLITYPE_Colour256 subchunk */
        gputi(chunk+4,4,2);                     /* 256 colour pal */
        gputi(chunk+6,1,2);                     /* 1 packet */
        gputi(chunk+8,0,1);                     /* start colour */
        gputi(chunk+9,0,1);                     /* numcolours (256) */

        for (i=0; i<256; ++i)
        {
            chunk[10+i*3+0] = ginfo->colourtbl[i].r;
            chunk[10+i*3+1] = ginfo->colourtbl[i].g;
            chunk[10+i*3+2] = ginfo->colourtbl[i].b;
        }

        ok = gwrite(gx->gstream, chunk, 6+2+1+1+256*3);


        gputi(chunk+0,6+width*height,4);        /* FLITYPE_NoCompression subchunk */
        gputi(chunk+4,16,2);                    /* type */
        ok = gwrite(gx->gstream, chunk, 6);

        for (y=0; y<height && ok; ++y)
        {
            ok = gwrite(gx->gstream, source, width);
            source += rowbytes;
        }

        ++gx->framenum;
    }

    return(ok);
}

int GCALL FLI_wclose(GINSTANCE *gx)
{
    int ok=0;
    char frames[2];

    if (gx)
    {
        /* Go back and write the number of frames in the header */
        gseek(gx->gstream, 6);
        gputi(frames, gx->framenum, 2);
        gwrite(gx->gstream, frames, 2);

        ok = gfree((void *) gx);

    }
    return(ok);
}


