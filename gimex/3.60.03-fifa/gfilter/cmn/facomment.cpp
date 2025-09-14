// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*       Alphacomment - Imbed text comment in alpha channel         */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 990809 FB based on invert                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* alphacomment(int flag)                                           */
/* int flag;  0=na,1=add,2=extract                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "facomment.h"
#include <string.h>

/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

#define AENCODE(NBIT) dest->a = (((unsigned char)(*comment)>>NBIT)&1)?255:0; ++dest
#define ADECODE(NBIT) *comment |= ((dest->a>>7)&1)?(1<<NBIT):0; ++dest

static void alphacomment(ARGB *dest,int n,int addextract,char *comment)
{
    int i;
    int bit;

    bit = 7;
    for (i=0; i<n; ++i)
    {
        if (addextract==1)
        {
            AENCODE(bit);
        }
        else
        {
            ADECODE(bit);
        }
        --bit;
        if (bit<0)
        {
            bit = 7;
            ++comment;
        }
    }
}

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT *aboutalphacomment(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('A','C','M','T');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "alphacomment");
        strcpy(about->longnamestr,  "Imbed comment in alpha");
        about->numoptions =         1;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           0;
        about->option[0].max.i =           1;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "addextract");
        strcpy(about->option[0].enumlist,  "");

    }
    return(about);
}

FPUBLIC int openalphacomment(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;

    fx = (FINSTANCE *) galloc(sizeof(FINSTANCE));
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

FPUBLIC GINFO *infoalphacomment(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    char *comment;
    unsigned int addextract=0;
    if (fx->option)
    {
        if (fx->numoptions>=1) addextract = fx->option[0].i;
    }

    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    comment = &ginfo->comment[0];
    if (ginfo)
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
        if (addextract==2) //extract
            memset(ginfo->comment,0,sizeof(ginfo->comment));

        if (addextract)
            alphacomment(ginfo->colourtbl,256,addextract,comment);
    }
    return(ginfo);
}

FPUBLIC int filteralphacomment(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int i;
    char *source= (char *) fx->source[0].image;
    int width  = ginfo->width;
    int bpl    = ginfo->width*ginfo->bpp/8;
    int todo=0;
    char *comment;
    unsigned int addextract=0;
    if (fx->option)
    {
        if (fx->numoptions>=1) addextract = fx->option[0].i;
    }

    comment = &ginfo->comment[0];


    for (i=0; i<ginfo->height; ++i)
    {
        memcpy(dest,source,bpl);
        if (ginfo->bpp==32 && addextract)
        {
            if (addextract==1) // add
                todo = gmin(((int)strlen(comment)+1)*8,width);
            else
                todo = gmin((int)sizeof(ginfo->comment)-(comment-&ginfo->comment[0])*8,width);
            if (todo)
                alphacomment((ARGB *)dest,todo,addextract,comment);
        }
        comment += (todo/8);
        if (comment-&ginfo->comment[0])
        source += fx->source[0].rowbytes;
        dest   += rowbytes;
    }
    return(1);
}

FPUBLIC int closealphacomment(FINSTANCE *fx)
{
    return(gfree((void *)fx));
}

