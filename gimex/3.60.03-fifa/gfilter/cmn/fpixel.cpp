// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 Pixel - Pixel Expression Filter                  */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 970708 FB based on fxcolour and -cx expr                 */
/*    1.01 971201 FB index expression to apply to 8 bit image       */
/*    1.02 990210 FB expressions are float now                      */
/*    1.03 990210 FB fps,dpi                                        */
/*    1.04 000211 FB halfalpha/halfpixel shortcuts                  */
/*    1.05 000225 FB halfalpha/halfpixel rounded                    */
/*    1.06 000613 FB ranger based                                   */
/*    1.07 000918 FB x/y expressions for remapping                  */
/*    1.08 010515 FB auxilary image                                 */
/*    1.09 010623 FB 20 hotspots                                    */
/*    1.10 010702 FB doublealpha shortcut                           */
/*    1.11 020228 FB fix check for a_ palette                       */
/*    1.12 020314 FB early out if error in expression               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* pixel(EXPR *expr,char *a,char *r,char *g,char *b,char *i)        */
/*                                                                  */
/* Applys general expressions to pixels using EXPR expression API.  */
/*                                                                  */
/* Symbols available:                                               */
/* a,r,g,b,x,y,i,argb,frame,framenum,width,height,bpp,startcolour,  */
/* numcolours,colours,subtype,packed,quality,framesize,alphabits,   */
/* redbits,greenbits,bluebits,centerx,centery,defaultx,defaulty,    */
/* numhotspots,hot0x,hot0y,hot1x,hot1y,hot2x,hot2y,hot3x,hot3y,     */
/* hot4x,hot4y,hot5x,hot5y,hot6x,hot6y,hot7x,hot7y,hot8x,hot8y,     */
/* hot9x,hot9y,hot10x,hot10y,hot11x,hot11y,hot12x,hot12y,hot13x,    */
/* hot13y,hot14x,hot14y,hot15x,hot15y,hot16x,hot16y,hot17x,hot17y,  */
/* hot18x,hot18y,hot19x,hot19y,adr,rowbytes,fps,dpi                 */
/*                                                                  */
/* a_* for auxillary image                                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include <stdio.h>
#include "fpixel.h"
#include "expr.h"
#include "ranger.h"

#if defined(__PPCBROADWAY__) || defined(_R5900)
#define uintptr_t unsigned int
#define intptr_t  int
#endif

#define SPECIALEXPR_NONE 0
#define SPECIALEXPR_HALFALPHA 1
#define SPECIALEXPR_HALFPIXEL 2
#define SPECIALEXPR_HALFALPHAROUND 3
#define SPECIALEXPR_HALFPIXELROUND 4
#define SPECIALEXPR_DOUBLEALPHAROUND 5

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

static double PIXEL_asciitonumber(const char *framename)
{
    double fn=0;
    for (int i=0; i<(int)strlen(framename) && i<8; ++i)
        fn = fn*256+(double)(unsigned char)(framename[i]);
    return fn;
}

static void PIXEL_setginfo(EXPR *expr,GINFO *ginfo,int ispal, const void *image, int rowbytes, GINFO *ginfo1, const void *image1, int rowbytes1)
{
    EXPR_set(expr,"ispal",ispal);
    EXPR_set(expr,"adr",(intptr_t)image);
    EXPR_set(expr,"rowbytes",rowbytes);
    EXPR_set(expr,"frame",ginfo->framenum);
    EXPR_set(expr,"framenum",ginfo->framenum);
    EXPR_set(expr,"framename",PIXEL_asciitonumber(ginfo->framename));
    EXPR_set(expr,"width",ginfo->width);
    EXPR_set(expr,"height",ginfo->height);
    EXPR_set(expr,"bpp",ginfo->originalbpp);
    EXPR_set(expr,"startcolour",ginfo->startcolour);
    EXPR_set(expr,"numcolours",ginfo->numcolours);
    EXPR_set(expr,"colours",ginfo->numcolours);
    EXPR_set(expr,"subtype",ginfo->subtype);
    EXPR_set(expr,"packed",ginfo->packed);
    EXPR_set(expr,"quality",ginfo->quality);
    EXPR_set(expr,"framesize",ginfo->framesize);
    EXPR_set(expr,"alphabits",ginfo->alphabits);
    EXPR_set(expr,"redbits",ginfo->redbits);
    EXPR_set(expr,"greenbits",ginfo->greenbits);
    EXPR_set(expr,"bluebits",ginfo->bluebits);
    EXPR_set(expr,"centerx",ginfo->centerx);
    EXPR_set(expr,"centery",ginfo->centery);
    EXPR_set(expr,"defaultx",ginfo->defaultx);
    EXPR_set(expr,"defaulty",ginfo->defaulty);
    EXPR_set(expr,"fps",ginfo->fps);
    EXPR_set(expr,"dpi",ginfo->dpi);
    EXPR_set(expr,"numhotspots",ginfo->numhotspots);
    EXPR_set(expr,"hot0x",ginfo->hotspottbl[0][0]);
    EXPR_set(expr,"hot0y",ginfo->hotspottbl[0][1]);
    EXPR_set(expr,"hot1x",ginfo->hotspottbl[1][0]);
    EXPR_set(expr,"hot1y",ginfo->hotspottbl[1][1]);
    EXPR_set(expr,"hot2x",ginfo->hotspottbl[2][0]);
    EXPR_set(expr,"hot2y",ginfo->hotspottbl[2][1]);
    EXPR_set(expr,"hot3x",ginfo->hotspottbl[3][0]);
    EXPR_set(expr,"hot3y",ginfo->hotspottbl[3][1]);
    EXPR_set(expr,"hot4x",ginfo->hotspottbl[4][0]);
    EXPR_set(expr,"hot4y",ginfo->hotspottbl[4][1]);
    EXPR_set(expr,"hot5x",ginfo->hotspottbl[5][0]);
    EXPR_set(expr,"hot5y",ginfo->hotspottbl[5][1]);
    EXPR_set(expr,"hot6x",ginfo->hotspottbl[6][0]);
    EXPR_set(expr,"hot6y",ginfo->hotspottbl[6][1]);
    EXPR_set(expr,"hot7x",ginfo->hotspottbl[7][0]);
    EXPR_set(expr,"hot7y",ginfo->hotspottbl[7][1]);
    EXPR_set(expr,"hot8x",ginfo->hotspottbl[8][0]);
    EXPR_set(expr,"hot8y",ginfo->hotspottbl[8][1]);
    EXPR_set(expr,"hot9x",ginfo->hotspottbl[9][0]);
    EXPR_set(expr,"hot9y",ginfo->hotspottbl[9][1]);
    EXPR_set(expr,"hot10x",ginfo->hotspottbl[10][0]);
    EXPR_set(expr,"hot10y",ginfo->hotspottbl[10][1]);
    EXPR_set(expr,"hot11x",ginfo->hotspottbl[11][0]);
    EXPR_set(expr,"hot11y",ginfo->hotspottbl[11][1]);
    EXPR_set(expr,"hot12x",ginfo->hotspottbl[12][0]);
    EXPR_set(expr,"hot12y",ginfo->hotspottbl[12][1]);
    EXPR_set(expr,"hot13x",ginfo->hotspottbl[13][0]);
    EXPR_set(expr,"hot13y",ginfo->hotspottbl[13][1]);
    EXPR_set(expr,"hot14x",ginfo->hotspottbl[14][0]);
    EXPR_set(expr,"hot14y",ginfo->hotspottbl[14][1]);
    EXPR_set(expr,"hot15x",ginfo->hotspottbl[15][0]);
    EXPR_set(expr,"hot15y",ginfo->hotspottbl[15][1]);
    EXPR_set(expr,"hot16x",ginfo->hotspottbl[16][0]);
    EXPR_set(expr,"hot16y",ginfo->hotspottbl[16][1]);
    EXPR_set(expr,"hot17x",ginfo->hotspottbl[17][0]);
    EXPR_set(expr,"hot17y",ginfo->hotspottbl[17][1]);
    EXPR_set(expr,"hot18x",ginfo->hotspottbl[18][0]);
    EXPR_set(expr,"hot18y",ginfo->hotspottbl[18][1]);
    EXPR_set(expr,"hot19x",ginfo->hotspottbl[19][0]);
    EXPR_set(expr,"hot19y",ginfo->hotspottbl[19][1]);


    if (ginfo1)
    {
        EXPR_set(expr,"a_adr",(intptr_t)image1);
        EXPR_set(expr,"a_rowbytes",rowbytes1);
        EXPR_set(expr,"a_frame",ginfo1->framenum);
        EXPR_set(expr,"a_framenum",ginfo1->framenum);
        EXPR_set(expr,"a_framename",PIXEL_asciitonumber(ginfo1->framename));
        EXPR_set(expr,"a_width",ginfo1->width);
        EXPR_set(expr,"a_height",ginfo1->height);
        EXPR_set(expr,"a_bpp",ginfo1->originalbpp);
        EXPR_set(expr,"a_startcolour",ginfo1->startcolour);
        EXPR_set(expr,"a_numcolours",ginfo1->numcolours);
        EXPR_set(expr,"a_colours",ginfo1->numcolours);
        EXPR_set(expr,"a_subtype",ginfo1->subtype);
        EXPR_set(expr,"a_packed",ginfo1->packed);
        EXPR_set(expr,"a_quality",ginfo1->quality);
        EXPR_set(expr,"a_framesize",ginfo1->framesize);
        EXPR_set(expr,"a_alphabits",ginfo1->alphabits);
        EXPR_set(expr,"a_redbits",ginfo1->redbits);
        EXPR_set(expr,"a_greenbits",ginfo1->greenbits);
        EXPR_set(expr,"a_bluebits",ginfo1->bluebits);
        EXPR_set(expr,"a_centerx",ginfo1->centerx);
        EXPR_set(expr,"a_centery",ginfo1->centery);
        EXPR_set(expr,"a_defaultx",ginfo1->defaultx);
        EXPR_set(expr,"a_defaulty",ginfo1->defaulty);
        EXPR_set(expr,"a_fps",ginfo1->fps);
        EXPR_set(expr,"a_dpi",ginfo1->dpi);
        EXPR_set(expr,"a_numhotspots",ginfo1->numhotspots);
        EXPR_set(expr,"a_hot0x",ginfo1->hotspottbl[0][0]);
        EXPR_set(expr,"a_hot0y",ginfo1->hotspottbl[0][1]);
        EXPR_set(expr,"a_hot1x",ginfo1->hotspottbl[1][0]);
        EXPR_set(expr,"a_hot1y",ginfo1->hotspottbl[1][1]);
        EXPR_set(expr,"a_hot2x",ginfo1->hotspottbl[2][0]);
        EXPR_set(expr,"a_hot2y",ginfo1->hotspottbl[2][1]);
        EXPR_set(expr,"a_hot3x",ginfo1->hotspottbl[3][0]);
        EXPR_set(expr,"a_hot3y",ginfo1->hotspottbl[3][1]);
        EXPR_set(expr,"a_hot4x",ginfo1->hotspottbl[4][0]);
        EXPR_set(expr,"a_hot4y",ginfo1->hotspottbl[4][1]);
        EXPR_set(expr,"a_hot5x",ginfo1->hotspottbl[5][0]);
        EXPR_set(expr,"a_hot5y",ginfo1->hotspottbl[5][1]);
        EXPR_set(expr,"a_hot6x",ginfo1->hotspottbl[6][0]);
        EXPR_set(expr,"a_hot6y",ginfo1->hotspottbl[6][1]);
        EXPR_set(expr,"a_hot7x",ginfo1->hotspottbl[7][0]);
        EXPR_set(expr,"a_hot7y",ginfo1->hotspottbl[7][1]);
        EXPR_set(expr,"a_hot8x",ginfo1->hotspottbl[8][0]);
        EXPR_set(expr,"a_hot8y",ginfo1->hotspottbl[8][1]);
        EXPR_set(expr,"a_hot9x",ginfo1->hotspottbl[9][0]);
        EXPR_set(expr,"a_hot9y",ginfo1->hotspottbl[9][1]);
        EXPR_set(expr,"a_hot10x",ginfo1->hotspottbl[10][0]);
        EXPR_set(expr,"a_hot10y",ginfo1->hotspottbl[10][1]);
        EXPR_set(expr,"a_hot11x",ginfo1->hotspottbl[11][0]);
        EXPR_set(expr,"a_hot11y",ginfo1->hotspottbl[11][1]);
        EXPR_set(expr,"a_hot12x",ginfo1->hotspottbl[12][0]);
        EXPR_set(expr,"a_hot12y",ginfo1->hotspottbl[12][1]);
        EXPR_set(expr,"a_hot13x",ginfo1->hotspottbl[13][0]);
        EXPR_set(expr,"a_hot13y",ginfo1->hotspottbl[13][1]);
        EXPR_set(expr,"a_hot14x",ginfo1->hotspottbl[14][0]);
        EXPR_set(expr,"a_hot14y",ginfo1->hotspottbl[14][1]);
        EXPR_set(expr,"a_hot15x",ginfo1->hotspottbl[15][0]);
        EXPR_set(expr,"a_hot15y",ginfo1->hotspottbl[15][1]);
        EXPR_set(expr,"a_hot16x",ginfo1->hotspottbl[16][0]);
        EXPR_set(expr,"a_hot16y",ginfo1->hotspottbl[16][1]);
        EXPR_set(expr,"a_hot17x",ginfo1->hotspottbl[17][0]);
        EXPR_set(expr,"a_hot17y",ginfo1->hotspottbl[17][1]);
        EXPR_set(expr,"a_hot18x",ginfo1->hotspottbl[18][0]);
        EXPR_set(expr,"a_hot18y",ginfo1->hotspottbl[18][1]);
        EXPR_set(expr,"a_hot19x",ginfo1->hotspottbl[19][0]);
        EXPR_set(expr,"a_hot19y",ginfo1->hotspottbl[19][1]);
    }
}

static __inline unsigned char PIXEL_get(RANGER *r, unsigned char v)
{
    int pixelvalue = (int)RANGER_closest(r, (double) (int) v, RANGER_ROUND_CLOSEST);

    return (unsigned char)(gmax(0,gmin(255,pixelvalue)));
}

static bool Xpixel32(ARGB *d,const ARGB *source,int bpl,int n,int height,EXPR *expr,const char *expra,const char *exprr,const char *exprg,const char *exprb,const char *exprx,const char *expry,int y, int specialexpr, const ARGB *source1,int width1, int height1, int bpl1, int bpp1, ARGB *colourtbl1)
{
    int x;
    bool ok=false;
    const ARGB *s = (ARGB *)((const char *) source + y*bpl);

    switch (specialexpr)
    {
        default:
        case SPECIALEXPR_NONE:
        {
            unsigned int argb;
            RANGER *ra = RANGER_open(expr, expra, 0.0, 255.0);
            RANGER *rr = RANGER_open(expr, exprr, 0.0, 255.0);
            RANGER *rg = RANGER_open(expr, exprg, 0.0, 255.0);
            RANGER *rb = RANGER_open(expr, exprb, 0.0, 255.0);
            RANGER *rx = RANGER_open(expr, exprx, 0.0, (double) n);
            RANGER *ry = RANGER_open(expr, expry, 0.0, (double) height);

            if (ra && rr && rg && rb && rx && ry)
            {
                ok = true;
                int mapx=0;

                if (strcmp(exprx,"x") || strcmp(expry,"y"))
                    mapx = 1;

                EXPR_set(expr,"y",y);
                for (x=0; x<n; ++x)
                {
                    int isx;
                    int isy;

                    isx = x;
                    isy = y;
                    if (mapx)
                    {
                        double sx = RANGER_closest(rx, (double) x, RANGER_ROUND_CLOSEST);
                        double sy = RANGER_closest(ry, (double) y, RANGER_ROUND_CLOSEST);
                        isx = (int)(sx+0.5);
                        isy = (int)(sy+0.5);
                        if (isx<0) isx = 0;
                        if (isy<0) isy = 0;
                        if (isx>=n) isx = n-1;
                        if (isy>=height) isy = height-1;
                    }
                    s = (ARGB *)((const char *) source + isy*bpl+isx*4);

                    if (source1)
                    {
                        const ARGB *s1 = NULL;

                        if (isx>=width1)  isx = width1-1;
                        if (isy>=height1) isy = height1-1;

                        if (bpp1==8)
                            s1 = colourtbl1+*((unsigned char *) source1 + isy*bpl1+isx*1);
                        else
                            s1 = (ARGB *)((const char *) source1 + isy*bpl1+isx*4);

                        argb = GMAKEID(s1->a,s1->r,s1->g,s1->b);
                        EXPR_set(expr,"a_a",(double) s1->a);
                        EXPR_set(expr,"a_r",(double) s1->r);
                        EXPR_set(expr,"a_g",(double) s1->g);
                        EXPR_set(expr,"a_b",(double) s1->b);
                        EXPR_set(expr,"a_argb",(double) argb);
                    }

                    argb = GMAKEID(s->a,s->r,s->g,s->b);
                    EXPR_set(expr,"a",(double) s->a);
                    EXPR_set(expr,"r",(double) s->r);
                    EXPR_set(expr,"g",(double) s->g);
                    EXPR_set(expr,"b",(double) s->b);
                    EXPR_set(expr,"x",(double) x);
                    EXPR_set(expr,"i",(double) x);
                    EXPR_set(expr,"argb",(double) argb);


                    if (strcmp(expra,"a"))
                    { d->a = PIXEL_get(ra, s->a);  if (ra->error) { ok = false; break; }}
                    else d->a = s->a;

                    if (strcmp(exprr,"r")) { d->r = PIXEL_get(rr, s->r);  if (rr->error) { ok = false; break; }} else d->r = s->r;
                    if (strcmp(exprg,"g")) { d->g = PIXEL_get(rg, s->g);  if (rg->error) { ok = false; break; }} else d->g = s->g;
                    if (strcmp(exprb,"b")) { d->b = PIXEL_get(rb, s->b);  if (rb->error) { ok = false; break; }} else d->b = s->b;

                    ++d;
                }
            }
            if (ry) RANGER_close(ry);
            if (rx) RANGER_close(rx);
            if (ra) RANGER_close(ra);
            if (rr) RANGER_close(rr);
            if (rg) RANGER_close(rg);
            if (rb) RANGER_close(rb);
            break;
        }

        case SPECIALEXPR_HALFALPHA:
            for (x=0; x<n; ++x)
            {
                d->a = (unsigned char) (s->a>>1);
                d->r = s->r;
                d->g = s->g;
                d->b = s->b;
                ++s;
                ++d;
            }
            ok = true;
            break;

        case SPECIALEXPR_HALFPIXEL:
            for (x=0; x<n; ++x)
            {
                *(int *) d= ((*(int *) s) >> 1) & 0x7f7f7f7f;
                ++s;
                ++d;
            }
            ok = true;
            break;

        case SPECIALEXPR_HALFALPHAROUND:
        {
            for (x=0; x<n; ++x)
            {
                d->a = (unsigned char)(((int)s->a+1)>>1);
                d->r = s->r;
                d->g = s->g;
                d->b = s->b;
                ++s;
                ++d;
            }
            ok = true;
            break;
        }

        case SPECIALEXPR_HALFPIXELROUND:
        {
            unsigned int *di = (unsigned int *)d;
            unsigned int *si = (unsigned int *)s;

            for (x=0; x<n; ++x)
            {
                *di = ((*si >> 1) & 0x7f7f7f7f) + (*si & 0x01010101);
                ++si;
                ++di;
            }
            ok = true;
            break;
        }

        case SPECIALEXPR_DOUBLEALPHAROUND:
        {
            for (x=0; x<n; ++x)
            {
                d->a = (unsigned char)((min(255,((int)s->a*255+64)/128)));
                d->r = s->r;
                d->g = s->g;
                d->b = s->b;
                ++s;
                ++d;
            }
            ok = true;
            break;
        }
    }
    return ok;
}

static bool Xpixel8(unsigned char *d,const unsigned char *source,int bpl,const ARGB *colourtbl,int n,int height,EXPR *expr,const char *expri,const char *exprx,const char *expry,int y, const unsigned char *source1,int width1, int height1, int bpl1, int, ARGB *)
{
    int x;
    int i;
    const unsigned char *s= source + y*bpl;
    const ARGB *p;
    unsigned int argb;
    bool ok=false;
    RANGER *ri = RANGER_open(expr, expri, 0.0, 255.0);
    RANGER *rx = RANGER_open(expr, exprx, 0.0, (double) n);
    RANGER *ry = RANGER_open(expr, expry, 0.0, (double) height);
    int mapx=0;

    if (strcmp(exprx,"x") || strcmp(expry,"y"))
        mapx = 1;

    if (ri && rx && ry)
    {
        ok = true;
        EXPR_set(expr,"y",y);
        for (x=0; x<n; ++x)
        {
            int isx;
            int isy;

            isx = x;
            isy = y;
            if (mapx)
            {
                double sx = RANGER_closest(rx, (double) x, RANGER_ROUND_CLOSEST);
                double sy = RANGER_closest(ry, (double) y, RANGER_ROUND_CLOSEST);
                isx = (int)(sx+0.5);
                isy = (int)(sy+0.5);
                if (isx<0) isx = 0;
                if (isy<0) isy = 0;
                if (isx>=n) isx = n-1;
                if (isy>=height) isy = height-1;
            }

            if (source1)
            {
                const unsigned char *s1 = NULL;
                const ARGB *p1;
                int i1;

                if (isx>=width1)  isx = width1;
                if (isy>=height1) isy = height1;

                s1 = source1 + isy*bpl1+isx;
                i1 = *s1;

                p1 = &colourtbl[i1];

                argb = GMAKEID(p1->a,p1->r,p1->g,p1->b);
                EXPR_set(expr,"a_a",(double) p1->a);
                EXPR_set(expr,"a_r",(double) p1->r);
                EXPR_set(expr,"a_g",(double) p1->g);
                EXPR_set(expr,"a_b",(double) p1->b);
                EXPR_set(expr,"a_argb",(double) argb);
                EXPR_set(expr,"a_i",(double) i1);
            }

            s = source + isy*bpl+isx;
            i = *s;

            p = &colourtbl[i];
            argb = GMAKEID(p->a,p->r,p->g,p->b);
            EXPR_set(expr,"a",(double) p->a);
            EXPR_set(expr,"r",(double) p->r);
            EXPR_set(expr,"g",(double) p->g);
            EXPR_set(expr,"b",(double) p->b);
            EXPR_set(expr,"x",(double) x);
            EXPR_set(expr,"i",(double) i);
            EXPR_set(expr,"argb",(double) argb);

            if (strcmp(expri,"i")) { *d = PIXEL_get(ri, (unsigned char)i); if (ri->error) { ok = false; break; }} else *d = *s;
            ++d;
        }
    }
    if (ry) RANGER_close(ry);
    if (rx) RANGER_close(rx);
    if (ri) RANGER_close(ri);
    return ok;
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FABOUT *aboutpixel(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('P','I','X','L');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         2;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.12");
        strcpy(about->namestr,      "Pixel");
        strcpy(about->longnamestr,  "Pixel Expression");
        about->numoptions =         6;

        about->option[0].type =            FOP_PTR;
        about->option[0].min.i =           0;
        about->option[0].max.i =           0xffffffff;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "Expression Handle");
        strcpy(about->option[0].enumlist,  "");

        about->option[1].type =            FOP_PTR;
        about->option[1].min.i =           0;
        about->option[1].max.i =           0xffffffff;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Alpha Expression");
        strcpy(about->option[1].enumlist,  "");

        about->option[2].type =            FOP_PTR;
        about->option[2].min.i =           0;
        about->option[2].max.i =           0xffffffff;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "Red Expression");
        strcpy(about->option[2].enumlist,  "");

        about->option[3].type =            FOP_PTR;
        about->option[3].min.i =           0;
        about->option[3].max.i =           0xffffffff;
        about->option[3].def.i =           0;
        strcpy(about->option[3].name,      "Green Expression");
        strcpy(about->option[3].enumlist,  "");

        about->option[4].type =            FOP_PTR;
        about->option[4].min.i =           0;
        about->option[4].max.i =           0xffffffff;
        about->option[4].def.i =           0;
        strcpy(about->option[4].name,      "Blue Expression");
        strcpy(about->option[4].enumlist,  "");

        about->option[5].type =            FOP_PTR;
        about->option[5].min.i =           0;
        about->option[5].max.i =           0xffffffff;
        about->option[5].def.i =           0;
        strcpy(about->option[5].name,      "Index Expression");
        strcpy(about->option[5].enumlist,  "");
    }
    return(about);
}

int openpixel(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

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

GINFO *infopixel(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    EXPR *exprh=0;
    const char *expra="a";
    const char *exprr="r";
    const char *exprg="g";
    const char *exprb="b";
    const char *expri="i";
    const char *exprx="x";
    const char *expry="y";
    int  specialexpr=SPECIALEXPR_NONE;

    if (fx->option)
    {
        if (fx->numoptions>=1) exprh = (EXPR *)fx->option[0].p;
        if (fx->numoptions>=2) expra = fx->option[1].p;
        if (fx->numoptions>=3) exprr = fx->option[2].p;
        if (fx->numoptions>=4) exprg = fx->option[3].p;
        if (fx->numoptions>=5) exprb = fx->option[4].p;
        if (fx->numoptions>=6) expri = fx->option[5].p;
        if (fx->numoptions>=7) exprx = fx->option[6].p;
        if (fx->numoptions>=8) expry = fx->option[7].p;
    }
    if (!strcmp(expra,"(a+1)/2") && !strcmp(exprr,"r") && !strcmp(exprg,"g") && !strcmp(exprb,"b"))
        specialexpr = SPECIALEXPR_HALFALPHAROUND;
    else if (!strcmp(expra,"(a+1)/2") && !strcmp(exprr,"(r+1)/2") && !strcmp(exprg,"g/2") && !strcmp(exprb,"(b+1)/2"))
        specialexpr = SPECIALEXPR_HALFPIXELROUND;
    else if (!strcmp(expra,"a/2") && !strcmp(exprr,"r") && !strcmp(exprg,"g") && !strcmp(exprb,"b"))
        specialexpr = SPECIALEXPR_HALFALPHA;
    else if (!strcmp(expra,"a/2") && !strcmp(exprr,"r/2") && !strcmp(exprg,"g/2") && !strcmp(exprb,"b/2"))
        specialexpr = SPECIALEXPR_HALFPIXEL;
    else if (!strcmp(expra,"min(255,(a*255+64)/128)") && !strcmp(exprr,"r") && !strcmp(exprg,"g") && !strcmp(exprb,"b"))
        specialexpr = SPECIALEXPR_DOUBLEALPHAROUND;

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        int i;
        unsigned char palmap[256];
        ARGB oldpal[256];

        for (i=0; i<256; ++i)
            palmap[i] = (unsigned char) i;

        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        if (fx->numsources>=2)
            PIXEL_setginfo(exprh,ginfo,1,fx->source[0].image,fx->source[0].rowbytes, fx->source[1].ginfo,fx->source[1].image,fx->source[1].rowbytes);
        else
            PIXEL_setginfo(exprh,ginfo,1,fx->source[0].image,fx->source[0].rowbytes, NULL, NULL, 0);

        Xpixel32(ginfo->colourtbl,&fx->source[0].ginfo->colourtbl[0],0,256,1,exprh,expra,exprr,exprg,exprb,"x","y",0,specialexpr,fx->numsources>=2?&fx->source[1].ginfo->colourtbl[0]:NULL,256,1,0,0,NULL);
        Xpixel8(&palmap[0],&palmap[0],0,&ginfo->colourtbl[0],256,1,exprh,expri,"x","y",0,NULL,0,0,0,0,NULL);

        memcpy(oldpal,ginfo->colourtbl,256*sizeof(ARGB));
        for (i=0; i<256; ++i)
            ginfo->colourtbl[palmap[i]] = oldpal[i];
    }
    return(ginfo);
}

int filterpixel(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    const char *source= (const char *)fx->source[0].image;
    int  width  = ginfo->width;
    int  height = ginfo->height;
    int  bpl    = ginfo->width*ginfo->bpp/8;
    const char *source1=NULL;
    int  width1  = 0;
    int  height1 = 0;
    int  bpl1    = 0;
    bool ok=true;
    ARGB *colourtbl1 = 0;
    int  bpp1    = 0;
    EXPR *exprh=0;
    const char *expra="a";
    const char *exprr="r";
    const char *exprg="g";
    const char *exprb="b";
    const char *expri="i";
    const char *exprx="x";
    const char *expry="y";
    int  specialexpr=SPECIALEXPR_NONE;

    if (fx->option)
    {
        if (fx->numoptions>=1) exprh = (EXPR *)fx->option[0].p;
        if (fx->numoptions>=2) expra = fx->option[1].p;
        if (fx->numoptions>=3) exprr = fx->option[2].p;
        if (fx->numoptions>=4) exprg = fx->option[3].p;
        if (fx->numoptions>=5) exprb = fx->option[4].p;
        if (fx->numoptions>=6) expri = fx->option[5].p;
        if (fx->numoptions>=7) exprx = fx->option[6].p;
        if (fx->numoptions>=8) expry = fx->option[7].p;
    }
    if (!strcmp(expra,"(a+1)/2") && !strcmp(exprr,"r") && !strcmp(exprg,"g") && !strcmp(exprb,"b"))
        specialexpr = SPECIALEXPR_HALFALPHAROUND;
    else if (!strcmp(expra,"(a+1)/2") && !strcmp(exprr,"(r+1)/2") && !strcmp(exprg,"g/2") && !strcmp(exprb,"(b+1)/2"))
        specialexpr = SPECIALEXPR_HALFPIXELROUND;
    else if (!strcmp(expra,"a/2") && !strcmp(exprr,"r") && !strcmp(exprg,"g") && !strcmp(exprb,"b"))
        specialexpr = SPECIALEXPR_HALFALPHA;
    else if (!strcmp(expra,"a/2") && !strcmp(exprr,"r/2") && !strcmp(exprg,"g/2") && !strcmp(exprb,"b/2"))
        specialexpr = SPECIALEXPR_HALFPIXEL;
    else if (!strcmp(expra,"min(255,(a*255+64)/128)") && !strcmp(exprr,"r") && !strcmp(exprg,"g") && !strcmp(exprb,"b"))
        specialexpr = SPECIALEXPR_DOUBLEALPHAROUND;

    if (fx->numsources>=2)
    {
        source1 = (const char *)fx->source[1].image;
        width1  = fx->source[1].ginfo->width;
        height1 = fx->source[1].ginfo->height;
        bpl1    = fx->source[1].rowbytes;
        colourtbl1 = fx->source[1].ginfo->colourtbl;
        bpp1    = fx->source[1].ginfo->bpp;

        PIXEL_setginfo(exprh,ginfo,0,fx->source[0].image,fx->source[0].rowbytes, fx->source[1].ginfo,fx->source[1].image,fx->source[1].rowbytes);
    }
    else
        PIXEL_setginfo(exprh,ginfo,0,fx->source[0].image,fx->source[0].rowbytes, NULL, NULL, 0);

    for (i=0; i<ginfo->height && ok; ++i)
    {
        if (ginfo->bpp==32)
            ok = Xpixel32((ARGB *)dest,(const ARGB *)source,bpl,width,height,exprh,expra,exprr,exprg,exprb,exprx,expry,i,specialexpr,(const ARGB *)source1,width1,height1,bpl1,bpp1,colourtbl1);
        else if (ginfo->bpp==8)
            ok = Xpixel8((unsigned char *)dest,(const unsigned char *)source,bpl,&ginfo->colourtbl[0],width,height,exprh,expri,exprx,expry,i,(const unsigned char *)source1,width1,height1,bpl1,bpp1,colourtbl1);
        dest   += rowbytes;
    }
    return ok?1:0;
}

int closepixel(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

