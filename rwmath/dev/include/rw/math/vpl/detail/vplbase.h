// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPLBASE_H
#define RW_MATH_VPLBASE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/vpu/vector_intrinsic.h"

///////////////////////////////////////////////////////////////////////////////
// Note: All the following defines are private, and should not be used by code
// outside the math library.
///////////////////////////////////////////////////////////////////////////////

// Define platforms
#define VPL_SSE      (1<<0)
#define VPL_PPU      (1<<1)
#define VPL_SPU      (1<<2)
#define VPL_XENON    (1<<3)
#define VPL_BROADWAY (1<<4)
#define VPL_SSE2     (1<<5)
#define VPL_NEON     (1<<6)

// Define "sets" that contain multiple platforms
#define VPL_ALL      (VPL_SSE | VPL_PPU | VPL_SPU | VPL_XENON | VPL_BROADWAY | VPL_SSE2 | VPL_NEON)
#define VPL_PS3      (VPL_PPU | VPL_SPU)
#define VPL_VMX      (VPL_PPU | VPL_XENON)
    
// Define the platform for the architecture being built

#if !defined( RW_MATH_TAG_API_DURING_PREPROCESSING )
    // Define away 'VPL_API' since it is just a token picked up by the api parsing tool.
    #define VPL_API
#else
    // Do not define VPL_API so that it still remains after
    // preprocessing.  API tools will then use the string "VPL_API" to extract
    // the API from the header files.
#endif

// Define VPL platform (based on platform defines)
#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION || defined RWCROSS
    // No operations are native on the reference implementation so use a bitmask of 0
    #define VPL_CURRENT_PLATFORM 0
#elif defined EA_PROCESSOR_ARM
    #ifdef RWMATH_USE_NEON
        #define VPL_CURRENT_PLATFORM VPL_NEON
    #else
        #define VPL_CURRENT_PLATFORM 0
    #endif
#elif defined EA_PLATFORM_PS3
    #define VPL_CURRENT_PLATFORM VPL_PPU
#elif defined EA_PLATFORM_PS3_SPU
    #define VPL_CURRENT_PLATFORM VPL_SPU
#elif defined EA_PLATFORM_XENON
    #define VPL_CURRENT_PLATFORM VPL_XENON
#elif EA_SSE > 0
    #if EA_SSE >= 2
        #define VPL_CURRENT_PLATFORM VPL_SSE2
    #elif EA_SSE == 1
        #define VPL_CURRENT_PLATFORM VPL_SSE
    #else
        #define VPL_CURRENT_PLATFORM 0
    #endif
#elif defined EA_PLATFORM_WINDOWS && ( defined(EA_PROCESSOR_X86) || defined(EA_PROCESSOR_X86_64) )
    // Even if EA_SSE is not defined, we assume that SSE is available on x86/x64 Windows.
    #define VPL_CURRENT_PLATFORM VPL_SSE
#elif defined EA_PLATFORM_REVOLUTION
    #define VPL_CURRENT_PLATFORM VPL_BROADWAY
#endif

// Define a macro for checking if the current platform exists in the mask.
#define VPL_SUPPORTED_PLATFORM( platformMask ) \
    ( (VPL_CURRENT_PLATFORM) & (platformMask) )

// Pull VectorIntrinsic typedef's into the vpl namespace for interoperability.
namespace rw
{
namespace math
{
namespace vpl
{
    using ::rw::math::vpu::VectorIntrinsic;
    using ::rw::math::vpu::VectorIntrinsicInParam;

#if VPL_SUPPORTED_PLATFORM ( VPL_SSE2 )

    union VectorIntrinsicSSEUnion
    {
        __m128i vectorSSE2Int;
        __m128d vectorSSE2Dbl;
        ::rw::math::vpu::VectorIntrinsic vector;
    };

#endif

} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_VPLBASE_H
