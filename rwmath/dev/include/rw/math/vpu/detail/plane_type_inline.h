// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_PLANE_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_PLANE_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/scalar.h"

#include "rw/math/fpu/plane.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"

namespace rw
{

namespace math
{

namespace vpu
{

RW_MATH_FORCE_INLINE  Plane& Plane::operator = (Plane::InParam v) 
{
    rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
    mV = v.mV;
    return *this;
}

#if RW_MATH_ENABLE_DEPRECATED_API
    RW_MATH_FORCE_INLINE  Plane& Plane::operator = (VectorIntrinsicInParam v) 
    {
        rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        mV = v;
        return *this;
    }

    RW_MATH_FORCE_INLINE  Plane::operator VectorIntrinsic&() 
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }

    RW_MATH_FORCE_INLINE  Plane::operator const VectorIntrinsic&() const 
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }
#endif

RW_MATH_FORCE_INLINE  void Plane::SetVector(VectorIntrinsicInParam v) 
{
    rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    mV = v;
}

RW_MATH_FORCE_INLINE  VectorIntrinsic& Plane::GetVector() 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE  const VectorIntrinsic& Plane::GetVector() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE  void Plane::Set(float a, float b, float c, float d) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, a, b, c, d );
}

RW_MATH_FORCE_INLINE  void Plane::Set(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam c, VecFloat::InParam d) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, a.mV, b.mV, c.mV, d.mV);
}

RW_MATH_FORCE_INLINE  void Plane::Set(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b, VecFloatRefIndex::InParam c, VecFloatRefIndex::InParam d) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic aV, bV, cV, dV;
    RW_MATH_VPU_Splat( aV, a.mV, a.mIndex );
    RW_MATH_VPU_Splat( bV, b.mV, b.mIndex );
    RW_MATH_VPU_Splat( cV, c.mV, c.mIndex );
    RW_MATH_VPU_Splat( dV, d.mV, d.mIndex );
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, aV, bV, cV, dV);
}

template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
void Plane::Set(VecFloatRef<XSRC_INDEX> a, VecFloatRef<YSRC_INDEX> b, VecFloatRef<ZSRC_INDEX> c, VecFloatRef<WSRC_INDEX> d)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorAxisA4_I(mV, a.mV, XSRC_INDEX, b.mV, YSRC_INDEX, c.mV, ZSRC_INDEX, d.mV, WSRC_INDEX);    
}

RW_MATH_FORCE_INLINE  void Plane::SetNormal(Vector3::InParam v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, v.mV, VectorAxisW, mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE  void Plane::SetComponent(int i, float value) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    // copy to temp to avoid aliasing, should be optimized out in release mode on platform/compiler
    // that doesn't have aliasing issue.
    detail::VectorIntrinsicUnion temp;
    temp.vector = mV;
    temp.scalarFloat[i] = value;
    mV = temp.vector;
}

RW_MATH_FORCE_INLINE  void Plane::SetComponent(int i, VecFloat::InParam value) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE  void Plane::SetComponent(int i, VecFloatRefIndex::InParam value) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_V( mV, mV, i, value.mV, value.mIndex );
}

template<VectorAxis INDEX>
void Plane::SetComponent(int i, VecFloatRef<INDEX> value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, INDEX );
}


RW_MATH_FORCE_INLINE  void Plane::SetA(float a) 
{
    SetComponent(0, a);
}

RW_MATH_FORCE_INLINE  void Plane::SetA(VecFloat::InParam a) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, a.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE  void Plane::SetA(VecFloatRefIndex::InParam a) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisX, a.mV, a.mIndex );
}

template<VectorAxis INDEX> void Plane::SetA(VecFloatRef<INDEX> a)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, a.mV, INDEX );
}

RW_MATH_FORCE_INLINE  void Plane::SetB(float b) 
{
    SetComponent(1, b);
}

RW_MATH_FORCE_INLINE  void Plane::SetB(VecFloat::InParam b) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, b.mV, VectorAxisY );
}

RW_MATH_FORCE_INLINE  void Plane::SetB(VecFloatRefIndex::InParam b) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisY, b.mV, b.mIndex );
}

template<VectorAxis INDEX> void Plane::SetB(VecFloatRef<INDEX> b)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, b.mV, INDEX );
}

RW_MATH_FORCE_INLINE  void Plane::SetC(float c) 
{
    SetComponent(2, c);
}

RW_MATH_FORCE_INLINE  void Plane::SetC(VecFloat::InParam c) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, c.mV, VectorAxisZ );
}

RW_MATH_FORCE_INLINE  void Plane::SetC(VecFloatRefIndex::InParam c) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisZ, c.mV, c.mIndex );
}

template<VectorAxis INDEX> void Plane::SetC(VecFloatRef<INDEX> c)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, c.mV, INDEX );
}

RW_MATH_FORCE_INLINE  void Plane::SetD(float d) 
{
    SetComponent(3, d);
}

RW_MATH_FORCE_INLINE  void Plane::SetD(VecFloat::InParam d) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, d.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE  void Plane::SetD(VecFloatRefIndex::InParam d) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisW, d.mV, d.mIndex );
}

template<VectorAxis INDEX> void Plane::SetD(VecFloatRef<INDEX> d)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, d.mV, INDEX );
}

RW_MATH_FORCE_INLINE  void Plane::A(float a) 
{
    SetComponent(0, a);
}

RW_MATH_FORCE_INLINE  void Plane::A(VecFloat::InParam a) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, a.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE  void Plane::A(VecFloatRefIndex::InParam a) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisX, a.mV, a.mIndex );
}

template<VectorAxis INDEX> void Plane::A(VecFloatRef<INDEX> a)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, a.mV, INDEX );
}

RW_MATH_FORCE_INLINE  void Plane::B(float b) 
{
    SetComponent(1, b);
}

RW_MATH_FORCE_INLINE  void Plane::B(VecFloat::InParam b) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, b.mV, VectorAxisY );
}

RW_MATH_FORCE_INLINE  void Plane::B(VecFloatRefIndex::InParam b) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisY, b.mV, b.mIndex );
}

template<VectorAxis INDEX> void Plane::B(VecFloatRef<INDEX> b)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, b.mV, INDEX );
}

RW_MATH_FORCE_INLINE  void Plane::C(float c) 
{
    SetComponent(2, c);
}

RW_MATH_FORCE_INLINE  void Plane::C(VecFloat::InParam c) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, c.mV, VectorAxisZ );
}

RW_MATH_FORCE_INLINE  void Plane::C(VecFloatRefIndex::InParam c) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisZ, c.mV, c.mIndex );
}

template<VectorAxis INDEX> void Plane::C(VecFloatRef<INDEX> c)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, c.mV, INDEX );
}

RW_MATH_FORCE_INLINE  void Plane::D(float d) 
{
    SetComponent(3, d);
}

RW_MATH_FORCE_INLINE  void Plane::D(VecFloat::InParam d) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, d.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE  void Plane::D(VecFloatRefIndex::InParam d) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisW, d.mV, d.mIndex );
}

template<VectorAxis INDEX> void Plane::D(VecFloatRef<INDEX> d)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, d.mV, INDEX );
}

RW_MATH_FORCE_INLINE  void Plane::SetZero() 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_GetZero(mV);
}

RW_MATH_FORCE_INLINE  void Plane::Set(Vector3::InParam normal, VecFloat::InParam d) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    mV = normal.mV;
    this->D() = d;
}

RW_MATH_FORCE_INLINE  VecFloatRefX Plane::A() 
{
    return VecFloatRefX(mV);
}

RW_MATH_FORCE_INLINE  const VecFloatRefX Plane::A() const 
{
    return VecFloatRefX(const_cast<Plane*>(this)->mV);
}

RW_MATH_FORCE_INLINE  VecFloatRefY Plane::B() 
{
    return VecFloatRefY(mV);
}

RW_MATH_FORCE_INLINE  const VecFloatRefY Plane::B() const 
{
    return VecFloatRefY(const_cast<Plane*>(this)->mV);
}

RW_MATH_FORCE_INLINE  VecFloatRefZ Plane::C() 
{
    return VecFloatRefZ(mV);
}

RW_MATH_FORCE_INLINE  const VecFloatRefZ Plane::C() const 
{
    return VecFloatRefZ(const_cast<Plane*>(this)->mV);
}

RW_MATH_FORCE_INLINE  VecFloatRefW Plane::D() 
{
    return VecFloatRefW(mV);
}

RW_MATH_FORCE_INLINE  const VecFloatRefW Plane::D() const 
{
    return VecFloatRefW(const_cast<Plane*>(this)->mV);
}

RW_MATH_FORCE_INLINE  VecFloatRefX Plane::GetA() 
{
    return VecFloatRefX(mV);
}

RW_MATH_FORCE_INLINE  const VecFloatRefX Plane::GetA() const 
{
    return VecFloatRefX(const_cast<Plane*>(this)->mV);
}

RW_MATH_FORCE_INLINE  VecFloatRefY Plane::GetB() 
{
    return VecFloatRefY(mV);
}

RW_MATH_FORCE_INLINE  const VecFloatRefY Plane::GetB() const 
{
    return VecFloatRefY(const_cast<Plane*>(this)->mV);
}

RW_MATH_FORCE_INLINE  VecFloatRefZ Plane::GetC() 
{
    return VecFloatRefZ(mV);
}

RW_MATH_FORCE_INLINE  const VecFloatRefZ Plane::GetC() const 
{
    return VecFloatRefZ(const_cast<Plane*>(this)->mV);
}

RW_MATH_FORCE_INLINE  VecFloatRefW Plane::GetD() 
{
    return VecFloatRefW(mV);
}

RW_MATH_FORCE_INLINE  const VecFloatRefW Plane::GetD() const 
{
    return VecFloatRefW(const_cast<Plane*>(this)->mV);
}

RW_MATH_FORCE_INLINE  VecFloatRefIndex Plane::GetComponent(int i) 
{
    return VecFloatRefIndex(mV, i);
}

RW_MATH_FORCE_INLINE  const VecFloatRefIndex Plane::GetComponent(int i) const 
{
    return VecFloatRefIndex(const_cast<Plane*>(this)->mV, i);
}

RW_MATH_FORCE_INLINE  VecFloatRefIndex Plane::operator [] (int i) 
{
    return VecFloatRefIndex(mV, i);
}

RW_MATH_FORCE_INLINE  const VecFloatRefIndex Plane::operator [] (int i) const 
{
    return VecFloatRefIndex(const_cast<Plane*>(this)->mV, i);
}

RW_MATH_FORCE_INLINE  Vector3 Plane::GetNormal() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return Vector3(mV);
}

RW_MATH_FORCE_INLINE  VecFloatRefW Plane::SignedDistance() 
{
    return VecFloatRefW(mV);
}

RW_MATH_FORCE_INLINE  const VecFloatRefW Plane::SignedDistance() const 
{
    return D();
}

RW_MATH_FORCE_INLINE  VecFloat Plane::Distance() const 
{
    return Abs(VecFloatRefW(const_cast<Plane*>(this)->mV));
}

template<class T>
RW_MATH_FORCE_INLINE Plane::Plane(const fpu::PlaneTemplate<T> &v) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, static_cast<float>(v.a), static_cast<float>(v.b), static_cast<float>(v.c), static_cast<float>(v.d));
}

RW_MATH_FORCE_INLINE  Plane::Plane() 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

RW_MATH_FORCE_INLINE  Plane::Plane(Vector3::InParam normal, VecFloat::InParam d) 
:mV(normal.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    this->D() = d;
}

RW_MATH_FORCE_INLINE  Plane::Plane(const float abcd[4]) 
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    :mV(RW_MATH_VPU_LoadFromAlignedFloatPointer(abcd))
#endif
{
    // for alignment check, we must check before we do an assignment
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*abcd, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = RW_MATH_VPU_LoadFromAlignedFloatPointer(abcd);
#endif 
}

RW_MATH_FORCE_INLINE  Plane::Plane(VectorIntrinsicInParam v) 
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    :mV(v)
#endif
{
    // for alignment check, we must check before we do an assignment
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = v;
#endif 
}

RW_MATH_FORCE_INLINE  Plane::Plane(float a, float b, float c, float d) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, a, b, c, d);
}

RW_MATH_FORCE_INLINE  Plane::Plane(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam c, VecFloat::InParam d) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, a.mV, b.mV, c.mV, d.mV);
}

RW_MATH_FORCE_INLINE  Plane::Plane(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b, VecFloatRefIndex::InParam c, VecFloatRefIndex::InParam d) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic aV, bV, cV, dV;
    RW_MATH_VPU_Splat( aV, a.mV, a.mIndex );
    RW_MATH_VPU_Splat( bV, b.mV, b.mIndex );
    RW_MATH_VPU_Splat( cV, c.mV, c.mIndex );
    RW_MATH_VPU_Splat( dV, d.mV, d.mIndex );
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, aV, bV, cV, dV);
}

template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
RW_MATH_FORCE_INLINE  Plane::Plane(VecFloatRef<XSRC_INDEX> a, VecFloatRef<YSRC_INDEX> b, VecFloatRef<ZSRC_INDEX> c, VecFloatRef<WSRC_INDEX> d) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorAxisA4_I(mV, a.mV, XSRC_INDEX, b.mV, YSRC_INDEX, c.mV, ZSRC_INDEX, d.mV, WSRC_INDEX);    
}

RW_MATH_FORCE_INLINE Plane::Plane(const Plane &rhs) 
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
: mV( rhs.mV )
#endif
{
    // for alignment check, we must check before we do assignment
    rwMathAlignmentConstructorCheckAssert(rhs.mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(rhs.mV);
#endif
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = rhs.mV;
#endif 
}

}

}

RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Plane& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::vpu::Plane), sizeof(float));
}

}

#endif
