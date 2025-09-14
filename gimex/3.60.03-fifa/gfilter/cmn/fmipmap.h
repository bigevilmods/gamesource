// (c) 1995-2007 Electronic Arts Inc.

#ifndef __FMIPMAP_H
#define __FMIPMAP_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Mipmap Filter                                                */
/****************************************************************/

FPUBLIC FABOUT *aboutmipmap(void);

FPUBLIC int     openmipmap(FINSTANCE* *fx, int numsources, GBITMAP *sources, int numoptions, FOP *options);
FPUBLIC GINFO * infomipmap(FINSTANCE *fx, int framenum);
FPUBLIC int     filtermipmap(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes);
FPUBLIC int     closemipmap(FINSTANCE *fx);

//---------------------------------------
// internal state information
typedef struct MipmapInternals
{
    int current;                     // current mipmap to be return on a call to filter
    int totalmipmaps;                // total number of mipmaps that were generated
    int sizex;                       // size of base texture
    int sizey;                       // size of base texture
    int OPTmipmapmethod;             // choice of mipmap method  BOX etc
    int OPTmipmaptrans;              // transparency flag for size filter
    int OPTmipmapclearlast;          // should I clear the last N frames to a blank colour?
    int OPTbackground;               // what colour should I clear to
    int OPTmipmapcollapse;           // turn of the nomipmapcollapse flag
    int OPTmipmap;                   // max number of mipmaps to generate
    int OPTmipmapmin;                // minimum number of levels to generate
    GINFO ** ginfomany;              // holders for generated mipmaps and ginfo's
    char  ** fmany;
}MipmapInternals;

//---------------------------------------
// structure holding information about
// my original bit depth for later
// colour reduction
typedef struct SavedItems
{
    int  SavedOriginalBpp;
    int  SavedOriginalNumColours;
    char SavedABits;
    char SavedRBits;
    char SavedGBits;
    char SavedBBits;
    bool did_upsample;
}SavedItems;

//---------------------------------------
// Method parameters and defaults
#define DEFAULT_NUM_MIPMAPLEVELS 16                    // the default number of mipmap levels
#define OPTMIPMAPDEFAULT          3
#define OPTMIPMAPSCALE            0                    // use the scale filter (point sample) to do mipmapping
#define OPTMIPMAPBOX              1                    // use the halfsize filter to do mipmapping
#define OPTMIPMAPSIZE             2                    // use the size filter to do mipmapping
#define OPTMIPMAPSIZECOLREDUCE    3                    // same as mipmaps but does global colour reduction to retain original bit depth

#define M_SUBNOMIPMAPCOLLAPSE       0x200                // internal gx subtype to be turned off is user requests it
#ifdef __cplusplus
}
#endif

#endif

