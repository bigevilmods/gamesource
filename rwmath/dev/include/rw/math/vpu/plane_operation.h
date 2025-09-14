// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_PLANE_OPERATION_H
#define RW_MATH_VPU_PLANE_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/vpu/plane.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/vpu/vector4.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/maskscalar.h"
#include "rw/math/vpu/mask4.h"

namespace rw
{
namespace math
{
namespace vpu
{

    
    RW_MATH_FORCE_INLINE bool operator ==(Plane::InParam a, Plane::InParam b);    
    RW_MATH_FORCE_INLINE bool operator !=(Plane::InParam a, Plane::InParam b);
    
    RW_MATH_FORCE_INLINE bool IsSimilar(Plane::InParam a, Plane::InParam b, VecFloat::InParam epsilon = VecFloat(SMALL_FLOAT));
    RW_MATH_FORCE_INLINE bool IsAnyNaN(Plane::InParam p);
    
    RW_MATH_FORCE_INLINE Plane PlaneFromPosNormal(Vector3::InParam pos, Vector3::InParam normal);    
    RW_MATH_FORCE_INLINE Plane PlaneFromPosNormal(Vector4::InParam pos, Vector4::InParam normal);
    
    RW_MATH_FORCE_INLINE Plane PlaneFromVertices(Vector3::InParam origin, Vector3::InParam vert1, Vector3::InParam vert2);
    RW_MATH_FORCE_INLINE Plane PlaneFromVertices(Vector4::InParam origin, Vector4::InParam vert1, Vector4::InParam vert2);
    
    RW_MATH_FORCE_INLINE Plane PlaneFromVectors(Vector3::InParam origin,  Vector3::InParam vec1, Vector3::InParam vec2);
    RW_MATH_FORCE_INLINE Plane PlaneFromVectors(Vector4::InParam origin,  Vector4::InParam vec1, Vector4::InParam vec2);

    RW_MATH_FORCE_INLINE PlaneIntersection IntersectPlaneLine(Plane::InParam plane, Vector3::InParam p0, Vector3::InParam p1, Vector3& impact_out, VecFloat::InParam epsilon = GetVecFloat_Zero() );    
    RW_MATH_FORCE_INLINE PlaneIntersection IntersectPlaneDirectedSegment(Plane::InParam plane, Vector3::InParam p0, Vector3::InParam p1, Vector3& impact_out, VecFloat::InParam epsilon = GetVecFloat_Zero());
    
    RW_MATH_FORCE_INLINE Plane TransformPlane(Matrix44::InParam m, Plane::InParam p);
    
    RW_MATH_FORCE_INLINE VecFloat Distance(Plane::InParam p, Vector3::InParam v);
    RW_MATH_FORCE_INLINE VecFloat Distance(Plane::InParam p, Vector4::InParam v);
    RW_MATH_FORCE_INLINE VecFloat Distance(Vector3::InParam v, Plane::InParam p);
    RW_MATH_FORCE_INLINE VecFloat Distance(Vector4::InParam v, Plane::InParam p);
    
    RW_MATH_FORCE_INLINE Mask4 CompLessThan(Plane::InParam a, Plane::InParam b);
    RW_MATH_FORCE_INLINE Mask4 CompGreaterThan(Plane::InParam a, Plane::InParam b);
    RW_MATH_FORCE_INLINE Mask4 CompLessEqual(Plane::InParam a, Plane::InParam b);
    RW_MATH_FORCE_INLINE Mask4 CompGreaterEqual(Plane::InParam a, Plane::InParam b);
    RW_MATH_FORCE_INLINE Mask4 CompEqual(Plane::InParam a, Plane::InParam b);
    RW_MATH_FORCE_INLINE Mask4 CompNotEqual(Plane::InParam a, Plane::InParam b);
    RW_MATH_FORCE_INLINE Plane Select(MaskScalar::InParam mask, Plane::InParam trueValue, Plane::InParam falseValue );
    RW_MATH_FORCE_INLINE Plane Select(Mask4::InParam mask, Plane::InParam trueValue, Plane::InParam falseValue );

    #if RW_MATH_ENABLE_DEPRECATED_API
        RW_MATH_FORCE_INLINE void TransformPlane(Matrix44::InParam m, Plane::InParam p, Plane& result) ;
    #endif

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/plane_operation_inline.h"
#endif

#endif
