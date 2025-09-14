// (c) 1994-2007 Electronic Arts Inc.

#ifndef STDGIMEX_H
#define STDGIMEX_H

#define STDGIMEX_VERSION 153

#if !defined(GCALL)
#if defined(_MSC_VER) && !defined(_XBOX)
#define GCALL __stdcall
#else
#define GCALL
#endif
#endif

#ifndef __cdecl
#define __cdecl
#endif

#if !defined(GPOS)
#if defined(_MSC_VER)
typedef __int64 GPOS;
#elif defined(__R5900)
typedef long GPOS;
#else
typedef long long GPOS;
#endif
#endif

#include <stdlib.h>     // for _byteswap_ulong

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#if defined(__SPU__) && defined(__ELF__)
// linux spu
#include <inttypes.h>
#include <unistd.h>
#endif

#ifndef __SPU__
#include <sys/stat.h>
#ifdef __GNUC__
#include <unistd.h>
#endif
#endif
#if defined(_M_PPC)
// for __loadshortbytereverse
#include <PPCIntrinsics.h>
#endif

#if defined(_MSC_VER)
#pragma warning(disable: 4996)  // warning C4996: 'fscanf' was declared deprecated
#endif
#if defined(__ICL)
#pragma warning(disable: 1478)  // (icl) warning #1478: function "strcpy" was declared "deprecated"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct GSTREAM;

extern int galloccount;
extern int __cdecl (*gprintf)(const char *format, ...);
extern FILE *gstdout;

/****************************************************************************/
/* Application Module Prototypes                                            */
/****************************************************************************/

/* File Stream Functions */

GSTREAM * GCALL gopen(const char *pathname);
GSTREAM * GCALL gwopen(const char *pathname);
GSTREAM * GCALL grwopen(const char *pathname);
bool      GCALL gclose(GSTREAM *g);
int       GCALL gread(GSTREAM *g, void *buf, int size);
int       GCALL gwrite(GSTREAM *g, const void *buf, int size);
bool      GCALL gseek(GSTREAM *g, GPOS offset);
GPOS      GCALL glen(GSTREAM *g);
GPOS      GCALL gtell(GSTREAM *g);

/* Memory Functions */

#if !defined(galloc)
void *  GCALL galloc(int size);
#endif
bool   GCALL gfree(void *memptr);

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif


#ifdef __cplusplus
}
#endif

/****************************************************************************/
/* Inline C++ (cross platform)                                              */
/****************************************************************************/

#ifdef _MSC_VER
#pragma warning(disable: 4514) // unreferenced inline function has been removed
#pragma warning(disable: 4996) // warning C4996: 'fscanf' was declared deprecated
#endif

#ifndef GMAKEID
#define GMAKEID(a,b,c,d) (((static_cast<int>(a))<<24)|((static_cast<int>(b))<<16)|((static_cast<int>(c))<<8)|static_cast<int>(d))
#endif

/* get motorola memory */

#if !defined(__GGETM_DEFINED) && !defined(__GIMEX_H)
#define __GGETM_DEFINED

// gimex.h also defines these
static __inline unsigned int ggetm(const void *src, int bytes)
{
    if (bytes==1)
        return static_cast<unsigned int>(*reinterpret_cast<const unsigned char *>(src));
#if defined(__APPLE__) || defined(_M_PPC) || defined(__PPC__) || defined(__SPU__) || defined(__PPCGEKKO__)
    else if (bytes==2)
        return static_cast<unsigned int>(*reinterpret_cast<const unsigned short *>(src));
    else if (bytes==4)
        return static_cast<unsigned int>(*reinterpret_cast<const unsigned int *>(src));
#elif defined(_MSC_VER) && _MSC_VER>=1300 && !defined(_M_PPC)
    else if (bytes==2)
            return _byteswap_ushort( *reinterpret_cast<const unsigned short *>(src) );
    else if (bytes==4)
            return _byteswap_ulong( *reinterpret_cast<const unsigned int *>(src) );
#else
    else if (bytes==2)
        return (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[0])<<8)
               | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[1]));
    else if (bytes==4)
        return (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[0])<<24)
                 | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[1])<<16)
                 | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[2])<<8)
                 | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[3]));
#endif
    else if (bytes==3)
        return (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[0])<<16)
                 | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[1])<<8)
                 | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[2]));
    return 0;
}

/* get intel memory */

#if defined(__PPC__)
/* gamecube/ps3 */
static __inline unsigned int lhbrx(const void *src) {  unsigned int result;  __asm__ volatile("lhbrx %0, 0, %1" : "=r" (result) : "r" (src));  return result;}
static __inline unsigned int lwbrx(const void *src)   {  unsigned int result;  __asm__ volatile("lwbrx %0, 0, %1" : "=r" (result) : "r" (src));  return result;}
#elif defined(_M_PPC)
/* xenon */
static __inline unsigned int lhbrx(const void *src)   {  return __loadshortbytereverse(0,src); }
static __inline unsigned int lwbrx(const void *src)   {  return __loadwordbytereverse(0,src); }
#endif

static __inline unsigned int ggeti(const void *src, int bytes)
{
    if (bytes==1)
        return static_cast<unsigned int>(*reinterpret_cast<const unsigned char *>(src));
#if defined(_M_PPC) || defined(__PPC__)
    else if (bytes==2)
        return lhbrx(src);
    else if (bytes==4)
        return lwbrx(src);
#elif (defined(_MSC_VER) && !defined(_M_PPC)) || defined(__i386__)
    else if (bytes==2)
        return static_cast<unsigned int>(*reinterpret_cast<const unsigned short *>(src));
    else if (bytes==4)
        return static_cast<unsigned int>(*reinterpret_cast<const unsigned int *>(src));
#else
    else if (bytes==2)
        return (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[1])<<8)
                     | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[0]));
    else if (bytes==4)
        return (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[3])<<24)
                             | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[2])<<16)
                             | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[1])<<8)
                             | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[0]));
#endif
    else if (bytes==3)
        return (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[2])<<16)
                             | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[1])<<8)
                             | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[0]));
    return 0;
}


/* put motorolla memory */

static __inline void gputm(void *dst, unsigned int data, int bytes)
{
    if (bytes==1)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
    }
#if defined(__APPLE__) || (defined(__MWERKS__) && defined(__PPCGEKKO__))
    else if (bytes==2)
    {
        (reinterpret_cast<unsigned short *>(dst))[0] = static_cast<unsigned short>(data);
    }
    else if (bytes==4)
    {
        (reinterpret_cast<unsigned int *>(dst))[0] = static_cast<unsigned int>(data);
    }
#else
    else if (bytes==2)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data);
    }
    else if (bytes==4)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>24);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>16);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[3] = static_cast<unsigned char>(data);
    }
#endif
    else if (bytes==3)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>16);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data);
    }
}


/* put intel memory */

static __inline void gputi(void *dst, unsigned int data, int bytes)
{
    if (bytes==1)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
    }
#if (defined(_MSC_VER)  && !defined(_M_PPC)) || defined(__i386__)
    else if (bytes==2)
    {
        (reinterpret_cast<unsigned short *>(dst))[0] = static_cast<unsigned short>(data);
    }
    else if (bytes==4)
    {
        (reinterpret_cast<unsigned int *>(dst))[0] = static_cast<unsigned int>(data);
    }
#else
    else if (bytes==2)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>8);
    }
    else if (bytes==4)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data>>16);
        (reinterpret_cast<unsigned char *>(dst))[3] = static_cast<unsigned char>(data>>24);
    }
#endif
    else if (bytes==3)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data>>16);
    }
}
#endif

#endif // STDGIMEX_H

