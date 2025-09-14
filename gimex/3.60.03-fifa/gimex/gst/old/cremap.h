// (c) 2004-2007 Electronic Arts Inc.
// CRAPI remap function included here due to necessity


#ifndef CREMAP_H
#define CREMAP_H

#include "gimex.h"

namespace Gst
{
    
void remap(void *destpixels,
           const GINFO *destginfo,
           int destrowbytes,
           void *sourcepixels,
           int sourcerowbytes,
           int sourcebpp,
           const unsigned char *colourmask,
           int dither);
 
}

#endif
