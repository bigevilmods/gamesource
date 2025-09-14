// (c) Electronic Arts. All Rights Reserved.

// The Wii has some optimizer bugs that are causing invalid code to be generated.  Some functions
// are being compiled "non-inlined" in this file to avoid the bugs.

#include "rw/math/detail/wii/aggressive_inlining.inc"

#include "rw/math/vpu/matrix44_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

namespace detail
{

    Vector4 Mult(Vector4::InParam v, Matrix44::InParam m) 
    {
        using namespace ::rw::math::ps::detail;
                
        Vector4 result;
        RW_MATH_PS_MultV4M44(result.mV.ps0, result.mV.ps1, v.mV.ps0, v.mV.ps1, 
            m.xAxis.mV.ps0, m.xAxis.mV.ps1, m.yAxis.mV.ps0, m.yAxis.mV.ps1, 
            m.zAxis.mV.ps0, m.zAxis.mV.ps1, m.wAxis.mV.ps0, m.wAxis.mV.ps1);
        return result;
    }

    Matrix44 Mult(Matrix44::InParam m, Matrix44::InParam b) 
    {
        Matrix44 result;
        using namespace ::rw::math::ps::detail;
        RW_MATH_PS_MultM44(result.xAxis.mV.ps0, result.xAxis.mV.ps1, result.yAxis.mV.ps0, result.yAxis.mV.ps1,
            result.zAxis.mV.ps0, result.zAxis.mV.ps1, result.wAxis.mV.ps0, result.wAxis.mV.ps1, 
            m.xAxis.mV.ps0, m.xAxis.mV.ps1, m.yAxis.mV.ps0, m.yAxis.mV.ps1, 
            m.zAxis.mV.ps0, m.zAxis.mV.ps1, m.wAxis.mV.ps0, m.wAxis.mV.ps1, 
            b.xAxis.mV.ps0, b.xAxis.mV.ps1, b.yAxis.mV.ps0, b.yAxis.mV.ps1, 
            b.zAxis.mV.ps0, b.zAxis.mV.ps1, b.wAxis.mV.ps0, b.wAxis.mV.ps1);
        return result;
    }
    void Matrix44Copy(Matrix44& a, const Matrix44& b)
    {
        using namespace ::rw::math::ps::detail;
        mw_ps x_ps0, x_ps1, y_ps0, y_ps1, z_ps0, z_ps1, w_ps0, w_ps1;
        x_ps0 = ps_load_0(&b.xAxis.mV.f[0]);
        x_ps1 = ps_load_0(&b.xAxis.mV.f[2]);
        y_ps0 = ps_load_0(&b.yAxis.mV.f[0]);
        y_ps1 = ps_load_0(&b.yAxis.mV.f[2]);
        z_ps0 = ps_load_0(&b.zAxis.mV.f[0]);
        z_ps1 = ps_load_0(&b.zAxis.mV.f[2]);
        w_ps0 = ps_load_0(&b.wAxis.mV.f[0]);
        w_ps1 = ps_load_0(&b.wAxis.mV.f[2]);

        a.xAxis.mV.ps0 = x_ps0;
        a.xAxis.mV.ps1 = x_ps1;
        a.yAxis.mV.ps0 = y_ps0;
        a.yAxis.mV.ps1 = y_ps1;
        a.zAxis.mV.ps0 = z_ps0;
        a.zAxis.mV.ps1 = z_ps1;
        a.wAxis.mV.ps0 = w_ps0;
        a.wAxis.mV.ps1 = w_ps1;
    }
}

} // namespace vpu
} // namespace math
} // namespace rw
