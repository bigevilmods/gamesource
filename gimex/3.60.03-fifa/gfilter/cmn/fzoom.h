// (c) 1995-2007 Electronic Arts Inc.

#ifndef __ZOOMFILTER
#define __ZOOMFILTER 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gfilter.h"

/****************************************************************/
/* Zoom Filter                                                  */
/****************************************************************/

FPUBLIC FABOUT*     aboutzoom(void);

FPUBLIC int         openzoom(FINSTANCE** fx, int numsources, GBITMAP* sources, int numoptions, FOP* options);
FPUBLIC GINFO*      infozoom(FINSTANCE* fx, int framenum);
FPUBLIC int         filterzoom(FINSTANCE* fx, GINFO* ginfo, char* dest, int rowbytes);
FPUBLIC int         closezoom(FINSTANCE* fx);

#ifdef __cplusplus
}
#endif
#endif

#if !defined(BICZOOM2DOTH)
#define BICZOOM2DOTH
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;

#define FIXZ        int
#define FIXZ_SHIFT  10
#define FIXZ_VALUE  (1024)
#define FIXZ_MASK   (1023)

FIXZ lZoomFactor(int numsrcpixels, int numdstpixels);
FIXZ lFillKernelTable( short pnKernel[], WORD wKernelSamples, FIXZ lFilter );
int iScanZoom( BYTE* pbySrc,
            FIXZ  lSrcCenter,
            WORD  wSrcPixelPitch,
            short nSrcPixels,
            BYTE* pbyDst,
            WORD  wDstPixelPitch,
            short nDstPixels,
            FIXZ  lScale,
            FIXZ  lKernelWidth,
            WORD  wKernelSamples,
            short* pnKernel  );
int iScanZoomARGB( BYTE* pbySrc,
            FIXZ  lSrcCenter,
            WORD  wSrcPixelPitch,
            short nSrcPixels,
            BYTE* pbyDst,
            WORD  wDstPixelPitch,
            short nDstPixels,
            FIXZ  lScale,
            FIXZ  lKernelWidth,
            WORD  wKernelSamples,
            short* pnKernel  );



#ifdef __cplusplus
}
#endif
#endif
