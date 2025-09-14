// (c) Electronic Arts. All Rights Reserved.

// The Wii has some optimizer bugs that are causing invalid code to be generated.  Some functions
// are being compiled "non-inlined" in this file to avoid the bugs.

#include "rw/math/detail/wii/aggressive_inlining.inc"

#include "rw/math/vpu/matrix44affine_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

namespace detail
{
    Matrix44Affine Mult(Matrix44Affine::InParam m, Matrix44Affine::InParam b) 
    {
        Matrix44Affine result;
        using namespace ::rw::math::ps::detail;
        mw_ps in_x_ps0 = b.xAxis.mV.ps0;
        mw_ps v_ps0 = m.xAxis.mV.ps0;
        mw_ps r_ps0 = ps_muls0( in_x_ps0, v_ps0 ); // 0
        mw_ps in_x_ps1 = b.xAxis.mV.ps1;
        mw_ps r_ps1 = ps_muls0( in_x_ps1, v_ps0 ); // 1
        mw_ps in_y_ps0 = b.yAxis.mV.ps0;
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); // 2
        mw_ps in_y_ps1 = b.yAxis.mV.ps1;
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); // 3    
        mw_ps v_ps1 = m.xAxis.mV.ps1;
        mw_ps in_z_ps0 = b.zAxis.mV.ps0;
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); // 4
        mw_ps in_z_ps1 = b.zAxis.mV.ps1;
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); // 5
        mw_ps in_w_ps0 = b.wAxis.mV.ps0;
        mw_ps in_w_ps1 = b.wAxis.mV.ps1;
        result.xAxis.mV.ps0 = r_ps0;
        result.xAxis.mV.ps1 = r_ps1;

        v_ps0 = m.yAxis.mV.ps0;
        r_ps0 = ps_muls0( in_x_ps0, v_ps0 ); // 0
        r_ps1 = ps_muls0( in_x_ps1, v_ps0 ); // 1
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); // 2
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); // 3    
        v_ps1 = m.yAxis.mV.ps1;
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); // 4
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); // 5
        result.yAxis.mV.ps0 = r_ps0;
        result.yAxis.mV.ps1 = r_ps1;

        v_ps0 = m.zAxis.mV.ps0;
        r_ps0 = ps_muls0( in_x_ps0, v_ps0 ); // 0
        r_ps1 = ps_muls0( in_x_ps1, v_ps0 ); // 1
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); // 2
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); // 3    
        v_ps1 = m.zAxis.mV.ps1;
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); // 4
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); // 5
        result.zAxis.mV.ps0 = r_ps0;
        result.zAxis.mV.ps1 = r_ps1;

        v_ps0 = m.wAxis.mV.ps0;
        r_ps0 = ps_madds0( in_x_ps0, v_ps0, in_w_ps0 ); // 0
        r_ps1 = ps_madds0( in_x_ps1, v_ps0, in_w_ps1 ); // 1
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); // 2
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); // 3    
        v_ps1 = m.wAxis.mV.ps1;
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); // 4
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); // 5
        result.wAxis.mV.ps0 = r_ps0;
        result.wAxis.mV.ps1 = r_ps1;

        return result;
    }

    Matrix44Affine Inverse(Matrix44Affine::InParam m, VecFloat& determinant)
    {
        RW_MATH_ASSERT_MSG((Determinant(m)!= GetVecFloat_Zero()), ("Matrix is singular."));
        VectorIntrinsic det, xAxisResult, yAxisResult, zAxisResult, wAxisResult;
        RW_MATH_VPU_InverseA4V3( det, xAxisResult, yAxisResult, zAxisResult, wAxisResult, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, m.wAxis.mV );

        determinant.mV = det;
        return Matrix44Affine(xAxisResult, yAxisResult, zAxisResult, wAxisResult);
    }

} // namespace detail

} // namespace vpu
} // namespace math
} // namespace rw

