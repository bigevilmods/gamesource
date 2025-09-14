// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Defines PS3 SPU host-specific types and macros.

#ifndef RW_HOSTTYPE_H
#define RW_HOSTTYPE_H

// Define some standard macros that should
// be defined sony headers as of 0_4_0 uses
// these defines in inconsistent ways,
// sometimes checking if they are defined
// and sometimes checking their values.
// 0_5_0 PPU yvals.h defines these in
// this way but the SPU version has not
// been fixed up at the time of this
// writing.
#ifndef __STDC_VERSION__
#define __STDC_VERSION__  0
#endif
#ifndef _HAS_C9X_FAST_FMA
#define _HAS_C9X_FAST_FMA 0
#endif
#ifndef _WIN32_C_LIB
#define _WIN32_C_LIB      0
// define this to avoid
// conflicts with gcc stdarg
// native va_list
#define _VA_LIST_DEFINED
#endif
// Additional defines causing warnings during an 0_5_0 build
#ifndef __embedded_cplusplus
#define __embedded_cplusplus 0
#endif
#ifndef _HAS_STRICT_LINKAGE
#define _HAS_STRICT_LINKAGE 0
#endif

#include <spu_intrinsics.h>

#define rwHOST_PS3        1 // Defined for both SPU and PPU defines main arch only
#define rwHOST_PS3_SPU    1 // PPU vs SPU sub-architecture, more useful than rwHOST_PS3
#define RWINLINE /* no op */

#define atoi64(expr) _atol(expr)

typedef signed char        RwInt8; /* signed */
typedef unsigned char      RwUInt8;
typedef signed short       RwInt16;
typedef unsigned short     RwUInt16;
typedef signed int         RwInt32;
typedef unsigned int       RwUInt32;
typedef signed long long   RwInt64;
typedef unsigned long long RwUInt64;
typedef RwUInt32           RwBool;
typedef float              RwReal32;
typedef double             RwReal64;
typedef unsigned long      RwSizet; // needs to match size_t
typedef RwUInt32           RwHostPtr;
typedef RwInt32            RwHostOffset;
typedef signed int RwInt128 __attribute__((vector_size(16)));
typedef unsigned int RwUInt128 __attribute__((vector_size(16)));

namespace rw
{
    typedef uint32_t HostPtr;
    typedef uint32_t HostOffset;
    typedef signed int int128_t __attribute__((vector_size(16)));
    typedef unsigned int uint128_t __attribute__((vector_size(16)));
}

#define RWUINT64CONST(x) (x##ull)
#define RWINT64CONST(x) (x##ll)
#define RWZERO64 ((RwUInt64)0)

#define rwHOST_ENDIAN (rwBIENDIAN)
#define rwHOST_POINTERSIZE 4
#define rwHOST_SIZETSIZE 4


// Double buffered variables
static const vec_uchar16 DoubleBufferedShuffle0
= {0x00, 0x01, 0x02, 0x03, 0x80, 0x80, 0x80, 0x80, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80};
static const vec_uchar16 DoubleBufferedShuffle1
= {0x08, 0x09, 0x0a, 0x0b, 0x80, 0x80, 0x80, 0x80, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80};


// ***********************************************************************************************************
//                                             SPU Definition of RwRGBA classes
// ***********************************************************************************************************

struct RwRGBTag
{
    uint8_t red;    /**< red component */
    uint8_t green;  /**< green component */
    uint8_t blue;   /**< blue component */
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

// ***********************************************************************************************************
//                                             SPU Definition of RGBA class
// ***********************************************************************************************************

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
        *reinterpret_cast<RwRGBA *>(&m_rgba) = rgba;
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
        RwRGBA rgba;
        rgba = *(reinterpret_cast<const RwRGBA *>(&m_rgba));
        rwRETURN(rgba);
    }

    inline void
    Set(RwRGBA rgba)
    {
        rwFUNCTION();
        *(reinterpret_cast<RwRGBA *>(&m_rgba)) = rgba;
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

 private:
    uint32_t  m_rgba;
};

} // namespace rw

#define RwRGBAClassMacro

// ***********************************************************************************************************
//                                                DoubleBuffered Definition
// ***********************************************************************************************************

template<class T>
class DoubleBuffered
{
public:
    T    GetCurrent() const;
    T    GetNext() const;
    void Initialize(T value0, T value1);
    void Swap();
    void Swap(T value);
private:
    vec_char16 mData;
};

template<class T>
inline T DoubleBuffered<T>::GetCurrent() const
{
    rwFUNCTION();
    rwRETURN((T)si_to_uint(mData));
}

template<class T>
inline T DoubleBuffered<T>::GetNext() const
{
    rwFUNCTION();
    rwRETURN((T)si_to_uint(spu_rlqwbyte(mData, 8)));
}

template<class T>
inline void DoubleBuffered<T>::Initialize(T value0, T value1)
{
    rwFUNCTION();
    mData = spu_shuffle(si_from_uint((uint32_t)value0), si_from_uint((uint32_t)value1),
        DoubleBufferedShuffle0);
    rwRETURNVOID();
}

template<class T>
inline void DoubleBuffered<T>::Swap()
{
    rwFUNCTION();
    mData = spu_rlqwbyte(mData, 8);
    rwRETURNVOID();
}

template<class T>
inline void DoubleBuffered<T>::Swap(T value)
{
    rwFUNCTION();
    mData = spu_shuffle(mData, si_from_uint((uint32_t)value), DoubleBufferedShuffle1);
    rwRETURNVOID();
}


// Triple buffered variables
static const vec_uchar16 TripleBufferedShuffle0and1
= {0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
static const vec_uchar16 TripleBufferedShuffle01and2
= {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80};
static const vec_uchar16 TripleBufferedShuffleSwapSelf
= {0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x00, 0x01, 0x02, 0x03, 0x80, 0x80, 0x80, 0x80};
static const vec_uchar16 TripleBufferedShuffleSwapIn
= {0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80};


// ***********************************************************************************************************
//                                                TripleBuffered Definition
// ***********************************************************************************************************

template<class T>
class TripleBuffered
{
public:
    T    GetCurrent() const;
    T    GetNext() const;
    T    GetAfter() const;
    void Initialize(T value0, T value1, T value2);
    void Swap();
    void Swap(T value);
private:
    vec_char16 mData;
};

template<class T>
inline T TripleBuffered<T>::GetCurrent() const
{
    rwFUNCTION();
    rwRETURN((T)si_to_uint(mData));
}

template<class T>
inline T TripleBuffered<T>::GetNext() const
{
    rwFUNCTION();
    rwRETURN((T)si_to_uint(spu_rlqwbyte(mData, 4)));
}

template<class T>
inline T TripleBuffered<T>::GetAfter() const
{
    rwFUNCTION();
    rwRETURN((T)si_to_uint(spu_rlqwbyte(mData, 8)));
}

template<class T>
inline void TripleBuffered<T>::Initialize(T value0, T value1, T value2)
{
    rwFUNCTION();
    vec_char16 v01 = spu_shuffle(si_from_uint((uint32_t)value0), si_from_uint((uint32_t)value1),
                                 TripleBufferedShuffle0and1);
    mData = spu_shuffle(v01, si_from_uint((uint32_t)value2), TripleBufferedShuffle01and2);
    rwRETURNVOID();
}

template<class T>
inline void TripleBuffered<T>::Swap()
{
    rwFUNCTION();
    mData = spu_shuffle(mData, mData, TripleBufferedShuffleSwapSelf);
    rwRETURNVOID();
}

template<class T>
inline void TripleBuffered<T>::Swap(T value)
{
    rwFUNCTION();
    mData = spu_shuffle(mData, si_from_uint((uint32_t)value), TripleBufferedShuffleSwapIn);
    rwRETURNVOID();
}

// Quadruple buffered variables
static const vec_uchar16 QuadrupleBufferedShuffle0and1
= {0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
static const vec_uchar16 QuadrupleBufferedShuffle2and3
= {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13};
static const vec_uchar16 QuadrupleBufferedShuffleSwapSelf
= {0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03};
static const vec_uchar16 QuadrupleBufferedShuffleSwapIn
= {0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13};


// ***********************************************************************************************************
//                                                QuadrupleBuffered Definition
// ***********************************************************************************************************

template<class T>
class QuadrupleBuffered
{
public:
    T    GetCurrent() const;
    T    GetNext() const;
    T    GetAfter() const;
    T    GetThat() const;
    void Initialize(T value0, T value1, T value2, T value2);
    void Swap();
    void Swap(T value);
private:
    vec_char16 mData;
};

template<class T>
inline T QuadrupleBuffered<T>::GetCurrent() const
{
    rwFUNCTION();
    rwRETURN((T)si_to_uint(mData));
}

template<class T>
inline T QuadrupleBuffered<T>::GetNext() const
{
    rwFUNCTION();
    rwRETURN((T)si_to_uint(spu_rlqwbyte(mData, 4)));
}

template<class T>
inline T QuadrupleBuffered<T>::GetAfter() const
{
    rwFUNCTION();
    rwRETURN((T)si_to_uint(spu_rlqwbyte(mData, 8)));
}

template<class T>
inline T QuadrupleBuffered<T>::GetThat() const
{
    rwFUNCTION();
    rwRETURN((T)si_to_uint(spu_rlqwbyte(mData, 12)));
}

template<class T>
inline void QuadrupleBuffered<T>::Initialize(T value0, T value1, T value2, T value3)
{
    rwFUNCTION();
    vec_char16 v01 = spu_shuffle(si_from_uint((uint32_t)value0), si_from_uint((uint32_t)value1), QuadrupleBufferedShuffle0and1);
    vec_char16 v02 = spu_shuffle(si_from_uint((uint32_t)value2), si_from_uint((uint32_t)value3), QuadrupleBufferedShuffle2and3);
    mData          = spu_or(v01, v02);
    rwRETURNVOID();
}

template<class T>
inline void QuadrupleBuffered<T>::Swap()
{
    rwFUNCTION();
    mData = spu_shuffle(mData, mData, QuadrupleBufferedShuffleSwapSelf);
    rwRETURNVOID();
}

template<class T>
inline void QuadrupleBuffered<T>::Swap(T value)
{
    rwFUNCTION();
    mData = spu_shuffle(mData, si_from_uint((uint32_t)value), QuadrupleBufferedShuffleSwapIn);
    rwRETURNVOID();
}

#endif /* !RW_HOSTTYPE_H */
