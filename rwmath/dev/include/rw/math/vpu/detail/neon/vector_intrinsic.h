// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CROSS_GENERIC_VECTOR_INTRINSIC_H
#define RW_MATH_VPU_DETAIL_CROSS_GENERIC_VECTOR_INTRINSIC_H

#include "rw/math/detail/base.h"

#include <arm_neon.h>

namespace rw {
namespace math {
namespace vpu {

    /*!
        \brief Platform specific vector intrinsic type
        
        This is the generic definition, on other platforms this type
        maps to hardware registers.
    */
    typedef float32x4_t VectorIntrinsic;

    // TODO test whether or not this should be passed by value.
    typedef const VectorIntrinsic& VectorIntrinsicInParam;

namespace detail
{
    extern const VectorIntrinsic gCosMinSlerpAngle;
}
}

namespace vpl
{
namespace detail
{
    using ::rw::math::vpu::VectorIntrinsic;

    extern const VectorIntrinsic gShiftLeftArray[16];
	extern const VectorIntrinsic gShiftRightArray[16];
}
}

}}


#endif
