// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Defines PS2 host-specific types and macros.

#ifndef RW_HOSTTYPE_H

#if defined(RWCROSS)

#include "../../../../../win/rw/core/base/detail/hosttype.h"

#else

#define RW_HOSTTYPE_H

#include <limits.h>
#include <eetypes.h>

#if !defined (size_t)
#define __need_ptrdiff_t
#define __need_wint_t
#define __need_wchar_t
#define __need_size_t
#include <stddef.h>
#endif

// It is standard on Windows and XBox to have wchar_t be uint16_t.
// For GameCube and PlayStation, GCC defines wchar_t as int by default,
// while Metrowerks and the new EDG-based SN compilers define wchar_t
// as uint16_t. Criterion has standardized on wchar_t being
// an unsigned 16 bit value on all console platforms. Given that there
// is currently no known way to tell at preprocessor time what the size
// of wchar_t is, we declare it to be 2, as this is the Criterion
// standard. If you have rwWCHAR_SIZE != sizeof(wchar_t), then your
// code might not be broken, but it also won't work with wchar libraries
// and data from other parts of RW. Under GCC, you can force wchar_t
// to two bytes with the -fshort-wchar compiler argument.
#ifndef rwWCHAR_SIZE
    #define rwWCHAR_SIZE 2
#endif


/*
\todoc
*/
#define rwHOST_PS2      1
/*
\todoc
*/
#define RWINLINE /* no op */
/*
\todoc
*/
#ifdef __GNUC__
#define RWALIGN(O, A) O __attribute__ ((__aligned__ (A)))
#define RWCLASSPREALIGN(A)
#define RWCLASSPOSTALIGN(A) __attribute__ ((__aligned__ (A)))
#endif /* __GNUC__ */
/*
\todoc
*/
#ifdef __MWERKS__
#undef RWALIGN
#undef RWCLASSPREALIGN
#undef RWCLASSPOSTALIGN
#define RWALIGN(O, A) O __attribute__ ((aligned(A)))
#define RWCLASSPREALIGN(A)
#define RWCLASSPOSTALIGN(A) __attribute__ ((aligned(A)))
#endif /* __MWERKS__ */

#define atoi64(expr) atol(expr)

typedef char             RwInt8; /* signed */
typedef unsigned char    RwUInt8;
typedef signed short     RwInt16;
typedef unsigned short   RwUInt16;
typedef signed int       RwInt32;
typedef unsigned int     RwUInt32;
typedef signed long      RwInt64;
typedef unsigned long    RwUInt64;
typedef RwUInt32         RwBool;
typedef float            RwReal32;
typedef double           RwReal64;
typedef RwUInt32         RwSizet;
typedef RwUInt32         RwHostPointer;     // deprecated
typedef RwUInt32         RwHostPtr;

namespace rw
{
    typedef uint32_t HostPtr;

    class RWCLASSPREALIGN(16) uint128_t
    {
    public:

        /*
        \todoc
        */
        inline
            uint128_t()
    {
            rwFUNCTION();
            rwRETURNVOID();
        }


        /*
        \todoc
        */
        explicit inline
            uint128_t(uint64_t low)
    {
            rwFUNCTION();
            asm ("pcpyld %0, $0, %1" : "=r" (m_data) : "r" (low));
            rwRETURNVOID();
        }


        /*
        \todoc
        */
        inline
            uint128_t(uint64_t low, uint64_t high)
        {
            rwFUNCTION();
            asm ("pcpyld %0, %1, %2" : "=r" (m_data) : "r" (high), "r" (low));
            rwRETURNVOID();
        }


        /*
        \todoc
        */
        inline uint128_t &
            operator=(uint128_t right)
        {
            rwFUNCTION();
            m_data = right.m_data;
            rwRETURN(*this);
        }


        /*
        \todoc
        */
        inline uint128_t &
            operator|=(uint128_t right)
        {
            rwFUNCTION();
            asm ("por %0, %0, %1" : "+r" (m_data) : "r" (right.m_data));
            rwRETURN(*this);
        }


        /*
        \todoc
        */
        inline uint128_t
            operator~() const
        {
            uint128_t result;

            rwFUNCTION();
            asm ("pnor %0, $0, %1" : "=r" (result.m_data) : "r" (m_data));
            rwRETURN(result);
        }


        /*
        \todoc
        */
        inline uint128_t
            operator&(uint128_t right) const
        {
            uint128_t result;

            rwFUNCTION();
            asm ("pand %0, %1, %2" : "=r" (result.m_data) : "r" (m_data), "r" (right.m_data));
            rwRETURN(result);
        }


        /*
        \todoc
        */
        inline uint128_t
            operator|(uint128_t right) const
        {
            uint128_t result;

            rwFUNCTION();
            asm ("por %0, %1, %2" : "=r" (result.m_data) : "r" (m_data), "r" (right.m_data));
            rwRETURN(result);
        }


        /*
        \todoc
        */
        inline uint128_t
            operator^(uint128_t right) const
        {
            uint128_t result;

            rwFUNCTION();
            asm ("pxor %0, %1, %2" : "=r" (result.m_data) : "r" (m_data), "r" (right.m_data));
            rwRETURN(result);
        }


        /*
        \todoc
        */
        inline uint64_t
            GetLow() const
        {
            rwFUNCTION();
            rwRETURN(static_cast<uint64_t>(m_data));
        }


        /*
        \todoc
        */
        inline uint64_t
            GetHigh() const
        {
            uint64_t result;
            rwFUNCTION();
            asm ("pcpyud %0, $0, %1" : "=r" (result) : "r" (m_data));
            rwRETURN(result);
        }


        /*
        \todoc
        */
        inline bool
            operator==(uint128_t right) const
        {
            uint128_t diff;

            rwFUNCTION();
            diff = *this ^ right;
            rwRETURN(0 == (diff.GetHigh() | diff.GetLow()));
        }


    private:


        u_long128 m_data;
    } RWCLASSPOSTALIGN(16);

    /*
    Placeholder for int128_t values
    \toqa
    */
    class RWCLASSPREALIGN(16) int128_t
    {
    private:
        u_long128 m_data;
    }  RWCLASSPOSTALIGN(16);
}

#if !defined( _INTPTR_T_DEFINED ) && !defined( _intptr_t_defined )
typedef int32_t         intptr_t;
#define _intptr_t_defined
#define _INTPTR_T_DEFINED
#endif

#if !defined( _UINTPTR_T_DEFINED ) && !defined( _uintptr_t_defined )
typedef uint32_t        uintptr_t;
#define _uintptr_t_defined
#define _UINTPTR_T_DEFINED
#endif

/*
Determines if we must specialize a size_t data traits for the conditioning pipelines.
Equal to 1 if we must specialize. Equal to 0 if we do not.
\toqa
*/
#define rwHOST_SPECIALIZE_SIZET_TRAITS (0)

#define RWUINT64CONST(x) (x##ul)

#define RWINT64CONST(x) (x##l)

/*
\todoc
*/
#define RWZERO64 ((RwUInt64)0)
/*
\todoc
*/
#define rwHOST_ENDIAN (rwLIENDIAN)
/*
\todoc
*/
#define rwHOST_POINTERSIZE 4
/*
\todoc
*/
#define rwHOST_SIZETSIZE 4



typedef rw::int128_t  RwInt128;
typedef rw::uint128_t RwUInt128;


#endif // defined(RWCROSS)
#endif /* !RW_HOSTTYPE_H */
