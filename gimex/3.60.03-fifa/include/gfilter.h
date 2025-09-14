// (c) 1995-2007 Electronic Arts Inc.

/*
; ABSTRACT
;
; gfilter.h - Primary header file for the GFilter API.
;
; SOURCE
;@
*/

/*------------------------------------------------------------------*/
/*                                                                  */
/*                        GFilter API v1.09                         */
/*                                                                  */
/*       by FrANK G. Barchard, Gerry Shaw, Spencer Cathey, EAC      */
/*                                                                  */
/*                    Header Module - Jun 15, 2002                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    0.90 950428 GS initial version (SCathey,GShaw,FBarchard)      */
/*    1.00 950510 FB name tweeks, gimex.h consistancy               */
/*    1.01 951017 FB moved FBITMAP to gimex.h and renamed GBITMAP   */
/*    1.02 951102 FB removed uref from FINSTANCE                    */
/*    1.03 951103 FB added draw class                               */
/*    1.04 951107 FB enum;removed DEBUG,external,anyorder,memory    */
/*    1.05 960209 GS added canoverwritesource FABOUT option         */
/*    1.06 010313 FB renamed to gfilter                             */
/*    1.07 010427 FB GFILTER_VERSION                                */
/*    1.08 010926 FB fref void *                                    */
/*    1.09 020615 FB fop.p const char *                             */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef __GFILTER_H
#define __GFILTER_H 1

#if defined(_MSC_VER)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* If gimex.h has been included before then this source is being included by */
/* something outside the gfilter libarary so this then make all the public    */
/* functions static */
#ifdef GFILTER_STATIC
#define FPUBLIC static
#else
#define FPUBLIC
#endif

#include "gimex.h"

#define GFILTER_VERSION 109

#if defined(_MSC_VER)
// warning C4996: 'strcpy' was declared deprecated
#pragma warning(disable: 4996)
#endif

/****************************************************************/
/*  Data Types                                                  */
/****************************************************************/

/* FPALETTE structure describes a palette */

typedef struct
{
    int   startcolour;
    int   numcolours;
    ARGB *colourtbl;
    void *inversetbl;
} FPALETTE;

/* floating point ARGB */
typedef struct
{
    float a;
    float r;
    float g;
    float b;
} FARGB;

typedef struct
{
    float x,y,z;            /* projected x & y */
    float w;                /* 1/z for prespective correcting */
    FARGB colour;           /* normal lighting */
    FARGB specular;         /* specular lighting */
    float u, v;
} FTEXTURE_VERTEX;

/* Option types */

#define FOP_INT         0x0001      /* default num gadget */
#define FOP_FLOAT       0x0002      /* default double gadget */
#define FOP_PTR         0x0003      /* default is block of memory */

/* Option modifiers */

#define FOP_ENUM        0x0200      /* option has enumerated types */
#define FOP_FPALETTE    0x0400      /* mask with FOP_PTR to make a FPALETTE structure */
#define FOP_TRUE_COLOR  0x0800      /* mask with FOP_INT */
#define FOP_INDEX_COLOR 0x1000      /* mask with FOP_INT */

/* GFilter Classes */

#define FCLASS_GENERAL   0
#define FCLASS_DRAW      1
#define FCLASS_COMPOSITE 2

/* FOP structure describes gfilter options */

typedef union
{
    int         i;    /* FOP_INT */
    double      f;    /* FOP_FLOAT */
    const char *p;    /* FOP_PTR */
#if REALGRAPH_VERSION<600
    ARGB        argb; /* FOP_INT | FOP_ARGB */
#endif
} FOP;

typedef struct
{
    int        type;           /* type of option (ie: FOP_INT) */
    FOP         min;            /* minimum value */
    FOP         max;            /* maximum value */
    FOP         def;            /* default value */
    char        name[32];       /* name of the option */
    char        enumlist[256];  /* enumerated list of values starting at 0 */
} FOPTION;

typedef struct
{
    int         frames;     /* number of resulting destination frames */
    void *      fref;       /* gfilter reference to additional memory */
    int         numsources; /* number of source frames */
    GBITMAP *   source;     /* list of GBITMAP pointers to source images */
    int         numoptions; /* number of options specified */
    FOP *       option;     /* list of option values */
} FINSTANCE;

/* FABOUT structure describes gfilter module capabilities */

#define FMAXOPTIONS  35

typedef struct
{
    int signature;                  /* gfilter module signature */
    int size;                       /* size of the FABOUT structure */
    int version;                    /* version of the FABOUT structure ie 104 */
    unsigned int filter8    :1;     /* supports processing 8-bit images */
    unsigned int filter32   :1;     /* supports processing 32-bit images */
    unsigned int multisize  :1;     /* supports different size source images */
    unsigned int pad        :13;    /* pad to 32 bits */
    unsigned int pad2       :16;    /* pad to 32 bits */
    int  minbitmaps;                /* min # of sources gfilter requires */
    int  maxbitmaps;                /* max # of sources gfilter requires */
    int  canoverwritesource;        /* supports passing dst buffer same as src buffer */
    int  filterclass;               /* kind of gfilter (0 general purpose) */
    char authorstr[32];             /* gfilter module author */
    char versionstr[8];             /* version number of gfilter module ie 1.00 */
    char namestr[16];               /* single word name of the gfilter */
    char longnamestr[32];           /* full name of the gfilter */
    int  numoptions;                /* maximum number of options gfilter can take */
    FOPTION option[FMAXOPTIONS];    /* gfilter arguments */
} FABOUT;


/****************************************************************/
/*  GFilter Module Example Prototypes                           */
/****************************************************************/

FPUBLIC FABOUT *      aboutflip(void);

FPUBLIC int           openflip(FINSTANCE **fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO *       infoflip(FINSTANCE *fx, int framenum);
FPUBLIC int           filterflip(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int           closeflip(FINSTANCE *fx);

#ifdef __cplusplus
}
#endif
#endif

/* END ABSTRACT */
