// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                         Line - Draw Line                         */
/*                                                                  */
/*                         by Gerry Shaw, EAC                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960209 GS based on frect                                 */
/*    1.01 000916 FB inclusive end points                           */
/*    1.02 010118 ID only on bottom layer for page type.            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* line(int x1, int y1, int x2, int y2, int colour)                 */
/*                                                                  */
/* (same prototype as eac lib)                                      */
/* Draw a line of solid colour into image.                          */
/* Can overwrite source.                                            */
/* Colour is 8 or 32 bit.                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fline.h"
#include <string.h>
#include <math.h>


/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/


#define ROUND(a)                ((int)(0.5 + a))

#define WRITEPIXEL(x,y,c)       if (x >= 0 && x < ginfo->width && y >= 0 && y < ginfo->height) \
                                    if (ginfo->bpp==8)  \
                                        *(unsigned char*)(dst + x + y*rowbytes)   = (unsigned char)c; \
                                    else \
                                        *(unsigned int*)(dst + x*4 + y*rowbytes) = (unsigned int)c

static void LINEdrawline(int x1, int y1, int x2, int y2, int value, GINFO* ginfo, char* dst, int rowbytes)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    double m  = dy/dx;

    if (dx == 0 && dy == 0)
    {
        /* single point */
        WRITEPIXEL(x1,y1, value);
    }
    else if (dx == 0) /* && (dy != 0) */
    {
        /* vertical line, m = oo */
        int y;
        for (y = gmin(y1,y2); y <= gmax(y1,y2); y++)
            WRITEPIXEL(x1,y, value);
    }
    else if (x1 <= x2 && fabs(m) <= 1.0)
    {
        double y = y1;
        int x;
        for (x = x1; x <= x2; x++)
        {
            WRITEPIXEL(x,ROUND(y), value);
            y += m;
        }
    }
    else if (x1 > x2 && fabs(m) <= 1.0)
    {
        double y = y1;
        int x;
        for (x = x1; x >= x2; x--)
        {
            WRITEPIXEL(x,ROUND(y), value);
            y -= m;
        }
    }
    else if (y1 <= y2 && fabs(m) > 1.0)
    {
        double x = x1;
        int y;
        m = 1/m;
        for (y = y1; y <= y2; y++)
        {
            WRITEPIXEL(ROUND(x),y, value);
            x += m;
        }
    }
    else if (y1 > y2 && fabs(m) > 1.0)
    {
        double x = x1;
        int y;
        m = 1/m;
        for (y = y1; y >= y2; y--)
        {
            WRITEPIXEL(ROUND(x),y, value);
            x -= m;
        }
    }
}

#undef ROUND
#undef WRITEPIXEL


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutline(void)
{
    FABOUT* about = (FABOUT*) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('L','I','N','E');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->canoverwritesource = 1;
        about->filterclass =        FCLASS_DRAW;
        strcpy(about->authorstr,    "Frank Barchard & Gerry Shaw");
        strcpy(about->versionstr,   "1.01" );
        strcpy(about->namestr,      "Line" );
        strcpy(about->longnamestr,  "Line Image" );
        about->numoptions =         5;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "X1");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           0;
        about->option[1].max.i =           0xffffffff;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Y1");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           0;
        about->option[2].max.i =           0xffffffff;
        about->option[2].def.i =           1;
        strcpy(about->option[2].name,      "X2");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           0;
        about->option[3].max.i =           0xffffffff;
        about->option[3].def.i =           1;
        strcpy(about->option[3].name,      "Y2");
        strcpy(about->option[3].enumlist,  "");

        about->option[4].type =            FOP_INT|FOP_INDEX_COLOR|FOP_TRUE_COLOR;
        about->option[4].min.i =           0;
        about->option[4].max.i =           0xffffffff;
        about->option[4].def.i =           0;
        strcpy(about->option[4].name,      "Colour");
        strcpy(about->option[4].enumlist,  "");
    }
    return about;
}

FPUBLIC int openline(FINSTANCE** filterInstance,  int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    int ok=0;
    FINSTANCE* fx=0;

    fx = (FINSTANCE*)galloc(sizeof(FINSTANCE));
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
    return ok;
}

FPUBLIC GINFO* infoline(FINSTANCE* fx, int framenum)
{
    GINFO* ginfo;

    ginfo = (GINFO*) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    return ginfo;
}

FPUBLIC int filterline(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes)
{
    char* src = (char *)fx->source[0].image;

    int x1=0;
    int y1=0;
    int x2=1;
    int y2=1;
    unsigned int color=0;

    if (fx->option)
    {
        if (fx->numoptions>=1) x1 = fx->option[0].i;
        if (fx->numoptions>=2) y1 = fx->option[1].i;
        if (fx->numoptions>=3) x2 = fx->option[2].i;
        if (fx->numoptions>=4) y2 = fx->option[3].i;
        if (fx->numoptions>=5) color = fx->option[4].i;
    }

    /* copy over bitmap, unless overwriting source */
    if (dst != src)
    {
        char* s = src;
        char* d = dst;
        int y;
        int bpl = ginfo->width*ginfo->bpp/8;

        for (y=0; y<ginfo->height; ++y)
        {
            memcpy(d,s,bpl);
            s += fx->source[0].rowbytes;
            d += rowbytes;
        }
    }

    if((fx->numoptions == 6) && (fx->option[5].i))  // if this is a pages type the only draw lines
    {                                               // on lowest layer.
        if(ginfo->framenum != 0)
            return 1;
//        else
//            printf("WARNING: Lines will only be drawn on lowest layer for page format files\n");
    }
    /* draw line */
    LINEdrawline(x1,y1,x2,y2, color, ginfo,dst,rowbytes);

    return 1;
}

FPUBLIC int closeline(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
