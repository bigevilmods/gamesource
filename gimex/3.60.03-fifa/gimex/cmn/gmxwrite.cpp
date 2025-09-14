// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include <math.h>

#include "gimex.h"
#include "gmxgimex.h"
#include "codex.h"
#include "huffcodex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL GMX_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok = 0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        char gmxheader[kGMXsectionSize];

        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->frames = 0;
        gx->signature=GMAKEID('G','I','M','X');
        gx->size=sizeof(GINSTANCE);
        gx->gref = 0;

        gputm(gmxheader+0, kGMXSection, 4);
        gputi(gmxheader+4, kGMXsectionSize, 4);
        gputi(gmxheader+8, 0, 4);
        gputi(gmxheader+12, 0, 4);
        gputm(gmxheader+16, GMAKEID('G',(GMX_VERSION/100)%10+'0',(GMX_VERSION/10)%10+'0',(GMX_VERSION)%10+'0'),4);
        gputi(gmxheader+20, 0, 4);

        gseek(g, 0);
        ok = gwrite(g, gmxheader, kGMXsectionSize);

        *gxptr = gx;
    }

    return(ok);
}

int GCALL GMX_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok = 0;

    if (gx)
    {
        int i, packedsize, ginfosize;
        GPOS position = gtell(gx->gstream);
        int framesize = (ginfo->width * ginfo->bpp/8) * ginfo->height;
        char *packed = NULL, *unpacked = NULL, *ginfopack = NULL;
        char gmxframe[kGMXheaderSize];
        V3GMXHeaderStruct gi;

        /* convert the GINFO to Intel format */
        memset((char*)&gi, 0, sizeof(V3GMXHeaderStruct));
        GMXPUTFIELD(gi.size, sizeof(V3GMXHeaderStruct));
        GMXPUTFIELD(gi.version, GIMEX_VERSION);
        GMXPUTFIELD(gi.framenum, gx->framenum++);       /* count frames as we go */
        GMXPUTFIELD(gi.width, ginfo->width);
        GMXPUTFIELD(gi.height, ginfo->height);
        GMXPUTFIELD(gi.bpp, ginfo->bpp);
        GMXPUTFIELD(gi.originalbpp, ginfo->originalbpp);
        GMXPUTFIELD(gi.numcolours, ginfo->numcolours);
        GMXPUTFIELD(gi.subtype, ginfo->subtype);
        GMXPUTFIELD(gi.packed, ginfo->packed);
        GMXPUTFIELD(gi.quality, ginfo->quality);
        GMXPUTFIELD(gi.framesize, ginfo->framesize);
        GMXPUTFIELD(gi.alphabits, ginfo->alphabits);
        GMXPUTFIELD(gi.redbits, ginfo->redbits);
        GMXPUTFIELD(gi.greenbits, ginfo->greenbits);
        GMXPUTFIELD(gi.bluebits, ginfo->bluebits);
        GMXPUTFIELD(gi.centerx, ginfo->centerx);
        GMXPUTFIELD(gi.centery, ginfo->centery);
        GMXPUTFIELD(gi.defaultx, ginfo->defaultx);
        GMXPUTFIELD(gi.defaulty, ginfo->defaulty);
        GMXPUTFIELD(gi.numhotspots, ginfo->numhotspots);
        GMXPUTFIELD(gi.dpi, *(int *)&ginfo->dpi); // note float endian
        GMXPUTFIELD(gi.fps, *(int *)&ginfo->fps);
        strncpy(gi.framename, ginfo->framename, gmin(sizeof(gi.framename),sizeof(ginfo->framename))-1);
        strncpy(gi.comment,   ginfo->comment,   gmin(sizeof(gi.comment),sizeof(ginfo->comment))-1);

        for (i = 0; i < 256; ++i)
        {
            unsigned char *cptr = (unsigned char*)(gi.colourtbl + i);
            cptr[0] = ginfo->colourtbl[i].b;
            cptr[1] = ginfo->colourtbl[i].g;
            cptr[2] = ginfo->colourtbl[i].r;
            cptr[3] = ginfo->colourtbl[i].a;
        }

        {
            int hottotrot=gmax(32,gmin(1024,ginfo->numhotspots));

            for (i=0; i<hottotrot; ++i)
            {
                GMXPUTFIELD(gi.hotspottbl[i][0], ginfo->hotspottbl[i][0]);
                GMXPUTFIELD(gi.hotspottbl[i][1], ginfo->hotspottbl[i][1]);
            }
        }

        ginfopack = (char *)galloc(sizeof(gi)*2+8192);
        if (ginfopack)
        {
            /* pack the GINFO header */
            ginfosize = HUFF_encode(ginfopack, (char*)&gi, sizeof(gi), 0);

            /* put the image data into the output format */
            unpacked = (char *)galloc(framesize);
            if (unpacked)
            {
                if (ginfo->bpp == 8)
                {
                    for (i = 0; i < ginfo->height; ++i)
                        memcpy(unpacked + ginfo->width * i, source + rowbytes * i, ginfo->width);
                }
                else
                {   /* convert from ARGB structures to a, r, g, b planes */
                    int planesize = ginfo->width * ginfo->height;
                    char* alpha = unpacked + planesize * 0;
                    char* red   = unpacked + planesize * 1;
                    char* green = unpacked + planesize * 2;
                    char* blue  = unpacked + planesize * 3;
                    for (i = 0; i < ginfo->height; ++i)
                    {
                        ARGB* spix = (ARGB*)(source + i * rowbytes);
                        int x;

                        for (x=0; x<ginfo->width; ++x)
                        {
                            alpha[x] = spix[x].a;
                            red[x]   = spix[x].r;
                            green[x] = spix[x].g;
                            blue[x]  = spix[x].b;
                        }

                        alpha += ginfo->width;
                        red   += ginfo->width;
                        green += ginfo->width;
                        blue  += ginfo->width;
                    }
                }

                /* pack the data and write (allow for 20% size increase) */
                // note using float to avoid int overflow on large frames
                packed = (char *)galloc((int)(framesize*1.2f + 32768.0f));
                if (packed)
                {
                    int opt=1;

                    /* clear pack buffer so that when the buffer get's expaneded to fit */
                    /* on 4 byte boundary get the same binary result everytime */
                    memset(packed, 0, (int)(framesize*1.2f + 32768.0f));

                    /* pack the frame and get the real size (delta huff) */
                    packedsize = HUFF_encode(packed, unpacked, framesize, &opt);

                    /* increase packed data size until chunk is a multiple of 4 bytes */
                    packedsize = (ginfosize + packedsize + 3) / 4 * 4 - ginfosize;

                    /* fill in the chunk header */
                    gputm(gmxframe+0, kGMXFrame, 4);
                    gputi(gmxframe+4, kGMXheaderSize + ginfosize + packedsize, 4);
                    gputi(gmxframe+8, ginfosize, 4);

                    /* write the chunk header, the GINFO, the image size, and the image  */
                    ok = gwrite(gx->gstream, gmxframe, kGMXheaderSize);     /* write chunk header */
                    ok = ok && gwrite(gx->gstream, ginfopack, ginfosize);   /* write packed GINFO */
                    ok = ok && gwrite(gx->gstream, packed, packedsize);     /* write packed data */

                    if (ok)
                        ++gx->frames;                   /* increment #frames written */


                    else
                        gseek(gx->gstream, position);       /* seek back to where we started */
                }
            }
        }

        ok = (ginfopack != NULL) ? (gfree(ginfopack) && ok) : 0;
        ok = (unpacked != NULL) ? (gfree(unpacked) && ok) : 0;
        ok = (packed != NULL)   ? (gfree(packed) && ok) : 0;
    }

    return ok;
}


int GCALL GMX_wclose(GINSTANCE *gx)
{
    int ok = 0;

    if (gx)
    {
        /* update the section size */
        char gmxheader[kGMXsectionSize];

        gputm(gmxheader+0, kGMXSection, 4);
        gputi(gmxheader+4, kGMXsectionSize, 4);
        gputi(gmxheader+8, (unsigned int) glen(gx->gstream), 4);
        gputi(gmxheader+12, gx->frames, 4);
        gputm(gmxheader+16, GMAKEID('G',(GMX_VERSION/100)%10+'0',(GMX_VERSION/10)%10+'0',(GMX_VERSION)%10+'0'),4);
        gputi(gmxheader+20, (unsigned int) (glen(gx->gstream)>>32), 4);

        gseek(gx->gstream, 0);
        ok = gwrite(gx->gstream, gmxheader, kGMXsectionSize);

        /* deallocate and leave */
        gfree((void *) gx);
    }

    return(ok);
}
