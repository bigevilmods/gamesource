// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 Qmt - Quake Model Texture Gimex                  */
/*                                                                  */
/*                      by Julian Fong, EAC                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 970822 JF canimport initial version (single frame only)     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit canimport                                                     */
/* No compression                                                   */
/* Currently only single frame, until documentation deciphered      */
/* Files: qmtread.cpp qmtwrite.cpp qmtgimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* Simple 8 bit texture embedded in Quake model files.  Use MDL     */
/* TRIX to read geometry.                                           */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "qmtgimex.h"

/* Default Quake palette */
static const unsigned char qmt_palette[256][3] =
{
    {  0,   0,   0}, { 15,  15,  15}, { 31,  31,  31}, { 47,  47,  47},
    { 63,  63,  63}, { 75,  75,  75}, { 91,  91,  91}, {107, 107, 107},
    {123, 123, 123}, {139, 139, 139}, {155, 155, 155}, {171, 171, 171},
    {187, 187, 187}, {203, 203, 203}, {219, 219, 219}, {235, 235, 235},
    { 15,  11,   7}, { 23,  15,  11}, { 31,  23,  11}, { 39,  27,  15},
    { 47,  35,  19}, { 55,  43,  23}, { 63,  47,  23}, { 75,  55,  27},
    { 83,  59,  27}, { 91,  67,  31}, { 99,  75,  31}, {107,  83,  31},
    {115,  87,  31}, {123,  95,  35}, {131, 103,  35}, {143, 111,  35},
    { 11,  11,  15}, { 19,  19,  27}, { 27,  27,  39}, { 39,  39,  51},
    {47,   47,  63}, { 55,  55,  75}, { 63,  63,  87}, { 71,  71, 103},
    { 79,  79, 115}, { 91,  91, 127}, { 99,  99, 139}, {107, 107, 151},
    {115, 115, 163}, {123, 123, 175}, {131, 131, 187}, {139, 139, 203},
    {  0,   0,   0}, {  7,   7,   0}, { 11,  11,   0}, { 19,  19,   0},
    { 27,  27,   0}, { 35,  35,   0}, { 43,  43,   7}, { 47,  47,   7},
    { 55,  55,   7}, { 63,  63,   7}, { 71,  71,   7}, { 75,  75,  11},
    { 83,  83,  11}, { 91,  91,  11}, { 99,  99,  11}, {107, 107,  15},
    {  7,   0,   0}, { 15,   0,   0}, { 23,   0,   0}, { 31,   0,   0},
    { 39,   0,   0}, { 47,   0,   0}, { 55,   0,   0}, { 63,   0,   0},
    { 71,   0,   0}, { 79,   0,   0}, { 87,   0,   0}, { 95,   0,   0},
    {103,   0,   0}, {111,   0,   0}, {119,   0,   0}, {127,   0,   0},
    { 19,  19,   0}, { 27,  27,   0}, { 35,  35,   0}, { 47,  43,   0},
    { 55,  47,   0}, { 67,  55,   0}, { 75,  59,   7}, { 87,  67,   7},
    { 95,  71,   7}, {107,  75,  11}, {119,  83,  15}, {131,  87,  19},
    {139,  91,  19}, {151,  95,  27}, {163,  99,  31}, {175, 103,  35},
    { 35,  19,   7}, { 47,  23,  11}, { 59,  31,  15}, { 75,  35,  19},
    { 87,  43,  23}, { 99,  47,  31}, {115,  55,  35}, {127,  59,  43},
    {143,  67,  51}, {159,  79,  51}, {175,  99,  47}, {191, 119,  47},
    {207, 143,  43}, {223, 171,  39}, {239, 203,  31}, {255, 243,  27},
    { 11,   7,   0}, { 27,  19,   0}, { 43,  35,  15}, { 55,  43,  19},
    { 71,  51,  27}, { 83,  55,  35}, { 99,  63,  43}, {111,  71,  51},
    {127,  83,  63}, {139,  95,  71}, {155, 107,  83}, {167, 123,  95},
    {183, 135, 107}, {195, 147, 123}, {211, 163, 139}, {227, 179, 151},
    {171, 139, 163}, {159, 127, 151}, {147, 115, 135}, {139, 103, 123},
    {127,  91, 111}, {119,  83,  99}, {107,  75,  87}, { 95,  63,  75},
    { 87,  55,  67}, { 75,  47,  55}, { 67,  39,  47}, { 55,  31,  35},
    { 43,  23,  27}, { 35,  19,  19}, { 23,  11,  11}, { 15,   7,   7},
    {187, 115, 159}, {175, 107, 143}, {163,  95, 131}, {151,  87, 119},
    {139,  79, 107}, {127,  75,  95}, {115,  67,  83}, {107,  59,  75},
    { 95,  51,  63}, { 83,  43,  55}, { 71,  35,  43}, { 59,  31,  35},
    { 47,  23,  27}, { 35,  19,  19}, { 23,  11,  11}, { 15,   7,   7},
    {219, 195, 187}, {203, 179, 167}, {191, 163, 155}, {175, 151, 139},
    {163, 135, 123}, {151, 123, 111}, {135, 111,  95}, {123,  99,  83},
    {107,  87,  71}, { 95,  75,  59}, { 83,  63,  51}, { 67,  51,  39},
    { 55,  43,  31}, { 39,  31,  23}, { 27,  19,  15}, { 15,  11,   7},
    {111, 131, 123}, {103, 123, 111}, { 95, 115, 103}, { 87, 107,  95},
    { 79,  99,  87}, { 71,  91,  79}, { 63,  83,  71}, { 55,  75,  63},
    { 47,  67,  55}, { 43,  59,  47}, { 35,  51,  39}, { 31,  43,  31},
    { 23,  35,  23}, { 15,  27,  19}, { 11,  19,  11}, {  7,  11,   7},
    {255, 243,  27}, {239, 223,  23}, {219, 203,  19}, {203, 183,  15},
    {187, 167,  15}, {171, 151,  11}, {155, 131,   7}, {139, 115,   7},
    {123,  99,   7}, {107,  83,   0}, { 91,  71,   0}, { 75,  55,   0},
    { 59,  43,   0}, { 43,  31,   0}, { 27,  15,   0}, { 11,   7,   0},
    {  0,   0, 255}, { 11,  11, 239}, { 19,  19, 223}, { 27,  27, 207},
    {  35,  35,191}, { 43,  43, 175}, { 47,  47, 159}, { 47,  47, 143},
    { 47,  47, 127}, { 47,  47, 111}, { 47,  47,  95}, { 43,  43,  79},
    { 35,  35,  63}, { 27,  27,  47}, { 19,  19,  31}, { 11,  11,  15},
    { 43,   0,   0}, { 59,   0,   0}, { 75,   7,   0}, { 95,   7,   0},
    {111,  15,   0}, {127,  23,   7}, {147,  31,   7}, {163,  39,  11},
    {183,  51,  15}, {195,  75,  27}, {207,  99,  43}, {219, 127,  59},
    {227, 151,  79}, {231, 171,  95}, {239, 191, 119}, {247, 211, 139},
    {167, 123,  59}, {183, 155,  55}, {199, 195,  55}, {231, 227,  87},
    {127, 191, 255}, {171, 231, 255}, {215, 255, 255}, {103,   0,   0},
    {139,   0,   0}, {179,   0,   0}, {215,   0,   0}, {255,   0,   0},
    {255, 243, 147}, {255, 247, 199}, {255, 255, 255}, {159,  91,  83}
};


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL QMT_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));
        info->signature       = QMTSIGNATURE;
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 0;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
#ifdef NOTYET
        info->multiframe      = 1;
#else
        info->multiframe      = 0;    /* supports multiple frames */
#endif

        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID( 0 , 0 , 0 , 0 );   /* mac file system types used */
        strcpy(info->extensions[0], ".mdl");                /* extensions */
        strcpy(info->authorstr,     "Julian Fong");         /* author */
        strcpy(info->versionstr,    "1.00");                /* version # */
        strcpy(info->shorttypestr,  "QMT");                 /* type */
        strcpy(info->wordtypestr,   "Quake Model");         /* wordtype */
        strcpy(info->longtypestr,   "Quake Model Texture Gimex");           /* longtype */
    }
    return(info);
}

int GCALL QMT_is(GSTREAM *g)
{
    int ok = 0;
    char buf[4];

    gseek(g, 0);
    if (gread(g, buf, sizeof(buf)))
    {
        if (strncmp(buf, "IDPO", 4) == 0)
        {
            ok = 100;
        }
    }
    gseek(g, 0);
    return (ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL QMT_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    int ok;
    mdl_t * header;
    GINSTANCE * gx = NULL;

    /* Allocate instance */
    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (!gx)
        goto Error;

    memset(gx, 0, sizeof(GINSTANCE));
    gx->signature = QMTSIGNATURE;
    gx->size = sizeof(GINSTANCE);
    gx->gstream = g;
    gx->framenum = -1;
    *gxptr = gx;

    /* Allocate copy of model header */
    gx->gref = galloc(sizeof(mdl_t));
    if (!gx->gref)
        goto Error;
    memset(gx->gref, 0, sizeof(mdl_t));
    header = (mdl_t *) gx->gref;

    ok = (sizeof(mdl_t) == gread(g, header, sizeof(mdl_t)));
    if (!ok)
        goto Error;

    /* byte-swap the relevant fields */
    header->numskins = QMTGETFIELD(header->numskins);
    header->skinwidth = QMTGETFIELD(header->skinwidth);
    header->skinheight = QMTGETFIELD(header->skinheight);

#ifdef NOTYET
    return (header->numskins);
#else
    return (1);
#endif

    Error:
    if (gx)
    {
        if (gx->gref)
            gfree(gx->gref);
        gfree(gx);
    }
    return(0);
}


GINFO * GCALL QMT_info(GINSTANCE *gx, int framenum)
{
    mdl_t * header;
    GINFO * ginfo = NULL;
    int i;
    if (gx)
    {
        header = (mdl_t *) gx->gref;
        if (framenum < gx->frames)
            ginfo = (GINFO *) galloc(sizeof(GINFO));

        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));
            ginfo->signature = QMTSIGNATURE;
            ginfo->size = sizeof(GINFO);
            ginfo->version = GIMEX_VERSION;
            ginfo->framenum = 0;
            ginfo->width = header->skinwidth;
            ginfo->height = header->skinheight;
            ginfo->bpp = 8;
            ginfo->originalbpp = 8;
            ginfo->startcolour = 0;
            ginfo->numcolours = 256;
            /* Apparently palette entry 0 is transparent */
            ginfo->colourtbl[0].a = 0x00;
            ginfo->colourtbl[0].r = 0x00;
            ginfo->colourtbl[0].g = 0x00;
            ginfo->colourtbl[0].b = 0x00;
            for (i = 1; i < 256; i++)
            {
                ginfo->colourtbl[i].a = 0xff;
                ginfo->colourtbl[i].r = qmt_palette[i][0];
                ginfo->colourtbl[i].g = qmt_palette[i][1];
                ginfo->colourtbl[i].b = qmt_palette[i][2];
            }
            ginfo->subtype = 0;
            ginfo->packed = 0;
            ginfo->quality = 0;
            ginfo->alphabits = 8;
            ginfo->redbits = 8;
            ginfo->greenbits = 8;
            ginfo->bluebits = 8;
        }
    }
    return(ginfo);
}

int GCALL QMT_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    mdl_t * header;
    int group, size;
    int ok = 0;

    if (gx && gx->gstream)
    {
        header = (mdl_t *) gx->gref;
        gseek(gx->gstream, 0x54);
        ok = (sizeof(group) == gread(gx->gstream, &group, sizeof(group)));
        if (!ok)
            goto Error;

        group = QMTGETFIELD(group);

        /* Single skin */
        if (group == 0)
        {
            size = header->skinwidth * header->skinheight;
            ok = (size == gread(gx->gstream, dest, size));
            if (!ok)
                goto Error;
        }
#ifdef NOTYET
        /* handle conglomerate skins??? */
        {
        }
#else
        else
            goto Error;
#endif
    }
    return (ok);
    Error:
    return(0);
}

int GCALL QMT_close(GINSTANCE *gx)
{
    int ok = 1;

    if (gx)
    {
        if (gx->gref)
        {
            gfree(gx->gref);
        }
        gfree(gx);
    }
    return (ok);
}


