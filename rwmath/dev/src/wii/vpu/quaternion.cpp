// (c) Electronic Arts. All Rights Reserved.

// The Wii has some optimizer bugs that are causing invalid code to be generated.  Some functions
// are being compiled "non-inlined" in this file to avoid the bugs.
#include "rw/math/detail/wii/aggressive_inlining.inc"

#include "rw/math/vpu/matrix33_operation.h"
#include "rw/math/vpu/quaternion_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

namespace detail
{

    Quaternion QuaternionFromMatrix33(Matrix33::InParam m, VecFloat::InParam epsilon)
    {
      RW_MATH_ASSERT_MSG(IsRotation(m, VecFloat(0.01f)), ("The matrix provided must be a rotation matrix in order to convert it to a quaternion.") );
      using namespace ::rw::math::ps::detail;
      float one = 1.0f;
      float half = 0.5f;
      float neg_one = -1.0f;
      mw_ps xX = ps_merge00( m.xAxis.mV.ps0, m.xAxis.mV.ps0);
      mw_ps xY = ps_merge11( m.xAxis.mV.ps0, m.xAxis.mV.ps0);
      mw_ps xZ = ps_merge00( m.xAxis.mV.ps1, m.xAxis.mV.ps1);
      mw_ps yX = ps_merge00( m.yAxis.mV.ps0, m.yAxis.mV.ps0);
      mw_ps yY = ps_merge11( m.yAxis.mV.ps0, m.yAxis.mV.ps0);
      mw_ps yZ = ps_merge00( m.yAxis.mV.ps1, m.yAxis.mV.ps1);
      mw_ps zX = ps_merge00( m.zAxis.mV.ps0, m.zAxis.mV.ps0);
      mw_ps zY = ps_merge11( m.zAxis.mV.ps0, m.zAxis.mV.ps0);
      mw_ps zZ = ps_merge00( m.zAxis.mV.ps1, m.zAxis.mV.ps1);

      mw_ps tr = ps_add( xX, yY );
      tr = ps_add( tr, zZ );
    
      mw_ps sub_tr_epsilon = ps_sub( tr, epsilon.mV.ps0 );
      mw_ps xX_gt_yY = ps_sub( xX, yY );
      mw_ps xX_gt_zZ = ps_sub( xX, zZ );
      mw_ps yY_gt_zZ = ps_sub( yY, zZ );
      
      // result 'if (tr > epsilon)'
      mw_ps s;
      mw_ps tr_add_one = ps_add( tr, one );
      RW_MATH_PS_SquareRoot( s, tr_add_one);
      mw_ps s2 = ps_div( half, s );
      mw_ps q_mV_0 = ps_sub( yZ, zY );
      q_mV_0 = ps_mul( q_mV_0, s2 );
      mw_ps q_mV_1 = ps_sub( zX, xZ );
      q_mV_1 = ps_mul( q_mV_1, s2 );
      mw_ps q_mV_2 = ps_sub( xY, yX );
      q_mV_2 = ps_mul( q_mV_2, s2 );
      mw_ps q_mV_3 = ps_mul( half, s );
      mw_ps q_mV_ps0 = ps_merge00( q_mV_0, q_mV_1 );
      mw_ps q_mV_ps1 = ps_merge00( q_mV_2, q_mV_3 );
      
      // result 'if ((m.xAxis.X() > m.yAxis.Y()) && (m.xAxis.X() > m.zAxis.Z()))'
      mw_ps s_c1;
      mw_ps xX_sub_yY_add_zZ_add_one = ps_add( ps_sub( xX, ps_add( yY, zZ ) ), one );
      RW_MATH_PS_SquareRoot( s_c1, xX_sub_yY_add_zZ_add_one );
      mw_ps s2_c1 = ps_div( half, s_c1 );
      mw_ps q_mV_0_c1 = ps_mul( half, s_c1 );
      mw_ps q_mV_1_c1 = ps_add( yX, xY );
      q_mV_1_c1 = ps_mul( q_mV_1_c1, s2_c1 );
      mw_ps q_mV_2_c1 = ps_add( xZ, zX );
      q_mV_2_c1 = ps_mul( q_mV_2_c1, s2_c1 );
      mw_ps q_mV_3_c1 = ps_sub( yZ, zY );
      q_mV_3_c1 = ps_mul( q_mV_3_c1, s2_c1 );
      mw_ps q_mV_ps0_c1 = ps_merge00( q_mV_0_c1, q_mV_1_c1 );
      mw_ps q_mV_ps1_c1 = ps_merge00( q_mV_2_c1, q_mV_3_c1 );
      // result 'else if (m.yAxis.Y() > m.zAxis.Z())'
      mw_ps s_c2;
      mw_ps yY_sub_zZ_add_xX_add_one = ps_add( ps_sub( yY, ps_add( zZ, xX ) ), one );
      RW_MATH_PS_SquareRoot( s_c2, yY_sub_zZ_add_xX_add_one );
      mw_ps s2_c2 = ps_div( half, s_c2 );
      mw_ps q_mV_0_c2 = ps_add( yX, xY );
      q_mV_0_c2 = ps_mul( q_mV_0_c2, s2_c2 );
      mw_ps q_mV_1_c2 = ps_mul( half, s_c2 );
      mw_ps q_mV_2_c2 = ps_add( zY, yZ );
      q_mV_2_c2 = ps_mul( q_mV_2_c2, s2_c2 );
      mw_ps q_mV_3_c2 = ps_sub( zX, xZ );
      q_mV_3_c2 = ps_mul( q_mV_3_c2, s2_c2 );
      mw_ps q_mV_ps0_c2 = ps_merge00( q_mV_0_c2, q_mV_1_c2 );
      mw_ps q_mV_ps1_c2 = ps_merge00( q_mV_2_c2, q_mV_3_c2 );
      // else
      mw_ps s_c3;
      mw_ps zZ_sub_xX_add_yY_add_one = ps_add( ps_sub( zZ, ps_add( xX, yY ) ), one );
      RW_MATH_PS_SquareRoot( s_c3, zZ_sub_xX_add_yY_add_one );
      mw_ps s2_c3 = ps_div( half, s_c3 );
      mw_ps q_mV_0_c3 = ps_add( xZ, zX );
      q_mV_0_c3 = ps_mul( q_mV_0_c3, s2_c3 );
      mw_ps q_mV_1_c3 = ps_add( zY, yZ );
      q_mV_1_c3 = ps_mul( q_mV_1_c3, s2_c3 );
      mw_ps q_mV_2_c3 = ps_mul( half, s_c3 );
      mw_ps q_mV_3_c3 = ps_sub( xY, yX );
      q_mV_3_c3 = ps_mul( q_mV_3_c3, s2_c3 );
      mw_ps q_mV_ps0_c3 = ps_merge00( q_mV_0_c3, q_mV_1_c3 );
      mw_ps q_mV_ps1_c3 = ps_merge00( q_mV_2_c3, q_mV_3_c3 );
      
      mw_ps ret_temp_ps0 = ps_sel( yY_gt_zZ, q_mV_ps0_c2, q_mV_ps0_c3 );
      mw_ps ret_temp_ps1 = ps_sel( yY_gt_zZ, q_mV_ps1_c2, q_mV_ps1_c3 );
      mw_ps condition = ps_sel( xX_gt_yY, xX_gt_zZ, neg_one ); // simulate '&&'
      mw_ps ret_ps0 = ps_sel( condition, q_mV_ps0_c1, ret_temp_ps0 );
      mw_ps ret_ps1 = ps_sel( condition, q_mV_ps1_c1, ret_temp_ps1 );
      
      Quaternion result;
      result.mV.ps0 = ps_sel( sub_tr_epsilon, q_mV_ps0, ret_ps0 );
      result.mV.ps1 = ps_sel( sub_tr_epsilon, q_mV_ps1, ret_ps1 );
      
      return result;
    }

    Quaternion Lerp(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t) 
    {
        using namespace ::rw::math::ps::detail;
        Quaternion result;
        RW_MATH_PS_Lerp(result.mV.ps0, result.mV.ps1, q0.mV.ps0, q0.mV.ps1, q1.mV.ps0, q1.mV.ps1, t.mV.ps0, t.mV.ps1);
        return result;
    }

    Quaternion NLerpEstimate(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t)
    {
        using namespace ::rw::math::ps::detail;
        Quaternion result;
        RW_MATH_PS_NLerpEstimate(result.mV.ps0, result.mV.ps1, q0.mV.ps0, q0.mV.ps1, q1.mV.ps0, q1.mV.ps1, t.mV.ps0, t.mV.ps1);
        return result;
    }

    Quaternion NLerpFast(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t)
    {
        using namespace ::rw::math::ps::detail;
        Quaternion result;
        RW_MATH_PS_NLerpFast(result.mV.ps0, result.mV.ps1, q0.mV.ps0, q0.mV.ps1, q1.mV.ps0, q1.mV.ps1, t.mV.ps0, t.mV.ps1);
        return result;
    }

    Quaternion NLerp(Quaternion::InParam q0, Quaternion::InParam q1, VecFloat::InParam t)
    {
        using namespace ::rw::math::ps::detail;
        Quaternion result;
        RW_MATH_PS_NLerp(result.mV.ps0, result.mV.ps1, q0.mV.ps0, q0.mV.ps1, q1.mV.ps0, q1.mV.ps1, t.mV.ps0, t.mV.ps1);
        return result;
    }

    Quaternion NormalizeEstimate(Quaternion::InParam q)
    {
        using namespace ::rw::math::ps::detail;
        Quaternion result;
        RW_MATH_PS_NormalizeEstimate(result.mV.ps0, result.mV.ps1, q.mV.ps0, q.mV.ps1);
        return result;
    }
    Quaternion NormalizeFast(Quaternion::InParam q)
    {
        using namespace ::rw::math::ps::detail;
        Quaternion result;
        RW_MATH_PS_NormalizeFast(result.mV.ps0, result.mV.ps1, q.mV.ps0, q.mV.ps1);
        return result;
    }
    Quaternion Normalize(Quaternion::InParam q)
    {
        using namespace ::rw::math::ps::detail;
        Quaternion result;
        RW_MATH_PS_Normalize(result.mV.ps0, result.mV.ps1, q.mV.ps0, q.mV.ps1);
        return result;
    }
}

} // namespace vpu
} // namespace math
} // namespace rw
