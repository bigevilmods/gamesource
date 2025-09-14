// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "sgigimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static int SGIpackrow(unsigned char *s,unsigned char *d,int width)
{
    int     len;
    int     count;

    len = 0;

    while (width > 0)
    {
        /* don't run-length encode unless we have > 2 equal values in a row */

        if ((width > 2) && (s[0*4] == s[1*4]) && (s[1*4] == s[2*4]))
        {
            for (count=2; count < width; count++)
            {
                if (s[count*4] != s[(count-1)*4])
                    break;
                if (count >= 127)
                    break;
            }

            /* write out count */
            *d++ = (unsigned char)count;
            len++;

            /* write out value */
            *d++ = *s;
            len++;
            width -= count;
            s += count*4;
        }
        else
        {
            for (count=1; count < width; count++)
            {
                /* don't run-length encode unless we have > 2 equal values in a row */
                if ((width-count > 2) && (s[count*4] == s[(count+1)*4]) && (s[(count+1)*4] == s[(count+2)*4]))
                    break;
                if (count >= 127)
                    break;
            }

            /* write out count */
            *d++ = (unsigned char)(0x80+count);
            len++;

            /* copy string of pixels */
            for (; count-- > 0; len++, width--)
            {
                *d++ = *s;
                s += 4;
            }
        }
    }
    *d++ = 0;
    ++len;
    return(len);
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/


int GCALL SGI_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL SGI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int width        = 0;
    int height       = 0;
    int channel      = 0;
    int channels     = 0;
    int y            = 0;
    int ok           = 0;
    int packedoffset = 0;
    int packedlen    = 0;
    char *offsettbl  = 0;
    char *buf        = 0;
    char *linebase   = 0;
    char *s          = 0;
    SGIHeaderStruct sgiheader;

    width = ginfo->width;
    height = ginfo->height;
    channels = ginfo->alphabits?4:3;

    /* SGI FILE HEADER structure 512 bytes */

    memset(&sgiheader,0,sizeof(sgiheader));

    /* SGI FILE HEADER structure 512 bytes */

    SGIPUTFIELD(sgiheader.magic,474);             /* IRIS image file magic number (474) */
    SGIPUTFIELD(sgiheader.storage,1);             /* Storage format 0=verbatim, 1=rle */
    SGIPUTFIELD(sgiheader.bpc,1);                 /* Bytes per pixel component (1or2) */
    SGIPUTFIELD(sgiheader.dimension,3);           /* Number of dimensions 1-3 */
    SGIPUTFIELD(sgiheader.xsize,width);           /* X size in pixels */
    SGIPUTFIELD(sgiheader.ysize,height);          /* Y size in pixels */
    SGIPUTFIELD(sgiheader.zsize,channels);        /* Number of channels */
    SGIPUTFIELD(sgiheader.pixmin,0);              /* Minimum pixel value (0) */
    SGIPUTFIELD(sgiheader.pixmax,255);            /* Maximum pixel value (255) */
    SGIPUTFIELD(sgiheader.dummy,0);               /* Ignored */
    strcpy(sgiheader.imagename,ginfo->framename); /* Image name */
    SGIPUTFIELD(sgiheader.colormap,0);            /* Colour map id 0=normal, 1=332, 2=indexed, 3=palette */


    if (gwrite(gx->gstream, (char *) &sgiheader, 512))
    {

        /* allocate scanline offset table */

        offsettbl = (char *) galloc(height*channels*8);
        if (offsettbl)
        {
            memset(offsettbl,255,height*channels*8);

            buf = (char *) galloc(width*2);
            if (buf)
            {
                /* write offset table as place holder */

                ok = gwrite(gx->gstream, (char *) offsettbl, height*channels*8);
            }
        }
    }
    if (ok)
    {
        for (channel=0; channel<channels; ++channel)
        {
            for (y=0; y<height && ok; ++y)
            {
                linebase = source + rowbytes*(height-1-y);

                switch (channel)
                {
                    case 0: s=(char*)&((ARGB *)linebase)->r; break;
                    case 1: s=(char*)&((ARGB *)linebase)->g; break;
                    case 2: s=(char*)&((ARGB *)linebase)->b; break;
                    case 3: s=(char*)&((ARGB *)linebase)->a; break;
                    default: ;
                }

                /* record location in file */

                packedoffset = (int)gtell(gx->gstream);

                packedlen = SGIpackrow((unsigned char*)s,(unsigned char*)buf,width);

                ok = gwrite(gx->gstream,buf,packedlen);

                gputm(offsettbl+(channel*height+y)*4,packedoffset,4);
                gputm(offsettbl+((channel+channels)*height+y)*4,packedlen,4);
            }
        }

        /* seek back to offset table, and write values */

        gseek(gx->gstream, 512L);
        ok = gwrite(gx->gstream, offsettbl, height*channels*8);
    }
    if (buf) gfree(buf);
    if (offsettbl) gfree(offsettbl);
    return(ok);
}

int GCALL SGI_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


