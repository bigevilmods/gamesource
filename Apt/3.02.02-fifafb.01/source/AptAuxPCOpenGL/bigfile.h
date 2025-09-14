/* Copyright (C) Electronic Arts Canada Inc. 1996-2002.  All rights reserved. */

#ifndef __EACBIG_H
#define __EACBIG_H 1

/* Default header size is allocated by setbig. More will be allocated
by loadbigheader if needed. Default header is large enough for
a lumpy header plus file header for 128 files with 12 character
names. */

#define BIG_DEFHEADERSIZE   (sizeof(BIGHEADER) + 128*(sizeof(BIGENTRY32)+12))

/* Default file extension for big files. */

#define BIG_DEFEXT          ".viv"

/* Old-style bigfiles used 24-bit offsets for files within the bigfile
(and various other changes to the header). New style uses 32-bit
offsets. This module detects and handles both. */

#define BIG24 1
#define BIG32 2
#define BIGN  3

/* Big file ID's for new and old formats. */
#define BIGNID  MAKEID('B','I','G','0')
#define BIG32ID MAKEID('B','I','G','F')
#define BIG24ID 0xc0fb

/* Old and new big file formats. Big file header is followed by a
table of contents entry for each file in the big file.

Note that all fields are stored in Motorola format, and
so should be retrieved with getm. */

typedef struct
{
    char    offset[2];      /* File offset relative to start of big file. */
    char    length[2];      /* Length of file. */
    char    name[1];        /* Null-terminated name of file (variable length). */
}
BIGENTRY16;

typedef struct
{
    char    offset[3];      /* File offset relative to start of big file. */
    char    length[3];      /* Length of file. */
    char    name[1];        /* Null-terminated name of file (variable length). */
}
BIGENTRY24;

typedef struct
{
    char    offset[4];      /* File offset relative to start of big file. */
    char    length[4];      /* Length of file. */
    char    name[1];        /* Null-terminated name of file (variable length). */
}
BIGENTRY32;

typedef struct
{
    char    offset[5];      /* File offset relative to start of big file. */
    char    length[4];      /* Length of file. */
    char    name[1];        /* Null-terminated name of file (variable length). */
}
BIGENTRY40;

/* 16/n24/n32/40-bit big file format. */

typedef struct
{
    char    type[4];        /* 'BIG4'/'BIGF' for big file. */
    char    len[4];         /* Length of file in bytes. */
    char    num[4];         /* Number of files in big file. */
    char    hlen[4];        /* Header length (including this structure). */
}
BIGHEADER;

/* 24-bit big file format. */
typedef struct
{
    char    type[2];        /* 0xc0fb for 24-bit big file. */
    char    hlen[2];        /* Header length (excluding this structure). */
    char    num[2];         /* Number of files in big file. */
}
BIGHEADER24;

/* External variables */

extern char currentbigfile[];       /* Name of current big file. */
extern char defaultbigext[];        /* Default extension. */
extern char *bigbufptr;             /* Header used by loadbig.c. */

void *BIG_locate(const void *bigfileptr, const char *name);
void *BIG_locatesize(const void *bigfileptr, const char *name, int *nSize);

#endif  /* __EACBIG_H */

