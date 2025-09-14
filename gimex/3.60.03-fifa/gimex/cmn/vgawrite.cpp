// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "vgagimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL VGA_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL VGA_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *lineptr;
    int  x;
    int  y;
    int  ok;
    int  i;
    int  cx;
    int  cy;
    int  width;
    int  height;
    int  vgawidth;
    int  vgaheight;
    char *s;
    char linebuf[1280];

    /* force to one of these sizes */

    width = ginfo->width;
    height = ginfo->height;
    if (width<=320 && height<=200)       { vgawidth = 320;  vgaheight = 200;  }
    else if (width<=640 && height<=400)  { vgawidth = 640;  vgaheight = 400;  }
    else if (width<=640 && height<=480)  { vgawidth = 640;  vgaheight = 480;  }
    else if (width<=800 && height<=600)  { vgawidth = 800;  vgaheight = 600;  }
    else if (width<=1024 && height<=768) { vgawidth = 1024; vgaheight = 768;  }
    else                                 { vgawidth = 1280; vgaheight = 1024; }

    for (i=0; i<256; ++i)
    {
        linebuf[i*3+0] = ginfo->colourtbl[i].r>>2;
        linebuf[i*3+1] = ginfo->colourtbl[i].g>>2;
        linebuf[i*3+2] = ginfo->colourtbl[i].b>>2;
    }

    ok = gwrite(gx->gstream,linebuf,768);

    cx = width/2-320/2;
    cy = height/2-200/2;
    for (y=cy; (y<cy+vgaheight) && ok; ++y)
    {
        lineptr = linebuf;
        if (y<0 || y>=height)
        {
            for (x=0; x<vgawidth; ++x)
            {
                *lineptr++ = 0;
            }
        }
        else
        {
            s = source+y*rowbytes+cx;
            for (x=cx; x<cx+vgawidth; ++x)
            {
                if (x<0 || x>=width)
                {
                    *lineptr++ = 0;
                    ++s;
                }
                else
                {
                    *lineptr++ = *s++;
                }
            }
        }
        ok = gwrite(gx->gstream,linebuf,vgawidth);
    }

    return(ok);
}


int GCALL VGA_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


