// (c) 1994-2007 Electronic Arts Inc.
// ------------------------------------------------------------
//                      Gimex Gst Glue
//                     ----------------
// The module contains prototypes for all common gst functions
// and includes the two shape specific modules.
// ------------------------------------------------------------


#include "gimex.h"
#include "gst.h"
#include "oldshpgstglue.h"
#include "newshpgstglue.h"

#ifndef GSTGLUE_H
#define GSTGLUE_H


namespace Gst
{
    // ------------------------------------------------------------
    // Create the FourCC packed code from the shape type.
    // FourCC is GST# where # ranges from 0..7 with bits
    // representing:
    // 0011 0bww
    // (b)  = detail bpp  0 = 1 bpp 1 = 2 bpp
    // (ww) = block width 0 = 1 pixels
    //                    1 = 2 pixels
    //                    2 = 4 pixels
    //                    3 = 8 pixels
    unsigned int GetGstFourCC(unsigned char stype);

	// ------------------------------------------------------------
	// get the gst type from the ginfo pack code
	// NOTE: this function takes the shape type for futher
	// expantion in the future. We may have types that don't
	// follow the shape rules that we have laid down.
	GstType GetGstType(unsigned int packed,int type);

    // ------------------------------------------------------------
    // returns true if the ginfo pack code referes to a GST type
	bool IsGstFourCC(unsigned int packed);

    // ------------------------------------------------------------
	// do the gst encode operation, shape type independant
	// returns a buffer with the compressed data and the ginfo
	// has been modified to have the new clut.
	void * DoGstEncode(GINFO * ginfo,GstImageInfo * info,const unsigned char * source,unsigned int * sizeofGstData,int type);


}

#endif
