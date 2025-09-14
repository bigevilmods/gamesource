// (c) 1994-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                 EAC Library Gimex interface v1.06                */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                     Code Module - Dec 25, 1994                   */
/*                                                                  */
/*------------------------------------------------------------------*/

#define __NOINLINE__ 1
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "eac.h"
#include "eacx.h"
#include "eacmenu.h"
#include "gimex.h"
#include "stdgimex.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

#define GIMEXBUFSIZE 8192

#define memcpy(d,s,l) blockmove(s,d,l)

static int gwriteflush(STDSTREAM *h);
static int isReadOnly(const char *filename);
static int isDirectory(const char *filename);

/* If compiler is WATCOM then use non-ANSI naming convention */
/* otherwise use ANSI naming convention */
#if defined (__WATCOMC__)

static int isReadOnly(const char *filename)
{
    struct _stat buf;
    if (_stat(filename, &buf) != -1)
        if ((buf.st_mode & S_IREAD) && !(buf.st_mode & S_IWRITE))
            return 1;
    return 0;
}

static int isDirectory(const char *filename)
{
    struct _stat buf;
    if (_stat(filename, &buf) != -1)
        if (buf.st_mode & S_IFDIR)
            return 1;
    return 0;
}

#elif defined (_MSC_VER)

static int isReadOnly(const char *filename)
{
    struct _stat buf;
    if (_stat(filename, &buf) != -1)
        if ((buf.st_mode & _S_IREAD) && !(buf.st_mode & _S_IWRITE))
            return 1;
    return 0;
}

static int isDirectory(const char *filename)
{
    struct _stat buf;
    if (_stat(filename, &buf) != -1)
        if (buf.st_mode & _S_IFDIR)
            return 1;
    return 0;
}

#else

static int isReadOnly(const char *filename)
{
    struct stat buf;
    if (stat(filename, &buf) != -1)
        if ((buf.st_mode & S_IREAD) && !(buf.st_mode & S_IWRITE))
            return 1;
    return 0;
}

static int isDirectory(const char *filename)
{
    struct stat buf;
    if (stat(filename, &buf) != -1)
        if (buf.st_mode & S_IFDIR)
            return 1;
    return 0;
}

#endif

/* Memory Functions */

#if 0
int galloccount=0;

void * GCALL galloc(long size)
{
    char *memptr;

    if (size<=0)
    {
        abortmessage("galloc - INVALID SIZE %ld\n",size);
        return((void *)0);
    }

    memptr = (void *) reservememadr("galloc",size+12,mb_default);
    if (!memptr)
    {
        abortmessage("galloc - FAILED. OUT OF MEMORY. SIZE REQUESTED %ld\n",size);
        return((void *)0);
    }

    memcpy(memptr,"GMEM",4);            /* sentinal */
    *(long *) ((char *)memptr+4) = size;
    memcpy(memptr+8+size,"GEND",4);

#ifdef _MSDEBUG
    /* make initial data garbage to bring out errors */
    memset(memptr+8, 0x66, size);
#endif
    ++galloccount;
    return((void *)(memptr+8));
}

int GCALL gfree(void *memptr)
{
    long size;
    int ok=0;

    if ((long)memptr>-1024 && (long)memptr<1024)
    {
        abortmessage("gfree - ATTEMPT TO FREE NULL POINTER\n");
    }
    else
    {
        size = *(long *) ((char *) memptr - 4);
        if (!memcmp(((char *) memptr-8),"GMEM",4) && size>0 && size<100*1024*1024)
            if (!memcmp(((char *) memptr+size),"GEND",4))
                ok = 1;
        if (!ok)
        {
            if (!memcmp((char *) memptr-8,"GFRE",4))
                abortmessage("gfree - ATTEMPT TO FREE MEMORY TWICE\n");
            else
                abortmessage("gfree - SENTINAL CORRUPTED; PROBABLE MEMORY DATA CORRUPTION\n");
        }
        else
        {
            memcpy((char *) memptr - 8,"GFRE",4);            /* mark as freed */
#ifdef _MSDEBUG
            /* make data garbage to bring out errors */
            memset(memptr, 'x', size);
#endif
            purgememadr((char *) memptr - 8);

            --galloccount;
        }
    }

    return(ok);
}
#endif

/* get motorola memory */

unsigned long ggetm(const void *src, int bytes)
{
    unsigned char *s = (unsigned char *) src;
    unsigned long value;

    value = 0L;
    while (bytes--)
    {
        value = (value<<8) + ((*s++));
    }
    return(value);
}

/* get intel memory */

unsigned long ggeti(const void *src, int bytes)
{
    unsigned char *s = (unsigned char *) src;
    int            i = 0;
    unsigned long  value;

    value = 0L;
    while (bytes--)
    {
        value += ((*s++)) << (i);
        i += 8;
    }
    return(value);
}

#ifdef gputm
#undef gputm
#undef gputi
#endif

/* put motorolla memory */

void gputm(void *dst, unsigned long data, int bytes)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned long       pval;

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

void gputi(void *dst, unsigned long data, int bytes)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned long   pval;

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
    FILE *handle;
    long len = 0;
    STDSTREAM *h;

    if (!strncmp(filename,"con:",4))
        handle = stdin;
    else
    {
/*        handle = fopen(filename,"r+b"); */
/*        if (!handle) */
            handle = fopen(filename,"rb");
    }

    h = 0;
    if (handle)
    {
        fseek(handle,0,SEEK_END);
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
            h->write  = 0;
            h->dirty  = 0;
            h->cachestart = 0;
        }
    }
    return((GSTREAM *) h);
}

GSTREAM * GCALL gwopen(const char *filename)
{
    FILE *handle;
    long len;
    STDSTREAM *h;    

    if (isReadOnly(filename))
    {
        printf("gwopen - ATTEMPT TO OPEN READONLY FILE FOR WRITING (%s)\n", filename);
        return NULL;
    }
    if (isDirectory(filename))
    {
        printf("gwopen - ATTEMPT TO OPEN DIRECTORY FOR WRITING (%s)\n", filename);
        return NULL;
    }

    if (!strncmp(filename,"con:",4))
        handle = stdout;
    else
    {
        handle = fopen(filename,"w+b");
        if (!handle)
            handle = fopen(filename,"wb");
    }

    len = 0;
    h = 0;
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
            h->write  = 1;
            h->dirty  = 0;
            h->cachestart = 0;
        }
    }        
    return((GSTREAM *) h);
}


int GCALL gclose(GSTREAM *g)
{
    STDSTREAM *h=(STDSTREAM *) g;
    int ok=1;

    if (g == NULL)
        return 0;

    /* Write out the cache */
    gwriteflush(h);

    /* Close the file and free the memory */
    if (g)
    {
        ok = !fclose(h->handle);
        ok &= gfree((char *) g);
    }

    return(ok);
}


int GCALL gread(GSTREAM *g, void *buf, long size)
{
    STDSTREAM *h=((STDSTREAM *) g);
    long len;
    long bytes=0, bytes2=0;

    if (g == NULL)
        return 0;

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
    size = gmin(size,h->len-h->offset);

    if (size < 0)
    {
        printf("gread - ATTEMPT TO READ PAST END OF FILE\n");
        return bytes;
    }

    /* If all that can be read from the cache has been read and there is more to read */
    if (!(h->bufend-h->bufptr) && size)
    {
        long blockstart = ((long)((h->offset + size) / GIMEXBUFSIZE)) * GIMEXBUFSIZE;

        gwriteflush(h);

        /* If the last block of the read is not from the same block as the offset */
        /* then read directly to the buffer */
        if (h->offset < blockstart)
        {
            bytes2 = (long)fread((void *) buf, (size_t) 1, (size_t) (blockstart - h->offset), h->handle);
            buf = (void *) ((char *)buf + bytes2);
            size -= bytes2;
        }
        

        /* Read remaining data into the cache         */
        h->cachestart = blockstart;
        len = gmin(GIMEXBUFSIZE, h->len - blockstart);
        fread((void *) h->buf, (size_t) 1, (size_t) len, h->handle);
        memcpy(buf, h->bufptr, (size_t)size);
        bytes2 += size;
        h->bufptr += size;
        h->bufend += len;
        h->offset += bytes2;
    }
    
    return(bytes+bytes2);
}


int GCALL gwrite(GSTREAM *g, const void *buf, long size)
{
    STDSTREAM *h=((STDSTREAM *) g);
    long len;
    long bytes=0;

    if (g == NULL)
        return 0;

    /* Check that file is not read-only */
    if (!h->write)
    {
        printf("gwrite - ATTEMPT TO WRITE TO A READ ONLY FILE\n");
        return 0;
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
        h->dirty = 1;
        buf = (void *) ((char *)buf + len);
        size -= len;
        bytes += len;
    }
    
    /* If the cache is full and there is more to write */
    if (!(h->bufend-h->bufptr) && size)
    {
        long blockstart = ((long)((h->offset + size) / GIMEXBUFSIZE)) * GIMEXBUFSIZE;

        gwriteflush(h);

        /* Read in the next block of cache if possible (for writes into the middle of a file)         */
        if (h->len > blockstart)
        {
            len = gmin(GIMEXBUFSIZE, h->len - blockstart);
            fseek(h->handle, blockstart, SEEK_SET);
            fread((void *) h->buf, (size_t) 1, (size_t) len, h->handle);
            h->bufend += len;
        }        
        
        /* Write out to the start of the next block to be cached */
        fseek(h->handle, h->offset, SEEK_SET);
        len = (long)fwrite((void *) buf, (size_t) 1, (size_t) (blockstart - h->offset), h->handle);
        if (len < (blockstart - h->offset))
        {
            printf("gwrite - OUT OF DISK SPACE\n");
            return bytes + len;
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
        h->dirty = 1;
    }

    return(bytes);
}


int GCALL gseek(GSTREAM *g, long offset)
{
    STDSTREAM *h=((STDSTREAM *) g);
    long relative;
    int ok = 1;
    int len;

    if (g == NULL)
        return 0;

    /* clip bad seeks  */
    if (offset < 0)
    {
        offset = 0;
        ok = 0;
    }
    else if (offset > h->len && !h->write)
    {
        offset = h->len;
        ok = 0;
    }

    relative = offset-h->offset;
    h->offset += relative;

    /* If the relative move is outside of the cache then  ( >= for right half??) */
    if ((relative < h->buf-h->bufptr) || (relative > h->bufend-h->bufptr))
    {
        long blockstart = ((long)(h->offset / GIMEXBUFSIZE)) * GIMEXBUFSIZE;

        gwriteflush(h);
        
        ok = !fseek(h->handle, blockstart, SEEK_SET) && ok;
        h->cachestart = blockstart;
        
        /* If not seeked past end of file read in the next block of cache and position bufptr */
        if (h->len > blockstart)
        {
            len = gmin(GIMEXBUFSIZE, h->len - blockstart);
            fread((void *) h->buf, (size_t) 1, (size_t) len, h->handle);
            h->bufend += len;
            h->bufptr += (h->offset - blockstart);
        }
        /* Otherwise set cache to 00 up to seek point */
        else
        {
            len = h->offset - blockstart;
            memset((void *) h->buf, (unsigned char)0x00, (size_t)len);
            h->bufptr = h->bufend = h->buf + len;
        }
    }
    else
        h->bufptr += relative;

    return(ok);
}


static int gwriteflush(STDSTREAM *h)
{
    long bytes=0;
    
    if (h->dirty && h->write)
    {
        fseek(h->handle, h->cachestart, SEEK_SET);
        bytes = fwrite((void *)h->buf, (size_t) 1, (size_t) (h->bufend - h->buf), h->handle);
        if (bytes < (h->bufend - h->buf))
            printf("gwriteflush - OUT OF DISK SPACE\n");
        h->dirty = 0;
    }
    h->bufptr = h->bufend = h->buf;
    fflush(h->handle);

    return bytes;
}


long GCALL glen(GSTREAM *g)
{
    return(((STDSTREAM *) g)->len);
}

long GCALL gtell(GSTREAM *g)
{
    return(((STDSTREAM *) g)->offset);
}


