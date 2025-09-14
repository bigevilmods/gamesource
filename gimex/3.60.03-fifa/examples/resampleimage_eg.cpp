// (c) 1994-2007 Electronic Arts Inc.

/********************************************************************************/
/* Demo program for resampling an image                                         */
/* Please see the <HTML a href="http://tools.eac.ea.com/gimex/sdk/gimex/example/resampleimage/readme.htm">description<HTML /a> of this file for an explanation of what it does   */ 
/********************************************************************************/

/*********************************************************************************
ABSTRACT 

resampleimage_eg.cpp - GIMEX example, resampling an image

DESCRIPTION

This is an example of resampling an image. This example simply reduces an 
image to the given bpp by using the resampleimage() function in resample.cpp. 
This function hides everything hides all the work of resampling and uses 
the CRAPI and FILTER libraries. 

This is a quick and easy way of resampling an image (just one function 
call). If you wish to see a more complicated version which allows more 
flexibility, please see the colourreduce_eg.cpp example which uses the
CRAPI library exclusively.

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that 
are required to build this example. You will also need to download 
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> in order to run this example.

A makefile (resampleimage_eg.mak) has been provided in your gimex\examples
directory for building this example.

SOURCE
@*********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gimex.h"
#include "gopen.h"
#include "resample.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

int main(int argc, char **argv)
{
    GSTREAM     *gstream;
    GINFO       *ginfo, *destginfo;
    GINSTANCE   *ginstance;
    GABOUT      *gabout;
    int         genum;
    char        *framebuffer, *destframebuffer;
    char        *sname;             // source file name
    char        *dname;             // destination file name
    char        rname[256] = "";    // "best match" source file name

	// these are needed for resampling
	int bpp;
	GBITMAP source, *destination;

    if(argc<3)
    {
        printf("USAGE: resampleimage <sourcefile> <destfile> <bpp>\n");
        return;
    }

    sname = argv[1];
    dname = argv[2];
	bpp = atoi(argv[3]);

    if (GIMEX_init())       // load and initialize library
    {
        // open source file and set best GIMEX module for file type:
        gstream = GIMEX_gopen(sname,rname);
        if (gstream)
        {
            printf("Opened source file \"%s\" for reading.\n",sname);
            if(strcmp(sname,rname))
                printf("\"%s\" opened as \"%s\".\n",sname,rname);
            if (GIMEX_is(gstream))
            {
                // get GIMEX module info:
                gabout = GIMEX_about();
                if (gabout)
                {
                    genum = GIMEX_get();
                    printf("Using GIMEX module \"%s\" (%d) for source file \"%s\".\n",
                        gabout->longtypestr, genum, sname);

                    gfree(gabout);
                }

                GIMEX_open(&ginstance,gstream,sname,0);
                ginfo = GIMEX_info(ginstance,0);
                framebuffer = galloc(ginfo->width*ginfo->height*ginfo->bpp/8);
                GIMEX_read(ginstance,ginfo,framebuffer,ginfo->width*ginfo->bpp/8);
                GIMEX_close(ginstance);
                gclose(gstream);

				/********** BEGIN RESAMPLE IMAGE **********/

				// fill in the source GBITMAP
				source.ginfo = ginfo;
				source.rowbytes = ginfo->width*ginfo->bpp/8;
				source.image = framebuffer;

				destination = resampleimage(&source, ginfo->width, ginfo->height, bpp);

				// get the values out of the destination GBITMAP
				destginfo = destination->ginfo;
				destframebuffer = destination->image;

				/********** END RESAMPLE IMAGE **********/

                if (strstr(dname, "."))
                {
                    genum = GIMEX_find(strstr(dname, ".")+1, 0);
                    if (!genum)
                        genum = GIMEX_find("fsh", 0);
                }
                else
                    genum = GIMEX_find("fsh", 0);

                // get GIMEX module info:
                gabout = GIMEX_about();
                if (gabout)
                {
                    printf("Using GIMEX module \"%s\" (%d) for destination file \"%s\".\n",
                        gabout->longtypestr, genum, dname);

                    gfree(gabout);
                }

                gstream = gwopen(dname);
                if (gstream)
                {
                    printf("Opened destination file \"%s\" for writing.\n",dname);
                    GIMEX_wopen(&ginstance,gstream,dname,1);
                    GIMEX_write(ginstance,destginfo,destframebuffer,destginfo->width*destginfo->bpp/8);
                    GIMEX_wclose(ginstance);
                    gclose(gstream);
                }
                else
                    printf("Failed to open destination file \"%s\" for writing!\n",dname);

                gfree((char *)framebuffer);
                gfree((char *)ginfo);
            }
            else
                printf("GIMEX_is() failed!\n");
        }
        else
            printf("Failed to open source file \"%s\" for reading!\n",sname);

        GIMEX_restore();
    }   
    else
        printf("GIMEX_init() failed!\n");
}

// END ABSTRACT
