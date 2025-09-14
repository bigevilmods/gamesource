// (c) 1994-2007 Electronic Arts Inc.
// ------------------------------------------------------------
//                NEWSHP Gst Gimex Glue Functions
//               ---------------------------------
// This module holds all the shape2 specific functions
// for use in Gimex.
// ------------------------------------------------------------


#ifndef NEWSHPGSTGLUE_H
#define NEWSHPGSTGLUE_H

#include "shpgimex.h"

namespace Gst
{
    // ------------------------------------------------------------
    void SHPDecompressGstShape(SHPHeaderStruct * shpin,char * shpout,int type);

    // ------------------------------------------------------------
    unsigned int SHPSetGstShape(SHPHeaderStruct ** image,GINFO * ginfo,const unsigned char * source, int type);
 
    // ------------------------------------------------------------
    // returns true if the shape itself is GST compressed.
	bool SHPIsGst(SHPHeaderStruct * AdrShape);

    // ------------------------------------------------------------
    // returns true if the image can be GST compressed
    // the type field is intended to control what shape types
    // can be gst compressed
    bool SHPIsGst(const GINFO * ginfo,int type);

	// ------------------------------------------------------------
	// Handle exceptional cases for gst textures. (ie to small
	// or mipmaps.
	void SHPGstValidate(GINFO * ginfo,int type);

}

#endif
