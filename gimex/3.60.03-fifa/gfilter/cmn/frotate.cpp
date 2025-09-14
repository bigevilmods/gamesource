// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                Rotate - Rotate Image by Any Angle                */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 960911 GS based on frot                                  */
/*    1.01 960913 GS takes a background color                       */
/*    1.02 970505 GS option to interpolate transparent colors       */
/*    1.03 010118 ID fix defaultx & y calc for page type            */
/*    1.04 010129 FB round pagesize up                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* rotate(angle,background)                                         */
/* float angle;       0..2*PI                                       */
/* int   background;  colour to fill with                           */
/* int   interpolate; true if we should interpolate with backgroudn */
/*                                                                  */
/* Rotate by any angle                                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "frotate.h"
#include <string.h>
#include <math.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define CLAMP(Val, Min,Max)     (Val > Max ? Max : (Val < Min ? Min : Val))
#define INRANGE(val, Min,Max)   ((val >= Min) && (val < Max))
#define FRAC(val)               (val - ((int)val))

/* these should be moved into the instance since they are needed during the actual filter */
static int xoffset, yoffset;

static void rotatedpoint(double* pX, double* pY, double Angle)
{
    double x = *pX;
    double y = *pY;

    *pX = x*cos(Angle) - y*sin(Angle);
    *pY = x*sin(Angle) + y*cos(Angle);
}

static void rotatepoint(int* pX, int* pY, double Angle)
{
    double x = *pX;
    double y = *pY;

    rotatedpoint(&x,&y, Angle);

    if (x>=0.0)
        *pX = (int)(x+0.5);
    else
        *pX = (int)(x-0.5);
    if (y>=0.0)
        *pY = (int)(y+0.5);
    else
        *pY = (int)(y-0.5);
}

static void offsetdpoint(double* pX, double* pY, int OffsetX, int OffsetY)
{
    *pX = *pX + OffsetX;
    *pY = *pY + OffsetY;
}

static void offsetpoint(int* pX, int* pY, int OffsetX, int OffsetY)
{
    *pX = *pX + OffsetX;
    *pY = *pY + OffsetY;
}

static ARGB readpixel(GBITMAP* src, int x, int y, int* valid, int Interpolate)
{
    ARGB Color;

    if (INRANGE(x, 0, src->ginfo->width) && INRANGE(y, 0, src->ginfo->height))
    {
        unsigned char* rowbase = (unsigned char*)src->image + y*src->rowbytes;

        if (src->ginfo->bpp > 8)
        {
            ARGB* base = (ARGB*)rowbase + x;
            Color = *base;
        }
        else
        {
            unsigned char* base = rowbase + x;
            unsigned char ColorIndex = *base;
            Color = src->ginfo->colourtbl[ColorIndex];
        }

        /* uncomment this to prevent aliasing to any transparent color */
        if (Interpolate)
        {
            *valid = 1;
        }
        else
        {
            if (Color.a > 0)
                *valid = 1;
            else
                *valid = 0;
        }
    }
    else
    {
        Color.a = Color.r = Color.g = Color.b = 0;
        *valid = 0;
    }
    return Color;
}

static unsigned char FindBestIndex(GBITMAP* src, ARGB Color)
{
    /* find best matching color in colortabl that matches Color */

    ARGB* Pal = src->ginfo->colourtbl;

    int BestIndex = 0;
    unsigned int BestDiff = 999999;

    unsigned char finda = Color.a;
    unsigned char findr = Color.r;
    unsigned char findg = Color.g;
    unsigned char findb = Color.b;

    int i;

    for (i = 0; i < 256; i++)
    {
        unsigned char a = Pal[i].a;
        unsigned char r = Pal[i].r;
        unsigned char g = Pal[i].g;
        unsigned char b = Pal[i].b;

        unsigned int Diff = (finda-a)*(finda-a)+(findr-r)*(findr-r)+(findg-g)*(findg-g)+(findb-b)*(findb-b);
        if (Diff < BestDiff)
        {
            BestDiff = Diff;
            BestIndex = i;
        }

        if (Diff == 0)
            break;
    }

    return (unsigned char)BestIndex;
}


static void rotatei(GBITMAP* src, GBITMAP* dst, double Angle, int BackIndex, ARGB BackColor, int Interpolate)
{
    int y;

    /* iterate over destination image, getting pixels from source image */
    for (y = 0; y < dst->ginfo->height; y++)
    {
        unsigned char* base = (unsigned char*)dst->image + y*dst->rowbytes;

        int x;
        for (x = 0; x < dst->ginfo->width; x++)
        {
            ARGB p;

            int topvalid = 1;
            int botvalid = 1;

            double x1 = x; /* source point that maps to current destination point (x,y) */
            double y1 = y;

            /* to inverse translation\rotation to get source point */
            offsetdpoint(&x1,&y1, -xoffset, -yoffset);
            rotatedpoint(&x1,&y1, -Angle);

            {
                /* source point (x1,y1) are real numbers (ie, 11.3, 12.4) */
                /* use bi-linear filtering on integer corner points to best value */

                int ax,ay;  /* a = top left */
                int bx,by;  /* b = top right */
                int cx,cy;  /* c = bottom left */
                int dx,dy;  /* d = bottom right */

                ARGB a,b,c,d;

                double w,h;

                int avalid;     /* valid flags indicate if pixel value is valid  */
                int bvalid;     /* (false if pixel is outside of source image) */
                int cvalid;
                int dvalid;

                double topa, topr, topg, topb;      /* top = a*(1-w)   + b*w */
                double bota, botr, botg, botb;      /* bot = c*(1-w)   + d*w */
                double vala, valr, valg, valb;      /* val = top*(1-h) + bot*h */

                /* kinda tricky to handle the case when points are negative (pain in the butt) */
                int ix = (int)x1;
                int iy = (int)y1;
                if (x1 >= 0.0)  { ax = ix; bx = ix+1;   cx = ix;   dx = ix+1; }
                else            { ax = ix; bx = ix-1;   cx = ix;   dx = ix-1; }

                if (y1 >= 0.0)  { ay = iy; by = iy;     cy = iy+1; dy = iy+1; }
                else            { ay = iy; by = iy;     cy = iy-1; dy = iy-1; }

                a = readpixel(src,ax,ay, &avalid, Interpolate);
                b = readpixel(src,bx,by, &bvalid, Interpolate);
                c = readpixel(src,cx,cy, &cvalid, Interpolate);
                d = readpixel(src,dx,dy, &dvalid, Interpolate);

                /* again handle negatives */
                if (x1 >= 0.0)
                    w = FRAC(x1);
                else
                    w = 1.0 - FRAC(x1);

                if (y1 >= 0.0)
                    h = FRAC(y1);
                else
                    h = 1.0 - FRAC(y1);


                if (avalid && bvalid)
                {
                    topa = a.a*(1-w) + b.a*w;
                    topr = a.r*(1-w) + b.r*w;
                    topg = a.g*(1-w) + b.g*w;
                    topb = a.b*(1-w) + b.b*w;
                }
                else if (avalid)
                {
                    topa = a.a;
                    topr = a.r;
                    topg = a.g;
                    topb = a.b;
                }
                else if (bvalid)
                {
                    topa = b.a;
                    topr = b.r;
                    topg = b.g;
                    topb = b.b;
                }
                else
                {
                    topa = 0.0;
                    topr = 0.0;
                    topg = 0.0;
                    topb = 0.0;
                    topvalid = 0;
                }

                if (cvalid && dvalid)
                {
                    bota = c.a*(1-w) + d.a*w;
                    botr = c.r*(1-w) + d.r*w;
                    botg = c.g*(1-w) + d.g*w;
                    botb = c.b*(1-w) + d.b*w;
                }
                else if (cvalid)
                {
                    bota = c.a;
                    botr = c.r;
                    botg = c.g;
                    botb = c.b;
                }
                else if (dvalid)
                {
                    bota = d.a;
                    botr = d.r;
                    botg = d.g;
                    botb = d.b;
                }
                else
                {
                    bota = 0.0;
                    botr = 0.0;
                    botg = 0.0;
                    botb = 0.0;
                    botvalid = 0;
                }

                if (topvalid && botvalid)
                {
                    vala = topa*(1-h) + bota*h;
                    valr = topr*(1-h) + botr*h;
                    valg = topg*(1-h) + botg*h;
                    valb = topb*(1-h) + botb*h;
                }
                else if (topvalid)
                {
                    vala = topa;
                    valr = topr;
                    valg = topg;
                    valb = topb;
                }
                else if (botvalid)
                {
                    vala = bota;
                    valr = botr;
                    valg = botg;
                    valb = botb;
                }
                else
                {
                    vala = 0.0;
                    valr = 0.0;
                    valg = 0.0;
                    valb = 0.0;
                }

                p.a = (unsigned char)CLAMP(vala, 0.0,255.0);
                p.r = (unsigned char)CLAMP(valr, 0.0,255.0);
                p.g = (unsigned char)CLAMP(valg, 0.0,255.0);
                p.b = (unsigned char)CLAMP(valb, 0.0,255.0);
            }

            if (dst->ginfo->bpp > 8)
            {
                ARGB* argb = (ARGB*)base;
                if (topvalid || botvalid)
                {
                    *argb = p;
                }
                else
                {
                    *argb = BackColor;
                }
                base += 4;
            }
            else
            {
                if (topvalid || botvalid)
                {
                    unsigned char Index = FindBestIndex(dst, p);
                    *base = Index;
                }
                else
                {
                    *base = (unsigned char)BackIndex;
                }
                base += 1;
            }
        }
    }
}

#undef CLAMP
#undef INRANGE
#undef FRAC

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* aboutrotate(void)
{
    FABOUT* about = (FABOUT*) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.04");
        strcpy(about->namestr,      "Rotate");
        strcpy(about->longnamestr,  "Rotate");
        about->numoptions =         3;

        about->option[0].type =            FOP_FLOAT;
        about->option[0].min.f =           0.0;
        about->option[0].max.f =           6.28318530718;   /* 2*PI */
        about->option[0].def.f =           0.0;
        strcpy(about->option[0].name,      "Angle");

        about->option[1].type =            FOP_INT|FOP_INDEX_COLOR|FOP_TRUE_COLOR;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Colour");

        about->option[2].type =            FOP_INT;
        about->option[2].def.i =           1;
        strcpy(about->option[2].name,      "Interpolate");
    }
    return(about);
}

FPUBLIC int openrotate(FINSTANCE** filterInstance, int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    int ok=0;
    FINSTANCE* fx=0;

    /* allocate instance */
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

FPUBLIC GINFO* inforotate(FINSTANCE* fx, int framenum)
{
    GINFO* ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        double angle = 0.0;

        int i; /* counter for rotating hot points */

        int ax, ay; /* topleft corner */
        int bx, by; /* top right */
        int cx, cy; /* bottom left */
        int dx, dy; /* bottom right */

        int minx, miny;
        int maxx, maxy;
        int psdFrameHeight;
        int psdFrameWidth;
        int psdNewFrameHeight;
        int psdNewFrameWidth;
        int typepage;
        double ix,iy;

        if (fx->numoptions == 7) // if the options list contains my new options for multilayer support
        {                        // in the rotate filter then...
            psdFrameHeight = fx->option[4].i;
            psdFrameWidth  = fx->option[5].i;
            psdNewFrameHeight = fx->option[2].i;
            psdNewFrameWidth  = fx->option[3].i;
            typepage = fx->option[6].i;
        }
        else
        {
            psdFrameHeight = ginfo->height;
            psdFrameWidth  = ginfo->width;
            psdNewFrameHeight = ginfo->height;
            psdNewFrameWidth  = ginfo->width;
            typepage = 0;
        }
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        ix = (double)ginfo->defaultx;
        iy = (double)ginfo->defaulty;

        if (fx->numoptions>=1) angle = fx->option[0].f;
        if(typepage)
        {
            if(ix != 0 || iy != 0/*ginfo->framenum != 0*/)
            {

                int roter = (int)(angle*180.0/3.141592654);
                if(roter < 0) roter = 360 + roter;  // allow for negative rotations by doing inverse.
                if(roter > 0 && roter <=90)  // if in first quadrant
                {                            // user upper left and lower left points to
                                             // calculate new draw box.
                    // calculate the upper left corner rotation

                    ix = ((double)ix - ((double)psdFrameWidth/2.0));
                    iy = ((double)iy - ((double)psdFrameHeight/2.0));
                    rotatedpoint(&(ix), &(iy), angle);
                    // use this rotation as my y coord of new draw box.
                    ginfo->defaulty = (int)(iy + ((double)psdFrameHeight/2.0) + ((psdNewFrameHeight - psdFrameHeight)/2.0));
                    // calculate the lower left corner rotation.
                    ix = ((double)fx->source[0].ginfo->defaultx - ((double)psdFrameWidth/2.0));
                    iy = ((double)fx->source[0].ginfo->defaulty + (double)ginfo->height - ((double)psdFrameHeight/2.0));
                    rotatedpoint(&(ix), &(iy), angle);
                    // use this rotation as my x coord of new draw box.
                    ginfo->defaultx = (int)(ix + ((double)psdFrameWidth/2.0) + ((psdNewFrameWidth - psdFrameWidth)/2.0));
                }
                else if(roter> 90 && roter <= 180) // if in second quadrant use lower left
                {                                  // and lower right corners to calc new
                                                   // draw box.
                    // lower right corner.
                    ix = ((double)ix + ginfo->width- ((double)psdFrameWidth/2.0));
                    iy = ((double)iy + ginfo->height - ((double)psdFrameHeight/2.0));
                    rotatedpoint(&(ix), &(iy), angle);
                    ginfo->defaultx = (int)(ix + ((double)psdFrameWidth/2.0)+ ((psdNewFrameWidth - psdFrameWidth)/2.0));
                    // lower left corner.
                    ix = ((double)fx->source[0].ginfo->defaultx - ((double)psdFrameWidth/2.0));
                    iy = ((double)fx->source[0].ginfo->defaulty + (double)ginfo->height - ((double)psdFrameHeight/2.0));
                    rotatedpoint(&(ix), &(iy), angle);
                    ginfo->defaulty = (int)(iy + ((double)psdFrameHeight/2.0)+ ((psdNewFrameHeight - psdFrameHeight)/2.0));

                }

                else if(roter > 180 && roter <= 270) // if in third quadrant use upper right
                {                                    // and lower right to calc new draw box.
                    // upper right corner..
                    ix = ((double)ix + (double)ginfo->width - ((double)psdFrameWidth/2.0));
                    iy = ((double)iy - ((double)psdFrameHeight/2.0));
                    rotatedpoint(&(ix), &(iy), angle);
                    ginfo->defaultx = (int)(ix + ((double)psdFrameWidth/2.0)+ ((psdNewFrameWidth - psdFrameWidth)/2.0));
                    // lower right corner.
                    ix = ((double)fx->source[0].ginfo->defaultx + (double)ginfo->width - ((double)psdFrameWidth/2.0));
                    iy = ((double)fx->source[0].ginfo->defaulty + (double)ginfo->height - ((double)psdFrameHeight/2.0));
                    rotatedpoint(&(ix), &(iy), angle);
                    ginfo->defaulty = (int)(iy + ((double)psdFrameHeight/2.0)+ ((psdNewFrameHeight - psdFrameHeight)/2.0));
                }
                else if(roter > 270)  // if in fourth quad. use upper left and upper right
                {                     // to calc new draw box.
                    // upper left corner.
                    ix = ((double)ix - ((double)psdFrameWidth/2.0));
                    iy = ((double)iy - ((double)psdFrameHeight/2.0));
                    rotatedpoint(&(ix), &(iy), angle);
                    ginfo->defaultx = (int)(ix + ((double)psdFrameWidth/2.0)+ ((psdNewFrameWidth - psdFrameWidth)/2.0));
                    // upper right corner.
                    ix = ((double)fx->source[0].ginfo->defaultx + (double)ginfo->width - ((double)psdFrameWidth/2.0));
                    iy = ((double)fx->source[0].ginfo->defaulty - ((double)psdFrameHeight/2.0));
                    rotatedpoint(&(ix), &(iy), angle);
                    ginfo->defaulty = (int)(iy + ((double)psdFrameHeight/2.0)+ ((psdNewFrameHeight - psdFrameHeight)/2.0));
                }
            }// if framenum != 0
        } // if typepage.
        /* calculate new width and height by rotating corner points and seeing where they lay */

        ax = 0;            ay = 0;
        bx = ginfo->width; by = 0;
        cx = 0;            cy = ginfo->height;
        dx = ginfo->width; dy = ginfo->height;

        rotatepoint(&ax,&ay, angle);
        rotatepoint(&bx,&by, angle);
        rotatepoint(&cx,&cy, angle);
        rotatepoint(&dx,&dy, angle);

        minx = gmin(gmin(gmin(ax,bx),cx),dx);
        miny = gmin(gmin(gmin(ay,by),cy),dy);
        maxx = gmax(gmax(gmax(ax,bx),cx),dx);
        maxy = gmax(gmax(gmax(ay,by),cy),dy);

        ginfo->width  = maxx-minx;
        ginfo->height = maxy-miny;

        xoffset = -(gmin(0,minx));
        yoffset = -(gmin(0,miny));

        /* rotate image points */
        rotatepoint(&(ginfo->centerx),  &(ginfo->centery),  angle);
        offsetpoint(&(ginfo->centerx),  &(ginfo->centery),  xoffset, yoffset);
/*IAN*/
        if(!typepage)
        {
            rotatepoint(&(ginfo->defaultx), &(ginfo->defaulty), angle);
            offsetpoint(&(ginfo->defaultx), &(ginfo->defaulty), xoffset, yoffset);
        }
        for (i = 0; i < ginfo->numhotspots; i++)
        {
            rotatepoint(&(ginfo->hotspottbl[i][0]), &(ginfo->hotspottbl[i][1]), angle);
            offsetpoint(&(ginfo->hotspottbl[i][0]), &(ginfo->hotspottbl[i][1]), xoffset, yoffset);
        }
/*IAN*/
        //psdNewFrameHeight = ginfo->height;
        //psdNewFrameWidth  = ginfo->width;

    }
    return ginfo;
}

FPUBLIC int filterrotate(FINSTANCE* fx, GINFO* ginfo, char* dstbase, int rowbytes)
{
    double Angle = 0.0;
    int    BackIndex=0;
    ARGB   BackColor={0,0,0,0};
    int    Interpolate=1;

    GBITMAP dst;
    dst.image    = dstbase;
    dst.ginfo    = ginfo;
    dst.rowbytes = rowbytes;

    if (fx->numoptions>=1) Angle = fx->option[0].f;
    if (fx->numoptions>=2)
    {
        unsigned int c = fx->option[1].i;
        BackColor.a = (unsigned char)(c>>24);
        BackColor.r = (unsigned char)(c>>16);
        BackColor.g = (unsigned char)(c>>8);
        BackColor.b = (unsigned char)c;
        BackIndex = c;
    }
    if (fx->numoptions>=3) Interpolate = fx->option[2].i;

    rotatei(&(fx->source[0]), &dst, Angle, BackIndex, BackColor, Interpolate);

    return 1;
}

FPUBLIC int closerotate(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
