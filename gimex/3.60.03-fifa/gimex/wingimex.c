// (c) 2000-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*               Win32 File and Memory Interface v1.00              */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                     Code Module - Nov 27, 2000                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Simple unbuffered Win32 IO                                       */
/*                                                                  */
/*------------------------------------------------------------------*/

#define __NOINLINE__ 1
#if !defined(_MSC_VER)
#error this module is only for Windows
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "gimex.h"
#include "stdgimex.h"

/* Memory Functions */

int galloccount=0;
int gopenmethod=3;  /* 0=con:, 1=fopen, 2=backup, 3=temp */

int stdprintf(const char *format, ...)
{
    va_list arglist;
    int r;
    char tempstr[512];

    va_start(arglist, format);
    r = vsprintf(tempstr, format, arglist);
    printf("%s",tempstr);
    va_end(arglist);
    return(r);
}

int __cdecl (*gprintf)(const char *, ...) = stdprintf;


void * GCALL galloc(int size)
{
    ++galloccount;
    return(malloc((size_t)size));
}

int GCALL gfree(void *memptr)
{
    --galloccount;
    free(memptr);
    return(1);
}

/* get motorola memory */

unsigned int ggetm(const void *src, int bytes)
{
    unsigned char *s = (unsigned char *) src;
    unsigned int value;

    value = 0L;
    while (bytes--)
    {
        value = (value<<8) + ((*s++));
    }
    return(value);
}

/* get intel memory */

unsigned int ggeti(const void *src, int bytes)
{
    unsigned char *s = (unsigned char *) src;
    int            i = 0;
    unsigned int  value;

    value = 0L;
    while (bytes--)
    {
        value += ((*s++)) << (i);
        i += 8;
    }
    return(value);
}

/* put motorolla memory */

void gputm(void *dst, unsigned int data, int bytes)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned int       pval;

    data <<= (4-bytes)*8;
    while (bytes)
    {
        pval = data >>  24;
        *d++  = (unsigned char) pval;
        data <<= 8;
        --bytes;
    }
}

/* put intel memory */

void gputi(void *dst, unsigned int data, int bytes)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned int   pval;

    while (bytes)
    {
        pval = data;
        *d++  = (unsigned char) pval;
        data >>= 8;
        --bytes;
    }
}

/* File Functions */

GSTREAM * GCALL gopen(const char *filename)
{
    HANDLE h;
    h = CreateFile(filename,                    // file name
                   GENERIC_READ,                // access mode
                   FILE_SHARE_READ,             // share mode
                   NULL,                        // SecurityAttributes
                   OPEN_EXISTING,               // how to create
                   FILE_ATTRIBUTE_NORMAL,       // file attributes
                   NULL);                       // handle to template file
    if (h==INVALID_HANDLE_VALUE) h = (HANDLE) 0;
    return((GSTREAM *) h);
}

GSTREAM * GCALL gwopen(const char *filename)
{
    HANDLE h;
    h = CreateFile(filename,                    // file name
                   GENERIC_WRITE,               // access mode
                   0,                           // share mode
                   NULL,                        // SecurityAttributes
                   OPEN_ALWAYS,                 // how to create
                   FILE_ATTRIBUTE_NORMAL,       // file attributes
//                   FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING,       // file attributes
                   NULL);                       // handle to template file
    if (h==INVALID_HANDLE_VALUE) h = (HANDLE) 0;
    return((GSTREAM *) h);
}

int GCALL gclose(GSTREAM *g)
{
    int ok=1;
    if (g)
        ok = (int) CloseHandle((HANDLE) g);
    return(ok);
}

int GCALL gread(GSTREAM *g, void *buf, int size)
{
    DWORD bytesread=0;
    int ok=0;
    ok = ReadFile((HANDLE) g, buf, size, &bytesread, NULL);
    if (!ok)
        bytesread = 0;
    return((int)bytesread);
}

int GCALL gwrite(GSTREAM *g, const void *buf, int size)
{
    DWORD byteswritten=0;
    int ok=0;
    ok = WriteFile((HANDLE) g, buf, size, &byteswritten, NULL);
    if (!ok)
        byteswritten = 0;
    return((int)byteswritten);
}

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER -1
#endif

// straight from MSDN: http://msdn.microsoft.com/library/psdk/winbase/filesio_8iwi.htm
static GPOS __gseek(GSTREAM *g, GPOS seekpos, int whence)
{
    LARGE_INTEGER li;

    li.QuadPart = seekpos;

    li.LowPart = SetFilePointer ((HANDLE) g, li.LowPart, &li.HighPart, (DWORD) whence);

    if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
    {
        li.QuadPart = -1;
    }

    return((GPOS)li.QuadPart);
}

int GCALL gseek(GSTREAM *g, GPOS seekpos)
{
    int ok=1;
    GPOS newpos;

    newpos = __gseek(g,seekpos,FILE_BEGIN);
    if (newpos==(GPOS)-1)
        ok = 0;

    return(ok);
}

GPOS GCALL glen(GSTREAM *g)
{
    GPOS oldpos;
    GPOS len;

    oldpos = __gseek(g,(GPOS) 0,FILE_CURRENT);

    len = __gseek(g,(GPOS) 0,FILE_END);

    oldpos = __gseek(g,oldpos,FILE_BEGIN);
    if (len==(GPOS)-1)
        len = (GPOS) 0;

//    li.LowPart = GetFileSize((HANDLE) g, (DWORD *)&li.HighPart);
//    if (GetLastError()==NO_ERROR)
//        len = li.QuadPart;

    return(len);
}

GPOS GCALL gtell(GSTREAM *g)
{
    GPOS currentpos;

    currentpos = __gseek(g,(GPOS) 0,FILE_CURRENT);

    return(currentpos);
}

