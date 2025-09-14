
#ifdef RWMATH_USE_NEON

#include "rw/math/detail/base.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw {
namespace math {
namespace vpu {
namespace detail {

    void TransposeA4V4(VectorIntrinsic &outX, VectorIntrinsic &outY, VectorIntrinsic &outZ, VectorIntrinsic &outW,
                       VectorIntrinsicInParam inX, VectorIntrinsicInParam inY, VectorIntrinsicInParam inZ, VectorIntrinsicInParam inW)
    {
        float32x4x2_t xy = vtrnq_f32( inX, inY ); 
        float32x4x2_t zw = vtrnq_f32( inZ, inW ); 
        
        outX = vcombine_f32( vget_low_f32(xy.val[0]), vget_low_f32(zw.val[0]) );
        outY = vcombine_f32( vget_low_f32(xy.val[1]), vget_low_f32(zw.val[1]) );
        outZ = vcombine_f32( vget_high_f32(xy.val[0]), vget_high_f32(zw.val[0]) );
        outW = vcombine_f32( vget_high_f32(xy.val[1]), vget_high_f32(zw.val[1]) );
    }

    bool FloatingPointCompareEqual(VectorIntrinsicInParam a, VectorIntrinsicInParam b)
    {
        bool equal;
        RW_MATH_VPU_IsEqualV4(equal, a, b);
        return equal;
    }

#if defined __llvm__ && defined RWMATH_USE_NEON

    VectorIntrinsic VPU_SquareRoot(VectorIntrinsicInParam in_vec)
    {
        const ::rw::math::vpu::VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
        ::rw::math::vpu::VectorIntrinsic isZeroMask;
        RW_MATH_VPU_IsEqual( isZeroMask, zero, in_vec );
        ::rw::math::vpu::VectorIntrinsic recipSqrt;
        RW_MATH_VPU_ReciprocalSquareRoot( recipSqrt, in_vec );
        ::rw::math::vpu::VectorIntrinsic result = RWMATH_VPU_DETAIL_VecMul(in_vec, recipSqrt );
        return RW_MATH_VPU_Select_R( result, zero, isZeroMask );
    }

    VectorIntrinsic VPU_SquareRootFast(VectorIntrinsicInParam in_vec)
    {
        const ::rw::math::vpu::VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
        ::rw::math::vpu::VectorIntrinsic isZeroMask;
        RW_MATH_VPU_IsEqual( isZeroMask, zero, in_vec );
        ::rw::math::vpu::VectorIntrinsic recipSqrt;
        RW_MATH_VPU_ReciprocalSquareRootFast( recipSqrt, in_vec );
        ::rw::math::vpu::VectorIntrinsic result;
        RW_MATH_VPU_Mult( result, in_vec, recipSqrt );
        return RW_MATH_VPU_Select_R( result, zero, isZeroMask );
    }

    VectorIntrinsic VPU_SquareRootEstimate(VectorIntrinsicInParam in_vec)
    {
        const ::rw::math::vpu::VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
        ::rw::math::vpu::VectorIntrinsic isZeroMask;
        RW_MATH_VPU_IsEqual( isZeroMask, zero, in_vec );
        ::rw::math::vpu::VectorIntrinsic recipSqrt;
        RW_MATH_VPU_ReciprocalSquareRootEstimate( recipSqrt, in_vec );
        ::rw::math::vpu::VectorIntrinsic result;
        RW_MATH_VPU_Mult( result, in_vec, recipSqrt );
        return RW_MATH_VPU_Select_R( result, zero, isZeroMask );
    }

#endif

}
}
}
}

#endif

