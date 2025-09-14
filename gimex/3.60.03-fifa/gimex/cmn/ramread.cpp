// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   RAM - Ram Image Cache Gimex                    */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 000304 FB Save/Restore temporary images                  */
/*    1.01 020211 FB import/export implemented                      */
/*    1.02 020213 FB nameless import                                */
/*    1.03 030325 FB 'ShpR' signature for shape family subtypes     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit import/export                                           */
/* Non-Reentrant                                                    */
/* Frame buffered                                                   */
/* Files: ramread.cpp ramwrite.cpp ramgimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Stores multiframe ram files by name                              */
/* Respects ginfo->size for extended ginfo                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "ramgimex.h"
#include "shpgimex.h"

RAMFILE *ramfiles=0;

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL RAM_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('S','h','p','R');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 0;    /* module is file based */
        info->globalpalette   = 0;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name character */
        info->resizable       = 0;    /* read respects ginfo width and height */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('S','h','p','R');   /* mac file system types used */
        strcpy(info->extensions[0], ".ram");                /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.04");                /* version # */
        strcpy(info->shorttypestr,  "Ram");                 /* type */
        strcpy(info->wordtypestr,   "Ram");                 /* wordtype (use this filename to get 8 bit ginfo) */
        strcpy(info->longtypestr,   "Ram Shape");           /* longtype */
    }
    return(info);
}

int GCALL RAM_is(GSTREAM *g)
{
    int ok=0;
    if (ramfiles)
        ok = 40;
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL RAM_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->signature=GMAKEID('S','h','p','R');
        gx->size=sizeof(GINSTANCE);
        gx->gref = NULL;

        RAMFILE *r;
        r = ramfiles;

        if (strcmp("ram",pathname))
        {
            while (r)
            {
                if (!strncmp(r->name,pathname,RAMMAXFILENAME))
                    break;
                r = r->next;
            }
        }
        if (r)
        {
            gx->gref = (void *) r;
            gx->frames = ok = r->framenum;
        }
    }

    return(ok);
}

GINFO * GCALL RAM_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    RAMFILE *r = (RAMFILE *) gx->gref;

    if (r && framenum<r->framenum)
    {
        RAMFRAME *ramframe = r->frame;

        for (int i=framenum; i<(r->framenum-1); ++i) // frames are stored in backwards order
        {
            ramframe = ramframe->next;
        }

        if (ramframe && ramframe->ginfo)
        {
            ginfo = (GINFO *) galloc(sizeof(GINFOSHP));//ramframe->ginfo->size);
            if (ginfo)
            {
                memset(ginfo, 0, sizeof(GINFOSHP));
                memcpy(ginfo,ramframe->ginfo,gmin((int)sizeof(GINFOSHP),ramframe->ginfo->size));
				ginfo->signature        = GMAKEID('S','h','p','R');
                ginfo->size             = sizeof(GINFOSHP);          /* Length of GINFO */
                ginfo->version          = GIMEX_VERSION;
                ginfo->framenum			= framenum;
				if (ginfo->packed==1)
					ginfo->packed = 0;
			}
        }
    }

    return(ginfo);
}

int GCALL RAM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    RAMFILE *r = (RAMFILE *) gx->gref;
    int framenum = ginfo->framenum;
    int ok=0;

    if (r && framenum<r->framenum)
    {
        RAMFRAME *ramframe = r->frame;
        for (int i=framenum; i<(r->framenum-1); ++i) // frames are stored in backwards order
        {
            ramframe = ramframe->next;
        }

        if (ramframe && ramframe->image)
        {
            int ramrowbytes=ginfo->width*ginfo->bpp/8;
            for (int y=0; y<ginfo->height; ++y)
            {
                memcpy((char *)dest + rowbytes*y,
                       (char *)ramframe->image + ramrowbytes*y,
                       ramrowbytes);
            }
            ok=1;
        }
    }
    return(ok);
}

int GCALL RAM_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        gx->gref = 0;
        ok = gfree((void *) gx);
    }
    return(ok);
}

