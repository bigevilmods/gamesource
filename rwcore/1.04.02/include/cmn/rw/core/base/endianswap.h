// (c) Electronic Arts. All Rights Reserved.
#ifndef RW_ENDIANSWAP_H
#define RW_ENDIANSWAP_H

/**
\file
Endian-swapping determination & implementation
*/

#include <rw/core/debug/compiletimeassert.h>
#include <rw/core/base/ostypes.h>


#if (rwHOST_ENDIAN != rwTARGET_ENDIAN)
#define rwNATIVE_ENDIAN_CONVERT_REQUIRED TRUE
#else /* (rwHOST_ENDIAN != rwTARGET_ENDIAN) */
#define rwNATIVE_ENDIAN_CONVERT_REQUIRED FALSE
#endif /* (rwHOST_ENDIAN != rwTARGET_ENDIAN) */

/* We stream PI data in "network byte order" */
#if (rwHOST_ENDIAN != rwBIENDIAN)
#define rwPI_ENDIAN_CONVERT_REQUIRED TRUE
#else /* (rwHOST_ENDIAN != rwBIENDIAN) */
#define rwPI_ENDIAN_CONVERT_REQUIRED FALSE
#endif /* (rwHOST_ENDIAN != rwBIENDIAN) */


#ifndef RWUNUSED_NOENDIANCONVERT
#if (FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED)
#define RWUNUSED_NOENDIANCONVERT(PARAM) RWUNUSED(PARAM)
#else /* (FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) */
#define RWUNUSED_NOENDIANCONVERT(PARAM) PARAM
#endif /* (FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) */
#endif /* !RWUNUSED_NOENDIANCONVERT */

#ifndef RWUNUSED_NOENDIANCONVERT_RELEASE
#if ((FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) && !defined(EA_DEBUG))
#define RWUNUSED_NOENDIANCONVERT_RELEASE(PARAM) RWUNUSED(PARAM)
#else /* ((FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) && !defined(EA_DEBUG)) */
#define RWUNUSED_NOENDIANCONVERT_RELEASE(PARAM) PARAM
#endif /* ((FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) && !defined(EA_DEBUG)) */
#endif /* !RWUNUSED_NOENDIANCONVERT */

#ifndef RWUNUSED_NOENDIANCONVERT_CROSS
#if ((FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) && defined(RWCROSS))
#define RWUNUSED_NOENDIANCONVERT_CROSS(PARAM) RWUNUSED(PARAM)
#else /* ((FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) && defined(RWCROSS)) */
#define RWUNUSED_NOENDIANCONVERT_CROSS(PARAM) PARAM
#endif /* ((FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) && defined(RWCROSS)) */
#endif /* !RWUNUSED_NOENDIANCONVERT */

#ifndef RWUNUSED_NOENDIANCONVERT_CROSS_RELEASE
#if ((FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) && defined(RWCROSS) && !defined(EA_DEBUG))
#define RWUNUSED_NOENDIANCONVERT_CROSS_RELEASE(PARAM) RWUNUSED(PARAM)
#else /* ((FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) && defined(RWCROSS) && !defined(EA_DEBUG)) */
#define RWUNUSED_NOENDIANCONVERT_CROSS_RELEASE(PARAM) PARAM
#endif /* ((FALSE == rwNATIVE_ENDIAN_CONVERT_REQUIRED) && defined(RWCROSS) && !defined(EA_DEBUG)) */
#endif /* !RWUNUSED_NOENDIANCONVERT */


/* Forcible endian-swap routines for use on any host */
#define RWFORCEDENDIANSWAP16(_x)                        \
    rw::ForcedEndianSwap16(_x)

#define RWFORCEDENDIANSWAP32(_x)                        \
    rw::ForcedEndianSwap32(_x)

#define RWFORCEDENDIANSWAP64(_x)                        \
    rw::ForcedEndianSwap64(_x)

#define RWFORCEDENDIANSWAP128(_x)                       \
    rw::ForcedEndianSwap128(_x)

#define RWFORCEDENDIANSWAP16_ARRAY(_m, _n, _s)          \
    rw::ForcedEndianSwap16_Array(_m, _n, _s)

#define RWFORCEDENDIANSWAP32_ARRAY(_m, _n, _s)          \
    rw::ForcedEndianSwap32_Array(_m, _n, _s)

#define RWFORCEDENDIANSWAP64_ARRAY(_m, _n, _s)          \
    rw::ForcedEndianSwap64_Array(_m, _n, _s)

#define RWFORCEDENDIANSWAPENUM(_x)                      \
    rw::ForcedEndianSwapEnum(_x)

/* We also need a bunch of target endian conversion as we may be hosted on
   a machine of differing endianess */
#if (rwNATIVE_ENDIAN_CONVERT_REQUIRED)

#define RWENDIANSWAP16(_x) RWFORCEDENDIANSWAP16(_x)

#define RWENDIANSWAP32(_x) RWFORCEDENDIANSWAP32(_x)

#define RWENDIANSWAP64(_x) RWFORCEDENDIANSWAP64(_x)

#define RWENDIANSWAP128(_x) RWFORCEDENDIANSWAP128(_x)

#define RWENDIANSWAP16_ARRAY(_m, _n, _s) RWFORCEDENDIANSWAP16_ARRAY(_m, _n, _s)

#define RWENDIANSWAP32_ARRAY(_m, _n, _s) RWFORCEDENDIANSWAP32_ARRAY(_m, _n, _s)

#define RWENDIANSWAP64_ARRAY(_m, _n, _s) RWFORCEDENDIANSWAP64_ARRAY(_m, _n, _s)

#define RWENDIANSWAPENUM(_x) RWFORCEDENDIANSWAPENUM(_x)

#else // !(rwNATIVE_ENDIAN_CONVERT_REQUIRED)

#define RWENDIANSWAP16(_x)
#define RWENDIANSWAP32(_x)
#define RWENDIANSWAP64(_x)
#define RWENDIANSWAP128(_x)
#define RWENDIANSWAP16_ARRAY(_m, _n, _s)
#define RWENDIANSWAP32_ARRAY(_m, _n, _s)
#define RWENDIANSWAP64_ARRAY(_m, _n, _s)
#define RWENDIANSWAPENUM(_x)

#endif // !(rwNATIVE_ENDIAN_CONVERT_REQUIRED)

namespace rw
{

template <typename T>
inline void ForcedEndianSwap16(T & _x)
{
    ((*((uint16_t *)(&(_x)))) = (uint16_t)
     (((*((uint16_t *)(&(_x)))) >> 8) |
      ((*((uint16_t *)(&(_x)))) << 8)));
}

template <typename T>
inline void ForcedEndianSwap32(T & _x)
{
    (((*(uint32_t *)(&(_x)))) =
     ((((*(uint32_t *)(&(_x))) >> 24))              |
      (((*(uint32_t *)(&(_x))) >>  8) & 0xff00UL)   |
      (((*(uint32_t *)(&(_x))) <<  8) & 0xff0000UL) |
      (((*(uint32_t *)(&(_x))) << 24))));
}

template <typename T>
inline void ForcedEndianSwap64(T & _x)
{
    RwChar *endianSwap64begin = (RwChar *)&(_x);
    RwChar *endianSwap64end = endianSwap64begin + 7;
    while (endianSwap64begin < endianSwap64end)
    {
        const RwChar endianSwap64temp = *endianSwap64begin;
        *endianSwap64begin = *endianSwap64end;
        *endianSwap64end = endianSwap64temp;
        endianSwap64begin++;
        endianSwap64end--;
    }
}

template <typename T>
inline void ForcedEndianSwap128(T & _x)
{
    RwChar *endianSwap128begin = (RwChar *)&(_x);
    RwChar *endianSwap128end = endianSwap128begin + 15;
    while (endianSwap128begin < endianSwap128end)
    {
        const RwChar endianSwap128temp = *endianSwap128begin;
        *endianSwap128begin = *endianSwap128end;
        *endianSwap128end = endianSwap128temp;
        endianSwap128begin++;
        endianSwap128end--;
    }
}

template <typename T>
inline void ForcedEndianSwap16_Array(T _m, int32_t _n, size_t _s)
{
    uint16_t    *_mem;
    int32_t     _i;

    _mem = reinterpret_cast<uint16_t *>(_m);

    _i = (_n);
    while (_i--)
    {
        RWFORCEDENDIANSWAP16(*_mem);
        _mem = reinterpret_cast<uint16_t *>(((uint8_t *)_mem) + (_s));
    }
}

template <typename T>
inline void ForcedEndianSwap32_Array(T _m, int32_t _n, size_t _s)
{
    uint32_t    *_mem;
    int32_t     _i;

    _mem = reinterpret_cast<uint32_t *>(_m);

    _i = (_n);
    while (_i--)
    {
        RWFORCEDENDIANSWAP32(*_mem);
        _mem = reinterpret_cast<uint32_t *>(((uint8_t *)_mem) + (_s));
    }
}

template <typename T>
inline void ForcedEndianSwap64_Array(T _m, int32_t _n, size_t _s)
{
    uint64_t    *_mem;
    int32_t     _i;

    _mem = reinterpret_cast<uint64_t *>(_m);

    _i = (_n);
    while (_i--)
    {
        RWFORCEDENDIANSWAP64(*_mem);
        _mem = reinterpret_cast<uint64_t *>(((uint8_t *)_mem) + (_s));
    }
}

// NULL-implementations that can be used to improve maintainability of
// code (e.g. if a type is 8-bit currently, but later grows to something
// larger, the 8-bit type can still have the EndianSwap function called
// on it and it does nothing.
inline void EndianSwap(int8_t&)
{
    // nothing to do
}

inline void EndianSwap(uint8_t&)
{
    // nothing to do
}

inline void EndianSwap(int16_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP16(t);
}

inline void EndianSwap(uint16_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP16(t);
}

inline void EndianSwap(int32_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP32(t);
}

inline void EndianSwap(uint32_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP32(t);
}

inline void EndianSwap(float32_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP32(t);
}

inline void EndianSwap(int64_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP64(t);
}

inline void EndianSwap(uint64_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP64(t);
}

inline void EndianSwap(float64_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP64(t);
}

inline void EndianSwap(int128_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP128(t);
}

inline void EndianSwap(uint128_t& RWUNUSED_NOENDIANCONVERT(t))
{
    RWENDIANSWAP128(t);
}

template <typename T>
inline void EndianSwap(T*& ptr)
{
    EndianSwap(*reinterpret_cast<rw::HostPtr*>(&ptr));
}

template <typename T>
inline void EndianSwapArray(T* RWUNUSED_NOENDIANCONVERT(array), uint32_t RWUNUSED_NOENDIANCONVERT(size))
{
#if rwNATIVE_ENDIAN_CONVERT_REQUIRED
    for (uint32_t i = 0; i < size; ++i)
    {
        EndianSwap(*(array + i));
    }
#endif
}

template <typename T>
inline void EndianSwapEnum(T & RWUNUSED_NOENDIANCONVERT(lEnum))
{
	RWENDIANSWAPENUM(lEnum);
}

inline void ForcedEndianSwap(int8_t& RWUNUSED(t))
{
    // nothing to do
}

inline void ForcedEndianSwap(uint8_t& RWUNUSED(t))
{
    // nothing to do
}

inline void ForcedEndianSwap(int16_t& t)
{
    RWFORCEDENDIANSWAP16(t);
}

inline void ForcedEndianSwap(uint16_t& t)
{
    RWFORCEDENDIANSWAP16(t);
}

inline void ForcedEndianSwap(int32_t& t)
{
    RWFORCEDENDIANSWAP32(t);
}

inline void ForcedEndianSwap(uint32_t& t)
{
    RWFORCEDENDIANSWAP32(t);
}

inline void ForcedEndianSwap(float32_t& t)
{
    RWFORCEDENDIANSWAP32(t);
}

inline void ForcedEndianSwap(int64_t& t)
{
    #ifdef _MSC_VER
      #pragma warning(push)
      #pragma warning(disable: 4127) // disable conditional expression is constant warning for W4 compatibility
    #endif
    RWFORCEDENDIANSWAP64(t);
    #ifdef _MSC_VER
      #pragma warning(pop)
    #endif
}

inline void ForcedEndianSwap(uint64_t& t)
{
    #ifdef _MSC_VER
      #pragma warning(push)
      #pragma warning(disable: 4127) // disable conditional expression is constant warning for W4 compatibility
    #endif
    RWFORCEDENDIANSWAP64(t);
    #ifdef _MSC_VER
      #pragma warning(pop)
    #endif
}

inline void ForcedEndianSwap(float64_t& t)
{
    #ifdef _MSC_VER
      #pragma warning(push)
      #pragma warning(disable: 4127) // disable conditional expression is constant warning for W4 compatibility
    #endif
    RWFORCEDENDIANSWAP64(t);
    #ifdef _MSC_VER
      #pragma warning(pop)
    #endif
}

inline void ForcedEndianSwap(int128_t& t)
{
    #ifdef _MSC_VER
      #pragma warning(push)
      #pragma warning(disable: 4127) // disable conditional expression is constant warning for W4 compatibility
    #endif
    RWFORCEDENDIANSWAP128(t);
    #ifdef _MSC_VER
      #pragma warning(pop)
    #endif
}

inline void ForcedEndianSwap(uint128_t& t)
{
    #ifdef _MSC_VER
      #pragma warning(push)
      #pragma warning(disable: 4127) // disable conditional expression is constant warning for W4 compatibility
    #endif
    RWFORCEDENDIANSWAP128(t);
    #ifdef _MSC_VER
      #pragma warning(pop)
    #endif
}

template <typename T>
inline void ForcedEndianSwap(T*& ptr)
{
    ForcedEndianSwap(reinterpret_cast<rw::HostPtr&>(ptr));
}

template <typename T>
inline void ForcedEndianSwapArray(T* array, uint32_t size)
{
    for (uint32_t i = 0; i < size; ++i)
    {
        ForcedEndianSwap(*(array + i));
    }
}

template <typename T>
inline void ForcedEndianSwapEnum(T & lEnum)
{
	// Copy the lEnum into a tmp enum, swap and copy back to prevent aliasing issues.
	
	rwASSERTATCOMPILETIME(sizeof(lEnum) == 4, ENDIAN_SWAP_expected_enum_type_passed_to_ForcedEndianSwapEnum_to_have_4_byte_size);
	
	union U
	{
		T e;
		uint32_t u;
	} tmp;
	
	tmp.e = lEnum; 
	ForcedEndianSwap32(tmp.u);
	lEnum = tmp.e;
}

} // namespace rw

#endif /* !RW_ENDIANSWAP_H */
