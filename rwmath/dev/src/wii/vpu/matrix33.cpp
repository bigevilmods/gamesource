// (c) Electronic Arts. All Rights Reserved.

// The Wii has some optimizer bugs that are causing invalid code to be generated.  Some functions
// are being compiled "non-inlined" in this file to avoid the bugs.

#include "rw/math/detail/wii/aggressive_inlining.inc"

#include "rw/math/fpu/matrix33_operation.h"
#include "rw/math/vpu/matrix33_operation.h"

#include <stdio.h>

namespace rw
{
namespace math
{
namespace vpu
{

namespace detail
{

Matrix33 OrthoNormalize(Matrix33::InParam matrix)
{
    // Workaround a compiler bug by calling the fpu function
    return Matrix33(
            rw::math::fpu::OrthoNormalize( rw::math::fpu::Matrix33(matrix) )
            );
}

Vector3 Mult(Vector3::InParam v, Matrix33::InParam m) 
{
    VectorIntrinsic tmp = v.mV;
    VectorIntrinsic xSplat, ySplat, zSplat, result;

    RW_MATH_VPU_Splat_I( xSplat, tmp, 0 );
    RW_MATH_VPU_Splat_I( ySplat, tmp, 1 );
    RW_MATH_VPU_Splat_I( zSplat, tmp, 2 );

       RW_MATH_VPU_Mult( result, m.xAxis.mV, xSplat );
    RW_MATH_VPU_MultAdd( result, m.yAxis.mV, ySplat, result );
    RW_MATH_VPU_MultAdd( result, m.zAxis.mV, zSplat, result );

    return Vector3(result);
}

Matrix33 Mult(Matrix33::InParam m, Matrix33::InParam b) 
{
    VectorIntrinsic bX = b.xAxis.mV;
    VectorIntrinsic bY = b.yAxis.mV;
    VectorIntrinsic bZ = b.zAxis.mV;
    VectorIntrinsic row0, row1, row2;

    RW_MATH_VPU_TransformVectorV3(row0, m.xAxis.mV, bX, bY, bZ);
    RW_MATH_VPU_TransformVectorV3(row1, m.yAxis.mV, bX, bY, bZ);
    RW_MATH_VPU_TransformVectorV3(row2, m.zAxis.mV, bX, bY, bZ);
    return Matrix33(row0, row1, row2);
}

VecFloat GetNormalError(Matrix33::InParam matrix) 
{
    float x_error = MagnitudeSquared( matrix.xAxis ) - 1.0f;
    float y_error = MagnitudeSquared( matrix.yAxis ) - 1.0f;
    float z_error = MagnitudeSquared( matrix.zAxis ) - 1.0f;
    VecFloat error = x_error * x_error + y_error * y_error + z_error * z_error;
    return error;
}

} // namespace detail

} // namespace vpu
} // namespace math
} // namespace rw
