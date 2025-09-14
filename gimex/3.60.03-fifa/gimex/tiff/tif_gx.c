// (c) 1988-2007 Electronic Arts Inc.
/* $Header: /Tools/Gimex/Tiff/tif_gx.c 3     1/05/98 2:37p Frank $ */

/*
 * Copyright (c) 1991-1996 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

/*
 * TIFF Library MSDOS-specific Routines.
 */

#include "gimex.h"
#include "tiffiop.h"


static tsize_t
_tiffReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    return(gread((GSTREAM *)fd, buf, size));
}

static tsize_t
_tiffWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
    return(gwrite((GSTREAM *)fd, buf, size));
}

static toff_t
_tiffSeekProc(thandle_t fd, toff_t off, int whence)
{
    int ok;
    switch (whence)
    {
        default:
        case SEEK_SET: break;
        case SEEK_CUR: off += (unsigned long)gtell((GSTREAM *)fd);break;
        case SEEK_END: off += (unsigned long)glen((GSTREAM *)fd);break;
    }

    ok  = gseek((GSTREAM *)fd,off);
    off = (unsigned long)gtell((GSTREAM *)fd);
    if (!ok)
        off = 0;//-1; possible problem here... off is ulong.
    return(off);
}

static int
_tiffCloseProc(thandle_t fd)
{
    return(0);
}

#include <sys/stat.h>

static toff_t
_tiffSizeProc(thandle_t fd)
{
    return((unsigned long)glen((GSTREAM *)fd));
}

static int
_tiffMapProc(thandle_t fd, tdata_t* pbase, toff_t* psize)
{
    return (0);
}

static void
_tiffUnmapProc(thandle_t fd, tdata_t base, toff_t size)
{
}

/*
 * Open a TIFF file descriptor for read/writing.
 */
TIFF*
TIFFFdOpen(int fd, const char* name, const char* mode)
{
    TIFF* tif;

    tif = TIFFClientOpen(name, mode,
        (thandle_t) fd,
        _tiffReadProc, _tiffWriteProc, _tiffSeekProc, _tiffCloseProc,
        _tiffSizeProc, _tiffMapProc, _tiffUnmapProc);
    if (tif)
         tif->tif_fd = fd;
    return (tif);
}

/*
 * Open a TIFF file for read/writing.
 */
TIFF*
TIFFOpen(const char* name, GSTREAM *g, const char* mode)
{
    static const char module[] = "TIFFOpen";
    int m;

    m = _TIFFgetMode(mode, module);
    if (m == -1)
        return ((TIFF*)0);
    if (!g)
    {
        TIFFError(module, "%s: Cannot open", name);
        return ((TIFF*)0);
    }
    return(TIFFFdOpen((int)g, name, mode));
}


// NOTE maintains length to allow realloc

tdata_t
_TIFFmalloc(tsize_t s)
{
    char *p;

    p = galloc((long)s+4);
    gputm(p,(long)s,4);
    return((tdata_t)(p+4));
}

void
_TIFFfree(tdata_t p)
{
    gfree((char *)p-4);
}

tdata_t
_TIFFrealloc(tdata_t p, tsize_t s)
{
    long oldsize;
    char *d;

    oldsize = ggetm((char *)p-4,4);
    d = galloc((long)s+4);
    gputm(d,(long)s,4);
    memcpy(d+4,(char *)p,gmin(oldsize,(long)s));
    gfree((char *)p-4);
    return((tdata_t)(d+4));
}



void
_TIFFmemset(tdata_t p, int v, tsize_t c)
{
    memset(p, v, (size_t) c);
}

void
_TIFFmemcpy(tdata_t d, const tdata_t s, tsize_t c)
{
    memcpy(d, s, (size_t) c);
}

int
_TIFFmemcmp(const tdata_t p1, const tdata_t p2, tsize_t c)
{
    return (memcmp(p1, p2, (size_t) c));
}

static void
msdosWarningHandler(const char* module, const char* fmt, va_list ap)
{
    if (module != NULL)
            ferrprintf(stderr, "%s: ", module);
    ferrprintf(stderr, "Warning, ");
    vferrprintf(stderr, fmt, ap);
    ferrprintf(stderr, ".\n");
}

TIFFErrorHandler _TIFFwarningHandler = msdosWarningHandler;

static void
msdosErrorHandler(const char* module, const char* fmt, va_list ap)
{
    if (module != NULL)
            ferrprintf(stderr, "%s: ", module);
    vferrprintf(stderr, fmt, ap);
    ferrprintf(stderr, ".\n");
}
TIFFErrorHandler _TIFFerrorHandler = msdosErrorHandler;

