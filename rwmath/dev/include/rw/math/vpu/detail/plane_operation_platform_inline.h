// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_PLANE_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_PLANE_OPERATION_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"
#include "rw/math/vpu/trig.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/matrix44_operation.h"
#include "rw/math/vpu/vector3_operation.h"
#include "rw/math/vpu/vector4_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Plane PlaneFromPosNormal(Vector3::InParam pos, Vector3::InParam normal)
{
    return Plane( normal, -Dot(normal, pos) );
}

RW_MATH_FORCE_INLINE Plane PlaneFromVectors(Vector3::InParam origin, Vector3::InParam vec1, Vector3::InParam vec2)
{
    Vector3 normal = Normalize( Cross(vec1, vec2) );
    return Plane( normal, -Dot(normal, origin) );
}

RW_MATH_FORCE_INLINE PlaneIntersection IntersectPlaneLine(Plane::InParam plane, Vector3::InParam p0, Vector3::InParam p1, Vector3& impact_out, VecFloat::InParam epsilon )
{
    Vector3 dif = p1-p0;
    VecFloat dn = Dot(plane.GetNormal(),dif);
    if (Abs(dn) <= epsilon)
    {
        // Check to see if the line lies on the plane.  If so, there are an infinite number of intersections.
        // Compute: testValue = a * p0.x + b * p0.y + c * p0.z + d  
        const Vector4 testVector1(plane.mV);
        Vector4 testVector2( p0.mV );
        testVector2.W() =1.0f;
        VecFloat testValue = Dot(testVector1, testVector2);
        if( Abs(testValue) <= epsilon )
        {
            return PI_INFINITY;
        }
        else
        {
            // no intersection because line is parallel to plane
            return PI_ZERO;
        }
    }
    VecFloat t = Negate(Add(plane.SignedDistance(), Dot(plane.GetNormal(),p0)))/dn;
    impact_out = p0 + (dif*t);
    // denote that intersection occurred
    return PI_ONE;
}

RW_MATH_FORCE_INLINE PlaneIntersection IntersectPlaneDirectedSegment(Plane::InParam plane, Vector3::InParam p0, Vector3::InParam p1, Vector3& impact_out, VecFloat::InParam epsilon )
{
    Vector3 dif = p1-p0;
    VecFloat dn = Dot(plane.GetNormal(),dif);
    if (Abs(dn) <= epsilon)
    {
        // Check to see if the line lies on the plane.  If so, there are an infinite number of intersections.
        // Compute: testValue = a * p0.x + b * p0.y + c * p0.z + d  
        const Vector4 testVector1(plane.mV);
        Vector4 testVector2( p0.mV );
        testVector2.W() =1.0f;
        VecFloat testValue = Dot(testVector1, testVector2);
        if( Abs(testValue) <= epsilon )
        {
            return PI_INFINITY;
        }
        else
        {
            // no intersection because line is parallel to plane
            return PI_ZERO;
        }
    }
    VecFloat t = Negate(Add(plane.SignedDistance(), Dot(plane.GetNormal(),p0)))/dn;
    if (t < GetVecFloat_Zero() || t > GetVecFloat_One())
    {
        // no intersection w/ line segment
        return PI_ZERO;
    }
    impact_out = p0 + (dif*t);
    // denote that intersection occurred
    return PI_ONE;
}

RW_MATH_FORCE_INLINE Plane TransformPlane(Matrix44::InParam m, Plane::InParam p)
{
    RW_MATH_ASSERT_MSG(Abs(Magnitude(p.GetNormal()) - GetVecFloat_One()) <  VecFloat(SMALL_FLOAT), ("normal of plane is not of Unit length") );

    Vector3 normal = Mult3x3(p.GetNormal(),m);
    VecFloat d = Subtract(p.SignedDistance(), Dot(GetXYZ(m.GetRow(3)),normal));

    return Plane(normal, d);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
