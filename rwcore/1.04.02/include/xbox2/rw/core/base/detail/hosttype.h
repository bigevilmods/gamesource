// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Defines Xbox2 host-specific types and macros.

#ifndef RW_HOSTTYPE_H

#if defined(RWCROSS)

#include "../../../../../win/rw/core/base/detail/hosttype.h"

#else

#define RW_HOSTTYPE_H

#pragma warning(push, 0)
#include <xtl.h>

#include <ppcintrinsics.h>
#include <xlocinfo.h>
#pragma warning(pop)

#define rwHOST_XBOX2    1

#define rwHOST_ENDIAN (rwBIENDIAN)

#define rwHOST_POINTERSIZE 4

#define rwHOST_SIZETSIZE 4

#define RWALIGN(O, A) __declspec(align(A)) O

#define RWINLINE  __inline

#define RWRESTRICT  __restrict

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
typedef __int64          RwInt64;
typedef unsigned __int64 RwUInt64;
typedef RwUInt32         RwSizet;
typedef RwUInt32         RwHostPointer;     // deprecated
typedef RwUInt32         RwHostPtr;

namespace rw
{
    typedef uint32_t HostPtr;

    // We'll do it with structures (can't do maths on these, but OK for allocation):
    struct uint128_t
    {
        uint64_t low;
        uint64_t high;
    };

    struct int128_t
    {
        uint64_t low;
        int64_t  high;
    };

    typedef struct uint128_t uint128_t;

    typedef struct int128_t int128_t;
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

#define rwHOST_SPECIALIZE_SIZET_TRAITS (0)

#define RWUINT64CONST(x) (x##ui64)

#define RWINT64CONST(x) (x##i64)

#define RWZERO64 ((RwUInt64)0)

#define rwHOST_MAX_NATIVE_BITS 64

typedef struct rw::uint128_t RwUInt128;

typedef struct rw::int128_t RwInt128;

#define RWZERO128 { RWZERO64, RWZERO64 }

#define RwMake128_64(R, MSP, LSP) \
do                                \
{                                 \
    (R).low = (LSP);              \
    (R).high = (MSP);             \
}                                 \
while(0)

#define RwMake128_32(R, MSP, HMP, LMP, LSP) \
do                                                                            \
{                                                                             \
    (R).low = (uint64_t)(uint32_t)(LSP) | ((uint64_t)(uint32_t)(LMP) << 32);  \
    (R).high = (uint64_t)(uint32_t)(HMP) | ((uint64_t)(uint32_t)(MSP) << 32); \
}                                                                             \
while (0)

#endif // defined(RWCROSS)
#endif /* !RW_HOSTTYPE_H */
