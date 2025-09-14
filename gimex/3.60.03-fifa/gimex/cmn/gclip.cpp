// (c) 1998-2007 Electronic Arts Inc.

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "gclip.h"

GCLIP *GCLIP_wopen(void)
{
    int ok = 0;

    if (OpenClipboard(NULL))
    {
        if (EmptyClipboard())
        {
            ok = 1;
        }
    }
    return((GCLIP *)ok);
}

int GCLIP_write(GCLIP *gclip, unsigned int Format, void *pSrc, int DataSize)
{
    HGLOBAL hData;
    void *pDst;
    int ok = 0;

    if (gclip)
    {
        hData = GlobalAlloc(GHND | GMEM_SHARE, DataSize);  /* note freed by clipboard itself */
        if (hData)
        {
            pDst = GlobalLock(hData);
            if (pDst)
            {
                memcpy(pDst, pSrc, DataSize);
/*              CopyMemory(pDst, pSrc, DataSize); */
                GlobalUnlock(hData);
                if (SetClipboardData(Format, hData))
                    ok = 1;
            }
        }
    }
    return(ok);
}

int GCLIP_wclose(GCLIP *gclip)
{
    int ok=0;

    if (gclip)
    {
        ok = CloseClipboard();
    }
    return(ok);
}


