// (c) 1997-2007 Electronic Arts Inc.

#ifndef __FTEXTURE_H
#define __FTEXTURE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Texture Map Filter                                           */
/****************************************************************/

FPUBLIC FABOUT* abouttexture(void);

FPUBLIC int     opentexture(FINSTANCE** fx, int numsources, GBITMAP sources[], int numoptions, FOP options[]);
FPUBLIC GINFO*  infotexture(FINSTANCE* fx, int framenum);
FPUBLIC int     filtertexture(FINSTANCE* fx, GINFO* ginfo, char* dst, int rowbytes);
FPUBLIC int     closetexture(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif

