// (c) 1994-2007 Electronic Arts Inc.
// ------------------------------------------------------------
//                OLDSHP Gst Gimex Glue Functions
//               ---------------------------------
// This module holds all the shape1 specific functions
// for use in Gimex.
// ------------------------------------------------------------


#ifndef OLDSHPGSTGLUE_H
#define OLDSHPGSTGLUE_H

#include "oldshpgimex.h"

namespace Gst
{
    // ------------------------------------------------------------
    void OLDSHPDecompressGstShape(::OLDSHPHeaderStruct * shpin,char * shpout,int type);

    // ------------------------------------------------------------
    unsigned int OLDSHPSetGstShape(::OLDSHPHeaderStruct ** image,GINFO * ginfo,const unsigned char * source, int type);
 
    // ------------------------------------------------------------
    // returns true if the shape itself is GST compressed.
    bool OLDSHPIsGst(::OLDSHPHeaderStruct * sinfo);

    // ------------------------------------------------------------
    // returns true if the image can be GST compressed
    // the type field is intended to control what shape types
    // can be gst compressed
    bool OLDSHPIsGst(const GINFO * ginfo,int type);

	// ------------------------------------------------------------
	// Handle exceptional cases for gst textures. (ie to small
	// or mipmaps.
	void OLDSHPGstValidate(GINFO * ginfo,int type);

}

#endif
