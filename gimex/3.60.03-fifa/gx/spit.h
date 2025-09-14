// (c) 1990-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*            Small Programs' Interface Tool (SPIT) v3.96           */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                           Nov 16, 2006                           */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef SPIT_H
#define SPIT_H

#if defined(_MSC_VER)
#pragma once
#endif

#define SPIT_VERSION 396

#include "stdgimex.h"

/****************************************************************************/
/* Structures                                                               */
/****************************************************************************/

typedef struct SPITMEM
{
    char *ptr;
    int  len;
} SPITMEM;

#if !defined(__cplusplus) && !defined(bool)
#define bool bool
typedef enum { false=0x0, true=0x1 } bool;
#endif

/****************************************************************************/
/* Microsoft Visual C defines                                               */
/****************************************************************************/

#if defined(_MSC_VER)

#pragma warning(push,1)
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#pragma warning(pop)
#define SPIT_MACHINE   "PC"
#define SPITDIRSYM    '\\'
#define SPITNL        "\015\012"
#define SPITMAXOPTBUF      32*1024*1024
#ifndef INT64
#define INT64 INT64
typedef __int64 INT64;
#endif
#ifndef UINT64
#define UINT64 UINT64
typedef unsigned __int64 UINT64;
#endif
#define FORMAT64 "I64"
#define CONST64(v) v##I64
#pragma warning(disable: 4514)  // warning C4514: unreferenced inline function has been removed
#pragma warning(disable: 4996)  // warning C4996: 'fscanf' was declared deprecated
// VC6 header bug fix
struct _RPC_ASYNC_STATE;
#define _WIN32 1

/****************************************************************************/
/* PS2 defined(__R5900)                                                     */
/****************************************************************************/

#elif defined(__R5900)
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#define SPIT_MACHINE       "PS2"
#define SPITDIRSYM        '/'
#define SPITNL            "\012"
#define SPITMAXOPTBUF      1*1024*1024
#if !defined(INT64)
typedef long INT64;
#define INT64 INT64
#endif
#if !defined(UINT64)
typedef unsigned long UINT64;
#define UINT64 UINT64
#endif
#define FORMAT64 "l"
#define CONST64(v) v##l

/****************************************************************************/
/* Linux 6.2/7.2 defines __i386__                                      */
/* SGI (unix)  __mips                                                       */
/* Mac OSX __APPLE__                                                        */
/****************************************************************************/

#elif defined(__unix__) || defined(__PPU__) || defined(__APPLE__) || defined(__BEOS__)
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#define strlwr(__s)   { int __i; for (__i=0; (__s)[__i]; ++__i) (__s)[__i]=(char)tolower((__s)[__i]); }
#define strupr(__s)   { int __i; for (__i=0; (__s)[__i]; ++__i) (__s)[__i]=(char)toupper((__s)[__i]); }
#if defined(__BEOS__)
#define SPIT_MACHINE       "BeOS"
#elif defined(__APPLE__)
#define SPIT_MACHINE       "OSX"
#elif defined(__i386__)
#define SPIT_MACHINE       "Linux"
#elif defined(__R5900)
#define SPIT_MACHINE       "PS2"
#elif defined(__PPU__)
#define SPIT_MACHINE       "PS3"
#elif defined(__mips)
#define SPIT_MACHINE       "Irix"
#elif defined(sparc)
#define SPIT_MACHINE       "Sun"
#else
#define SPIT_MACHINE       "Unix"
#endif
#define SPITDIRSYM        '/'
#define SPITNL            "\012"
#define SPITMAXOPTBUF      4*1024*1024
#ifndef INT64
typedef long long INT64;
#define INT64 INT64
#endif
#ifndef UINT64
typedef unsigned long long UINT64;
#define UINT64 UINT64
#endif

/****************************************************************************/
/* ANSI defines                                                             */
/****************************************************************************/

#else

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#define strlwr(__s)   { int __i; for (__i=0; (__s)[__i]; ++__i) (__s)[__i]=(char)tolower((__s)[__i]); }
#define strupr(__s)   { int __i; for (__i=0; (__s)[__i]; ++__i) (__s)[__i]=(char)toupper((__s)[__i]); }
#define SPIT_MACHINE       "ANSI"
#define SPITDIRSYM        '/'
#define SPITNL            "\n"
#define SPITMAXOPTBUF      256*1024
#ifndef INT64
typedef long long INT64;
#define INT64 INT64
#endif
#ifndef UINT64
typedef unsigned long long UINT64;
#define UINT64 UINT64
#endif

#endif

/****************************************************************************/
/* Common defines                                                           */
/****************************************************************************/

#define SPITYES        -1
#define SPITNO         0
#define SPITMAXLINE    16384
#define SPITOPTIONHELP 4
#define SPITMAXFNAME   512
#ifndef max
#define max(a,b)    (((a)>(b))?(a):(b))
#endif
#ifndef min
#define min(a,b)    (((a)<(b))?(a):(b))
#endif
#ifndef FORMAT64
#define FORMAT64 "ll"
#endif
#ifndef CONST64
#define CONST64(v) v##LL
#endif

/****************************************************************************/
/* Spit Options                                                             */
/****************************************************************************/

extern int  OPTdirectory;

extern int  OPTsorttype;
extern int  OPTstartsortat;
extern int  OPTskiprules;
extern int  OPTrecurse;
extern int  OPTstartrecurseat;
extern int  OPTautoquote;

extern int  OPTverbose;
extern int  OPTwrite;
extern int  OPTbanner;
extern int  OPTminargs;
extern int  OPTdestdir;
extern int  OPTkeepopts;
extern int  OPTlog;
extern int  OPTtime;
extern char *OPTextrule;
extern char *OPTpathrule;
extern char *OPTurl;
extern char *SPITmachine;

extern int   SPITargnum;
extern char **SPITsargv;
extern char **SPITdargv;
extern int SPIThelpgiven;
extern int SPITerrorcount;

extern char OPTdot;

/****************************************************************************/
/* Public Functions                                                         */
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Option Parsing */

bool    SPIT_parse(int argc, char * const *argv, const char *ToolName, int ToolVersion);
char *  SPIT_getopt2(int *optvalptr,const char *optptr,const char *optname, int optmin, int optmax, int optdef);
char *  SPIT_getopt(int *optvalptr,const char *optptr,const char *optname, int optmin, int optmax);
char *  SPIT_getstropt(char *optvalptr,const char *optptr,const char *optname, const char *separator, int doit, int optmax);
void    SPIT_restore(void);
char *  SPIT_getfield(char *dest,const char *optptr, bool strip, bool list);
char *  SPIT_nextfield(const char *optptr);
char *  Zop(const char *opt);

/* File/Memory Support */

char *  SPIT_loadfilev(SPITMEM *mem, const char *fname, int verbose);
char *  SPIT_loadfile(SPITMEM *mem, const char *fname);
bool    SPIT_savefile(SPITMEM *mem, const char *fname);
char *  SPIT_alloc(SPITMEM *mem, int len);
void    SPIT_free(SPITMEM *mem);

/* Miscellaneous */

double  SPIT_gettime(void);
char *  SPIT_strstr(const char *str, const char *substr);
void    SPIT_flush(void);
#ifdef __cplusplus
char *  SPIT_tempname(char *d,const char *s, const char *ext=NULL);
#else
char *  SPIT_tempname(char *d,const char *s, const char *ext);
#endif
char *  SPIT_changeext(char *d,const char *s, char *ext);
char *  SPIT_changepath(char *d,const char *s, char *rule);
char *  SPIT_getpath(char *d,const char *s);
char *  SPIT_getname(char *d,const char *s);
char *  SPIT_percentstr(char *buf, int olen, int nlen);
char *  SPIT_loadstr(char *buf, const char *fname, int verbose);
bool    SPIT_openurl(const char *url);
char *  SPIT_geturl(char *url, const char *prgnamebuf);
int     SPIT_getnum(const char *s);
bool    SPIT_isdigit(char *operand);
bool    SPIT_iswildcards(const char *sourceptr);
char *  SPIT_savestr(char *buf, const char *fname, int verbose);
char *  SPIT_convertname( char *d,const char *s);

/* Sorting Function Prototype */
void    SPIT_qsort(int p, int r, int sorttype);

/* File type identification */
bool SPIT_isdirectory(unsigned char attrib);
bool SPIT_isreadonly(unsigned char attrib);
bool SPIT_issystem(unsigned char attrib);
bool SPIT_ishidden(unsigned char attrib);

int SPIT_filedate(const char *fname);

void SPIT_init(void(*initfunc)(void),
          void(*usagefunc)(const char *prgname,
          const char *envname,const char *urlname,int utype),
          char *(*optfunc)(const char *optptr),
          const char *spithistory[]);
bool SPIT_haswild(char *s);

#if !defined(MSDOS) && !defined(_WIN32)
int strnicmp(const char *d, const char *s, size_t n);
int stricmp(const char *d, const char *s);
#endif

#ifdef __cplusplus
}
#endif

/****************************************************************************/
/* User Defined Functions                                                   */
/****************************************************************************/

#endif // SPIT_H


