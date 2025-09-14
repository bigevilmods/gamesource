// (c) 2002-2007 Electronic Arts Inc.

/*!
 * \file        realgraph.h
 * \brief       The RealGraph header.
 *
 * \internal
 * Version      : $Version$
 * File         : $Header$
 * Platform     : Common
 * Maintained by:
 * Last Modified: $Author$ on $DateTime$
 */

#ifndef REALGRAPH_H
#define REALGRAPH_H

#if defined(_MSC_VER)
#pragma once
#endif

#if !defined(REALGRAPH_DEBUGLEVEL)
#define REALGRAPH_DEBUGLEVEL 0
#endif

#include <stdarg.h>

/****************************************************************************/
/* Platform defines                                                         */
/****************************************************************************/

/*! \ingroup g_lib_debug
    \brief Major version number of the GRAPH REAL SDK.

    All versions of the SDK which have the same Major version number are
    guaranteed to be source compatible with previous releases on the same
    major version number.  You may have to rebuild your project after taking
    an update, but you will never have to modify your code base to keep it
    working the same as before the update.  When the major version number is
    incremented, the minor version and patch version are reset to 0.
*/
#define REALGRAPH_VERSION_MAJOR  6

/*! \ingroup g_lib_debug
    \brief Minor version number of the GRAPH REAL SDK.

    We increment the minor version number any time we add something to the
    current API.  The addition might consist of a single macro or it could be
    a large module addition.  It could also be that an existing piece of
    functionality was optimized.  No existing functionality is ever changed or
    deleted on a minor revision.  If you want to take advantage of the new
    functionality, you must update your code base to do so.  When the minor
    version number is incremented, the patch version number is reset to 0.
*/
#define REALGRAPH_VERSION_MINOR  0

/*! \ingroup g_lib_debug
    \brief Patch version number of the GRAPH REAL SDK.

    We increment the patch version number any time we fix a bug in an
    existing function.
*/
#define REALGRAPH_VERSION_PATCH  90          // >"50" indicates this is a prerelease, counts down from 99

/*! \ingroup g_lib_debug
    \brief Make a version number for comparison purposes

    This macro combines a major, minor, and patch version number into a single
    number for comparsion purposes.  The number is of the form:

    0x00<major number><minor number><patch number>

    For example, in your code, you can do this:

    #if REALGRAPH_VERSION >= REALGRAPH_MAKE_VERSION(6,1,0)
    // this code will only compile in only if we are on 6.01.00 or higher of REAL GRAPH.
    #endif

    \sa REALGRAPH_VERSION
*/
#define REALGRAPH_MAKE_VERSION(major,minor,patch) (((major) * 10000) + ((minor) * 100) + (patch))

/*! \ingroup g_lib_debug
    \brief Unified version number of the GRAPH REAL SDK.

    The unified version number combines REALGRAPH_VERSION_MAJOR,
    REALGRAPH_VERSION_MINOR, and REALGRAPH_VERSION_PATCH into a single macro
    for convenience purposes.  You can use this number to verify that your
    REALGRAPH headers and libraries are the same version number.

    \sa REALGRAPH_MAKE_VERSION
*/
#define REALGRAPH_VERSION        REALGRAPH_MAKE_VERSION(REALGRAPH_VERSION_MAJOR, REALGRAPH_VERSION_MINOR, REALGRAPH_VERSION_PATCH)


/* Define REALAPI */
#ifndef REALAPI
#ifdef  REALDLL
#define REALAPI __declspec(dllimport)
#else
#define REALAPI
#endif
#endif

#if (defined(__R5900) || defined(__R5900__)) && !defined(PS2)
#define PS2 1
#elif (defined(__SN__) && defined(__PPC__)) && !defined(GCSN)
#define GCSN 1
#elif defined(__MWERKS__) && !defined(GCMW)
#define GCMW 1
#elif defined(__i386__) && !defined(LINUX)
#define LINUX 1
#elif defined(sparc) && !defined(SUN)
#define SUN 1
#elif defined(__APPLE__) && !defined(OSX)
#define OSX 1
#elif defined(_MSC_VER) && !defined(PC) && !defined(_XBOX)
#define PC 1
#elif defined(_MSC_VER) && !defined(XBOX) && defined(_XBOX)
#define XBOX 1
#endif

#if !defined(GC) && (defined(GCSN) || defined(GCMW))
#define GC 1
#endif

// NOTE correct usage is ifdef REAL_BIGENDIAN
#if !defined(REAL_BIGENDIAN) && (defined(GC) || defined(SUN) || defined(OSX) || defined(SGI))
#define REAL_BIGENDIAN 1
#endif

#if defined(XBOX) && !defined(__GNUC__) && !defined(__ICL)
#if (_MSC_VER < 1300)
#error REAL XBOX expects VC7 or later
#endif
#endif

#ifndef NULL
#define NULL 0
#endif

#if defined(REAL_BIGENDIAN)
    #define ggetx ggetm
    #define gputx gputm
#else
    #define ggetx ggeti
    #define gputx gputi
#endif

// define OSPanic outside the namespace (copy the prototype from Nintendo's os.h file)
// to avoid including Nintendo headers
#if defined (GC) && !defined(__DOLPHIN_RC__)
#include <dolphin/os.h>     // OSPanic
//extern "C" void OSPanic  ( const char* file, int line, const char* msg, ... );
#endif

namespace RealShape
{

/*!
* \ingroup g_shape_api
*
* \name Memory Type Flags.
*
* \brief memory type flags.  These are mapped to realcore's memory type flags defined in realmem.h
*
* These flags give the user some control over the type and alignment of memory allocated by the realmem library.
*
* MB_SHAPE_LOW allocates memory on a first-fit basis from the lowest available address to the highest.
* MB_SHAPE_HIGH allocates memory on a first-fit basis from the highest available address to the lowest.
* Careful usage of these flags can prevent memory fragmentation.  Mark any permanent or long-term allocations as MB_SHAPE_LOW, and
* transient or short-term allocations as MB_SHAPE_HIGH.
*
* MB_SHAPE_ALIGN indicates that the data block should be aligned to the optimal boundary for DMAtransfers to the GPU.
* The default (not using this flag) alignment boundary is generally smaller and sufficient for CPU alignment.
*
* \sa SetMemCallbacks
*/
//@{
static const int MB_SHAPE_LOW      = 0x0000; // from realmem.h
static const int MB_SHAPE_HIGH     = 0x0100;
static const int MB_SHAPE_ALIGN    = 0x0400;
//@}

/*!
 * \ingroup g_shape_api
 *
 * \brief GraphObject is a static container class for a number of global functions including
 * assert, memory management and low-level read/write routines.
 *
 * As GraphObject is a static class, there is no need to create an instance of it before calling
 * member functions.
 * \n Call a member function like this:  GraphObject::Function_Name();
 *
 */
class REALAPI GraphObject
{

public:
    static void SetMemCallbacks( void * (*MEMallocalign)(const char *name, int size, int alignment, int alignheadersize, int type), void (*MEMfree)(void *ptr) );
    static void GetMemCallbacks( void * (**MEMallocalign)(const char *name, int size, int alignment, int alignheadersize, int type), void (**MEMfree)(void *ptr) );
    static void AbortMessage (const char *format, ... );
    static void SetAbortMessageCallback( void (*AbortMessage)(const char *format, ...) );
    static void GetAbortMessageCallback( void (**AbortMessage)(const char *format, ...) );

   /*!
    * \brief Halt program execution.
    *
    * \param none.
    * \return none.
    */
    #if defined(PC) || defined(XBOX)
        static inline void Break() { _asm { int 3 } }
    #elif defined(PS2)
        static inline void Break() { asm("break"); }
    #elif defined(GC)
        static inline void Break() { OSPanic( "", 0, "" ); }
    #else
        static inline void Break() { return; }
    #endif

   /*!
    * \brief Halt program execution if a condition is met.
    * Assert behaves like the standard C 'assert'.
    *
    * \param expr If 'expr' is true, do nothing.  If false, halt execution.
    * \return none.
    */
    static void Assert( bool expr ) { if (!expr) Break(); }

   /*!
    * \brief Find log2 of an integer.
    *
    * \param val Find log2 of this number
    * \return log2 of 'val'.
    */
    static __inline int IntLog2(int val)
    {
        int t;
        *reinterpret_cast<float *>(&t) = static_cast<float>(val);
        return((t>>23)-127);
    }

  /*!
    * \brief Is an integer a power of 2?
    *
    * \param val Determine if this number is a power of 2.
    * \return true if 'val' is a power of 2, false otherwise.
    */
    static __inline bool IsLog2( int val )
    {
        return (val & (val-1))?false:true;
    }

    // internal functions for reading/writing bytes
    // in platform-dependent order

    /* get motorola memory */
    static __inline unsigned int ggetm(const void *src, int bytes)
    {
        if (bytes==1)
            return static_cast<unsigned int>( *reinterpret_cast<const unsigned char *>(src));
    #if defined(__APPLE__) || (defined(__MWERKS__) && defined(__PPCGEKKO__))
        else if (bytes==2)
            return static_cast<unsigned int>(*reinterpret_cast<const unsigned short *>(src));
        else if (bytes==3)
            return ((static_cast<unsigned int>( *reinterpret_cast<const unsigned int *>(src))) >> 8);
        else if (bytes==4)
            return static_cast<unsigned int>( *reinterpret_cast<const unsigned int *>(src));
    #else
        else if (bytes==2)
            return ((static_cast<unsigned int>( *reinterpret_cast<const unsigned char *>(src)))<<8) | static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src))+1);
        else if (bytes==3)
            return ((static_cast<unsigned int>( *reinterpret_cast<const unsigned char *>(src)))<<16) | ((static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src))+1))<<8) | static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src))+2);
        else if (bytes==4)
            return ((static_cast<unsigned int>( *reinterpret_cast<const unsigned char *>(src)))<<24) | ((static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src))+1))<<16) | ((static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src))+2))<<8) | static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src))+3);
    #endif
        return 0;
    }

    /* put motorola memory */
    static __inline void gputm(void *dst, unsigned int data, int bytes)
    {
        if (bytes==1)
        {
            (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>( data );
        }
    #if defined(__APPLE__) || (defined(__MWERKS__) && defined(__PPCGEKKO__))
        else if (bytes==2)
        {
            (reinterpret_cast<unsigned short *>(dst))[0] = static_cast<unsigned short>( data );
        }
        else if (bytes==4)
        {
            (reinterpret_cast<unsigned int *>(dst))[0] = static_cast<unsigned int>( data );
        }
    #else
        else if (bytes==2)
        {
            (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>8);
            (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>( data );
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

    /* get intel memory */
    static __inline unsigned int ggeti(const void *src, int bytes)
    {
        if (bytes==1)
            return static_cast<unsigned int>( *reinterpret_cast<const unsigned char *>(src));
    #if defined(_MSC_VER) || defined(__i386__)
        else if (bytes==2)
            return static_cast<unsigned int>( *reinterpret_cast<const unsigned short *>(src));
        else if (bytes==3)
            return ((static_cast<unsigned int>( *reinterpret_cast<const unsigned int *>(src))) & 0x00ffffff);
        else if (bytes==4)
            return static_cast<unsigned int>( *reinterpret_cast<const unsigned int *>(src));
    #else
        else if (bytes==2)
            return ((static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src)+1)))<<8) | static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src)+0));
        else if (bytes==3)
            return ((static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src)+2)))<<16) | ((static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src)+1)))<<8) | static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src)+0));
        else if (bytes==4)
            return ((static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src)+3)))<<24) | ((static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src)+2)))<<16) | ((static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src)+1)))<<8) | static_cast<unsigned int>( *(reinterpret_cast<const unsigned char *>(src)+0));
    #endif
        return 0;
    }

    /* put intel memory */
    static __inline void gputi(void *dst, unsigned int data, int bytes)
    {
        if (bytes==1)
        {
            (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
        }
    #if defined(_MSC_VER) || defined(__i386__)
        else if (bytes==2)
        {
            ((unsigned short *) dst)[0] = static_cast<unsigned short>(data);
        }
        else if (bytes==4)
        {
            ((unsigned int *) dst)[0] = static_cast<unsigned int>(data);
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
};  // end class GraphObject

class REALAPI Point
{
public:
    Point(void) {}
    Point( int x, int y)
    {
        mX = x;
        mY = y;
    }

    // Accessor get methods
    inline int GetX(void) const { return( mX); }
    inline int GetY(void) const { return( mY); }

    // Accessor set methods
    inline void SetX(int x){ mX = x;}
    inline void SetY(int y){ mY = y;}

public:
    int mX,mY;
};

class REALAPI PointF
{
public:
    PointF(void) {}

    PointF( float x, float y)
    {
        mX = x;
        mY= y;
    }

    // Accessor get methods
    inline float GetX(void) const { return( mX); }
    inline float GetY(void) const { return( mY); }

    // Accessor set methods
    inline void SetX(float x){ mX = x;}
    inline void SetY(float y){ mY = y;}

public:
    float mX,mY;
};

} // end namespace RealShape

#endif // REALGRAPH_H

