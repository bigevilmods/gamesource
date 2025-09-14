// (c) Electronic Arts. All Rights Reserved.

// This function fails to execute properly when optimizations are disabled.
// I suspect this is due to a hardware bug when paired registers are not
// saved and restored properly.

#include "rw/math/detail/wii/aggressive_inlining.inc"

#include "rw/math/fpu/matrix44affine_operation.h"

namespace rw
{
namespace math
{
namespace fpu
{

    template<>
    Matrix44AffineTemplate<float> Mult(const Matrix44AffineTemplate<float> &m, const Matrix44AffineTemplate<float> &b) 
    {
        Matrix44Affine result;
        using namespace ::rw::math::ps::detail;
        mw_ps in_x_ps0 = ps_load_0(&b.xAxis.x);
        mw_ps v_ps0 = ps_load_0(&m.xAxis.x);
        mw_ps r_ps0 = ps_muls0( in_x_ps0, v_ps0 ); // 0
        mw_ps in_x_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(b.xAxis.z);
        mw_ps r_ps1 = ps_muls0( in_x_ps1, v_ps0 ); // 1
        mw_ps in_y_ps0 = ps_load_0(&b.yAxis.x);
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); // 2
        mw_ps in_y_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(b.yAxis.z);
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); // 3    
        mw_ps v_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(m.xAxis.z);
        mw_ps in_z_ps0 = ps_load_0(&b.zAxis.x);
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); // 4
        mw_ps in_z_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(b.zAxis.z);
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); // 5
        mw_ps in_w_ps0 = ps_load_0(&b.wAxis.x);
        mw_ps in_w_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(b.wAxis.z);
        ps_store_0( r_ps0, &result.xAxis.x );
        ps_store_as_float_0( r_ps1, &result.xAxis.z );

        v_ps0 = ps_load_0(&m.yAxis.x);
        r_ps0 = ps_muls0( in_x_ps0, v_ps0 ); // 0
        r_ps1 = ps_muls0( in_x_ps1, v_ps0 ); // 1
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); // 2
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); // 3    
        v_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(m.yAxis.z);
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); // 4
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); // 5
        ps_store_0( r_ps0, &result.yAxis.x );
        ps_store_as_float_0( r_ps1, &result.yAxis.z );

        v_ps0 = ps_load_0(&m.zAxis.x);
        r_ps0 = ps_muls0( in_x_ps0, v_ps0 ); // 0
        r_ps1 = ps_muls0( in_x_ps1, v_ps0 ); // 1
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); // 2
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); // 3    
        v_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(m.zAxis.z);
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); // 4
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); // 5
        ps_store_0( r_ps0, &result.zAxis.x );
        ps_store_as_float_0( r_ps1, &result.zAxis.z );

        v_ps0 = ps_load_0(&m.wAxis.x);
        r_ps0 = ps_madds0( in_x_ps0, v_ps0, in_w_ps0 ); // 0
        r_ps1 = ps_madds0( in_x_ps1, v_ps0, in_w_ps1 ); // 1
        r_ps0 = ps_madds1( in_y_ps0, v_ps0, r_ps0 ); // 2
        r_ps1 = ps_madds1( in_y_ps1, v_ps0, r_ps1 ); // 3    
        v_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(m.wAxis.z);
        r_ps0 = ps_madds0( in_z_ps0, v_ps1, r_ps0 ); // 4
        r_ps1 = ps_madds0( in_z_ps1, v_ps1, r_ps1 ); // 5
        ps_store_0( r_ps0, &result.wAxis.x );
        ps_store_as_float_0( r_ps1, &result.wAxis.z );

        return result;
    }

    template<>
    Matrix44AffineTemplate<float> Inverse(const Matrix44AffineTemplate<float>& m, float& determinant)
    {
        using namespace ::rw::math::ps::detail;
        Matrix44AffineTemplate<float> result;
        mw_ps in_x_ps0 = ps_load_0(&m.xAxis.x);
        mw_ps in_x_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(m.xAxis.z);
        mw_ps in_y_ps0 = ps_load_0(&m.yAxis.x);
        mw_ps in_y_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(m.yAxis.z);
        mw_ps in_z_ps0 = ps_load_0(&m.zAxis.x);
        mw_ps in_z_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(m.zAxis.z);
        mw_ps in_w_ps0 = ps_load_0(&m.wAxis.x);
        mw_ps in_w_ps1 = RW_MATH_PS_FROM_FLOAT_VAR(m.wAxis.z);
        mw_ps res_x_ps0, res_x_ps1, res_y_ps0, res_y_ps1, res_z_ps0, res_z_ps1, res_w_ps0, res_w_ps1, det_ps; \
        RW_MATH_PS_InverseA4V3(ret_d, res_x_ps0, res_x_ps1, res_y_ps0, res_y_ps1, res_z_ps0, res_z_ps1, res_w_ps0, res_w_ps1, in_x_ps0, in_x_ps1, in_y_ps0, in_y_ps1, in_z_ps0, in_z_ps1, in_w_ps0, in_w_ps1 ) \
        ps_store_0( res_x_ps0, &result.xAxis.x );
        ps_store_as_float_0( res_x_ps1, &result.xAxis.z );
        ps_store_0( res_y_ps0, &result.yAxis.x );
        ps_store_as_float_0( res_y_ps1, &result.yAxis.z );
        ps_store_0( res_z_ps0, &result.zAxis.x );
        ps_store_as_float_0( res_z_ps1, &result.zAxis.z );
        ps_store_0( res_w_ps0, &result.wAxis.x );
        ps_store_as_float_0( res_w_ps1, &result.wAxis.z );
        ps_store_as_float_0( det_ps, &determinant );
        return result;
    }
} // namespace fpu
} // namespace math
} // namespace rw
