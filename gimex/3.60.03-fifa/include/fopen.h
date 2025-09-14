// (c) 1995-2007 Electronic Arts Inc.

/*
; ABSTRACT
;
; fopen.h - Header file defining the filter interface.
;
; SOURCE
;@
*/

#ifndef __FOPEN_H
#define __FOPEN_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

#define FTYPEACENTER          FILTER_find("acenter")
#define FTYPEALPHACOMMENT     FILTER_find("alphacomment")
#define FTYPEACOPY            FILTER_find("acopy")
#define FTYPEBLEND            FILTER_find("blend")
#define FTYPEPSNR             FILTER_find("psnr")
#define FTYPEBLUR             FILTER_find("blur")
#define FTYPECHROMA           FILTER_find("chroma")
#define FTYPECOLBAL           FILTER_find("colbal")
#define FTYPECONTRAST         FILTER_find("contrast")
#define FTYPECREEP            FILTER_find("creep")
#define FTYPECROP             FILTER_find("crop")
#define FTYPECSMAP            FILTER_find("csmap")
#define FTYPEDEBAND           FILTER_find("deband")
#define FTYPEDEINTERLACE      FILTER_find("deinterlace")
#define FTYPEDILATE           FILTER_find("dilate")
#define FTYPEDOUBLED          FILTER_find("doubled")
#define FTYPEEDGE             FILTER_find("edge")
#define FTYPEERODE            FILTER_find("erode")
#define FTYPEEQUALIZE         FILTER_find("equalize")
#define FTYPEFASTBLUR         FILTER_find("fast blur")
#define FTYPECHANMAP          FILTER_find("chanmap")
#define FTYPEFILL             FILTER_find("fill")
#define FTYPEFLIP             FILTER_find("flip")
#define FTYPEFNIL             FILTER_find("fnil")
#define FTYPEGAMMA            FILTER_find("gamma")
#define FTYPEGRB              FILTER_find("grb")
#define FTYPEGREY             FILTER_find("grey")
#define FTYPEGRID             FILTER_find("grid")
#define FTYPEHALFSIZE         FILTER_find("halfsize")
#define FTYPEHSL              FILTER_find("hsl")
#define FTYPEHSV              FILTER_find("hsv")
#define FTYPEINVERT           FILTER_find("invert")
#define FTYPELINE             FILTER_find("line")
#define FTYPEMATH             FILTER_find("math")
#define FTYPEMEDIAN           FILTER_find("median")
#define FTYPEMIPMONT          FILTER_find("mipmont")
#define FTYPEPALSORT          FILTER_find("palsort")
#define FTYPEPIXEL            FILTER_find("pixel")
#define FTYPEPRETEN           FILTER_find("preten")
#define FTYPEPSORT            FILTER_find("psort")
#define FTYPERADFILL          FILTER_find("radfill")
#define FTYPERECT             FILTER_find("rect")
#define FTYPEREMAP            FILTER_find("remap")
#define FTYPEROT              FILTER_find("rot")
#define FTYPEROTATE           FILTER_find("rotate")
#define FTYPESAFE             FILTER_find("safe")
#define FTYPESCALE            FILTER_find("scale")
#define FTYPESHADE            FILTER_find("shade")
#define FTYPESCREEN           FILTER_find("screen")
#define FTYPESHARPEN          FILTER_find("sharpen")
#define FTYPESHRINK           FILTER_find("shrink")
#define FTYPESIZE             FILTER_find("size")
#define FTYPESMOOTH           FILTER_find("smooth")
#define FTYPESTEREO           FILTER_find("stereo")
#define FTYPESWAP             FILTER_find("swap")
#define FTYPETEDGE            FILTER_find("tedge")
#define FTYPETEXTURE          FILTER_find("texture")
#define FTYPEWRAP             FILTER_find("wrap")
#define FTYPEXCOLOUR          FILTER_find("xcolour")
#define FTYPEZOOM             FILTER_find("zoom")
#define FTYPEEMBM             FILTER_find("embm")
#define FTYPEUNEMBM           FILTER_find("unembm")
#define FTYPEDROPSHAD         FILTER_find("drops")
#define FTYPEDOT3             FILTER_find("dot3")
#define FTYPEUNDOT3           FILTER_find("undot3")
#define FTYPEMAKTEXMAP        FILTER_find("ftxmp")
#define FTYPEWARP             FILTER_find("warp")
#define FTYPEDXT              FILTER_find("fdxt")
#define FTYPETRANSPOSE        FILTER_find("trans")



int     GCALL FILTER_init(void);
void    GCALL FILTER_restore(void);

FABOUT* GCALL FILTER_about(void);

int     GCALL FILTER_open(FINSTANCE **fx, int numsources, const GBITMAP *sources, int numoptions, const FOP *options);
GINFO * GCALL FILTER_info(FINSTANCE *fx, int framenum);
int     GCALL FILTER_filter(FINSTANCE *fx, const GINFO *ginfo, void *dest, int rowbytes);
int     GCALL FILTER_close(FINSTANCE *gx);

int     GCALL FILTER_find(const char *name); /* choose which filter "Zoom"... */
void    GCALL FILTER_set(int typenum);
int     GCALL FILTER_get(void);
int     GCALL FILTER_count(void);

/****************************************************************/
/* Old Interface for Compatibility                              */
/****************************************************************/
#define aboutfilter() FILTER_about()
#define openfilter(fx, numsources, sources, numoptions, options) FILTER_open(fx,numsources,sources,numoptions,options)
#define infofilter(fx, framenum) FILTER_info(fx,framenum)
#define filterfilter(fx, ginfo, dest, rowbytes) FILTER_filter(fx,ginfo,dest,rowbytes)
#define closefilter(fx) FILTER_close(fx)
#define setfilter(typenum) FILTER_set(typenum)
#define getfilter() FILTER_get()
#define getnumfilter() FILTER_count()


#ifdef __cplusplus
}
#endif
#endif

/* END ABSTRACT */
