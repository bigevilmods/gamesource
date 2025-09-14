// (c) 1994-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT 

any2any_eg.cpp - GIMEX example, simple file-format converter

DESCRIPTION

This is an example of how to use the GIMEX API. It converts a single 
source image of one file format and writes it out as another file 
format. Specifially, it reads in the data from the source image and 
writes it to another file as the file format specified by the 
extension of the destination filename. 

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that 
are required to build this example. You will also need to download 
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> in order to run this example.

SOURCE
@*********************************************************************************/

#include <stdio.h>
#include <string.h>
#include "gimex.h"
#include "gopen.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

int main(int argc, char **argv)
{
    GSTREAM     *gstream;
    GINFO       *ginfo;
    GINSTANCE   *ginstance;
    GABOUT      *gabout;
    int         genum;
    void        *framebuffer;
    char        *sname;             // source file name
    char        *dname;             // destination file name
    char        rname[256] = "";    // "best match" source file name

    if(argc<3)
    {
        printf("USAGE: any2any <sourcefile> <destfile>\n");
        return 10;
    }

    sname = argv[1];
    dname = argv[2];

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
                if(gabout)
                {
                    genum = GIMEX_get();
                    printf("Using GIMEX module \"%s\" (%d) for source file \"%s\".\n",
                        gabout->longtypestr, genum, sname);
                }
                gfree(gabout);

                GIMEX_open(&ginstance,gstream,sname,0);
                ginfo = GIMEX_info(ginstance,0);
                framebuffer = galloc(ginfo->width*ginfo->height*ginfo->bpp/8);
                GIMEX_read(ginstance,ginfo,framebuffer,ginfo->width*ginfo->bpp/8);
                GIMEX_close(ginstance);
                gclose(gstream);

                // zero out format specific ginfo fields
                ginfo->subtype = 0;
                ginfo->framesize = 0;
                ginfo->quality = 100;
                ginfo->packed = 0;

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
                if(gabout)
                {
                    printf("Using GIMEX module \"%s\" (%d) for destination file \"%s\".\n",
                        gabout->longtypestr, genum, dname);
                }
                gfree(gabout);

                gstream = gwopen(dname);
                if (gstream)
                {
                    printf("Opened destination file \"%s\" for writing.\n",dname);
                    GIMEX_wopen(&ginstance,gstream,dname,1);
                    GIMEX_write(ginstance,ginfo,framebuffer,ginfo->width*ginfo->bpp/8);
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
