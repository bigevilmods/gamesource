// (c) 2000-2007 Electronic Arts Inc.

#include <stdio.h>
#include <stdlib.h>
#include "real.h"

#include "bmpgimex.h"
#include "savegimex_eg.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

/*********************************************************************************
ABSTRACT 

saveany_eg.cpp - GIMEX example, save out an image from scratch

DESCRIPTION

This is an example of how to save out an image from scratch with 
the GIMEX API. This example shows how to set up the GINFO structure 
needed for writing out the file.

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that 
are required to build this example. You will also need to download 
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> in order to run this example.

SOURCE
@*********************************************************************************/

int main(int argc, char **argv)
{
	char *filename="john.fsh";
	void *shapefile;
	SHAPE *shp;
	int t;

    if (argc>1)
    {
        filename = argv[1];
    }

    MEM_init();
    FILESYS_init(0,0,0);

    printf("loading %s %s\n",filename, argc<2?"(you can specify this on the command line)":"");
	shapefile=SHAPE_loadfile(filename, MB_LOW);
	shp = SHAPE_pointer(shapefile, 0);

    printf("saving savegimex_eg.bmp\n");
    t = CPU_getcycle();

    SHAPE_savegimex(shp, "savegimex_eg.bmp");

    t = CPU_getcycle() - t;

    MEM_free(shapefile);

    printf("done.\n%d cycles SHAPE_savegimex\n",t);

	REAL_exit();
    return(0);
}

// END ABSTRACT
