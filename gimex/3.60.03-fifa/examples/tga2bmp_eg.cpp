// (c) 1994-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT 

tga2bmp_eg.cpp - GIMEX example, read and display ILBM file

DESCRIPTION

This is an example of how to use GIMEX modules. It is very 
similar to the any2any_eg.cpp example which uses the GIMEX API for 
accessing the GIMEX modules. This example takes a single Targa(TGA) 
image and writes out a Bitmap(BMP) image. 

In order to build this example, the following files are required:
  <HTML a href="TGA.htm">tgaread.cpp<HTML /a>  - tga GIMEX module for reading 
  <HTML a href="BMP.htm">bmpwrite.cpp<HTML /a> - bmp GIMEX module for writing 
  stdgimex.cpp - file and memory access interface

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that 
are required to build this example. You will also need to download 
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> in order to run this example.

SOURCE
@*********************************************************************************/

#include <stdio.h>
#include <string.h>
#include "gimex.h"
#include "tgagimex.h"
#include "bmpgimex.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

void main(int argc, char **argv)
{
    GSTREAM     *gstream;
    GINFO       *ginfo;
    GINSTANCE   *ginstance;
    char        *framebuffer;
    char        *sname;             // source file name
    char        *dname;             // destination file name
    char		*extbuf;
	int			len;

    if(argc<3)
    {
        printf("USAGE: tga2bmp <sourcefile.tga> <destfile.bmp>\n");
        return;
    }

    sname = argv[1];
    dname = argv[2];

    gstream = gopen(sname); // open source file 
    if (gstream)
    {
        printf("Opened source file \"%s\" for reading.\n",sname);
 
        if (TGA_is(gstream))
        {
            TGA_open(&ginstance,gstream,sname,0);   // open tga GIMEX module for reading
            ginfo = TGA_info(ginstance,0);                // get info about the image file; needed for reading
            framebuffer = galloc(ginfo->width*ginfo->height*ginfo->bpp/8);
            TGA_read(ginstance,ginfo,framebuffer,ginfo->width*ginfo->bpp/8); // read the tga image file
            TGA_close(ginstance);                                                                                        // close the tga GIMEX module
            gclose(gstream);												// close the source image file

			len = strlen(dname);
			if(strcmp(dname+len-3,"bmp") != 0) // a crude method for checking correct extension
			{
				printf("Changing extension to bmp\n");
				extbuf = dname+len-3;
				while ((extbuf-1)[0] != '.')
					extbuf--;
				strcpy(extbuf,"bmp");
			}

            gstream = gwopen(dname); // opening the file for writing
            if (gstream)
            {
                printf("Opened destination file \"%s\" for writing.\n",dname);
                BMP_wopen(&ginstance,gstream,dname,1);                                                           // open bmp GIMEX for writing
                BMP_write(ginstance,ginfo,framebuffer,ginfo->width*ginfo->bpp/8);        // write to file
                BMP_wclose(ginstance);                                                                                           // close bmp GIMEX module 
                gclose(gstream);													// close the destination file
            }
            else
                printf("Failed to open destination file \"%s\" for writing!\n",dname);

            gfree(ginfo);
            gfree(framebuffer);
        }
        else
            printf("%s is not a TGA file; no file conversion performed\n",sname);
    }
    else
        printf("Failed to open source file \"%s\" for reading!\n",sname);
}

// END ABSTRACT
