// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*           PalSort - Sort Palette Entries and Remap Image         */
/*                                                                  */
/*                by Gerry Shaw and FrANK Barchard, EAC             */
/*                                                                  */
/*                    Filter Module - Oct 24, 1996                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 950831 GS initial version                                */
/*    1.01 950912 GS force color 0 to be black                      */
/*    1.02 951102 FB use FOPs, remap image, renamed fpalsort        */
/*    1.03 960507 FB FOP handling                                   */
/*    1.04 960921 FB force transparent 0 or 255                     */
/*    1.05 961011 FB better transparent 255                         */
/*    1.06 961024 FB argb, not argr (shelby hubick)                 */
/*    1.07 990511 LC method is a string; can sort in any order      */
/*    1.08 990513 LC sort by YUV                                    */
/*    1.09 990517 LC HSV=>HSL; YUW=>YUV; filterpalsort returns # of */
/*                   valid characters                               */
/*    1.10 990518 LC combined all compare functions into one        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit                                                            */
/* palsort(int method,int force)                                    */
/* int method;     method of sorting.  see below                    */
/* int force;      0=nothing, 1=0 black, 2=255 trans,3=0 trans      */
/*                                                                  */
/* methods:                                                         */
/*  0  histogram                                                    */
/*  1  argb                                                         */
/*  2  ahls                                                         */
/*  3  weighted ahls                                                */
/*  4  inverse histogram                                            */
/*  5  yuv                                                          */
/*  ARGBHSLYUVP can be in any order; capital=large=>small;          */
/*                                   lowercase = small=>large       */
/*  A = alpha; R = red; G = green; B = blue; H = hue; S = saturation*/
/*  L = lightness; Y = luminance; U = chrominance; V = chrominance  */
/*  P = popularity(histogram)                                       */
/*                                                                  */
/* force:                                                           */
/*  0  nothing                                                      */
/*  1  black must be in colour index 0                              */
/*  2  transparent must be in colour index 255                      */
/*  3  transparent must be in colour index 0                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
/*                                                                  */
/* Helper Functions:                                                */
/* ----------------                                                 */
/*                                                                  */
/* FPALETTE* sortpalette(GBITMAP* source, int method)               */
/*                                                                  */
/* Returns palette used by 8 bit bitmap sorted based on popularity. */
/*                                                                  */
/*------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "fpalsort.h"
#include "math.h"

/****************************************************************/
/* Helper Functions                                             */
/****************************************************************/

/* pal sort methods */

#define PALSORT_COUNT   '0'
#define PALSORT_ARGB    '1'
#define PALSORT_HVS     '2'
#define PALSORT_WHVS    '3'
#define PALSORT_ICOUNT  '4'
#define PALSORT_YUV     '5'

/* pal sort methods */

#define PALSORT_FORCENOTHING 0
#define PALSORT_FORCEBLACK   1
#define PALSORT_FORCET255    2
#define PALSORT_FORCET0      3

FPALETTE* sortpalette(GBITMAP* source, int method)
{
    FPALETTE *pal;
    GINFO *ginfo; /* ginfo returned by filter info call (contains sorted colourtbl) */

    int startcolour = source->ginfo->subtype; /* HACK: once GINFO has startcolour fix this */
    int numcolours  = source->ginfo->numcolours;

    FINSTANCE *fx;
    FOP options[1];
    GBITMAP sources[1];

    options[0].i        = method;
    sources[0].ginfo    = source->ginfo;
    sources[0].image    = source->image;
    sources[0].rowbytes = source->rowbytes;

    openpalsort(&fx, 1, sources, 1, options);
    ginfo = infopalsort(fx, 0);
    closepalsort(fx);

    /* extract palette from ginfo and return a FPALETTE */
    pal = (FPALETTE *) galloc(sizeof(FPALETTE) + numcolours*sizeof(ARGB));
    pal->startcolour = startcolour;
    pal->numcolours  = numcolours;
    pal->inversetbl  = NULL;
    pal->colourtbl   = (ARGB*)(((char*)pal) + sizeof(FPALETTE));
    memcpy(pal->colourtbl, ginfo->colourtbl, ginfo->numcolours*sizeof(ARGB));

    gfree(ginfo);

    return(pal);
}

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

typedef struct
{
    unsigned char a;        /* alpha channel 0..255 */
    unsigned char r;        /* red channel 0..255 */
    unsigned char g;        /* green channel 0..255 */
    unsigned char b;        /* blue channel 0..255 */
    int  count;             /* histogram */
    int  index;             /* original index */
} COLOURINFO;

/****************************************************************/
/*  HSV                                                         */
/****************************************************************/

/* structures for rgb<->hsv conversion */
typedef struct { int r, g, b; } fpsRGB;
typedef struct { int h, s, v; } fpsHSV;

#define FPSUNDEFINED -1

/* normalize factor = 256 */
#define FPSSCLEXP 8

#define FPSMAXINT 0xefff

#define FPSLROUND 0x80L
/* (1 << (FPSSCLEXP - 1))L */
#define FPSF1 (1 << FPSSCLEXP)
#define FPSF2 (2 << FPSSCLEXP)
#define FPSF4 (4 << FPSSCLEXP)

#define FPSnormrgb(x)  ((x)*256/63)
#define FPSunnormsv(x) (x)*255/256

static void FPSrgb2hsv0(fpsRGB *source, fpsHSV *dest)
{
    int r,g,b;
    unsigned int h=0, s, v, rc, gc, bc, max, min, diffDenom, diffRound;

    r = FPSnormrgb(source->r);    /* 0..63 => 0..256 */
    g = FPSnormrgb(source->g);
    b = FPSnormrgb(source->b);
    max = (r > g) ? ((r > b) ? r : b) : (g > b) ? g : b;
    min = (r < g) ? ((r < b) ? r : b) : (g < b) ? g : b;
    v = max;                        /* value 0..256 */

    #define FPSdiffNum FPSF1  /* Numerator of hue-unit-step at this max/min. */
    diffDenom = max - min;  /* Denominator of hue-unit-step */
    diffRound = diffDenom >> 1; /* For rounding when "/ diffDenom". */

    if ((max != 0)  &&  (max != min))
    {
        s = FPSunnormsv((unsigned int)(((unsigned int)diffDenom << 8) / max));        /* saturation 0..256 */
    }
    else
        s = 0;
    if (!s)
    {
        h = (unsigned int)FPSUNDEFINED;
    }
    else        /* diffDenom != 0, since min != max */
    {           /* saturation not zero, so determine hue */
        /* rc measures distance of color from red */
        rc = (unsigned int)(((unsigned int)(max - r) * FPSdiffNum + diffRound) / diffDenom);
        gc = (unsigned int)(((unsigned int)(max - g) * FPSdiffNum + diffRound) / diffDenom);
        bc = (unsigned int)(((unsigned int)(max - b) * FPSdiffNum + diffRound) / diffDenom);

        if (r == (int)max)
        {
            h = bc - gc;        /* color between yellow & magenta */
        }
        else
        {
            if (g == (int)max)
            {
                h = FPSF2 + rc - bc; /* color between cyan & yellow */
            }
            else
            {
                if (b == (int)max)
                    h = FPSF4 + gc - rc; /* color between magenta & cyan */
            }
        }
    }
    /* chromatic case */
    v = FPSunnormsv(v);    /* 0..256 => 0..255 */

    if (h < 3)
        h = 0;      /* [TBD] So much for "FPSUNDEFINED"! */
    if (s < 3)
        s = 0;

    dest->h = h;
    dest->s = s;
    dest->v = v;
}

static void FPSrgb256hsv(fpsRGB *source, fpsHSV *dest)
{
    fpsRGB FPSrgb255;

    FPSrgb255.r = source->r >> 2;
    FPSrgb255.g = source->g >> 2;
    FPSrgb255.b = source->b >> 2;
    FPSrgb2hsv0(&FPSrgb255, dest);
}


/****************************************************************/
/*  YUV                                                         */
/****************************************************************/
typedef struct { double y, u, v; } fpsYUV;

static void rgb_yuv(fpsRGB *rgb, fpsYUV *yuv)
{
    yuv->y =  0.299*rgb->r +  0.587*rgb->g +  0.114*rgb->b;
    yuv->u = -0.147*rgb->r + -0.289*rgb->g +  0.437*rgb->b;
    yuv->v =  0.615*rgb->r + -0.515*rgb->g + -0.100*rgb->b;
}

/****************************************************************/
/*  Compare                                                     */
/****************************************************************/

static struct
{
    char method[11];
    int length;
} sort;

static int compare(const void* elem1, const void* elem2)
{

    const COLOURINFO* c1 = (const COLOURINFO*)elem1;
    const COLOURINFO* c2 = (const COLOURINFO*)elem2;
    int i = 0;
    int result = 0;

    fpsHSV hsv1, hsv2;
    fpsRGB rgb1, rgb2;
    fpsYUV yuv1, yuv2;

    while (i < sort.length)
    {
        switch (sort.method[i])
        {
            case 'A':   result = c2->a - c1->a; break;
            case 'R':   result = c2->r - c1->r; break;
            case 'G':   result = c2->g - c1->g; break;
            case 'B':   result = c2->b - c1->b; break;

            case 'P':   result = c2->count - c1->count; break;

            case 'H':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                FPSrgb256hsv(&rgb1, &hsv1);
                FPSrgb256hsv(&rgb2, &hsv2);
                result = hsv2.h - hsv1.h;
                break;
            case 'S':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                FPSrgb256hsv(&rgb1, &hsv1);
                FPSrgb256hsv(&rgb2, &hsv2);
                result = hsv2.s - hsv1.s;
                break;
            case 'L':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                FPSrgb256hsv(&rgb1, &hsv1);
                FPSrgb256hsv(&rgb2, &hsv2);
                result = hsv2.v - hsv1.v;
                break;

            case 'Y':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                rgb_yuv(&rgb1, &yuv1);
                rgb_yuv(&rgb2, &yuv2);
                result = (int)(10000*(yuv2.y - yuv1.y));
                break;
            case 'U':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                rgb_yuv(&rgb1, &yuv1);
                rgb_yuv(&rgb2, &yuv2);
                result = (int)(10000*(yuv2.u - yuv1.u));
                break;
            case 'V':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                rgb_yuv(&rgb1, &yuv1);
                rgb_yuv(&rgb2, &yuv2);
                result = (int)(10000*(yuv2.v - yuv1.v));
                break;

            case 'a':   result = c1->a - c2->a; break;
            case 'r':   result = c1->r - c2->r; break;
            case 'g':   result = c1->g - c2->g; break;
            case 'b':   result = c1->b - c2->b; break;

            case 'p':   result = c1->count - c2->count; break;

            case 'h':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                FPSrgb256hsv(&rgb1, &hsv1);
                FPSrgb256hsv(&rgb2, &hsv2);
                result = hsv1.h - hsv2.h;
                break;
            case 's':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                FPSrgb256hsv(&rgb1, &hsv1);
                FPSrgb256hsv(&rgb2, &hsv2);
                result = hsv1.s - hsv2.s;
                break;
            case 'l':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                FPSrgb256hsv(&rgb1, &hsv1);
                FPSrgb256hsv(&rgb2, &hsv2);
                result = hsv1.v - hsv2.v;
                break;

            case 'y':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                rgb_yuv(&rgb1, &yuv1);
                rgb_yuv(&rgb2, &yuv2);
                result = (int)(10000*(yuv1.y - yuv2.y));
                break;
            case 'u':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                rgb_yuv(&rgb1, &yuv1);
                rgb_yuv(&rgb2, &yuv2);
                result = (int)(10000*(yuv1.u - yuv2.u));
                break;
            case 'v':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                rgb_yuv(&rgb1, &yuv1);
                rgb_yuv(&rgb2, &yuv2);
                result = (int)(10000*(yuv1.v - yuv2.v));
                break;

            case 'w':
                rgb1.r = c1->r;
                rgb1.g = c1->g;
                rgb1.b = c1->b;

                rgb2.r = c2->r;
                rgb2.g = c2->g;
                rgb2.b = c2->b;

                FPSrgb256hsv(&rgb1, &hsv1);
                FPSrgb256hsv(&rgb2, &hsv2);

                result = (c1->a-c2->a)*256
                       + (hsv1.h-hsv2.h)*8
                       + (hsv1.v-hsv2.v)*100
                       + (hsv1.s-hsv2.s)*1;
                break;

            default:
                break;
        }
        if (result == 0)
            i++;
        else
            break;
    }
    return (result);
}

static void buildcolourinfotbl(FINSTANCE *fx, COLOURINFO *colourinfotbl, int force)
{
    GBITMAP source = fx->source[0];

    memset(colourinfotbl, 0, sizeof(COLOURINFO)*256);

    /* get information based on palette */
    {
        int i;
        ARGB *colourtbl = source.ginfo->colourtbl;

        for (i=0; i<source.ginfo->numcolours; ++i)
        {
            colourinfotbl[i].a = colourtbl[i].a;
            colourinfotbl[i].r = colourtbl[i].r;
            colourinfotbl[i].g = colourtbl[i].g;
            colourinfotbl[i].b = colourtbl[i].b;
            colourinfotbl[i].count = 0;
            colourinfotbl[i].index = i;
        }
    }

    /* get information based on image */
    {
        unsigned char *base = (unsigned char*)source.image;
        int rowbytes = source.rowbytes;
        int x;
        int y;

        for (y = 0; y < source.ginfo->height; y++)
        {
            for (x = 0; x < source.ginfo->width; x++)
            {
                int pixel = base[y*rowbytes + x];
                colourinfotbl[pixel].count += 1;
            }
        }
    }

    {
        int i;
        ARGB *colourtbl = source.ginfo->colourtbl;

        for (i=0; i<source.ginfo->numcolours; ++i)
        {
            switch (force)
            {
                case PALSORT_FORCENOTHING: break;
                case PALSORT_FORCEBLACK:
                    if (colourtbl[i].r==0 && colourtbl[i].g==0 && colourtbl[i].b==0)
                    {
                        if (colourtbl[i].a==0)
                            colourinfotbl[i].count += 100001;
                        else
                            colourinfotbl[i].count += 100000;
                    }
                    break;

                case PALSORT_FORCET255:
                    if (colourtbl[i].a==0)
                        colourinfotbl[i].count = -100000-colourinfotbl[i].count;
                    break;

                case PALSORT_FORCET0:
                    if (colourtbl[i].a==0)
                    {
                        colourinfotbl[i].count += 100000;
                    }
                    break;
            }
        }
    }
}

static void updatecolourtbl(COLOURINFO *colourinfotbl, int numcolours, ARGB *colourtbl)
{
    int i;

    for (i = 0; i < numcolours; i++)
    {
        colourtbl[i].a = colourinfotbl[i].a;
        colourtbl[i].r = colourinfotbl[i].r;
        colourtbl[i].g = colourinfotbl[i].g;
        colourtbl[i].b = colourinfotbl[i].b;
    }
}

static int validcharacter(void)
{
    int i=0, j=0;
    //unsigned short int valid = 0;
    unsigned short int same = 0;

    while (i < 11)
    {
        // checking if its a valid character and if not the same as a previous character
        if ((sort.method[i]=='A') || (sort.method[i]=='R') || (sort.method[i]=='G') || (sort.method[i]=='B')
            || (sort.method[i]=='a') || (sort.method[i]=='r') || (sort.method[i]=='g') || (sort.method[i]=='b')
            || (sort.method[i]=='H') || (sort.method[i]=='S') || (sort.method[i]=='L')
            || (sort.method[i]=='h') || (sort.method[i]=='s') || (sort.method[i]=='l')
            || (sort.method[i]=='Y') || (sort.method[i]=='U') || (sort.method[i]=='V')
            || (sort.method[i]=='y') || (sort.method[i]=='u') || (sort.method[i]=='v')
            || (sort.method[i]=='P') || (sort.method[i]=='p'))
        {
            for (j=i-1; j>=0; j--)
            {
                if (sort.method[i] == sort.method[j])
                {
                    same = 1;
                    break;
                }
            }
            // if found a repeated letter, treat it as invalid
            if (same)
            {
                i = -1*i;
                break;
            }
            i++;
            continue;
        }
        // if reached invalid character, sort on the first valid ones
        else
        {
            i = -1*i;
            break;
        }
    }

    if (strcmp(sort.method, "wwww")==0)
        return(4);
    return(i);
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutpalsort(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('P','S','R','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           0;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "G Shaw/F Barchard");
        strcpy(about->versionstr,   "1.08");
        strcpy(about->namestr,      "PalSort");
        strcpy(about->longnamestr,  "Palette Sort");
        about->numoptions =         2;

        about->option[0].type =            FOP_INT | FOP_ENUM;
        about->option[0].min.i =           0;
        about->option[0].max.i =           3;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "Method");
        strcpy(about->option[0].enumlist,  "Count,ARGB,HVS,WHVS,ICOUNT,YUV");

        about->option[1].type =            FOP_INT | FOP_ENUM;
        about->option[1].min.i =           0;
        about->option[1].max.i =           1;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "ForceBlack");
        strcpy(about->option[1].enumlist,  "No,Yes");
    }
    return(about);
}

FPUBLIC int openpalsort(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;
    void *colourinfotbl;

    fx = (FINSTANCE *)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        colourinfotbl = galloc(sizeof(COLOURINFO)*256);
        if (colourinfotbl)
        {
            memset(fx, 0, sizeof(FINSTANCE));
            fx->frames      = 1;
            fx->numsources  = numsources;
            fx->source      = sources;
            fx->numoptions  = numoptions;
            fx->option      = options;
            fx->fref        = colourinfotbl;
            ok = 1;
        }
    }
    *filterInstance = fx;
    return(ok);
}

FPUBLIC GINFO *infopalsort(FINSTANCE *fx, int framenum)
{

    GINFO *ginfo;
    COLOURINFO *colourinfotbl = (COLOURINFO *) fx->fref;

    int  force=0;
    int numchar = 11; // number of characters in method; up to 11 chars
    int i;

    sort.method[0] = PALSORT_COUNT;

    if (fx->option)
    {
        if (fx->numoptions>=1)
        {
            for (i=0; i < 11; i++)
                sort.method[i] = fx->option[0].p[i];
        }
        if (fx->numoptions>=2) force = fx->option[1].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        buildcolourinfotbl(fx, colourinfotbl, force); /* qsortable struct */

        if (sort.method[0] == PALSORT_ARGB)
        {
            strcpy(sort.method, "argb");
        }
        else if (sort.method[0] == PALSORT_HVS)
        {
            strcpy(sort.method, "ahls");
        }
        else if (sort.method[0] == PALSORT_WHVS)
        {
            strcpy(sort.method, "wwww");
        }
        else if (sort.method[0] == PALSORT_ICOUNT)
        {
            strcpy(sort.method, "p");
        }
        else if (sort.method[0] == PALSORT_YUV)
        {
            strcpy(sort.method, "yuv");
        }
        else if (sort.method[0] == PALSORT_COUNT)
        {
            strcpy(sort.method, "P");
        }

        numchar = abs(validcharacter());
        if (!numchar)
        {
            strcpy(sort.method, "p");
            sort.length = 1;
        }
        sort.length = numchar;
        qsort(colourinfotbl, ginfo->numcolours, sizeof(COLOURINFO), compare);
        updatecolourtbl(colourinfotbl, ginfo->numcolours, ginfo->colourtbl);
    }
    return(ginfo);
}

FPUBLIC int filterpalsort(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    int i;
    unsigned char *s;
    unsigned char *d;
    char *source= (char *) fx->source[0].image;
    COLOURINFO *colourinfotbl = (COLOURINFO *) fx->fref;
    unsigned char colourx[256];

    for (x=0; x<256; ++x)
        colourx[x] = x;

    for (x=0; x<256; ++x)
    {
        if (colourinfotbl[x].index<ginfo->numcolours && x<ginfo->numcolours)
            colourx[colourinfotbl[x].index] = (unsigned char)x;
    }

    for (y=0; y<ginfo->height; ++y)
    {
        s = (unsigned char *) source;
        d = (unsigned char *) dest;
        for (x=0; x<ginfo->width; ++x)
        {
            *d++ = colourx[*s++];
        }
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }

    if (fx->option)
    {
        if (fx->numoptions>=1)
        {
            for (i=0; i < 11; i++)
                sort.method[i] = fx->option[0].p[i];
        }
    }

    if ((sort.method[0] == 0) || (sort.method[0] == 1) || (sort.method[0] == 2) || (sort.method[0] == 3) || (sort.method[0] == 4) ||
        (sort.method[0] == 5))
        return (1);
    else
    {
        i=validcharacter();
        if (i<=0)
            return (i);
    }
    return (1);
}

FPUBLIC int closepalsort(FINSTANCE *fx)
{
    if (fx)
    {
        if (fx->fref)
            gfree(fx->fref);
        gfree((void *)fx);
    }
    return(1);
}

