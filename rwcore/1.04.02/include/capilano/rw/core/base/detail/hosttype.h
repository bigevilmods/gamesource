// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Defines Durnago host-specific types and macros.

#ifndef RW_HOSTTYPE_H
#define RW_HOSTTYPE_H

#define rwHOST_ENDIAN (rwLIENDIAN)

#define rwHOST_POINTERSIZE 8
#define rwHOST_SIZETSIZE 8

#if defined(_MSC_VER)
#define RWALIGN(O, A) __declspec(align(A)) O
#elif defined(__GNUC__)
#define RWALIGN(O, A) O __attribute__ ((__aligned__ (A)))
#endif /* defined(_MSC_VER) || defined(__GNUC__) */

#if defined(_MSC_VER) || defined(__ICL) || defined(__MWERKS__)
#define RWINLINE  __inline
#elif defined(__GNUC__)
#define RWINLINE  __inline__
#else
#define RWINLINE
#endif

#if (defined(RWPLUGIN))
#define RWAPIEXPORT __declspec(dllexport)
#define RWAPIIMPORT __declspec(dllimport)
#else
#define RWAPIEXPORT
#define RWAPIIMPORT
#endif /* (defined(RWPLUGIN)) */

#define atoi64(expr) _atoi64(expr)

typedef char             RwInt8; /* signed */
typedef unsigned char    RwUInt8;
typedef signed short     RwInt16;
typedef unsigned short   RwUInt16;
typedef signed int       RwInt32;
typedef unsigned int     RwUInt32;
typedef RwUInt32         RwBool;
typedef float            RwReal32;
typedef double           RwReal64;
typedef RwUInt32         RwSizet;
typedef RwUInt32         RwHostPointer;     // deprecated
typedef RwUInt32         RwHostPtr;         // deprecated use rw::HostPtr

namespace rw
{
    typedef size_t HostPtr;

    class uint128_t
    {
    public:

        inline
            uint128_t()
        {
            rwFUNCTION();
            rwRETURNVOID();
        }

        explicit inline
            uint128_t(uint64_t low)
        {
            rwFUNCTION();
            m_low = low;
            m_high = 0;
            rwRETURNVOID();
        }

        inline
            uint128_t(uint64_t low, uint64_t high)
        {
            rwFUNCTION();
            m_low = low;
            m_high = high;
            rwRETURNVOID();
        }

        inline uint128_t &
            operator=(uint128_t right)
        {
            rwFUNCTION();
            m_low = right.m_low;
            m_high = right.m_high;
            rwRETURN(*this);
        }

        inline uint128_t &
            operator|=(uint128_t right)
        {
            rwFUNCTION();
            m_low |= right.m_low;
            m_high |= right.m_high;
            rwRETURN(*this);
        }

        inline uint128_t
            operator~() const
        {
            uint128_t result;

            rwFUNCTION();
            result.m_low = ~m_low;
            result.m_high = ~m_high;
            rwRETURN(result);
        }

        inline uint128_t
            operator&(uint128_t right) const
        {
            uint128_t result;

            rwFUNCTION();
            result.m_low = m_low & right.m_low;
            result.m_high = m_high & right.m_high;
            rwRETURN(result);
        }

        inline uint128_t
            operator|(uint128_t right) const
        {
            uint128_t result;
            rwFUNCTION();
            result.m_low = m_low | right.m_low;
            result.m_high = m_high | right.m_high;
            rwRETURN(result);
        }

        inline uint128_t
            operator^(uint128_t right) const
        {
            uint128_t result;

            rwFUNCTION();
            result.m_low = m_low ^ right.m_low;
            result.m_high = m_high ^ right.m_high;
            rwRETURN(result);
        }

        inline bool
            operator==(uint128_t right) const
        {
            rwFUNCTION();
            rwRETURN(m_low == right.m_low && m_high == right.m_high);
        }

        inline bool
            operator!=(uint128_t right) const
        {
            rwFUNCTION();
            rwRETURN(m_low != right.m_low || m_high != right.m_high);
        }

        inline uint64_t
            GetLow() const
        {
            rwFUNCTION();
            rwRETURN(m_low);
        }

        inline uint64_t
            GetHigh() const
        {
            rwFUNCTION();
            rwRETURN(m_high);
        }
    private:
        uint64_t m_low;
        uint64_t m_high;
    };

    class int128_t
    {
    private:
        uint64_t m_low;
        int64_t  m_high;
    };
}


#if !defined( _INTPTR_T_DEFINED ) && !defined( _intptr_t_defined )

    typedef int64_t intptr_t;
#define _intptr_t_defined
#define _INTPTR_T_DEFINED
#endif

#if !defined( _UINTPTR_T_DEFINED ) && !defined( _uintptr_t_defined )

    typedef uint64_t uintptr_t;
#define _uintptr_t_defined
#define _UINTPTR_T_DEFINED
#endif


/**
Determines if we must specialize a size_t data traits for the conditioning pipelines.
Equal to 1 if we must specialize. Equal to 0 if we do not.
*/
#define rwHOST_SPECIALIZE_SIZET_TRAITS (0)

#if defined(_MSC_VER)
typedef __int64 RwInt64;
typedef unsigned __int64 RwUInt64;

#define RWUINT64CONST(x) (x##ui64)

#define RWINT64CONST(x) (x##i64)

#define RWZERO64 ((RwUInt64)0)

#define rwHOST_MAX_NATIVE_BITS 64

#elif defined(__GNUC__)

typedef long long RwInt64;
typedef unsigned long long RwUInt64;

#define RWUINT64CONST(x) (x##ull)

#define RWZERO64 ((RwUInt64)0)

#define rwHOST_MAX_NATIVE_BITS 64

#else

#error Unsupported compiler

#endif /* _MSC_VER */

typedef rw::uint128_t RwUInt128;
typedef rw::int128_t RwInt128;

#endif /* !RW_HOSTTYPE_H */
