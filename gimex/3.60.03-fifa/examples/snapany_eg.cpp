// (c) 2000-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT 

saveany_eg.cpp - GIMEX example, creates a simple movie file from scratch

DESCRIPTION

This is an example of how to use the GIMEX API to make a movie file from scratch.
It fills in the GINFO structure with the necessary information for the file format.

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that 
are required to build this example. You will also need to download 
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> in order to run this example.

SOURCE
@*********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
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
    int  genum;
    int  i;
    int  x;
    int  y;
    int  frame;
    int  frames;
    void *framebuffer;
    char *dname;             // destination file name

    if(argc<2)
    {
        printf("USAGE: snapany <destfile>\n");
        return 10;
    }

    dname = argv[1];

    if (GIMEX_init())       // load and initialize library
    {
        if (strstr(dname, "."))
        {
            genum = GIMEX_find(strstr(dname, ".")+1, 0);
            if (!genum)
                genum = GIMEX_find("fsh", 0);
        }
        else
            genum = GIMEX_find("fsh", 0);

        // get GIMEX module info:
        frames = 1;
        gabout = GIMEX_about();
        if (gabout)
        {
            if (gabout->multiframe)
                frames = 30;
            printf("Using GIMEX module \"%s\" (%d) for destination file \"%s\".\n",
                gabout->longtypestr, genum, dname);
            gfree(gabout);
        }

        gstream = gwopen(dname);
        if (gstream)
        {
            printf("Opened destination file \"%s\" for writing.\n",dname);
            GIMEX_wopen(&ginstance,gstream,dname,frames);

            for (frame=0; frame<frames; ++frame)
            {
                ginfo = (GINFO *) galloc(sizeof(GINFO));
                memset(ginfo,0,sizeof(GINFO));

                ginfo->signature = GMAKEID('S','A','V','E');    // incase you want to recognise you own ginfos later
                ginfo->size = sizeof(GINFO);    /* size of GINFO structure - important to some modules */
                ginfo->version = 300;           /* version number of GINFO structure (300) */
                ginfo->framenum = 0;            /* not used on export */
                ginfo->width = 320;             /* width of bitmap in pixels */
                ginfo->height = 240;            /* height of bitmap in pixels */
                ginfo->bpp = 32;                /* bits per pixel (8 or 32) */
                ginfo->originalbpp = 16;        /* bits per pixel in original image (1 to 32) */
                ginfo->startcolour = 0;         /* first colour in palette */
                ginfo->numcolours = 256;        /* number of colours in original indexed palette - wont be used in this demo */

                for (i=0; i<ginfo->numcolours; ++i)
                {
                    ginfo->colourtbl[i].a = (unsigned char) 255;    // lets make a grey ramp
                    ginfo->colourtbl[i].r = (unsigned char) i;
                    ginfo->colourtbl[i].g = (unsigned char) i;
                    ginfo->colourtbl[i].b = (unsigned char) i;
                }

                ginfo->subtype = 0;           /* internal format sub-type 0-default */
                ginfo->packed = 3;            /* type of packing on original image. 0 none, 1 run, n other. 3 = s3tc on FSH */
                ginfo->quality = 80;          /* quality of lossy packing 0..100 */
                ginfo->framesize = 0;         /* size of frame in bytes. 0 means use quality or a default */
                ginfo->alphabits = 1;         /* number of bits in alpha channel */
                ginfo->redbits = 5;           /* number of bits in red channel */
                ginfo->greenbits = 5;         /* number of bits in green channel */
                ginfo->bluebits = 5;          /* number of bits in blue channel */
                ginfo->centerx = 320;         /* center point relative to upper left corner */
                ginfo->centery = 240;
                ginfo->defaultx = 64;         /* default coordinate point */
                ginfo->defaulty = 64;
                ginfo->numhotspots = 10;       /* number of hot spots defined */
                strcpy(ginfo->framename,"fred");            // used for shape files
                strcpy(ginfo->comment,"Made in Canada");    // whatever you want
                for (i=0; i<ginfo->numhotspots; ++i)
                {
                    ginfo->hotspottbl[i][0] = i*10;    // X
                    ginfo->hotspottbl[i][0] = i;       // Y
                }
                ginfo->dpi = 72.0f;             /* dots per inch ie 72.0 */
                ginfo->fps = 15.0f;             /* frame per second (one over duration).  15.0 is typical */

                framebuffer = galloc(ginfo->width*ginfo->height*ginfo->bpp/8);

                // make an image
                for (y=0; y<ginfo->height; ++y)
                {
                    for (x=0; x<ginfo->width; ++x)
                    {
                        float x1,y1,a,h;
                        ARGB *p = (ARGB *) ((char *)framebuffer+y*ginfo->width*ginfo->bpp/8 + x*4);

                        a = (float)atan2((double)(y-240),(double)(x-320));
                        h = (float)sqrt((double)((x-320)*(x-320) + (y-240)*(y-240)));
                        x1 = (float)(320+cos(a+frame*2.0/30.0)*h);
                        y1 = (float)(240+sin(a+frame*2.0/30.0)*h);

                        p->a = 255;
                        p->r = (unsigned char)(128.0+128.0/3.0*(sin((double)(frame*20+sqrt(600+x1*x1+y1*y1))*1.5*3.0/640.0) + sin(frame*1.3/100.0) + cos((double)(50+frame*1+x1*3+y1)*1.8*3.0/640.0)));
                        p->g = (unsigned char)(128.0+128.0/3.0*(sin((double)(frame*10+sqrt(400+x1*x1+y1*y1))*2.5*4.0/640.0) + sin(frame*1.4/100.0) + cos((double)(60+frame*1+x1*2+y1)*1.5*6.0/640.0)));
                        p->b = (unsigned char)(128.0+128.0/3.0*(sin((double)((frame+30)*17+1.5*sqrt(100+2*x1*x1+y1*y1)+1)*2.3*4.0/640.0) + sin(2+frame*2.0/100.0) + cos((double)(70+frame*5+x1*4+y1)*1.4*3.0/640.0)));
                    }
                }

                printf("Writing frame %d of %d.\n",frame+1,frames);
                GIMEX_write(ginstance,ginfo,framebuffer,ginfo->width*ginfo->bpp/8);

                gfree(framebuffer);
                gfree(ginfo);
            }
            GIMEX_wclose(ginstance);
            gclose(gstream);
        }
        else
            printf("Failed to open destination file \"%s\" for writing!\n",dname);

        GIMEX_restore();
    }
    else
        printf("GIMEX_init() failed!\n");
}

// END ABSTRACT
