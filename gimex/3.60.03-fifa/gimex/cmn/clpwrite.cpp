// (c) 1994-2007 Electronic Arts Inc.

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <string.h>
#include "gimex.h"
#include "clpgimex.h"

/*
 * Include bmpwrite.c replacing the file output with a write to a file buffer
 * bmpwrite only uses gwrite, no gtell or gseek (defined them just in case).
 */
#define BMPCLIPBOARD 1

/* used by bmpwrite.c */
#define gwrite      CLPmemwrite
#define gtell(g)    0
#define gseek(g,val) 0

/* defined by bmpwrite.c */
#define BMP_wopen    CLPwopenbmp
#define BMP_write    CLPwritebmp
#define BMP_wclose   CLPwclosebmp

static int CLPmemwrite(GSTREAM *g, void *buf, int size);

#include "bmpgimex.h"
#include "bmpwrite.cpp"

#undef BMPCLIPBOARD
#undef gwrite
#undef gtell
#undef gseek
#undef BMP_wopen
#undef BMP_write
#undef BMP_wwclose

/*******************
 * Local functions *
 *******************/

static char *CLPfilebuf;
static char *CLPfileptr;

static int CLPinitmembuf(int maxsize)
{
    int ok=0;

    CLPfilebuf = (char *) galloc(maxsize);
    if (CLPfilebuf)
    {
        ok = 1;
        CLPfileptr=CLPfilebuf;
    }

    return(ok);
}

static int CLPdestroymembuf(void)
{
    int ok=0;

    if (CLPfilebuf)
    {
        ok = gfree(CLPfilebuf);
        CLPfilebuf=0;
        CLPfileptr=0;
    }
    return(ok);
}

static int CLPmemwrite(GSTREAM *g, void *buf, int size)
{
    int ok=0;

    if (CLPfileptr)
    {
        if (memcpy(CLPfileptr, buf, size))
        {
            CLPfileptr+=size;
            ok = 1;
        }
    }
    return(ok);
}


static int CLPwopen(void)
{
    int ok = 0;

    if (OpenClipboard(NULL))
    {
        if (EmptyClipboard())
        {
            ok = 1;
        }
    }
    return(ok);
}

static int CLPwrite(unsigned int Format, void *pSrc, int DataSize)
{
    HGLOBAL hData;
    void *pDst;
    int ok = 0;

    hData = GlobalAlloc(GHND | GMEM_SHARE, DataSize);  /* note freed by clipboard itself */
    if (hData)
    {
        pDst = GlobalLock(hData);
        if (pDst)
        {
            memcpy(pDst, pSrc, DataSize);
/*          CopyMemory(pDst, pSrc, DataSize); */
            GlobalUnlock(hData);
            if (SetClipboardData(Format, hData))
                ok = 1;
        }
    }

    return(ok);
}

static int CLPwclose(void)
{
    int ok=0;

    ok = CloseClipboard();

    return(ok);
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL CLP_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    int ok=0;
    GINSTANCE *gx;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->frames = numframes;
        *gxptr = gx;

        ok = CLPwopen();
    }

    return(ok);
}

int GCALL CLP_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok=0;
    int memsize;
    int imagesize;
    int headersize;
    GINSTANCE *gxbmp;
    char pathname[]="clip.bmp";
    GINFO ginfo0;

    /*
     * Clipboard seems to require all of the colours to exist
     */
    ginfo0 = *ginfo;
    if (ginfo0.bpp == 8)
    {
        ginfo0.numcolours = 256;
        if (ginfo0.originalbpp <= 4)
            ginfo0.numcolours = 16;
    }
    else
        ginfo0.numcolours = 0;

    /*
     * Allocate enough memory to hold the bitmap file
     */
    headersize = 122;
    imagesize = ((ginfo0.width*ginfo0.bpp+31)&~31)/8 * ginfo0.height;
    memsize = headersize + ginfo0.numcolours * 4  + imagesize;
    ginfo0.packed = 0;
    if (CLPinitmembuf(memsize))
    {
        if (CLPwopenbmp(&gxbmp, 0/*gx->gstream*/, pathname, 1))
        {
            if (CLPwritebmp(gxbmp, &ginfo0, source, rowbytes))
            {
                if (CLPwclosebmp(gxbmp))
                {
                    if (CLPwrite(CF_DIB, CLPfilebuf+14, CLPfileptr-CLPfilebuf-14))
                    {
                        ok = CLPdestroymembuf();
                    }
                }
            }
        }
    }

    return(ok);
}

int GCALL CLP_wclose(GINSTANCE *gx)
{
    int ok = 0;

    if (gx)
    {
        if (gfree((void*)gx))
            ok = CLPwclose();
    }

    return(ok);
}


