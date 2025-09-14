// (c) 1994-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT 

showlbm_eg.cpp - GIMEX example, read and display ILBM file

DESCRIPTION

This is an example of how to use a GIMEX module. This example opens 
an Amiga ILBM image file uses the LBM GIMEX module for reading the 
file and then displays it. This example also uses the REAL libraries 
for displaying the image within a window. Please refer to the REAL 
documentation for more information. 

Requirements for building this example:
- <HTML a href="ILBM.htm">lbmread.cpp<HTML /a> - lbm GIMEX module for reading 
- libgimex.cpp - GIMEX library interface 
- <HTML a href="http://tools.eac.ea.com/lib/download.htm">REAL libraries<HTML /a> - for creating and displaying the windows 
A makefile (showlbm.mak) has been provided for building this example.

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that 
are required to build this example. You will also need to download 
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> and the <HTML a href="http://tools.eac.ea.com/pc">REAL libraries<HTML /a>
in order to run this example.

SOURCE
@*********************************************************************************/

#include <stdio.h>
#include "real.h"
#include "gimex.h"
#include "realgraph.h"
#include "lbmgimex.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

void main (char argc,char **argv)
{
	SHAPE *w;
	GSTREAM *g;
    GINFO *ginfo;
    GINSTANCE *gx;

	if(argc<2)
    {
        printf("USAGE: showlbm <imagefile>\n");
        return;
    }

	SCREENmode &= !GINIT_FULLSCREEN; // setting the screenmode to a window and not fullscreen
	REAL_init();

	g = gopen(argv[1]);				// open the image file for reading
	printf("Opened image file \"%s\" for reading.\n",argv[1]);
        LBM_open(&gx,g,argv[1],0);                 // prepare the lbm module
        ginfo = LBM_info(gx,0);                  // get the image information

	tgapalette(0,256,(unsigned long *)ginfo->colourtbl);				// sets palette for 8 bit display
	w = SHAPE_create(ginfo->width, ginfo->height, ginfo->bpp, MB_LOW);	// create the window for displaying the image
	
	printf("Reading image data from \"%s\".\n",argv[1]);
        LBM_read(gx, ginfo, SHAPE_data(w), ginfo->width*ginfo->bpp/8);                // read in the image data
        LBM_close(gx);                                                                                                   // close the lbm module                                          
	gclose(g);														// close the image file
	printf("Closed image file \"%s\" for reading.\n",argv[1]);

	WINDOW_display();
	WINDOW_clear(C_BLACK);		// clear the window to black

	movfxya(w,0,0);				// move image to the top left corner of the window

	pageflip();					// swaps buffers to display image
	WINDOW_null();				// sets the current window to null

	gfree(ginfo);				// free the ginfo structure

    KEY_waitevent(0);        
	REAL_exit();
}

// END ABSTRACT
