// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*            PSort - Sort Palette Entries and Remap Image          */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*                    Filter Module - Nov 13, 1996                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 961113 GS total rewrite based on fpalsort                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit                                                            */
/* psort(char sortstring)                                           */
/* char sortstring; order of how to sort colors "ARGBHSVP"          */
/*                                                                  */
/* valid sortstring characters (CASE DEPENDENT):                    */
/*  A  alpha                                                        */
/*  R  red                                                          */
/*  G  green                                                        */
/*  B  blue                                                         */
/*  H  hue                                                          */
/*  S  saturation                                                   */
/*  V  value                                                        */
/*  I  color table index (location in color table)                  */
/*  P  popularity (most used colors)                                */
/*  p  inverse popularity (least used colors)                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <stdlib.h>
#include <string.h>
#include "fpsort.h"

// CiX code for color space conversion
#define QPUBLIC static
#include "hsv.h"
//#include "rgb_hls.c"
//#include "rgb_yiv.c"
//#include "rgb_cmyk.c"
#undef QPUBLIC

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

typedef struct
{
    unsigned char a;    /* alpha channel 0..255 */
    unsigned char r;    /* red channel 0..255 */
    unsigned char g;    /* green channel 0..255 */
    unsigned char b;    /* blue channel 0..255 */
    int  i;             /* original index 0..255 */
    int  p;             /* histogram 0.. */
} PSORTCOLOUR;


/* Compare Functions */

static int ACompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;
    return (c2->a - c1->a);
}

static int RCompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;
    return (c2->r - c1->r);
}

static int GCompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;
    return (c2->g - c1->g);
}

static int BCompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;
    return (c2->b - c1->b);
}

static int HCompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;

    double rgb1[3];
    double rgb2[3];
    double hsv1[3];
    double hsv2[3];

    double result;

    rgb1[0] = c1->r / 255.0;
    rgb1[1] = c1->g / 255.0;
    rgb1[2] = c1->b / 255.0;
    rgb_hsv(rgb1, hsv1);

    rgb2[0] = c2->r / 255.0;
    rgb2[1] = c2->g / 255.0;
    rgb2[2] = c2->b / 255.0;
    rgb_hsv(rgb2, hsv2);

    //return (int)((hsv2[0] - hsv1[0])*10); // *10 to ensure -0.003 does not be come 0
    result = hsv2[0] - hsv1[0];
    if (result != 0.0)
    {
        result = hsv2[1] - hsv1[1];
        if (result != 0.0)
        {
            result = hsv2[2] - hsv1[2];
        }
    }

    return (int)(result*100);

}

static int SCompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;

    double rgb1[3];
    double rgb2[3];
    double hsv1[3];
    double hsv2[3];

    rgb1[0] = c1->r / 255.0;
    rgb1[1] = c1->g / 255.0;
    rgb1[2] = c1->b / 255.0;
    rgb_hsv(rgb1, hsv1);

    rgb2[0] = c2->r / 255.0;
    rgb2[1] = c2->g / 255.0;
    rgb2[2] = c2->b / 255.0;
    rgb_hsv(rgb2, hsv2);

    return (int)((hsv2[1] - hsv1[1])*10);   // *10 to ensure -0.003 does not be come 0
}

static int VCompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;

    double rgb1[3];
    double rgb2[3];
    double hsv1[3];
    double hsv2[3];

    double result;

    rgb1[0] = c1->r / 255.0;
    rgb1[1] = c1->g / 255.0;
    rgb1[2] = c1->b / 255.0;
    rgb_hsv(rgb1, hsv1);

    rgb2[0] = c2->r / 255.0;
    rgb2[1] = c2->g / 255.0;
    rgb2[2] = c2->b / 255.0;
    rgb_hsv(rgb2, hsv2);

    //return (int)((hsv2[2] - hsv1[2])*10); // *10 to ensure -0.003 does not be come 0

    result = hsv2[2] - hsv1[2];
    if (result != 0.0)
    {
        result = hsv2[1] - hsv1[1];
        if (result != 0.0)
        {
            result = hsv2[0] - hsv1[0];
        }
    }

    return (int)(result*100);

}


static int ICompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;
    return (c2->i - c1->i);
}

static int PCompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;
    return (c2->p - c1->p);
}

static int pCompare(const void *elem1, const void *elem2)
{
    PSORTCOLOUR *c1 = (PSORTCOLOUR*)elem1;
    PSORTCOLOUR *c2 = (PSORTCOLOUR*)elem2;
    return (c1->p - c2->p);
}


static void BuildPSortColourTable(GBITMAP* bmp, PSORTCOLOUR* table)
{
    // clear table
    memset(table, 0, sizeof(PSORTCOLOUR)*256);

    // get info based on palette
    {
        ARGB* colourtbl = bmp->ginfo->colourtbl;
        int i;

        for (i = 0; i < bmp->ginfo->numcolours; i++)
        {
            table[i].i = i;
            table[i].a = colourtbl[i].a;
            table[i].r = colourtbl[i].r;
            table[i].g = colourtbl[i].g;
            table[i].b = colourtbl[i].b;

            // TODO: calc hsv values
        }
    }

    // get info based on image
    {
        unsigned char* base = (unsigned char*)bmp->image;
        int rowbytes = bmp->rowbytes;
        int x;
        int y;
        int w = bmp->ginfo->width;
        int h = bmp->ginfo->height;

        for (y = 0; y < h; y++)
        {
            unsigned char* s = base + y*rowbytes;
            for (x = 0; x < w; x++)
            {
                int index = *s++;
                table[index].p += 1;
            }
        }
    }
}

static void BuildGInfoColourTable(PSORTCOLOUR* table, GINFO* ginfo)
{
    int i;
    for (i = 0; i < ginfo->numcolours; i++)
    {
        ginfo->colourtbl[i].a = table[i].a;
        ginfo->colourtbl[i].r = table[i].r;
        ginfo->colourtbl[i].g = table[i].g;
        ginfo->colourtbl[i].b = table[i].b;
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FABOUT* aboutpsort(void)
{
    FABOUT* about = (FABOUT*) galloc(sizeof(FABOUT));
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
        strcpy(about->authorstr,    "G.Shaw");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "PSort");
        strcpy(about->longnamestr,  "Palette Sort");
        about->numoptions =         1;

        about->option[0].type =            FOP_PTR;
        strcpy(about->option[0].name,      "SortParms");
    }
    return(about);
}

int openpsort(FINSTANCE** filterInstance, int numsources, GBITMAP* sources, int numoptions, FOP* options)
{
    FINSTANCE* fx = 0;
    int        ok = 0;
    void*      table;

    fx = (FINSTANCE*)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        table = galloc(sizeof(PSORTCOLOUR)*256);
        if (table)
        {
            memset(fx, 0, sizeof(FINSTANCE));
            fx->frames      = 1;
            fx->numsources  = numsources;
            fx->source      = sources;
            fx->numoptions  = numoptions;
            fx->option      = options;
            fx->fref        = table;
            ok = 1;
        }
    }
    *filterInstance = fx;
    return(ok);
}

GINFO* infopsort(FINSTANCE* fx, int framenum)
{
    GINFO *ginfo;
    PSORTCOLOUR* table = (PSORTCOLOUR*) fx->fref;
    char sortoptions[128];

    strcpy(sortoptions, "RGB"); // default sort options

    if (fx->option)
    {
        if (fx->numoptions>=1) strcpy(sortoptions, fx->option[0].p);
    }

    ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)
    {
        char* option;

        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);

        BuildPSortColourTable(&(fx->source[0]), table);

        // now sort the table using all the methods in the sort options string
        // go backwards through the string though (remember CMPT 101 - sorting chapter)
        option = sortoptions + strlen(sortoptions) - 1;

        while (option >= sortoptions)
        {
            switch (*option)
            {
            case 'A':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), ACompare);
                break;

            case 'R':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), RCompare);
                break;

            case 'G':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), GCompare);
                break;

            case 'B':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), BCompare);
                break;

            case 'H':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), HCompare);
                break;

            case 'S':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), SCompare);
                break;

            case 'V':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), VCompare);
                break;

            case 'I':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), ICompare);
                break;

            case 'P':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), PCompare);
                break;

            case 'p':
                qsort(table, ginfo->numcolours, sizeof(PSORTCOLOUR), pCompare);
                break;

            }

            option--;
        }

        BuildGInfoColourTable(table, ginfo);
    }

    return ginfo;
}

int filterpsort(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x;
    int y;
    unsigned char *s;
    unsigned char *d;
    char *source= (char *) fx->source[0].image;
    PSORTCOLOUR* table = (PSORTCOLOUR*)fx->fref;
    unsigned char colourx[256];

    for (x=0; x<256; ++x)
        colourx[x] = (unsigned char)x;

    for (x=0; x<256; ++x)
    {
        if (table[x].i < ginfo->numcolours && x<ginfo->numcolours)
            colourx[table[x].i] = (unsigned char)x;
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
    return 1;
}

int closepsort(FINSTANCE *fx)
{
    if (fx)
    {
        if (fx->fref)
            gfree(fx->fref);
        gfree((void *)fx);
    }
    return 1;
}

