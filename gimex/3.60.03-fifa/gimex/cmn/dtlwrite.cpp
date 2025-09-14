// (c) 1995-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"

#ifdef MSDOS
#include <conio.h>
#include <dos.h>
#include "dtlgimex.h"
#include "dtl.h"

/****************************************************************/
/* Local Functions                                              */
/****************************************************************/

/* if MAXWIDTH or MAXHEIGHT change update SetVideoMode call */
#define MAXWIDTH    512
#define MAXHEIGHT   240

union REGS myregisters;

#define _AX myregisters.w.ax
#define _BX myregisters.w.bx
#define _CX myregisters.w.cx
#define _DX myregisters.w.dx

#define geninterrupt(intnum) int386(intnum, &myregisters, &myregisters);

UWORD Seg0000;
UWORD gClutID;
UWORD gImageTPage;
UWORD gImageW;
UWORD gImageH;


#if 0
static void DTLwritepixel(int x, int y, int bgr)
{
    if (x >= 0 && x <= MAXWIDTH && y >= 0 && y <= MAXHEIGHT)
    {
        Poly_F4(x,y, x+1,y, x,y+1, x+1,y+1, bgr);
        DrawSync();
    }
}
#endif

/* optimized draw routines for PSX ADS card */
static int DTLmov8xy(GINFO* ginfo, char* base, int rowbytes, int posx, int posy)
{
    int x;
    int y;
    int w = ginfo->width;
    int h = ginfo->height;

    unsigned int* longpal = (unsigned int*)(ginfo->colourtbl);
    char* buffer = galloc(w*h*2);
    if (!buffer)
        return 0;

    /* NOTE: draw routine only works for 16 bpp dst */

    for (y = 0; y < h; ++y)
    {
        char* src = base + rowbytes*y;
        short* dst = (short*)(buffer + w*2*y);

        for (x = 0; x < w; ++x)
        {
            int argb = longpal[*src++];
            int r = (argb >> 19) & 0x1f;
            int g = (argb >> 11) & 0x1f;
            int b = (argb >>  3) & 0x1f;
            short bgr = b << 10 | g << 5 | r;
            *dst++ = bgr;
        }
    }

    DTLloadimage(posx,posy, w,h, buffer);
    DrawSync();
    gfree(buffer);

    return 1;
}

static int DTLmov32xy(GINFO* ginfo, char* base, int rowbytes, int posx, int posy)
{
    int x;
    int y;
    int w = ginfo->width;
    int h = ginfo->height;

    char* buffer = galloc(w*h*2);
    if (!buffer)
        return 0;

    /* NOTE: draw routine only works for 16 bpp dst */

    for (y = 0; y < h; ++y)
    {
        unsigned int* src = (unsigned int*)(base + rowbytes*y);
        short* dst = (short*)(buffer + w*2*y);

        for (x = 0; x < w; ++x)
        {
            int argb = *src++;
            int r = (argb >> 19) & 0x1f;
            int g = (argb >> 11) & 0x1f;
            int b = (argb >>  3) & 0x1f;
            short bgr = b << 10 | g << 5 | r;
            *dst++ = bgr;
        }
    }

    DTLloadimage(posx,posy, w,h, buffer);
    DrawSync();
    gfree(buffer);

    return 1;
}

/****************************************************************/
/* Export Functions                                             */
/****************************************************************/

int GCALL DTL_wopen(GINSTANCE** gxptr, GSTREAM* g, const char* pathname, int numframes)
{
    GINSTANCE* gx;
    int ok = 0;

    _AX = 2;
    _BX = 0;
    geninterrupt(0x31);
    Seg0000 = _AX;

    if (InitializeDTL(0x1340))
    {
        /* add code to support different video modes here */
        SetVideoMode(1);

        SetDisplayMask(0);
        SetDispXY(0, 0);
        SetDrawClipTop(0, 0);
        SetDrawClipBot(1023, 511);
        SetDrawXYLoc(0, 0);

        Poly_F4(0,0, MAXWIDTH,0, 0,MAXHEIGHT, MAXWIDTH,MAXHEIGHT, 0);
        DrawSync();

        gx = (GINSTANCE*) galloc(sizeof(GINSTANCE));
        if (gx)
        {
            memset(gx, 0, sizeof(GINSTANCE));
            gx->framenum = 0;
            *gxptr = gx;
            ok = 1;
        }
    }
    return(ok);
}

int GCALL DTL_write(GINSTANCE* gx, const GINFO* ginfo, char* source, int rowbytes)
{
#if 0
    int x;
    int y;
#endif

    int screenwidth  = ginfo->subtype == 2 ? 512 : 320;
    int screenheight = 240;

    int startx = gmax(0, (screenwidth  -  ginfo->width) / 2);
    int starty = gmax(0, (screenheight - ginfo->height)/ 2);

    switch (ginfo->subtype)
    {
        case 1:  SetVideoMode(1);  break; /* 320x240 */
        case 2:  SetVideoMode(2);  break; /* 512x240 */
        default: SetVideoMode(1);  break; /* default */
    }

#if 0
    for (y = 0; y < min(ginfo->height, MAXHEIGHT); y++)
    {
        if (ginfo->bpp == 8)
        {
            char* src = source + y*rowbytes;
            for (x = 0; x < min(ginfo->width, MAXWIDTH); x++)
            {
                ARGB argb = ginfo->colourtbl[*src++];
                int bgr = argb.r << 16 | argb.g << 8 | argb.b;
                DTLwritepixel(startx + x,starty + y, bgr);
            }
        }
        else /* bpp == 32 */
        {
            int* src = (int*)(source + y*rowbytes);
            for (x = 0; x < min(ginfo->width, MAXWIDTH); x++)
                DTLwritepixel(startx + x,starty + y, *src++);
        }
    }
#else
    if (ginfo->bpp == 8)
        DTLmov8xy(ginfo, source, rowbytes, startx, starty);
    else /* ginfo->bpp == 32 */
        DTLmov32xy(ginfo, source, rowbytes, startx, starty);
#endif

    return 1;
}

int GCALL DTL_wclose(GINSTANCE* gx)
{
    int ok = 0;

    if (gx)
        ok = gfree((void*)gx);

    return(ok);
}

#else

int GCALL DTL_wopen(GINSTANCE** gxptr, GSTREAM* g, const char* pathname, int numframes)
{
    return(0);
}

int GCALL DTL_write(GINSTANCE* gx, const GINFO* ginfo, char* source, int rowbytes)
{
    return(0);
}

int GCALL DTL_wclose(GINSTANCE* gx)
{
    return(0);
}

#endif
