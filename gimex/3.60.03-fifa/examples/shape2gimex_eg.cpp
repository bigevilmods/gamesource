// (c) 1998-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT 

shape2gimex_eg.cpp - GIMEX example, save out a shape file from scratch 

DESCRIPTION

This is an example of how to save out a shape file from scratch with 
the GIMEX API. This example shows how to set up the <HTML a href="ginfo.htm">GINFO<HTML /a> structure 
needed for writing out the shape file to bmp file format. This example 
uses shape functions from the <HTML a href="http://tools.eac.ea.com/lib/download.htm">REAL library<HTML /a>. The output file is given 
the name shape2gimex.bmp. Please review the <HTML a href="../formats/shapeSpec.htm">shape file specification<HTML /a> 
for more information on shape files.

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that 
are required to build this example. You will also need to download 
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> and the <HTML a href="http://tools.eac.ea.com/pc">REAL libraries<HTML /a>
in order to run this example.

SOURCE
@*********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "real.h"
#include "bmpgimex.h"
#include "gimex.h"
#include "realgraph.h"

#include "bmpwrite.cpp"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif


// Saves the shape into a file called filename.

// Currently only saves bmp files, but could use the extension of the
// filename to determine the best output format.

void SHAPE_savegimex(const SHAPE *shape, const char *filename)
{
    GINFO ginfo;
    SHAPE *tempshape;
    GSTREAM *g;

    MEM_fill(&ginfo, 0, sizeof(GINFO));

    ginfo.signature = GMAKEID('S','H','P','I'); // this ID indicates the shape file type
    ginfo.size = sizeof(GINFO);
    ginfo.version = GIMEX_VERSION;
    ginfo.framenum = 0;
    ginfo.width = shape->width;
    ginfo.height = shape->height;
    ginfo.subtype = 0;
    ginfo.packed = 0;
    ginfo.quality = 0;
    ginfo.framesize = 0;

    /* Always save as 24bpp for now as this is the best supported BMP format */
    ginfo.bpp = 32;
    ginfo.originalbpp = 24;
    ginfo.alphabits = 0;
    ginfo.redbits = 8;
    ginfo.greenbits = 8;
    ginfo.bluebits = 8;

    /* Convert all to 32 bpp */
    tempshape = SHAPE_converttypex(shape, SHAPE_type(32), 0, 0);

    g = gwopen(filename);
    if (g)
    {
        GINSTANCE *gx;
        BMP_wopen(&gx, g, filename, 1);
        BMP_write(gx, &ginfo, SHAPE_data(tempshape), SHAPE_rowbytes(tempshape));
        BMP_wclose(gx);
        gclose(g);
    }
#ifdef DEBUG
    else
    {
        TRACE("SHAPE_savegimex - Error writing file\n");
    }
#endif
}    

int main(int argc, char **argv)
{
	char *filename="movf_eg";
	void *shapefile;
	SHAPE *shp;
	int t;

    if (argc>1)
    {
        filename = argv[1];
    }

    MEM_init();
    FILESYS_init(0,0,0);

	shapefile=SHAPE_loadfile(filename, MB_LOW);
	shp = SHAPE_pointer(shapefile, 0);

    t = CPU_getcycle();

    SHAPE_savegimex(shp, "shape2gimex_eg.bmp");

    t = CPU_getcycle() - t;

    MEM_free(shapefile);

    printf("%d cycles SHAPE_savegimex\n",t);

	REAL_exit();
    return(0);
}

// END ABSTRACT
