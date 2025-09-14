// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_PLANE_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_PLANE_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_intrinsic_operation.h"
#include "rw/math/vpu/trig.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/matrix44_operation.h"
#include "rw/math/vpu/vector3_operation.h"
#include "rw/math/vpu/vector4_operation.h"
#include "rw/math/vpu/detail/plane_operation_platform_inline.h"

namespace rw
{

namespace math
{

namespace vpu
{

RW_MATH_FORCE_INLINE bool operator ==(Plane::InParam a, Plane::InParam b)
{
    return  _asmIsEqualV4(a.mV, b.mV);
}

RW_MATH_FORCE_INLINE bool operator !=(Plane::InParam a, Plane::InParam b)
{
    return !(a==b);
}

RW_MATH_FORCE_INLINE bool IsSimilar(Plane::InParam a, Plane::InParam b, VecFloat::InParam epsilon)
{
    return _asmIsSimilarV4(a.mV, b.mV, epsilon.mV);
}

RW_MATH_FORCE_INLINE bool IsAnyNaN(Plane::InParam p) 
{
    bool equal;
    RW_MATH_VPU_IsEqualV4(equal, p.mV, p.mV);
    return !equal;
}

// basic functions

RW_MATH_FORCE_INLINE Plane PlaneFromPosNormal(Vector4::InParam pos, Vector4::InParam normal)
{
    return PlaneFromPosNormal(GetXYZ(pos), GetXYZ(normal));
}

RW_MATH_FORCE_INLINE Plane PlaneFromVertices(Vector3::InParam origin, Vector3::InParam vert1, Vector3::InParam vert2)
{
    Vector3 normal = Normalize( Cross(vert1-origin, vert2-origin) );
    return Plane( normal, -Dot(normal, origin) );
}

RW_MATH_FORCE_INLINE Plane PlaneFromVertices(Vector4::InParam origin, Vector4::InParam vert1, Vector4::InParam vert2)
{
    return PlaneFromVertices(GetXYZ(origin), GetXYZ(vert1), GetXYZ(vert2));
}

RW_MATH_FORCE_INLINE Plane PlaneFromVectors(Vector4::InParam origin, Vector4::InParam vec1, Vector4::InParam vec2)
{
    return PlaneFromVectors(GetXYZ(origin), GetXYZ(vec1), GetXYZ(vec2));
}

RW_MATH_FORCE_INLINE VecFloat Distance(Plane::InParam p, Vector3::InParam v) { return p.A() * v.X() + p.B() * v.Y() + p.C() * v.Z() + p.D(); }
RW_MATH_FORCE_INLINE VecFloat Distance(Plane::InParam p, Vector4::InParam v) { return p.A() * v.X() + p.B() * v.Y() + p.C() * v.Z() + p.D(); }
RW_MATH_FORCE_INLINE VecFloat Distance(Vector3::InParam v, Plane::InParam p) { return p.A() * v.X() + p.B() * v.Y() + p.C() * v.Z() + p.D(); }
RW_MATH_FORCE_INLINE VecFloat Distance(Vector4::InParam v, Plane::InParam p) { return p.A() * v.X() + p.B() * v.Y() + p.C() * v.Z() + p.D(); }

RW_MATH_FORCE_INLINE Mask4   CompLessThan(Plane::InParam a, Plane::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, b.mV, a.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompGreaterThan(Plane::InParam a, Plane::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreater( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompLessEqual(Plane::InParam a, Plane::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, b.mV, a.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompGreaterEqual(Plane::InParam a, Plane::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsGreaterEqual( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompEqual(Plane::InParam a, Plane::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Mask4   CompNotEqual(Plane::InParam a, Plane::InParam b)
{
    VectorIntrinsic result;
    RW_MATH_VPU_IsEqual( result, a.mV, b.mV );
    RW_MATH_VPU_Not( result, result );
    return Mask4(result);
}

RW_MATH_FORCE_INLINE Plane Select(MaskScalar::InParam mask, Plane::InParam trueValue, Plane::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Plane(result);
}

RW_MATH_FORCE_INLINE Plane Select(Mask4::InParam mask, Plane::InParam trueValue, Plane::InParam falseValue )
{
    VectorIntrinsic result;
    RW_MATH_VPU_Select( result, falseValue.mV, trueValue.mV, mask.mV );
    return Plane(result);
}

// functions return vector by reference
#if RW_MATH_ENABLE_DEPRECATED_API

RW_MATH_FORCE_INLINE void TransformPlane(Matrix44::InParam m, Plane::InParam p, Plane& result)
{
    result = TransformPlane(m, p);
}

#endif

}

}

}

#endif
