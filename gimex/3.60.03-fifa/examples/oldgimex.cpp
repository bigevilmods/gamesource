// (c) 1994-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*           Gimex Standard File and Memory Interface v1.30         */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                     Code Module - Oct 22, 2001                   */
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
/*    1.28 010222 FB set GALLOCMAX=n provoke galloc error nth call  */
/*    1.29 011009 FB remove max size galloc/free limit              */
/*    1.30 011022 FB printf allow 8192 and check for overflow       */
/*    1.31 011022 FB printf allow 8192 and check for overflow       */
/*    1.32 011120 FB use .old not .bak to avoid name completion     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* This modules makes an easy basis for new Gimex low level         */
/* functions.  You will need to make the following changes:         */
/*                                                                  */
/* function              what to change                             */
/* --------              --------------                             */
/* galloc/free           put in your memory manager                 */
/* STDSTREAM.handle      handle for your file system                */
/* gopen/gwopen          fopen, fseek, ftell (find file size)       */
/* gclose                fclose                                     */
/* gread                 fread and possibly memcpy                  */
/* gwrite                fwrite                                     */
/* gseek                 fseek                                      */
/*                                                                  */
/* The other routines should not need changing.                     */
/*                                                                  */
/* If _DEFINE is defined then all memory allocated and freed will   */
/* have memset called on it to reproduce memory bugs faster         */
/*                                                                  */
/*------------------------------------------------------------------*/

#include <stdarg.h>
#include "stdgimex.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

int galloccount=0;
int gopenmethod=3;  /* 0=con:, 1=fopen, 2=backup, 3=temp */

/****************************************************************************/
/* Internal Functions                                                       */
/****************************************************************************/

#define STDPRINTF_MAXSTRING 8192
int __cdecl stdprintf(const char *format, ...)
{
    va_list arglist;
    int r;
    char tempstr[STDPRINTF_MAXSTRING];

    va_start(arglist, format);
    tempstr[STDPRINTF_MAXSTRING-1] = '\0';
#if defined (_MSC_VER)
    r = _vsnprintf(tempstr, STDPRINTF_MAXSTRING, format, arglist);
#else
    r = vsprintf(tempstr, format, arglist);
#endif
    if (tempstr[STDPRINTF_MAXSTRING-1] != '\0')
        printf("ERROR - stdprintf string longer than maximum %d\n",STDPRINTF_MAXSTRING);
    printf("%s",tempstr);
    va_end(arglist);
    return(r);
}

int __cdecl (*gprintf)(const char *, ...) = stdprintf;

static int gwriteflush(STDSTREAM *h)
{
    int bytes=0;

    if (h->dirty && h->write)
    {
        fseek(h->handle, (int)h->cachestart, SEEK_SET);
        if ((h->bufend - h->buf)>0)
            bytes = fwrite((void *)h->buf, (size_t) 1, (size_t) (h->bufend - h->buf), h->handle);
        if (bytes < (h->bufend - h->buf))
            gprintf("gwriteflush - OUT OF DISK SPACE\n");
        h->dirty = false;
    }
    h->bufptr = h->bufend = h->buf;
    fflush(h->handle);

    return(bytes);
}

static void GIMEX_tempname(char *d, const char *s, const char *ext)
{
    char *x;
    strcpy(d,s);

    x = d+(strlen(d))-1;            /* find extension */
    while (x>d)
    {
        if (*x=='.')
            break;
        if (*x==(char)':' || *x==(char)'/' || *x==(char)'\\')
            break;
        --x;
    }
    if (*x!='.')
    {
        x = d+(strlen(d));
    }
    *x++ = '\0'; /* remove '.' */
    strcat(d,ext);
}

#if defined (_MSC_VER)

static bool GIMEX_isreadonly(const char *filename)
{
    struct _stat buf;
    if (_stat(filename, &buf) != -1)
        if ((buf.st_mode & _S_IREAD) && !(buf.st_mode & _S_IWRITE))
            return true;
    return false;
}

static bool GIMEX_isdirectory(const char *filename)
{
    struct _stat buf;
    if (_stat(filename, &buf) != -1)
        if (buf.st_mode & _S_IFDIR)
            return true;
    return false;
}

#else

static bool GIMEX_isreadonly(const char *filename)
{
#ifndef __BEOS__
    struct stat buf;
    if (stat(filename, &buf) != -1)
        if ((buf.st_mode & S_IREAD) && !(buf.st_mode & S_IWRITE))
            return true;
#endif
    return false;
}

static bool GIMEX_isdirectory(const char *filename)
{
    struct stat buf;
    if (stat(filename, &buf) != -1)
        if (buf.st_mode & S_IFDIR)
            return true;
    return false;
}

#endif

/****************************************************************************************/
/* Memory Functions                                                                     */
/****************************************************************************************/

#if 0
/****************************************************************************************
; ABSTRACT
;
; galloc - Allocate a block of memory.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
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

void * GCALL galloc(int size)
{
    char *memptr;

    if (size<=0)
    {
        gprintf("galloc - INVALID SIZE %ld\n",size);
        return((void *)0);
    }
#ifdef DEBUG
    if (size>=40*1024*1024)
    {
        gprintf("galloc - WARNING VERY LARGE SIZE %ld\n",size);
    }
#endif

    // diagnostic for regression testing gimex modules/tools
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

    memptr = (char*) malloc((size_t)size+12);
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
    return((void *)(memptr+8));
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; gfree - Free a block of memory.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
;
; bool gfree(memptr)
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
; true if successful.
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

bool GCALL gfree(void *memptr)
{
    int size;
    bool ok=false;

    if ((int)memptr>-2048 && (int)memptr<2048)
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
        }
    }

    return(ok);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; ggetm - Get memory in Motorola (big-endian) format.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
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
; #include "gimex.h"       Required for function & structure declarations
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
; #include "gimex.h"       Required for function & structure declarations
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
; #include "gimex.h"       Required for function & structure declarations
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

/****************************************************************************************/
/* File Functions                                                                       */
/****************************************************************************************/

#if 0
/****************************************************************************************
; ABSTRACT
;
; GSTREAM - File stream handle.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
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
; #include "gimex.h"       Required for function & structure declarations
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

GSTREAM * GCALL gopen(const char *filename)
{
    FILE *handle;
    int len = 0;
    STDSTREAM *h;
    int gmethod;

    if (!strncmp(filename,"con:",4))
    {
        handle = stdin;
        gmethod = 0;
    }
    else
    {
        handle = fopen(filename,"rb");
        gmethod = 1;
    }

    h = 0;
    if (handle)
    {
        bool ok = !fseek(handle,0,SEEK_END);
        if (ok)
        {
            len = ftell(handle);
            fseek(handle,0,SEEK_SET);

            h = (STDSTREAM *) galloc(sizeof(STDSTREAM));
            if (h)
            {
                h->handle = handle;
                h->offset = 0;
                h->len    = len;
                h->bufptr = h->buf;
                h->bufend = h->buf;
                h->write  = false;
                h->dirty  = false;
                h->gopenmethod = gmethod;
                h->cachestart = 0;
                strcpy(h->filename,filename);
            }
            else
                gprintf("gopen - UNABLE TO ALLOCATE MEMORY FOR FILE HANDLE (%s)\n", filename);
        }
        else
            gprintf("gopen - SEEK ON OPEN FAILED (%s)\n", filename);
    }
    return((GSTREAM *) h);
}

GSTREAM * GCALL gwopen(const char *filename)
{
    FILE *handle=0;
    int len;
    STDSTREAM *h=0;
    int gmethod=gopenmethod;
    char tempname[GIMEXMAXPATH];

    tempname[0] = (char) 0;
    if (!strncmp(filename,"con:",4))
    {
        handle = stdout;
        gmethod = 0;
    }
    else
    {
        if (GIMEX_isreadonly(filename))
        {
            gprintf("gwopen - ATTEMPT TO OPEN READONLY FILE FOR WRITING (%s)\n", filename);
        }
        else
        {
            if (GIMEX_isdirectory(filename))
            {
                gprintf("gwopen - ATTEMPT TO OPEN DIRECTORY FOR WRITING (%s)\n", filename);
            }
            else
            {
                if (gmethod==2 || gmethod==3)
                    GIMEX_tempname(tempname,filename,".tm1");
                else
                    strcpy(tempname,filename);

                handle = fopen(tempname,"w+b");
                if (!handle)
                {
                    handle = fopen(tempname,"wb");
                    gprintf("gwopen - UNABLE TO OPEN FILE FOR WRITE (%s)\n", tempname);
                }
            }
        }
    }

    len = 0;
    if (handle)
    {
        h = (STDSTREAM *) galloc(sizeof(STDSTREAM));
        if (h)
        {
            h->handle = handle;
            h->offset = 0;
            h->len    = len;
            h->bufptr = h->buf;
            h->bufend = h->buf;
            h->write  = true;
            h->dirty  = false;
            h->gopenmethod = gmethod;
            h->cachestart = 0;
            strcpy(h->filename,filename);
            strcpy(h->tempname,tempname);
        }
        else
            gprintf("gwopen - UNABLE TO ALLOCATE MEMORY FOR FILE HANDLE (%s)\n", filename);
    }
    return((GSTREAM *) h);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; gclose - Close an open file.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
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
; true if successful.
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

#define RENAME_filename(f) f

bool GCALL gclose(GSTREAM *g)
{
    STDSTREAM *h=(STDSTREAM *) g;
    bool ok=true;

    if (g)
    {
        /* Write out the cache */
        gwriteflush(h);

        /* Close the file and free the memory */
        {
            int gmethod=h->gopenmethod;
            if (gmethod)
            {
                ok = !fclose(h->handle);
                if (ok && (gmethod==2 || gmethod==3))
                {
                    char tm2name[GIMEXMAXPATH];

                    GIMEX_tempname(tm2name,h->filename,".tm2");

                    remove(tm2name);                                 /* remove old temp2 */
                    rename(h->filename,RENAME_filename(tm2name));     /* change filename to temp2 */
                    ok = !rename(h->tempname,RENAME_filename(h->filename)); /* change temp to filename */
                    if (ok)
                    {
                        if (gmethod==2)
                        {
                            char bakname[GIMEXMAXPATH];
                            GIMEX_tempname(bakname,h->filename,".old");

                            remove(bakname);                          /* remove old backup */
                            rename(tm2name,RENAME_filename(bakname));  /* change temp 2 to backup */
                        }
                        else /* (gmethod==3) */
                            remove(tm2name);                          /* temp2 */
                    }
                }
            }
            ok &= gfree((char *) g);
        }
    }

    return(ok);
}

#undef RENAME_filename

#if 0
/****************************************************************************************
; ABSTRACT
;
; gread - Read from an open file stream.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
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

int GCALL gread(GSTREAM *g, void *buf, int size)
{
    STDSTREAM *h=((STDSTREAM *) g);
    int len;
    int bytes=0, bytes2=0;

    if (g == NULL)
        return(0);

    if (size < 0)
        size = 0;

    /* Read what you can from the cache */
    len = gmin(size,(h->bufend-h->bufptr));
    if (len>0)
    {
        memcpy(buf,h->bufptr,(size_t)len);
        h->bufptr += len;
        h->offset += len;
        buf = (void *) ((char *)buf + len);
        size -= len;
        bytes += len;
    }

    /* Update size to hold how much to really read after cache emptied */
    size = (int)gmin(size,(h->len-h->offset));

    if (size < 0)
    {
        gprintf("gread - ATTEMPT TO READ PAST END OF FILE\n");
        return(bytes);
    }

    /* If all that can be read from the cache has been read and there is more to read */
    if (!(h->bufend-h->bufptr) && size)
    {
        int blockstart = ((int)((h->offset + size) / GIMEXBUFSIZE)) * GIMEXBUFSIZE;

        gwriteflush(h);

        /* If the last block of the read is not from the same block as the offset */
        /* then read directly to the buffer */
        if (h->offset < blockstart)
        {
            bytes2 = (int)fread((void *) buf, (size_t) 1, (size_t) (blockstart - h->offset), h->handle);
            buf = (void *) ((char *)buf + bytes2);
            size -= bytes2;
        }

        /* Read remaining data into the cache         */
        h->cachestart = blockstart;
        len = (int)gmin(GIMEXBUFSIZE, h->len - blockstart);
        fread((void *) h->buf, (size_t) 1, (size_t) len, h->handle);
        memcpy(buf, h->bufptr, (size_t)size);
        bytes2 += size;
        h->bufptr += size;
        h->bufend += len;
        h->offset += bytes2;
    }

    return(bytes+bytes2);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; gwrite - Write to an open file stream.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
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

int GCALL gwrite(GSTREAM *g, const void *buf, int size)
{
    STDSTREAM *h=((STDSTREAM *) g);
    int len;
    int bytes=0;

    if (g == NULL)
        return(0);

    /* Check that file is not read-only */
    if (!h->write)
    {
        gprintf("gwrite - ATTEMPT TO WRITE TO A READ ONLY FILE\n");
        return(0);
    }

    /* First fill up cache */
    len = gmin(size, h->buf+GIMEXBUFSIZE-h->bufptr);
    if (len>0)
    {
        memcpy((void *)h->bufptr, (void *)buf, (size_t)len);
        h->bufptr += len;
        h->offset += len;
        if (h->bufend < h->bufptr)
            h->bufend = h->bufptr;
        if (h->offset > h->len)
            h->len = h->offset;
        h->dirty = true;
        buf = (void *) ((char *)buf + len);
        size -= len;
        bytes += len;
    }

    /* If the cache is full and there is more to write */
    if (!(h->bufend-h->bufptr) && size)
    {
        int blockstart = ((int)((h->offset + size) / GIMEXBUFSIZE)) * GIMEXBUFSIZE;

        gwriteflush(h);

        /* Read in the next block of cache if possible (for writes into the middle of a file)         */
        if (h->len > blockstart)
        {
            len = (int)gmin(GIMEXBUFSIZE, h->len - blockstart);
            fseek(h->handle, blockstart, SEEK_SET);
            fread((void *) h->buf, (size_t) 1, (size_t) len, h->handle);
            h->bufend += len;
        }

        /* Write out to the start of the next block to be cached */
        fseek(h->handle, (int) h->offset, SEEK_SET);
        len = 0;
        if ((blockstart - h->offset)>0)
            len = (int)fwrite((void *) buf, (size_t) 1, (size_t) (blockstart - h->offset), h->handle);

        if (len < (blockstart - h->offset))
        {
            gprintf("gwrite - OUT OF DISK SPACE\n");
            return(bytes + len);
        }
        h->offset += len;
        bytes += size;
        size -= len;

        /* Write the rest of the data into the cache */
        memcpy((void *)h->buf, (void *)((char *)buf + len), (size_t) size);
        h->offset += size;
        h->cachestart = blockstart;
        h->bufptr += size;
        if (h->offset > h->len)
            h->len = h->offset;
        if (h->bufptr > h->bufend)
            h->bufend = h->bufptr;
        h->dirty = true;
    }

    return(bytes);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; gseek - Seek to position in file stream.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
;
; bool gseek(g, offset)
; GSTREAM *g            File handle as returned by gopen or gwopen
; int    offset        Position in bytes from start of file to seek to
;
;
; DESCRIPTION
;
; The gseek function moves the current file position to a position specified
; in number of bytes from the start of the file.  An offset of 0 seeks to
; the first byte of the file.
;
;
; RETURN VALUE
;
; true value if successful.
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

bool GCALL gseek(GSTREAM *g, GPOS offset)
{
    STDSTREAM *h=((STDSTREAM *) g);
    GPOS relative;
    bool ok=true;
    int len;

    if (g == NULL)
        return false;

    /* clip bad seeks  */
    if (offset < 0)
    {
        offset = 0;
        ok = false;
    }
    else if (offset > h->len && !h->write)
    {
        offset = h->len;
        ok = false;
    }

    relative = (GPOS)(offset-h->offset);
    h->offset += (long) relative;

    /* If the relative move is outside of the cache then  ( >= for right half??) */
    if ((relative < h->buf-h->bufptr) || (relative > h->bufend-h->bufptr))
    {
        int blockstart = ((int)(h->offset / GIMEXBUFSIZE)) * GIMEXBUFSIZE;

        gwriteflush(h);

        ok = !fseek(h->handle, blockstart, SEEK_SET) && ok;
        h->cachestart = blockstart;

        /* If not seeked past end of file read in the next block of cache and position bufptr */
        if (h->len > blockstart)
        {
            len = (int)gmin(GIMEXBUFSIZE, h->len - blockstart);
            fread((void *) h->buf, (size_t) 1, (size_t) len, h->handle);
            h->bufend += len;
            h->bufptr += (h->offset - blockstart);
        }
        /* Otherwise set cache to 00 up to seek point */
        else
        {
            len = (int)h->offset - blockstart;
            memset((void *) h->buf, (unsigned char)0x00, (size_t)len);
            h->bufptr = h->bufend = h->buf + len;
        }
    }
    else
        h->bufptr += relative;

    return(ok);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; glen - Get length of file.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
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

GPOS GCALL glen(GSTREAM *g)
{
    return(((STDSTREAM *) g)->len);
}

#if 0
/****************************************************************************************
; ABSTRACT
;
; gtell - Get current position in file.
;
;
; SUMMARY
;
; #include "gimex.h"       Required for function & structure declarations
;
; GPOS gtell(g)
; GSTREAM *g            File handle as returned by gopen or gwopen
;
;
; DESCRIPTION
;
; The gtell function returns the current position in the file as an offset
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

GPOS GCALL gtell(GSTREAM *g)
{
    return(((STDSTREAM *) g)->offset);
}
