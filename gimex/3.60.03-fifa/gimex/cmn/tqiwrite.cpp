// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "tqigimex.h"
#include "tqi444encode.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL TQI_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL TQI_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int len;
    void *buf;
    int ok=0;
    int flags;

    if (ginfo->bpp>8)
    {
        buf = galloc(ginfo->width*ginfo->height*4 * 5/4+16384);
        if (buf)
        {
            ARGB ColorKey={ 0xff,0,0xff,0x64 };

            flags = TQI444_ALPHA | TQI444_COLOURKEY;
            if (ginfo->subtype)
                flags |= TQI444_YUV;

            len = tqi444_encode((ARGB *)source, (unsigned char *)buf,
              ginfo->width, ginfo->height, rowbytes,
              ColorKey, flags);

            if (len)
            {
                ok = gwrite(gx->gstream,buf,len);
            }
            gfree(buf);
        }
    }

    return(ok);
}

int GCALL TQI_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


