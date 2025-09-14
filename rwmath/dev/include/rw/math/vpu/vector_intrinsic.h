// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_VECTOR_INTRINSIC_H
#define RW_MATH_VPU_VECTOR_INTRINSIC_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#ifndef RW_MATH_TAG_API_DURING_PREPROCESSING

namespace rw
{
namespace math
{
namespace vpl
{
namespace detail
{
    struct FourUnsignedInt // for cross build and PC build
    {
        uint32_t a;
        uint32_t b;
        uint32_t c;
        uint32_t d;
    };
}}}}

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
    #include "rw/math/vpu/detail/cross/vector_intrinsic.h"
#elif defined RWCROSS 
	#include "rw/math/vpu/detail/cross/vector_intrinsic.h"
#elif defined RWMATH_USE_NEON
    #include "rw/math/vpu/detail/neon/vector_intrinsic.h"
#elif defined EA_PROCESSOR_ARM
	#include "rw/math/vpu/detail/cross/vector_intrinsic.h"
#elif defined EA_PLATFORM_REVOLUTION
    #include "rw/math/vpu/detail/wii/vector_intrinsic.h"
#elif defined EA_PLATFORM_PS3
    #include "rw/math/vpu/detail/ps3/ppu/vector_intrinsic.h"
#elif defined EA_PLATFORM_PS3_SPU
    #include "rw/math/vpu/detail/ps3/spu/vector_intrinsic.h"
#elif defined EA_PLATFORM_XENON
    #include "rw/math/vpu/detail/xbox2/vector_intrinsic.h"
#elif defined RW_MATH_TARGET_SUPPORTS_SSE
    #include "rw/math/vpu/detail/x86/vector_intrinsic.h"
#elif defined EA_PLATFORM_POSIX
    #include "rw/math/vpu/detail/cross/vector_intrinsic.h"
#elif defined EA_PLATFORM_AIRPLAY
    #include "rw/math/vpu/detail/cross/vector_intrinsic.h"
#else
    #error Unsupported platform
#endif

#endif

namespace rw
{
namespace math
{
namespace vpu
{

    #if RW_MATH_ENABLE_DEPRECATED_API
        //! \internal
        typedef VectorIntrinsic Vector4Intrinsic_32;
                
        //! \internal
        typedef VectorIntrinsicInParam Vector4Intrinsic_32_InParam;
    #endif
    
    //! \internal
    namespace detail
    {
        //! \internal
        union VectorIntrinsicUnion
        {
            uint32_t          scalarUInt[ 4];
            int32_t            scalarInt[ 4];
            float            scalarFloat[ 4];
            uint16_t        scalarUShort[ 8];
            int16_t          scalarShort[ 8];
            uint8_t          scalarUChar[16];
            int8_t            scalarChar[16];
            VectorIntrinsic       vector;
        };

        #define RW_MATH_DETAIL_VECFLOAT_APPLY_UNARY(out, in, func) \
            { \
                rw::math::vpu::detail::VectorIntrinsicUnion inU, outU; \
                inU.vector = in; \
                outU.scalarFloat[0] = func( inU.scalarFloat[0] ); \
                outU.scalarFloat[1] = func( inU.scalarFloat[1] ); \
                outU.scalarFloat[2] = func( inU.scalarFloat[2] ); \
                outU.scalarFloat[3] = func( inU.scalarFloat[3] ); \
                out = outU.vector; \
            }
                
        #define RW_MATH_DETAIL_VECFLOAT_APPLY_BINARY(out, in0, in1, func) \
            { \
                rw::math::vpu::detail::VectorIntrinsicUnion in0U, in1U, outU; \
                in0U.vector = in0; \
                in1U.vector = in1; \
                outU.scalarFloat[0] = func( in0U.scalarFloat[0], in1U.scalarFloat[0] ); \
                outU.scalarFloat[1] = func( in0U.scalarFloat[1], in1U.scalarFloat[1] ); \
                outU.scalarFloat[2] = func( in0U.scalarFloat[2], in1U.scalarFloat[2] ); \
                outU.scalarFloat[3] = func( in0U.scalarFloat[3], in1U.scalarFloat[3] ); \
                out = outU.vector; \
            }
    
    } // namespace detail

} // namespace vpu
} // namespace math
} // namespace rw

#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
void RW_MATH_VPU_MEMBER_INIT_FUNCTION(rw::math::vpu::VectorIntrinsic& vi);
#endif

#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2
void RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2(rw::math::vpu::VectorIntrinsicInParam vi);
#endif

#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_3
void RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_3(rw::math::vpu::VectorIntrinsicInParam vi);
#endif

#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
void RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(rw::math::vpu::VectorIntrinsicInParam vi);
#endif

#endif
