// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*         Rotate - Rotate Image by Multiplies of 90 Degrees        */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 951101 FB based on fwrap                                 */
/*    1.01 951103 FB 32 bit images working                          */
/*    1.02 010118 ID fix defaultx & y calc for page type.           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* rot(int numrotations)                                            */
/* int numrotations;   0..3 (negatives ok)                          */
/*                                                                  */
/* Rotate by multiple of 90 degrees                                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "frot.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

void rotatecoord(int *xy)
{
    int x;
    int y;
    x = xy[0];
    y = xy[1];
    xy[0] = -y;
    xy[1] = x;
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutrot(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','R','O','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Rot");
        strcpy(about->longnamestr,  "Rotate");
        about->numoptions =         1;

        about->option[0].type =            FOP_INT | FOP_ENUM;
        about->option[0].min.i =           0;
        about->option[0].max.i =           3;
        about->option[0].def.i =           1;
        strcpy(about->option[0].name,      "Rot");
        strcpy(about->option[0].enumlist,  "0,90,180,-90");
    }
    return(about);
}

FPUBLIC int openrot(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    /* allocate instance */
    fx = (FINSTANCE *)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        ok = 1;

    }
    *filterInstance = fx;
    return(ok);
}

FPUBLIC GINFO *inforot(FINSTANCE *fx, int framenum)
{
    int i;
    int j;
    GINFO *ginfo;
    GINFO cen;
    GINFO newginfo;
    int width;
    int psdFrameHeight = 0;
    int psdFrameWidth  = 0;
    int typepage=0;
    unsigned int rot=1;

    if (fx->option)
    {
        if (fx->numoptions>=1) rot = fx->option[0].i;
        if(fx->numoptions == 4)
        {
            psdFrameHeight = fx->option[1].i;
            psdFrameWidth  = fx->option[2].i;
            typepage = fx->option[3].i;
        }
        else
        {
            psdFrameWidth  = 0;//fx->source[0].ginfo->width;
            psdFrameHeight = 0;//fx->source[0].ginfo->height;
            typepage = 0;
        }
    }
    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        newginfo = *ginfo;
        if (typepage)
        {
            switch(rot&3)
            {
                case 0:
                            newginfo.defaultx = ginfo->defaultx;
                            newginfo.defaulty = ginfo->defaulty;
                            break;

                case 1:
                            newginfo.defaultx = psdFrameHeight - (ginfo->height + ginfo->defaulty);
                            newginfo.defaulty = ginfo->defaultx;
                            break;
                case 2:
                            newginfo.defaultx = psdFrameWidth -  (ginfo->width + ginfo->defaultx);
                            newginfo.defaulty = psdFrameHeight - (ginfo->height + ginfo->defaulty);
                            break;
                case 3:
                            newginfo.defaultx = ginfo->defaulty;
                            newginfo.defaulty = psdFrameWidth - (ginfo->width + ginfo->defaultx);
                            break;
            }
        }
        for (j=0; (unsigned int)j<((rot&3)); ++j)
        {
            width = ginfo->height;
            ginfo->height = ginfo->width;
            ginfo->width  = width;

            /* convert to centered */

            cen.defaultx = -ginfo->centerx;
            cen.defaulty = -ginfo->centery;
            rotatecoord(&cen.defaultx);
            if(!typepage)
            {
                newginfo.defaultx = ginfo->defaultx+ginfo->centerx+cen.defaultx-(ginfo->width-1);
                newginfo.defaulty = ginfo->defaulty+ginfo->centery+cen.defaulty;
            }
            newginfo.centerx = ginfo->defaultx+ginfo->centerx-newginfo.defaultx;
            newginfo.centery = ginfo->defaulty+ginfo->centery-newginfo.defaulty;

            for (i=0; i<ginfo->numhotspots; ++i)
            {
                cen.hotspottbl[i][0] = ginfo->hotspottbl[i][0]-ginfo->centerx;
                cen.hotspottbl[i][1] = ginfo->hotspottbl[i][1]-ginfo->centery;
                rotatecoord(cen.hotspottbl[i]);

                newginfo.hotspottbl[i][0] = ginfo->defaultx+ginfo->centerx+cen.hotspottbl[i][0]-newginfo.defaultx;
                newginfo.hotspottbl[i][1] = ginfo->defaulty+ginfo->centery+cen.hotspottbl[i][1]-newginfo.defaulty;
            }

            ginfo->defaultx = newginfo.defaultx;
            ginfo->defaulty = newginfo.defaulty;
            ginfo->centerx  = newginfo.centerx;
            ginfo->centery  = newginfo.centery;
            for (i=0; i<ginfo->numhotspots; ++i)
            {
                ginfo->hotspottbl[i][0] = newginfo.hotspottbl[i][0];
                ginfo->hotspottbl[i][1] = newginfo.hotspottbl[i][1];
            }
        }
    }
    return(ginfo);
}

FPUBLIC int filterrot(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    int sx=0;
    int sy=0;
    char *s;
    char *d;
    int  width  = ginfo->width;
    int  height = ginfo->height;
    int  bpp    = ginfo->bpp;
    unsigned int rot=1;


    if (fx->option)
    {
        if (fx->numoptions>=1) rot = fx->option[0].i;
    }

    for (y=0; y<height; ++y)
    {
        d = dest;
        for (x=0; x<width; ++x)
        {
            switch (rot&3)
            {
                case 0:
                    sx = x;
                    sy = y;
                    break;

                case 1:
                    sx = y;
                    sy = fx->source[0].ginfo->height-1-x;
                    break;

                case 2:
                    sx = fx->source[0].ginfo->width-1-x;
                    sy = fx->source[0].ginfo->height-1-y;
                    break;

                case 3:
                    sx = fx->source[0].ginfo->width-1-y;
                    sy = x;
                    break;
            }

            s = (char *)fx->source[0].image+sy*fx->source[0].rowbytes+sx*bpp/8;

            if (bpp==8)
                *d++ = *s;
            else
            {
                *(int *)d = *(int *)s;
                d += 4;
            }
        }
        dest += rowbytes;
    }
    return(1);
}

FPUBLIC int closerot(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

