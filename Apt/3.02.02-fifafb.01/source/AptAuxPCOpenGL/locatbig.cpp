// Copyright (C) Electronic Arts Canada Inc. 1988-2002.  All rights reserved.

#include <string.h>
#include <ctype.h>

#ifndef MAKEID
#define MAKEID(a,b,c,d) (((int)(a)<<24)|((int)(b)<<16)|((int)(c)<<8)|(int)(d))
#endif

#include "bigfile.h"

/* get motorola memory */
static __inline unsigned int getm(const void *src, int bytes)
{
    if (bytes==1)
        return (unsigned int) *(const unsigned char *) src;
    else if (bytes==2)
        return (((unsigned int) *(const unsigned char *) src)<<8) | (((unsigned int) *((const unsigned char *) src+1)));
    else if (bytes==3)
        return (((unsigned int) *(const unsigned char *) src)<<16) | (((unsigned int) *((const unsigned char *) src+1))<<8) | (((unsigned int) *((const unsigned char *) src+2)));
    else if (bytes==4)
        return (((unsigned int) *(const unsigned char *) src)<<24) | (((unsigned int) *((const unsigned char *) src+1))<<16) | (((unsigned int) *((const unsigned char *) src+2))<<8) | (((unsigned int) *((const unsigned char *) src+3)));
    return 0;
}



/* case independent compare n characters */

static int BIG_stricmp(const char *d, const char *s)
{
    int v=0;

    v=tolower(*d)-tolower(*s);

    while (!v && *d)
    {
        ++s;
        ++d;
        v=tolower(*d)-tolower(*s);
    }
    return(v);
}

/* get motorola memory */
static __inline unsigned __int64 getm64(const void *src, int bytes)
{
    if (bytes<=4)
        return (unsigned __int64) getm(src,bytes);

    return (((unsigned __int64) getm(src,bytes-4)) << 32) | (unsigned __int64) getm((const char *)src+(bytes-4),4);
}

/*
;
; ABSTRACT
;
; BIG_typeofheader - which type of big file is this?
;
; SUMMARY
;
; #include "realfile.h"
;
; int BIG_typeofheader(const void *bigfileptr)
; const void *bigfileptr - pointer to bigfile in memory
;
;
; DESCRIPTION
;
; Tells if the bigfile has a 32-bit or 24-bit header.  If the file is not a
; bigfile and the REALabortflag is set, REAL_abortmessage will be called with
; an error message.
;
; RETURN VALUE
;
; If the file is not a recognised bigfile, 0 will be returned.
; If the file has a 24-bit header, 1 will be returned.
; If the file has a 32-bit header, 2 will be returned.
; If the file has a n-bit header, 3 will be returned.
;
; SEE ALSO
;
; BIG_sizeofheader, BIG_locateoffset, BIG_pointer, lumpy
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
;
*/

int BIG_typeofheader(const void *bigfileptr)
{
    int type = 0;

    /* Old-style if 2-byte type is 0xc0fb; new style if */
    /* 4-byte type is 'BIGF'; error otherwise. */
    if (getm(bigfileptr, 2) == BIG24ID)
    {
        type = BIG24;
    }
    else if (getm(bigfileptr, 4) == BIG32ID)
    {
        type = BIG32;
    }
    else if ((getm(bigfileptr, 4)&0xffffff00) == (BIGNID&0xffffff00))
    {
        type = BIGN;
    }

    return(type);
}

/*
;
; ABSTRACT
;
; BIG_sizeofheader - get the size of a big file header
;
; SUMMARY
;
; #include "realfile.h"
;
; int BIG_sizeofheader(const void *bigfileptr)
; const void *bigfileptr - pointer to bigfile in memory
;
; DESCRIPTION
;
; Allows you to retrieve the size of the bigfile header.  The size is contained
; in the first 16 bytes of the file.
;
; RETURN VALUE
;
; Size in bytes of the bigfile header.  If it is not a recognised big file type,
; 0 is returned.
;
;
; SEE ALSO
;
; BIG_typeofheader, BIG_locateoffset, BIG_pointer, lumpy
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
;
*/

int BIG_sizeofheader(const void *bigfileptr)
{
    int size = 0;

    switch (BIG_typeofheader(bigfileptr))
    {
        case BIG24:
            size = (int)getm((char *)bigfileptr+2, 2) + 4;  // First two fields weren't included
            break;                                          // in old-style size, so add 4.

        case BIG32:
        case BIGN:
            size = (int)getm((char *)bigfileptr+12, 4);     // 32 bit includes size of all fields.
            break;

        default:
            break;
    }

    return(size);
}

/*
;
; ABSTRACT
;
; BIG_version - get version number of big file format
;
;
; SUMMARY
;
; #include "realfile.h"
;
; int BIG_version(const void *bigfileptr);
; const void *bigfileptr;         Pointer to big file
;
;
; DESCRIPTION
;
; The BIG_version function gets the version number of lumpy that
; wrote the big file.  The number is returned as an integer. ie 218
; Venom and lumpy tools also returns this version.
;
; RETURN VALUE
;
; The version of the big file as an integer
;
;
; SEE ALSO
;
; venom, lumpy
;
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
;
*/

// returns number of debug info bytes (0 or 8)

// done as a macro incase we want to ifdef it
#define LUMPY_DEBUGINFO 8

#define BIG_isalpha(a) (((a)>='A' && (a)<='Z') || ((a)>='a' && (a)<='z'))
#define BIG_isdigit(a) ((a)>='0' && (a)<='9')

static int BIG_debuginfo(const void *bigfileptr, int *infovenom=NULL, unsigned int *infoflags=NULL)
{
    int infosize=0;
    const char *venomptr=(const char *)bigfileptr+BIG_sizeofheader(bigfileptr)-LUMPY_DEBUGINFO;
    if (BIG_isalpha(venomptr[0]) && BIG_isdigit(venomptr[1]) && BIG_isdigit(venomptr[2]) && BIG_isdigit(venomptr[3]))
    {
        infosize=LUMPY_DEBUGINFO;
        if (infovenom) *infovenom=(venomptr[1]-'0')*100 + (venomptr[2]-'0')*10 + (venomptr[3]-'0');
        if (infoflags) *infoflags=getm(venomptr+4,4);
    }
    return infosize;
}

int BIG_version(const void *bigfileptr)
{
    int infovenom=0;
    BIG_debuginfo(bigfileptr, &infovenom);
    return infovenom;
}

/*
;
; ABSTRACT
;
; BIG_locateentry - locate file in bigfile return size, offset, name
;@BIG_locateentryz
;
; SUMMARY
;
; #include "realfile.h"
;
; char *BIG_locateentry(bigfileptr, entryname, entrynum, entryoffset, entrysize)
; char *BIG_locateentryz(bigfileptr, entryname, entrynum, entryoffset, entrysize)
;   const void *bigfileptr    pointer to big file
;   char *name          name of file to locate (can be NULL)
;   int entrynum        index number of file to locate, used only if name is NULL
;   int *entryoffset   pointer to returned offset (can be NULL)
;   int *entrysize     pointer to returned size (can be NULL)
;
;
; DESCRIPTION
;
; The BIG_locateentry function searches a big file directory for the specified
; file and returns the offset and size of the file. If entryname is not NULL
; then a case-insensitive search for that file name is performed, otherwise
; the entrynum-th file in the big file is located. The returned offset is
; from the start of the bigfile.
;
; BIG_locateentry aborts if the entry was not found, BIG_locateentryz simply
; returns 0.
;
; RETURN VALUE
;
; A pointer to the name of the lump file is returned, or NULL is returned on
; failure to find the requested file.  The offset and size are returned via
; the pointers entryoffset and entrysize.
;
; SEE ALSO
;
; BIG_locateoffset, BIG_pointer, lumpy
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
;
*/

// NOTE entryoffset is not big enough for 64 bit

const char *BIG_locateentryz(const void *bigfileptr, const char *name, int entrynum,
                      int *entryoffset, int *entrysize)
{
    int i = 0;

    int infosize = BIG_debuginfo(bigfileptr);

    const unsigned char *bigentry = ((unsigned char *)bigfileptr + sizeof(BIGHEADER));
    int sizeofoffset = 4;
    int sizeoflen = 4;

    /* Locate end of header. */
    unsigned char *hend = (unsigned char *)bigfileptr + BIG_sizeofheader(bigfileptr) - infosize;

    switch (BIG_typeofheader(bigfileptr))
    {
        case BIGN:
            sizeofoffset = ((const char *)bigfileptr)[3]-'0'; // 'BIG2'..'BIG7'
            sizeoflen = sizeofoffset; if (sizeoflen>4) sizeoflen = 4;
            break;

        case BIG24: // legacy
            bigentry = ((unsigned char *)bigfileptr + sizeof(BIGHEADER24));
            sizeofoffset = 3;
            sizeoflen = 3;
            break;

        case BIG32: // defaults are ok
            break;
    }

    if (name)
    {
        while ((unsigned char *)bigentry < hend)
        {
            const char *bigentryname = (const char *)bigentry+sizeofoffset+sizeoflen;
            if (!BIG_stricmp(bigentryname, name))
            {
                if (entryoffset) *entryoffset = (int) getm64(bigentry,sizeofoffset);
                if (entrysize)   *entrysize =   (int) getm(bigentry+sizeofoffset,sizeoflen);
                return(bigentryname);
            }
            bigentry = (const unsigned char *) bigentryname + strlen(bigentryname) + 1;
            ++i;
        }
    }
    else // index search
    {
        while ((unsigned char *)bigentry < hend)
        {
            const char *bigentryname = (const char *)bigentry+sizeofoffset+sizeoflen;
            if (i == entrynum)
            {
                if (entryoffset) *entryoffset = (int) getm64(bigentry,sizeofoffset);
                if (entrysize)   *entrysize =   (int) getm(bigentry+sizeofoffset,sizeoflen);
                return(bigentryname);
            }
            bigentry = (const unsigned char *) bigentryname + strlen(bigentryname) + 1;
            ++i;
        }
    }

    if (entryoffset) *entryoffset = 0;
    if (entrysize)   *entrysize = 0;

    return(NULL);
}

const char *BIG_locateentry(const void *bigfileptr, const char *name, int entrynum, int *entryoffset, int *entrysize)
{
    const char *result = BIG_locateentryz(bigfileptr,name,entrynum,entryoffset,entrysize);

    return(result);
}

/*
;
; ABSTRACT
;
; BIG_locateoffset - locate file in bigfile and return offset
;@BIG_locateoffsetz
;
;
; SUMMARY
;
; #include "realfile.h"
;
; int BIG_locateoffset(bigfileptr, name)
;   const void *bigfileptr    pointer to big file
;   char *name          name of file to locate
;
;
; DESCRIPTION
;
; Returns the offset to a file in a bigfile by calling:
;
;    BIG_locateentry(bigfileptr, name, 0, &retval, NULL);
;
; RETURN VALUE
;
; Offset to the requested bigfile.  0 if not found
;
; SEE ALSO
;
; BIG_locateentry, BIG_pointer, lumpy
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
*/

int BIG_locateoffset(const void *bigfileptr, const char *name)
{
    int bigoffset=0;

    if (name)
        BIG_locateentry(bigfileptr, name, 0, &bigoffset, NULL);

    return(bigoffset);
}

int BIG_locateoffsetz(const void *bigfileptr, const char *name)
{
    int bigoffset=0;

    if (name)
        BIG_locateentryz(bigfileptr, name, 0, &bigoffset, NULL);

    return(bigoffset);
}


/*
;
; ABSTRACT
;
; BIG_pointer - locate big and return pointer from a number
;@BIG_pointerz
;
; SUMMARY
;
; #include "realfile.h"
;
; char *BIG_pointer(bigfile, number)
; char *BIG_pointerz(bigfile, number)
;   const void *bigfileptr    pointer to big file
;   int number          file number (0-based index)
;
;
; DESCRIPTION
;
; Returns a pointer to a file in a bigfile by calling
;
; BIG_locateentry(bigfileptr, NULL, number, &offset, NULL);
;
; and adding the offset to the bigfileptr.
;
; RETURN VALUE
;
; A pointer to a file is returned if the file is found in the bigfile.
; If the file is not found, the system is aborted and an error message is
; displayed.
;
; SEE ALSO
;
; BIG_locateoffset, BIG_locateentry, BIG_locate, lumpy
;
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
;
*/

void *BIG_pointer(const void *bigfileptr, int number)
{
    int bigoffset;

    if (BIG_locateentry(bigfileptr, NULL, number, &bigoffset, NULL))
    {
        return((char *)bigfileptr+bigoffset);
    }

    return(NULL);
}

void *BIG_pointerz(const void *bigfileptr, int number)
{
    int bigoffset;

    if (BIG_locateentryz(bigfileptr, NULL, number, &bigoffset, NULL))
    {
        return((char *)bigfileptr+bigoffset);
    }

    return(NULL);
}


/*
;
; ABSTRACT
;
; BIG_locate - locate big and return pointer from a name
;@BIG_locatez
;
; SUMMARY
;
; #include "realfile.h"
;
; char *BIG_locate(bigfileptr, name)
; char *BIG_locatez(bigfileptr, name)
;   const void *bigfileptr  pointer to big file
;   char *name              name of file to locate
;
;
; DESCRIPTION
;
; Returns a pointer to the requested file by calling BIG_locateoffset
; and adding the return value to the bigfileptr.
;
; RETURN VALUE
;
; A pointer to a file is returned if the file is found in the bigfile.
; If the file is not found, the system is aborted and an error message is
; displayed.
;
; SEE ALSO
;
; BIG_locateoffset, BIG_locateentry, lumpy
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
;
*/

void *BIG_locate(const void *bigfileptr, const char *name)
{
    char *p=0;
    int bigoffset;

    bigoffset = BIG_locateoffset(bigfileptr, name);
    if (bigoffset)
    {
        p = (char *)bigfileptr + bigoffset;
    }

    return(p);
}

void *BIG_locatez(const void *bigfileptr, const char *name)
{
    char *p=0;
    int bigoffset;

    bigoffset = BIG_locateoffsetz(bigfileptr, name);
    if (bigoffset)
    {
        p = (char *)bigfileptr + bigoffset;
    }

    return(p);
}

void *BIG_locatesize(const void *bigfileptr, const char *name, int *size)
{
    char *p=0;
    int bigoffset = 0;

    BIG_locateentryz(bigfileptr, name, 0, &bigoffset, size);
    if (bigoffset)
    {
        p = (char *)bigfileptr + bigoffset;
    }

    return(p);
}


/*
;
; ABSTRACT
;
; BIG_locatemany - locate bigs by returning an array of pointers
;
; SUMMARY
;
; #include "realfile.h"
;
; void BIG_locatemany(bigfileptr, names, bigptrs)
;   const void *bigfileptr    pointer to big file
;   char *names         comma-separated names of files to get offsets to
;   void *bigptrs[]     array of pointers
;
; DESCRIPTION
;
; The BIG_locatemany function uses BIG_locate to get an offset to each file.
; The offsets are put in the bigptrs array.
;
; If one of the files cannot be found and the REALabortflag is set, an
; REAL_abortmessage is issued.
;
; RETURN VALUE
;
; none
;
; SEE ALSO
;
; BIG_locate, BIG_locateoffset, BIG_locateentry, lumpy
;
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
;
*/

void BIG_locatemany(const void *bigfileptr, const char *names, void *bigptrs[])
{
    char *p = NULL;
    void *u;

    do
    {
        p = const_cast<char*>( strchr(names,',') );
        if (p)
        {
            *p = '\0';
        }
        u = BIG_locate(bigfileptr, names);
        *(bigptrs++) = u;
        if (p)
        {
            *p++ = ',';
            names = p;
        }
    } while (p);
}

/*
;
; ABSTRACT
;
; BIG_count - get number of files in bigfile
;
; SUMMARY
;
; #include "realfile.h"
;
; int BIG_count(bigfile)
;   const void *bigfileptr    pointer to big file
;
;
; DESCRIPTION
;
; Returns the number of files contained in a big file.  If the pointer
; does not point to a bigfile, the system aborts.
;
; RETURN VALUE
;
; Number of files lumped together in the bigfile.
;
; SEE ALSO
;
; lumpy
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
;
*/

int BIG_count(const void *bigfileptr)
{
    int count=0;

    switch (BIG_typeofheader(bigfileptr))
    {
        case BIG24:
            count = (int)getm(((BIGHEADER24 *)bigfileptr)->num, 2);
            break;

        case BIG32:
        case BIGN:
            count = (int)getm(((BIGHEADER *)bigfileptr)->num, 4);
            break;

        default:
            break;
    }

    return(count);
}

/*
;
; ABSTRACT
;
; BIG_name - get name of nth file in bigfile
;@BIG_namez
;
; SUMMARY
;
; #include "realfile.h"
;
; char *BIG_name(bigfileptr, number, BIG_name)
; char *BIG_namez(bigfileptr, number, BIG_name)
;   const void *bigfileptr    pointer to big file
;   int number          file number (0-based index)
;   char *BIG_name;      string buffer
;
;
; DESCRIPTION
;
; Uses BIG_locateentry to find the name from the index.
;
; RETURN VALUE
;
; pointer to the supplied string buffer.
;
; SEE ALSO
;
; BIG_locateentry, lumpy
;
; EXAMPLE
;
; bigfile_eg.cpp
;
; END ABSTRACT
;
*/

char *BIG_name(const void *bigfileptr, int number, char *name)
{
    const char *filenameptr;

    filenameptr = BIG_locateentry(bigfileptr, NULL, number, NULL, NULL);

    if (filenameptr)
    {
        strcpy(name, filenameptr);
        return(name);
    }
    else
    {
        name[0] = '\0';
        return(NULL);
    }
}

char *BIG_namez(const void *bigfileptr, int number, char *name)
{
    const char *filenameptr;

    filenameptr = BIG_locateentryz(bigfileptr, NULL, number, NULL, NULL);

    if (filenameptr)
    {
        strcpy(name, filenameptr);
        return(name);
    }
    else
    {
        name[0] = '\0';
        return(NULL);
    }
}

