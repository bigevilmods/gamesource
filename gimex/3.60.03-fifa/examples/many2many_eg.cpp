// (c) 1994-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT 

many2many_eg.cpp - GIMEX example, simple multiframe file-format converter

DESCRIPTION

This is an example of how to use the GIMEX API. It converts a 
multiframe image from one file format to another. Specifially, 
it converts the contents of the source file into the file format 
specified by the extension of the destination filename. If the 
destination file format supports multiple frames, then the 
contents of the source file will be in the destination file. 
If the destination file format does not support multiple frames, 
then each frame in the source file will be converted to the output 
file format and stored as a single independent file with the 
following naming convention: destname_XXX.ext where XXX is the 
number of the frame and ext is the extension for the file format 
that doesn't support multiframes.

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
    GSTREAM   *gin;
    GSTREAM   *gout=0;
    GINSTANCE *gxin;
    GINSTANCE *gxout=0;
    GABOUT    *gabout;
    GINFO     *ginfo;
    char     namebuf[20] = "";
    char     extbuf[10]  = "";
    char     name[40]    = "";
    void     *f;
    char     *x;
    int      numframes;
    int      intype;
    int      outtype = 0;
    int      i;

    if(argc<3)
    {
        printf("USAGE: many2many <sourcefile> <destfile>\n");
        return 10;
    }

    /* Get the name components */
    x = strstr(argv[2], ".");
    if (x)
    {
        strncpy(namebuf, argv[2], x - argv[2]);
        strcpy(extbuf, x+1);
    }
    else
    {
        strcpy(namebuf, argv[2]);
    }
	
    if (GIMEX_init())
    {
        gin  = GIMEX_gopen(argv[1],0);
        if (gin)
        {
			printf("Opened source file \"%s\" for reading.\n",argv[1]);
            if (GIMEX_is(gin))
            {
                /* Get number of frames, input type, and open the stream */
                numframes = GIMEX_open(&gxin,gin,argv[1],1);
                intype    = GIMEX_get();
				gabout = GIMEX_about();
				printf("Using GIMEX module \"%s\" (%d) for source file \"%s\".\n", gabout->longtypestr, intype, argv[1]);

				printf("Source file has %d frames\n", numframes);

                /* Get output type, determine if it is multi-frame, and open the stream */
                outtype = GIMEX_find(extbuf, 0);
                if (!outtype)
                    outtype = GIMEX_find("fsh", 0);
                gabout = GIMEX_about();
				 
				printf("Using GIMEX module \"%s\" (%d) for destination file \"%s\".\n", gabout->longtypestr, outtype, argv[2]);

                /* If output is multi frame then open the file */
                if (gabout->multiframe)
                {
					printf("Destination file is multiframe; there will be one output file\n");
                    gout = gwopen(argv[2]);
                    GIMEX_wopen(&gxout,gout,argv[2],numframes);
                }
				else
					printf("Destination file is not multiframe; output files will be numbered %s_XXX.%s\n",namebuf, extbuf);

                /* Loop through all frames */
                for (i = 0; i < numframes; ++i)
                {
                    /* Read input frame */
                    GIMEX_set(intype);
                    ginfo = GIMEX_info(gxin,i);
                    f = galloc(ginfo->width*ginfo->height*ginfo->bpp/8);
                    GIMEX_read(gxin,ginfo,f,ginfo->width*ginfo->bpp/8);

                    /* Write output frame */
                    GIMEX_set(outtype);
                    if (gabout->multiframe)
                    {
                        GIMEX_write(gxout,ginfo,f,ginfo->width*ginfo->bpp/8);
						if (!i)
							printf("Opened destination file \"%s\" for writing.\n",argv[2]);
                    }
                    else
                    {
                        sprintf(name, "%s_%i.%s", namebuf, i, extbuf);
                        gout = gwopen(name);
                        if (gout)
                        {
                            GIMEX_wopen(&gxout,gout,name,1);
                            GIMEX_write(gxout,ginfo,f,ginfo->width*ginfo->bpp/8);
                            GIMEX_wclose(gxout);
                            gclose(gout);
                        }
                    }
                    gfree((char *)f);
                    gfree((char *)ginfo);
                }
				if (!gabout->multiframe)
					printf("Opened %d destination files for writing.\n",i);

                GIMEX_set(intype);
                GIMEX_close(gxin);
                gclose(gin);

                if (gabout->multiframe)
                {
                    GIMEX_set(outtype);
                    GIMEX_wclose(gxout);
                    gclose(gout);
                }
                gfree(gabout);
            }
        }    
        GIMEX_restore();
    }   
}

// END ABSTRACT
