// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "ramgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

extern void (*GIMEX_freeram)(void);

void RAM_freeram(void)
{
    RAMFILE *r;
    r = ramfiles;
    while (r)
    {
        RAMFRAME *ramframe = r->frame;
        while (ramframe)
        {
            if (ramframe->ginfo)
                gfree(ramframe->ginfo);
            if (ramframe->image)
                gfree(ramframe->image);
            RAMFRAME *ramframenext = ramframe->next;
            gfree(ramframe);
            ramframe = ramframenext;
        }

        RAMFILE *rnext=r->next;
        gfree(r);
        r = rnext;
    }
}

int GCALL RAM_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        RAMFILE *r;
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;

        r = (RAMFILE *) galloc(sizeof(RAMFILE));
        memset(r,0,sizeof(RAMFILE));
        strncpy(r->name,pathname,RAMMAXFILENAME);
        r->framenum = 0;
        r->frame = NULL;
        r->next = ramfiles;
        if (!ramfiles)
            GIMEX_freeram = RAM_freeram;
        ramfiles = r;

        gx->gref = (void *) r;
        *gxptr = gx;

        ok = 1;
    }
    return(ok);
}

int GCALL RAM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    RAMFILE *r = (RAMFILE *) gx->gref;
    int ok=0;
    if (r)
    {
        ++r->framenum;

        RAMFRAME *ramframe = (RAMFRAME *) galloc(sizeof(RAMFRAME));
        if (ramframe)
        {
            memset(ramframe,0,sizeof(RAMFRAME));

            ramframe->ginfo = (GINFO *) galloc(ginfo->size);
            if (ramframe->ginfo)
                memcpy(ramframe->ginfo,ginfo,ginfo->size);

            int ramrowbytes=ginfo->width*ginfo->bpp/8;
            ramframe->image = galloc(ramrowbytes*ginfo->height);
            if (ramframe->image)
            {
                for (int y=0; y<ginfo->height; ++y)
                {
                    memcpy((char *)ramframe->image + ramrowbytes*y,
                           (char *)source + rowbytes*y,
                           ramrowbytes);
                }
            }
            ramframe->next = r->frame;
            r->frame = ramframe;
            if (ramframe->ginfo && ramframe->image)
                ok = 1;
        }
    }

    return(ok>0);
}

int GCALL RAM_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


