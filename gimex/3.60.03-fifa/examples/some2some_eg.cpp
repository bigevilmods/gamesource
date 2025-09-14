// (c) 2000-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT

some2some_eg.cpp - GIMEX example, read and display ILBM file

DESCRIPTION

This is an example of how to use the static GIMEX library but
linking in only a subset of the total number of modules available.
This example will show you how to specify which modules and which
functions can be linked it to the application. The example also
shows how to use the GIMEX API. Specifially, it reads in the data
from the source image and writes it to another file as the file
format specified by the extension of the destination filename.
It will only execute successfully for the GIMEX modules that
are linked in to the application.

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

// For static gimex.lib version, include this to limit which modules are linked in
#if 1
// You will need to include the header for each module that you intend to link in
// The headers can be found in the GIMEX SDK

#include "nilgimex.h"
#include "fshgimex.h"
#include "gmxgimex.h"

// Fill in this table with all the function modules needed in your app
// Replace a function name with NULL if you dont want to link in that function
// THESE functions must be linked in: XXX_IS, XXX_about, XXX_open
// Please do not change the order of the function names
struct GFUNCTIONS gfunctions[]=
{
    NIL_about,NIL_is,NIL_open,NIL_info,NIL_read,NIL_close,NIL_wopen,NIL_write,NIL_wclose,

    FSH_about,FSH_is,FSH_open,FSH_info,FSH_read,FSH_close,FSH_wopen,FSH_write,FSH_wclose,
	GMX_about,GMX_is,GMX_open,NULL,GMX_read,GMX_close,GMX_wopen,GMX_write,GMX_wclose,

    NULL   ,NULL,  NULL,  NULL,NULL  ,NULL   ,NULL   ,NULL   ,NULL    ,
};

#endif

int main(int argc, const char **argv)
{
    GSTREAM     *gstream;
    GINFO       *ginfo;
    GINSTANCE   *ginstance;
    GABOUT      *gabout;
    int         genum;
    void        *framebuffer;
    char        rname[256] = "";    // "best match" source file name

    if(argc<3)
    {
        printf("USAGE: some2some <sourcefile> <destfile>\n");
		printf("       This program only supports SSH, FSH, MAD, SIM, STR, GMX, MPG\n");
        return 10;
    }

    const char * sname = argv[1];
    const char *dname = argv[2];

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
