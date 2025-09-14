// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*                                                                  */
/*           Gimex Standard File and Memory Interface v1.63         */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                     Code Module - Jul 14, 2008                   */
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
/*    1.34 020226 FB ifdef gtell for printf                        */
/*    1.35 020227 FB unistd for lseek64 on unix                     */
/*    1.36 020415 ID added __USE_LARGEFILE64 macro for unix         */
/*    1.37 020605 FB mkdir on gwopen                                */
/*    1.38 020728 FB off_t lseek osx, USEFILE64 for pc win98        */
/*    1.39 020730 FB USEFILE64 2 for 64 bit exclusively             */
/*    1.40 021001 FB printf to logfile                             */
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
/*    1.54 080107 FB snc version, removing assembly                 */
/*    1.55 080318 FB track high water galloc                        */
/*    1.56 080331 FB galloc/gread/gwrite take size_t for 64 bit     */
/*    1.57 080401 FB xenon and wii remerged                         */
/*    1.58 080416 FB xenon use D:\ by default                       */
/*    1.59 080421 FB xenon getm/putm, wii buildable                 */
/*    1.60 080509 FB wii open/read fix                              */
/*    1.61 080613 FB gopen PC check for negative and fail           */
/*    1.62 080625 FB remove USEFILE64 and windows for pc64 bit      */
/*    1.63 080714 FB remove debug GetLastError                      */
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
#include <sys/stat.h>
#include <fcntl.h>
#if defined(__PPCGEKKO__)
// needed for DVDOpen
#include <inttypes.h> // intptr_t
#include <dolphin/dvd.h>
#define close(hdl) DVDClose((DVDFileInfo *)hdl)
#define read(hdl,buf,size) DVDRead((DVDFileInfo *)hdl,buf,size,pos)
#endif

#if defined(__unix__) || defined(__apple__)
#include <stdint.h>
#endif

#if defined(__unix__) && !defined(__USE_LARGEFILE64)
#define __USE_LARGEFILE64
#endif

#if defined(_MSC_VER) && !defined(_XBOX)
struct _RPC_ASYNC_STATE;
//include <windows.h>
#endif
#if defined(_MSC_VER)
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#if defined(_XBOX)
// for OutputDebugString
#include <xtl.h>
#endif
#if defined(_R5900)
#include <sifdev.h>
#include <eekernel.h>
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
size_t gallocsizeallocated=0;
size_t gallocmaxsizeallocated=0;
int gopenmethod=3;  /* 0=con:, 1=fopen, 2=backup, 3=temp */

FILE *gstdout=stdout;

//%lld -> %I64d
static int __cdecl stdprintf(const char *format, ...)
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


//gekko
#if defined(__PPCGEKKO__)
static void WiiInit(void)
{
    static bool WiiInitNeeded=true;

    if (WiiInitNeeded)
    {
        WiiInitNeeded = false;
        OSInit();
        DVDInit();
    }

    return;
}

#endif


#define DEBUGMEMORY 0
#if DEBUGMEMORY
static size_t gHighWater=0;
static size_t gMemUsage=0;
static int gAllocCount=0;
#endif

void * GCALL galloc(size_t size)
{
    char *memptr;

    if (size==0)
    {
        printf("galloc - INVALID SIZE %llu\n",(unsigned long long)size);
        return((void *)0);
    }
#ifdef DEBUG
    if (size>=1*1024*1024*1024)
    {
        printf("galloc - WARNING VERY LARGE SIZE %llu\n",(unsigned long long)size);
    }
#endif

#if DEBUGMEMORY
    gMemUsage += (size_t) size;
    if (gMemUsage > gHighWater)
        gHighWater = gMemUsage;
    ++gAllocCount;
    printf("alloc %7llu bytes, MemUsage %7llu, HighWater %7llu, Count %2d\n",(unsigned long long) size, (unsigned long long) gMemUsage, (unsigned long long) gHighWater, gAllocCount);
#endif


    // diagnostic for regression testing gimex modules/tools
#if !defined(__PPU__) && !defined(_XBOX)
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
            printf("galloc - ERROR, PURPOSELY FAILING ON GALLOC COUNT %d, SIZE %llu\n",galloccount,(unsigned long long) size);
            return((void *)0);
        }
    }
#endif

//gekko
#define HEADERSIZE 32
#define TAILSIZE 32

    size_t asize = ((size+HEADERSIZE+TAILSIZE)+31)&~31;

    memptr = (char*) malloc(asize);
    if (!memptr || memptr==(char *)-1)
    {
        printf("galloc - FAILED. OUT OF MEMORY. SIZE REQUESTED %llu\n",(unsigned long long) size);
        return((void *)0);
    }

    memcpy(memptr,"GMEM",4);            /* sentinal */
    *(size_t *) ((char *)memptr+4) = size;
    memcpy(memptr+HEADERSIZE+size,"GEND",4);

    ++galloccount;
    gallocsizeallocated += size;
    if (gallocsizeallocated>gallocmaxsizeallocated)
        gallocmaxsizeallocated = gallocsizeallocated;

    return (void *)(memptr+HEADERSIZE);
}

bool GCALL gfree(void *memptr)
{
    size_t size;
    bool ok=false;

    if ((intptr_t)memptr>-2048 && (intptr_t)memptr<2048)
    {
        printf("gfree - ATTEMPT TO FREE NULL POINTER\n");
    }
    else
    {
        size = *(size_t *) ((char *) memptr - HEADERSIZE + 4);

#if DEBUGMEMORY
        gMemUsage -= size;
        printf("free %7llu bytes , MemUsage %7llu, HighWater %7llu, Count %2d\n",(unsigned long long) size, (unsigned long long) gMemUsage, (unsigned long long) gHighWater,gAllocCount);
#endif

        if (!memcmp(((char *) memptr-HEADERSIZE),"GMEM",4) && size>0)
            if (!memcmp(((char *) memptr+size),"GEND",4))
                ok = true;
        if (!ok)
        {
            if (!memcmp((char *) memptr-HEADERSIZE,"GFRE",4))
                printf("gfree - ATTEMPT TO FREE MEMORY TWICE\n");
            else
                printf("gfree - SENTINAL CORRUPTED; PROBABLE MEMORY DATA CORRUPTION\n");
        }
        else
        {
            memcpy((char *) memptr - HEADERSIZE,"GFRE",4);            /* mark as freed */
#ifdef _DEBUG
            /* make data garbage to bring out errors */
            memset(memptr, 'x', size);
#endif
            free((char *) memptr - HEADERSIZE);

            --galloccount;
            gallocsizeallocated -= size;
        }
    }

    return(ok);
}

/* File Functions */

// open for read

static void gconvertFileName(char *namebuf, const char *filename, bool createSubDirs)
{
#if defined(_XBOX)
    if (filename[1]!=':')
    {
        sprintf(namebuf,"D:\\%s",filename);
        filename = namebuf;
    }
    else
#elif defined(__PPU__)
    if (filename[0]!='/')
    {
        sprintf(namebuf,"/app_home/%s",filename);
        filename = namebuf;
    }
    else
#endif
    strcpy(namebuf,filename);
#if defined(_MSC_VER)
    for (int i=0; namebuf[i]; ++i)
    {
        if (namebuf[i]=='/') namebuf[i] = '\\';

        if (createSubDirs && namebuf[i] == '\\')
        {
            namebuf[i] = '\0';
            mkdir(namebuf);
            namebuf[i] = '\\';
        }
    }
#endif
#if defined(__PPCGEKKO__) || defined(__PPU__)
    for (int i=0; namebuf[i]; ++i)
        if (namebuf[i]=='\\') namebuf[i] = '/';
#endif

}

#if defined(__PPCGEKKO__)
static int gpos=0;
#endif


GSTREAM * GCALL gopen(const char *filename)
{
    if (!strcmp(filename,"stdin:"))
        return (GSTREAM *) 1;
    char namebuf[512];
    gconvertFileName(namebuf,filename,false);

#if defined(_MSC_VER)
    int h=open(namebuf, O_RDONLY|O_BINARY);
    if (h<0) h = 0;
#elif defined(__PPCGEKKO__)
    WiiInit();
    DVDChangeDir("/");
    DVDFileInfo *fileInfo = (DVDFileInfo *) galloc(sizeof(DVDFileInfo));
    int ok = DVDOpen(namebuf, fileInfo);
    int h = ok ? (int) fileInfo : 0;
    gpos = 0;
#elif defined(O_LARGEFILE) && defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(namebuf, O_RDONLY|O_LARGEFILE);
    if (h<0) h = 0;
#elif defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(namebuf, O_RDONLY);
    if (h<0) h = 0;
#elif defined (O_BINARY)
    int h=open(namebuf, O_RDONLY|O_BINARY);
    if (h<0) h = 0;
#else
    int h=open(namebuf, O_RDONLY);
    if (h<0) h = 0;
#endif
    return((GSTREAM *) h);
}

#if !defined(__PPCGEKKO__)
// open for read & write

GSTREAM * GCALL grwopen(const char *filename)
{
    if (!strcmp(filename,"stdout:"))
        return (GSTREAM *) 2;
    char namebuf[512];
    gconvertFileName(namebuf,filename,true);

#if defined(_MSC_VER)
    int h=open(namebuf, O_RDWR|O_CREAT|O_BINARY, _S_IREAD|_S_IWRITE);
#elif defined(O_LARGEFILE) && defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(namebuf, O_RDWR|O_CREAT|O_LARGEFILE, S_IRWXU|S_IRGRP|S_IROTH);
#elif defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(namebuf, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IROTH);
#else
    int h=open(namebuf, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IROTH);
#endif
    if (h<0) h = 0;
    return((GSTREAM *) h);
}

// open for write

GSTREAM * GCALL gwopen(const char *filename)
{
    if (!strcmp(filename,"stdout:"))
        return (GSTREAM *) 2;
    char namebuf[512];
    gconvertFileName(namebuf,filename,true);

#if defined(_MSC_VER)
    int h=open(namebuf, O_RDWR|O_CREAT|O_BINARY|O_TRUNC, _S_IREAD|_S_IWRITE);
#elif defined(O_LARGEFILE) && defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(namebuf, O_RDWR|O_CREAT|O_TRUNC|O_LARGEFILE, S_IRWXU|S_IRGRP|S_IROTH);
#elif defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    int h=open64(namebuf, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH);
#else
    int h=open(namebuf, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH);
#endif
    if (h<0) h = 0;
    return((GSTREAM *) h);
}
#endif

bool GCALL gclose(GSTREAM *g)
{
    bool ok=true;
    int h = (int)(intptr_t) g;
    if (h>2)
    {
#if defined(__PPCGEKKO__)
        ok = (bool) DVDClose((DVDFileInfo *)g);
        gfree(g);
#else
        ok = !close(h);
#endif
    }
    return ok;
}


#if defined(__PPCGEKKO__)

size_t GCALL gread(GSTREAM *g, void *buf, size_t size)
{

    size_t asize = (size+31)&~31;

    char *tempbuf = (char *) galloc(asize+32);
    char *tempptr = (char *)((((uintptr_t)tempbuf)+31)&~31);

    if (gpos%4)
        printf("read warning. gpos %d not on multiple of 4 boundary. Use chimp to align your data\n", gpos);

    size_t amountread = (size_t)DVDRead((DVDFileInfo *) g, tempptr, (long)asize, gpos);

    memcpy(buf,tempptr,size);
    gfree(tempbuf);
    if (size<amountread)
        amountread = size;
    if (amountread>0)
        gpos += amountread;
    return amountread;
}

// cant write to DVD
size_t GCALL gwrite(GSTREAM *, const void *, size_t )
{
    return 0;
}

GSTREAM * GCALL gwopen(const char *)
{
    return NULL;
}


bool GCALL gseek(GSTREAM *, GPOS seekpos)
{
    gpos=(int)seekpos;
    return true;
}

GPOS GCALL glen(GSTREAM *g)
{
    return (GPOS) DVDGetLength((DVDFileInfo *) g);
}

GPOS GCALL gtell(GSTREAM *)
{
    return gpos;
}




#else

size_t GCALL gread(GSTREAM *g, void *buf, size_t size)
{
    size_t totalread=0;
    int retry = 3;
    do
    {
        const size_t MAXREADSIZE = 16*1024*1024; // Note network shared drives have a maximum read size limitation
        size_t amounttoread = gmin(size,MAXREADSIZE);
        int amountread = (int) read((int) (uintptr_t)g, buf, (unsigned int)amounttoread);
        if (amountread < (int)amounttoread)
        {
#if defined(_MSC_VER) && defined(DEBUG) && !defined(gtell)
            printf("read warning.  %d requested, got %d. gtell %lld\n", size, amountread, gtell(g));
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

size_t GCALL gwrite(GSTREAM *g, const void *buf, size_t size)
{
    size_t totalwrite=0;
    int retry = 3;
    do
    {
        const size_t MAXWRITESIZE = 16*1024*1024;
        size_t amounttowrite = gmin(size,MAXWRITESIZE);
        int amountwrite = (int) write((int) (uintptr_t) g, buf, (unsigned int)amounttowrite);

        if (amountwrite < (int)amounttowrite)
        {
#if defined(_MSC_VER) && defined(DEBUG) && !defined(gtell)
            printf("write warning.  %d requested, got %d. gtell %lld\n", size, amountwrite, gtell(g));
#endif
            --retry;
            if (!retry) break;
        }
        if (amountwrite<0)
            amountwrite = 0;
        buf = (void *)((char *)buf+amountwrite);
        size -= amountwrite;
        totalwrite += amountwrite;
    } while (size>0);
    return totalwrite;
}

static GPOS __gseek(GSTREAM *g, GPOS seekpos, int whence)
{
    GPOS pos;

#if defined(_MSC_VER) && !defined(_XBOX)
    pos = (GPOS) _lseeki64((int) (uintptr_t) g, (__int64)seekpos, whence);
#elif defined(__unix__) && (defined(__i386__) || defined(__mips) || defined(sparc))
    pos=lseek64((int) g, seekpos, whence);
#else // osx beos
    pos=lseek((intptr_t) g, (off_t) seekpos, whence);
#endif

    // retry with near seek
    if (pos==(GPOS)-1)
    {
#ifdef DEBUG
        printf("__gseek warning.  pos attempted %lld whence %d\n", seekpos, whence);
#endif
        pos = (GPOS) lseek((int)(intptr_t) g, (int) seekpos, whence);
#ifdef DEBUG
        printf("__gseek reseek returned %lld\n", pos);
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
    GPOS oldpos;
    GPOS len;

    oldpos = __gseek(g,(GPOS) 0,SEEK_CUR);

    len = __gseek(g,(GPOS) 0,SEEK_END);

    oldpos = __gseek(g,oldpos,SEEK_SET);
    if (len==(GPOS)-1)
        len = (GPOS) 0;

    return(len);
}

GPOS GCALL gtell(GSTREAM *g)
{
    GPOS currentpos;

    currentpos = __gseek(g,(GPOS) 0,SEEK_CUR);

    return(currentpos);
}

#endif


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


