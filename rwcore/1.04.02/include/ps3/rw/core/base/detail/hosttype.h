// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Defines PS3 host-specific types and macros.

#ifndef RW_HOSTTYPE_H

#if defined(RWCROSS)

#include "../../../../../win/rw/core/base/detail/hosttype.h"

#else

#if defined(__SPU__)
#include "rw/core/base/detail/spu_hosttype.h"
#else

#define RW_HOSTTYPE_H

#include <stddef.h>      // size_t typedef
#include <inttypes.h>    // C99 typedefs
#include <sdk_version.h> // sdk version

#define rwHOST_PS3        1 // host PS3 not useful for SPU/PPU differentiation
#define rwHOST_PS3_PPU    1 // PPU vs SPU subarchitectures more useful

#define RWINLINE /* no op */

#define atoi64(expr) _atol(expr)

typedef signed char      RwInt8; /* signed */
typedef unsigned char    RwUInt8;
typedef signed short     RwInt16;
typedef unsigned short   RwUInt16;
typedef signed int       RwInt32;
typedef unsigned int     RwUInt32;
typedef signed long long   RwInt64;
typedef unsigned long long RwUInt64;
typedef RwUInt32         RwBool;
typedef float            RwReal32;
typedef double           RwReal64;
typedef unsigned long    RwSizet; // needs to match size_t
typedef RwUInt32         RwHostPointer; // deprecated
typedef RwUInt32         RwHostPtr;
typedef RwInt32          RwHostOffset;
#ifdef __SNC__
typedef vector signed int       RwInt128;
typedef vector unsigned int     RwUInt128;
#else
typedef signed int       RwInt128 __attribute__((vector_size(16)));
typedef unsigned int     RwUInt128 __attribute__((vector_size(16)));
#endif

namespace rw
{
    typedef uint32_t HostPtr;
    typedef uint32_t HostOffset;
#ifdef __SNC__
    typedef vector signed int int128_t;
    typedef vector unsigned int uint128_t;
#else
    typedef signed int int128_t __attribute__((vector_size(16)));
    typedef unsigned int uint128_t __attribute__((vector_size(16)));
#endif
}

// Determines if we must specialize a size_t data traits for the conditioning pipelines.
// Equal to 1 if we must specialize. Equal to 0 if we do not.
#define rwHOST_SPECIALIZE_SIZET_TRAITS (0)

#define RWUINT64CONST(x) (x##ull)
#define RWINT64CONST(x) (x##ll)
#define RWZERO64 ((RwUInt64)0)

#define rwHOST_ENDIAN (rwBIENDIAN)
#define rwHOST_POINTERSIZE 4
#define rwHOST_SIZETSIZE 4
#define rwHOST_MAX_NATIVE_BITS 64

struct RwRGBTag
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
#define RwRGBTagMacro struct RwRGBTag
typedef RwRGBTagMacro RwRGB;


struct RwRGBATag
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};
#define RwRGBATagMacro struct RwRGBATag
typedef RwRGBATagMacro RwRGBA;


namespace rw
{
class RGBA
{
 public:

    inline
    RGBA()
    {
        rwFUNCTION();
        // empty
        rwRETURNVOID();
    }
     
    inline explicit RGBA(RwRGBA rgba)
    {
        rwFUNCTION();
        Set(rgba);
        rwRETURNVOID();
    }

    inline
    RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a=0xff)
    {
        rwFUNCTION();
        m_rgba  = (uint32_t)r << 24;
        m_rgba |= (uint32_t)g << 16;
        m_rgba |= (uint32_t)b << 8;
        m_rgba |= (uint32_t)a << 0;
        rwRETURNVOID();
    }

    inline
    RGBA(RwRGB rgb, unsigned char a=0xff)
    {
        rwFUNCTION();
        m_rgba  = (uint32_t)rgb.red   << 24;
        m_rgba |= (uint32_t)rgb.green << 16;
        m_rgba |= (uint32_t)rgb.blue  << 8;
        m_rgba |= (uint32_t)a << 0;
        rwRETURNVOID();
    }

    inline RwRGBA
    GetRwRGBA() const
    {
        rwFUNCTION();
        RwRGBA rgba = { GetRed(), GetGreen(), GetBlue(), GetAlpha() };
        rwRETURN(rgba);
    }

    inline void
    Set(RwRGBA rgba)
    {
        rwFUNCTION();
        m_rgba  = (uint32_t)rgba.red   << 24;
        m_rgba |= (uint32_t)rgba.green << 16;
        m_rgba |= (uint32_t)rgba.blue  << 8;
        m_rgba |= (uint32_t)rgba.alpha << 0;
        rwRETURNVOID();
    }


    inline uint8_t
    GetRed() const
    {
        rwFUNCTION();
        rwRETURN((uint8_t)(m_rgba >> 24));
    }

    inline void
    SetRed(uint8_t value)
    {
        rwFUNCTION();
        m_rgba &= ~(0xff << 24);
        m_rgba |= ((uint32_t)value << 24);
        rwRETURNVOID();
    }

    inline uint8_t
    GetGreen() const
    {
        rwFUNCTION();
        rwRETURN((uint8_t)(m_rgba >> 16));
    }

    inline void
    SetGreen(uint8_t value)
    {
        rwFUNCTION();
        m_rgba &= ~(0xff << 16);
        m_rgba |= ((uint32_t)value << 16);
        rwRETURNVOID();
    }

    inline uint8_t
    GetBlue() const
    {
        rwFUNCTION();
        rwRETURN((uint8_t)(m_rgba >> 8));
    }

    inline void
    SetBlue(uint8_t value)
    {
        rwFUNCTION();
        m_rgba &= ~(0xff << 8);
        m_rgba |= ((uint32_t)value << 8);
        rwRETURNVOID();
    }

    inline uint8_t
    GetAlpha() const
    {
        rwFUNCTION();
        rwRETURN((uint8_t)(m_rgba >> 0));
    }

    inline void
    SetAlpha(uint8_t value)
    {
        rwFUNCTION();
        m_rgba &= ~(0xff << 0);
        m_rgba |= ((uint32_t)value << 0);
        rwRETURNVOID();
    }

    bool operator==(const RGBA &other) const
    {
        return (m_rgba == other.m_rgba);
    }

private:
    uint32_t  m_rgba;
};

} // namespace rw

#define RwRGBAClassMacro

#endif // defined(__SPU__)
#endif // defined(RWCROSS)
#endif /* !RW_HOSTTYPE_H */
