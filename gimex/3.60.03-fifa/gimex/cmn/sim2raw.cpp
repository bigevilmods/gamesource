// (c) 1994-2007 Electronic Arts Inc.

#include "eac.h"
#include "eacx.h"
#include "gimex.h"
#include "simread.cpp"
#include "simwrite.cpp"
#include "libgimex.cpp"

void main(int argc, char **argv)
{
    GSTREAM *g;
    GINFO *ginfo;
    GINSTANCE *gx;
    char *f;

    initmemman(100);

    g = gopen(argv[1]);
    if (g)
    {
        if (issim(g))
        {
            opensim(&gx,g,argv[1]);
            ginfo = infosim(gx,0);
            f = galloc(ginfo->width*ginfo->height*ginfo->bpp/8);
            readsim(gx,ginfo,f,ginfo->width*ginfo->bpp/8);
            closesim(gx);
            gclose(g);

            g = gwopen(argv[2]);
            if (g)
            {
                wopensim(&gx,g,argv[2],1);
                writesim(gx,ginfo,f,ginfo->width*ginfo->bpp/8);
                wclosesim(gx);
                gclose(g);
            }
            gfree((char *)ginfo);
        }
    }
}

