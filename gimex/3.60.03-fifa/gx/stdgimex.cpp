// (c) 1994-2007 Electronic Arts Inc.

#define USEFILE64 1

// set to 1 for 32+64 bit, 2 for 64 bit only

/*------------------------------------------------------------------*/
/*                                                                  */
/*           Gimex Standard File and Memory Interface v1.53         */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                     Code Module - Sep 4, 2018                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.xx xxxxxx FB No version history available                   */
/*    1.06 941225 FB Debugged final version                         */
/*    1.07 941225 GS Fixed bug in gopen                             */
/*    1.08 950524 FB r+b and w+b for read/write access              */
/*    1.09 950529 GS clear memory on galloc/gfree (#ifdef _DEBUG)   */
/*    1.10 950613 FB bumped to 100 megs for SGI huge pix            */
/*    1.11 951102 FB con: for stdin and stdout                      */
/*    1.12 961022 FB seek and write                                 */
/*    1.13 961114 FB stdgimex.h                                     */
/*    1.14 970109 FB gread eof set endptr                           */
/*    1.15 970227 GS check for null, pass lint check                */
/*    1.16 970828 KO added GCALL                                    */
/*    1.17 971209 KO gopen opens with "rb" not "r+b"                */
/*    1.18 980124 SK implemented caching in all file io functions   */
/*    1.19 980128 SK added isDirectory, isReadOnly for error checks */
/*    1.20 981116 FB gwopen use temporary file                      */
/*    1.21 000623 FB avoid fwrite of 0 bytes                        */
/*    1.22 001121 FB 64 bit file io                                 */
/*    1.23 001123 FB lseeki64 on windows                            */
/*    1.24 001204 FB perror on read/write errors                    */
/*    1.25 001204 FB if lseek64 fails, try lseek. stdgimex.h protos */
/*    1.26 001221 FB galloc revert to old version with sentinels    */
/*    1.27 010222 FB VC6 header bug fix - struct _RPC_ASYNC_STATE   */
/*    1.28 011011 FB convert / to \ on pc                           */
/*    1.31 020220 FB merged unixgimex into stdgimex                 */
/*    1.34 020226 FB ifdef gtell for gprintf                        */
/*    1.35 020227 FB unistd for lseek64 on unix                     */
/*    1.36 020415 ID added __USE_LARGEFILE64 macro for unix         */
/*    1.37 020605 FB mkdir on gwopen                                */
/*    1.38 020728 FB off_t lseek osx, USEFILE64 for pc win98        */
/*    1.39 020730 FB USEFILE64 2 for 64 bit exclusively             */
/*    1.40 021001 FB gprintf to logfile                             */
/*    1.41 030213 ID use 64 bit I/O only under IntelC               */
/*    1.42 030218 FB use O_BINARY for metrowerks PC                 */
/*    1.43 040428 FB set sizeof before calling GetVersionEx         */
/*    1.44 040709 FB pragma disable of 64 bit pointer warnings      */
/*    1.45 050428 FB gread 16 MB at a time for win32                */
/*    1.46 0504xx FB unknown                                        */
/*    1.47 0504xx FB unknown                                        */
/*    1.48 050603 FB add grwopen and ggetm to be self contained     */
/*    1.49 050606 FB printf for unix no %ll reformat                */
/*    1.50 060710 FB calculate high water on galloc                 */
/*    1.51 061205 FB ps3 no getenv, gx gstdout for -log             */
/*    1.52 070324 FB xbox360                                        */
/*    1.53 070924 FB wii                                            */
/*    1.53 180905 FR use calloc to ensure deterministic outputs     */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* This module uses unbuffered unix IO with 64 bit extensions for   */
/* gseek, gtell and glen                                            */
/*                                                                  */
/*------------------------------------------------------------------*/

#define __NOINLINE__ 1
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(__unix__) && !defined(__USE_LARGEFILE64)
#define __USE_LARGEFILE64
#endif

#include <fcntl.h>
#if defined(_MSC_VER)
// VC6 header bug fix
struct _RPC_ASYNC_STATE;
#include <windows.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif
#include "stdgimex.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4311 4312)
// warning C4311: 'type cast' : pointer truncation from 'GSTREAM *' to 'int'
// warning C4312: 'type cast' : conversion from 'int' to 'GSTREAM *' of greater size
#endif

/* Memory Functions */

int galloccount=0;
int gallocsizeallocated=0;
int gallocmaxsizeallocated=0;
int gopenmethod=3;  /* 0=con:, 1=fopen, 2=backup, 3=temp */

FILE *gstdout=stdout;

//%lld -> %I64d
int __cdecl stdprintf(const char *format, ...)
{
    va_list arglist;
    int r;
    char format2[8192];
    char tempstr[8192];
    const char *s=format;
    char *d=format2;
    while (*s)
    {
#if defined (_MSC_VER)
        if (!strncmp(s,"%lld",4))
        {
            strcpy(d,"%I64d");
            d += 5;
            s += 4;
        }
        else
#endif
            *d++ = *s++;
    }
    *d = 0;
    va_start(arglist, format);
#if defined (_MSC_VER)
    r = _vsnprintf(tempstr, 8192, format2, arglist);
#else
    r = vsprintf(tempstr, format2, arglist);
#endif
    fprintf(gstdout,"%s",tempstr);
    va_end(arglist);
    return(r);
}

int __cdecl (*gprintf)(const char *, ...) = stdprintf;

void * GCALL galloc(int size)
{
    char *memptr;

    if (size<=0)
    {
        gprintf("galloc - INVALID SIZE %ld\n",size);
        return((void *)0);
    }
#ifdef DEBUG
    if (size>=100*1024*1024)
    {
        gprintf("galloc - WARNING VERY LARGE SIZE %ld\n",size);
    }
#endif

    // diagnostic for regression testing gimex modules/tools
#ifndef __PPU__
    {
        static int gallocmax=0;
        if (!gallocmax)
        {
            char *forcedriver;
            forcedriver = getenv("GALLOCMAX");
            if (forcedriver)
                gallocmax = atoi(forcedriver);
            else
                gallocmax = -1;
        }
        if (galloccount==gallocmax)
        {
            gprintf("galloc - ERROR, PURPOSELY FAILING ON GALLOC COUNT %d, SIZE %ld\n",galloccount,size);
            return((void *)0);
        }
    }
#endif

    // todo fed: using calloc instead, make sure buffers are always initialized
	//memptr = (char*) malloc((size_t)size+12);
	memptr = (char*)calloc(1, (size_t)size + 12);
    if (!memptr || memptr==(char *)-1)
    {
        gprintf("galloc - FAILED. OUT OF MEMORY. SIZE REQUESTED %ld\n",size);
        return((void *)0);
    }

    memcpy(memptr,"GMEM",4);            /* sentinal */
    *(int *) ((char *)memptr+4) = size;
    memcpy(memptr+8+size,"GEND",4);

#ifdef _DEBUG
    /* make initial data garbage to bring out errors */
    memset(memptr+8, 0x66, size);
#endif
    ++galloccount;
    gallocsizeallocated += size;
    if (gallocsizeallocated>gallocmaxsizeallocated)
        gallocmaxsizeallocated = gallocsizeallocated;
    return((void *)(memptr+8));
}

bool GCALL gfree(void *memptr)
{
    int size;
    bool ok=false;

    if ((intptr_t)memptr>-2048 && (intptr_t)memptr<2048)
    {
        gprintf("gfree - ATTEMPT TO FREE NULL POINTER\n");
    }
    else
    {
        size = *(int *) ((char *) memptr - 4);

        if (!memcmp(((char *) memptr-8),"GMEM",4) && size>0)
            if (!memcmp(((char *) memptr+size),"GEND",4))
                ok = true;
        if (!ok)
        {
            if (!memcmp((char *) memptr-8,"GFRE",4))
                gprintf("gfree - ATTEMPT TO FREE MEMORY TWICE\n");
            else
                gprintf("gfree - SENTINAL CORRUPTED; PROBABLE MEMORY DATA CORRUPTION\n");
        }
        else
        {
            memcpy((char *) memptr - 8,"GFRE",4);            /* mark as freed */
#ifdef _DEBUG
            /* make data garbage to bring out errors */
            memset(memptr, 'x', size);
#endif
            free((char *) memptr - 8);

            --galloccount;
            gallocsizeallocated -= size;
        }
    }

    return(ok);
}

/* File Functions */

// open for read

GSTREAM * GCALL gopen(const char *filename)
{
    if (!strcmp(filename,"stdin:"))
        return (GSTREAM *) 1;
#if defined(_MSC_VER)
    char namebuf[512];
    strcpy(namebuf,filename);
    for (int i=0; namebuf[i]; ++i)
        if (namebuf[i]=='/') namebuf[i] = '\\';
    int h=open(namebuf, O_RDONLY|O_BINARY);
#elif defined(O_LARGEFILE) && defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(filename, O_RDONLY|O_LARGEFILE);
#elif defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(filename, O_RDONLY);
#elif defined (O_BINARY)
    int h=open(filename, O_RDONLY|O_BINARY);
#else
    int h=open(filename, O_RDONLY);
#endif
    if (h<0) h = 0;
    return((GSTREAM *) h);
}

// open for read & write

GSTREAM * GCALL grwopen(const char *filename)
{
    if (!strcmp(filename,"stdout:"))
        return (GSTREAM *) 2;
#if defined(_MSC_VER)
    char namebuf[512];
    strcpy(namebuf,filename);
    for (int i=0; namebuf[i]; ++i)
    {
        if (namebuf[i]=='/' || namebuf[i]=='\\')
        {
            namebuf[i] = '\0';
            mkdir(namebuf);
            namebuf[i] = '\\';
        }
    }
    int h=open(namebuf, O_RDWR|O_CREAT|O_BINARY, _S_IREAD|_S_IWRITE);
#elif defined(O_LARGEFILE) && defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(filename, O_RDWR|O_CREAT|O_LARGEFILE, S_IRWXU|S_IRGRP|S_IROTH);
#elif defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(filename, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IROTH);
#else
    int h=open(filename, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IROTH);
#endif
    if (h<0) h = 0;
    return((GSTREAM *) h);
}


// open for write

GSTREAM * GCALL gwopen(const char *filename)
{
    if (!strcmp(filename,"stdout:"))
        return (GSTREAM *) 2;

#if defined(_MSC_VER)
    char namebuf[512];
    strcpy(namebuf,filename);
    for (int i=0; namebuf[i]; ++i)
    {
        if (namebuf[i]=='/' || namebuf[i]=='\\')
        {
            namebuf[i] = '\0';
            mkdir(namebuf);
            namebuf[i] = '\\';
        }
    }
    int h=open(namebuf, O_RDWR|O_CREAT|O_BINARY|O_TRUNC, _S_IREAD|_S_IWRITE);
#elif defined(O_LARGEFILE) && defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(filename, O_RDWR|O_CREAT|O_TRUNC|O_LARGEFILE, S_IRWXU|S_IRGRP|S_IROTH);
#elif defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(filename, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH);
#else
    int h=open(filename, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH);
#endif
    if (h<0) h = 0;
    return((GSTREAM *) h);
}
bool GCALL gclose(GSTREAM *g)
{
    bool ok=true;
    int h = (int)(intptr_t) g;
    if (h>2)
        ok = !close(h);
    return ok;
}

int GCALL gread(GSTREAM *g, void *buf, int size)
{
    int totalread=0;
    int retry = 3;
    do
    {
        const int MAXREADSIZE = 16*1024*1024; // Note network shared drives have a maximum read size limitation
        int amounttoread = gmin(size,MAXREADSIZE);
        int amountread = read((intptr_t) g, buf, (unsigned int) amounttoread);

        if (amountread < amounttoread)
        {
#if defined(_MSC_VER) && defined(DEBUG) && !defined(gtell)
            int err;
            err = GetLastError();
            gprintf("read warning.  %d requested, got %d. Error %d.  gtell %lld\n", size, amountread, err, gtell(g));
#endif
            --retry;
            if (!retry) break;
        }
        if (amountread<0)
            amountread = 0;
        buf = (void *)((char *)buf+amountread);
        size -= amountread;
        totalread += amountread;
    } while (size>0);
    return totalread;
}

int GCALL gwrite(GSTREAM *g, const void *buf, int size)
{
    int amountwritten = write((intptr_t) g, buf, (unsigned int) size);
    if (amountwritten < size)
    {
        gprintf("write warning.  %d requested, got %d\n", size, amountwritten);
#ifdef DEBUG
        perror("write warning-");
#endif
    }
    return(amountwritten);
}

static GPOS __gseek(GSTREAM *g, GPOS seekpos, int whence)
{
    GPOS pos;
#if USEFILE64

#if defined(_MSC_VER)
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx (&osvi);

#if USEFILE64>=2
    if ((whence==SEEK_SET && seekpos<2147483648I64) || osvi.dwPlatformId < VER_PLATFORM_WIN32_NT)
        pos = (GPOS) lseek((int) g, (int) seekpos, whence);
    else
#endif
        pos = (GPOS) _lseeki64((int) g, (__int64)seekpos, whence);
#elif defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    pos=lseek64((int) g, seekpos, whence);
#else // osx beos
    pos=lseek((intptr_t) g, (off_t) seekpos, whence);
#endif

#else /* USEFILE64 */
    pos = (GPOS) lseek((int) g, (int) seekpos, whence);
#endif

    // retry with near seek
    if (pos==(GPOS)-1)
    {
#ifdef DEBUG
        gprintf("__gseek warning.  pos attempted %lld whence %d\n", seekpos, whence);
#endif
        pos = (GPOS) lseek((intptr_t) g, (int) seekpos, whence);
#ifdef DEBUG
        gprintf("__gseek reseek returned %lld\n", pos);
#endif
    }

#ifdef DEBUG
    if (pos==(GPOS)-1)
        perror("__gseek -");
#endif
    return(pos);
}

bool GCALL gseek(GSTREAM *g, GPOS seekpos)
{
    bool ok=true;
    GPOS newpos;

    newpos = __gseek(g,seekpos,SEEK_SET);
    if (newpos==(GPOS)-1)
        ok = false;

    return ok;
}

GPOS GCALL glen(GSTREAM *g)
{
//steve webers w2k sp1 machine fails on glen (-1)
//#if defined(_MSC_VER)
//    return((GPOS)_filelengthi64((int) g));
//#else

    GPOS oldpos;
    GPOS len;

    oldpos = __gseek(g,(GPOS) 0,SEEK_CUR);

    len = __gseek(g,(GPOS) 0,SEEK_END);

    oldpos = __gseek(g,oldpos,SEEK_SET);
    if (len==(GPOS)-1)
        len = (GPOS) 0;

    return(len);
//#endif
}

GPOS GCALL gtell(GSTREAM *g)
{
//#if defined(_MSC_VER)
//    return((GPOS)_telli64((int) g));
//#else

    GPOS currentpos;

    currentpos = __gseek(g,(GPOS) 0,SEEK_CUR);

    return(currentpos);
//#endif
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; galloc - Allocate a block of memory.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; void *galloc(size)
; int size                Number of bytes of memory to allocate
;
;
; DESCRIPTION
;
; The galloc function allocates a block of memory, returning a pointer to
; the memory block.
; Free the memory block with the gfree function.
;
;
; RETURN VALUE
;
; Void pointer to newly allocated block of memory.  NULL if not successful.
;
;
; SEE ALSO
;
; gfree
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; gfree - Free a block of memory.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; int gfree(memptr)
; void *memptr             Pointer to block of memory as allocated by galloc
;
;
; DESCRIPTION
;
; The gfree function frees a block of memory that was allocated by the
; galloc function.
;
;
; RETURN VALUE
;
; Non-zero if successful.
;
;
; SEE ALSO
;
; galloc
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; ggetm - Get memory in Motorola (big-endian) format.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; unsigned int ggetm(src, bytes)
; const void *src       Pointer to block of source memory
; int bytes             Number of bytes to obtain (1..4)
;
;
; DESCRIPTION
;
; The ggetm function reads a block of memory in Motorola (big-endian) format.  The returned
; value is zero extended.
;
;
; RETURN VALUE
;
; Up to 4-bytes in big-endian format.
;
;
; SEE ALSO
;
; ggetm, ggeti, gputm, gputi
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; ggeti - Get memory in Intel (little-endian) format.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; unsigned int ggeti(src, bytes)
; const void *src       Pointer to block of source memory
; int bytes             Number of bytes to obtain (1..4)
;
;
; DESCRIPTION
;
; The ggeti function reads a block of memory in Intel (little-endian) format.
;
;
; RETURN VALUE
;
; Up to 4-bytes in little-endian format.
;
;
; SEE ALSO
;
; ggetm, ggeti, gputm, gputi
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; gputm - Put memory in Motorola (big-endian) format.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; void gputm(dst, data, bytes)
; void *dst             Pointer to block of destination memory
; unsigned int data    Source data
; int bytes             Number of bytes to write (1..4)
;
;
; DESCRIPTION
;
; The gputm function writes a block of memory in Motorola (big-endian) format.
;
;
; SEE ALSO
;
; ggetm, ggeti, gputm, gputi
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif


#if 0
/****************************************************************************************
; ABSTRACT
;
; gputi - Put memory in Intel (little-endian) format.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; void gputi(dst, data, bytes)
; void *dst             Pointer to block of destination memory
; unsigned int data    Source data
; int bytes             Number of bytes to write (1..4)
;
;
; DESCRIPTION
;
; The gputi function writes a block of memory in Intel (little-endian) format.
;
;
; SEE ALSO
;
; ggetm, ggeti, gputm, gputi
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; GSTREAM - File stream handle.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; typedef struct GSTREAM GSTREAM;
;
;
; DESCRIPTION
;
; The GSTREAM file handle is returned by the gopen function for subsequent
; reading and writing to and from a file.
;
;
; SEE ALSO
;
; gopen, GIMEX_gopen
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; gopen  - Open a file for read access.
; gwopen - Open a file for write access.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; GSTREAM *gopen(filename)
; GSTREAM *gwopen(filename)
; const char *filename           Pathname of file to open
;
;
; DESCRIPTION
;
; The gopen function opens a file for read access, and returns a file handle.
;
; The gwopen function opens a file for write access, and returns a file handle.
; gwopen will fail on attempting to open a read-only file.
;
; The filename may either be relative to the current directory or an absolute
; pathname.
;
;
; RETURN VALUE
;
; Handle to opened file.  NULL if not successful.
;
;
; SEE ALSO
;
; gclose
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; gclose - Close an open file.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; int gclose(g)
; GSTREAM *g               File handle as returned by gopen or gwopen
;
;
; DESCRIPTION
;
; The gclose function closes a file that has been open by gopen or gwopen.
;
;
; RETURN VALUE
;
; Non-zero if successful.
;
;
; SEE ALSO
;
; gopen, gwopen
;
;
; EXAMPLE
;
; any2any_eg.c
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; gread - Read from an open file stream.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; int gread(g, buf, size)
; GSTREAM *g            File handle as returned by gopen
; void    *buf          Destination buffer to write to
; int    size          Number of bytes to read
;
;
; DESCRIPTION
;
; The gread function reads data from a file at the current file position.  The given
; number of bytes of data is read into a specified buffer which must be allocated
; before calling gread. The file position is advanced to the end of the read data.
;
; RETURN VALUE
;
; Number of bytes successfully read.
;
;
; SEE ALSO
;
; gopen, gwopen, gwrite
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; gwrite - Write to an open file stream.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; int gwrite(g, buf, size)
; GSTREAM    *g            File handle as returned by gwopen
; const void *buf          Source buffer to read from
; int       size          Number of bytes to write
;
;
; DESCRIPTION
;
; The gwrite function writes data to a file at the current file position from
; a buffer. The given number of bytes of data is read into a specified buffer which
; must be allocated before calling gwrite.  The file position is advanced to the end
; of the write data.
;
;
; RETURN VALUE
;
; Number of bytes successfully written.
;
;
; SEE ALSO
;
; gopen, gwopen, gread
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; gseek - Seek to position in file stream.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; int gseek(g, seekpos)
; GSTREAM *g            File handle as returned by gopen or gwopen
; int    seekpos        Position in bytes from start of file to seek to
;
;
; DESCRIPTION
;
; The gseek function moves the current file position to a position specified
; in number of bytes from the start of the file.  An seekpos of 0 seeks to
; the first byte of the file.
;
;
; RETURN VALUE
;
; Non-zero value if successful.
;
;
; SEE ALSO
;
; gread, gwrite
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; glen - Get length of file.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; GPOS glen(g)
; GSTREAM *g            File handle as returned by gopen or gwopen
;
;
; DESCRIPTION
;
; The glen function returns the length of the file in bytes.
;
;
; RETURN VALUE
;
; Length of file in bytes.
;
;
; SEE ALSO
;
; gopen, gread, gwrite
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif

#if 0
/****************************************************************************************
; ABSTRACT
;
; gtell - Get current position in file.
;
;
; SUMMARY
;
; #include "stdgimex.h"       Required for function & structure declarations
;
; GPOS gtell(g)
; GSTREAM *g            File handle as returned by gopen or gwopen
;
;
; DESCRIPTION
;
; The gtell function returns the current position in the file as an seekpos
; in bytes from the beginning of the file.
;
;
; RETURN VALUE
;
; Offset in bytes.
;
;
; SEE ALSO
;
; gopen, gseek
;
;
; EXAMPLE
;
; none available yet
;
;
; END ABSTRACT
****************************************************************************************/
#endif


