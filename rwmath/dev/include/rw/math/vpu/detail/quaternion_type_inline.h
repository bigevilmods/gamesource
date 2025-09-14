// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_QUATERNION_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_QUATERNION_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"

#include "rw/math/fpu/quaternion.h"

namespace rw
{

namespace math
{

namespace vpu
{

RW_MATH_FORCE_INLINE Quaternion::Quaternion( VecFloat::InParam x, VecFloat::InParam y, VecFloat::InParam z, VecFloat::InParam w ) 
    : mV(_asmCreateVectorAxis<VectorAxisX, VectorAxisY, VectorAxisZ, VectorAxisW>(
        x.mV, y.mV, z.mV, w.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Quaternion::Quaternion(VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y, VecFloatRefIndex::InParam z, VecFloatRefIndex::InParam w) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic xV, yV, zV, wV;
    RW_MATH_VPU_Splat( xV, x.mV, x.mIndex );
    RW_MATH_VPU_Splat( yV, y.mV, y.mIndex );
    RW_MATH_VPU_Splat( zV, z.mV, z.mIndex );
    RW_MATH_VPU_Splat( wV, w.mV, w.mIndex );
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, xV, yV, zV, wV);
}

template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
RW_MATH_FORCE_INLINE Quaternion::Quaternion(VecFloatRef<XSRC_INDEX> x, VecFloatRef<YSRC_INDEX> y, VecFloatRef<ZSRC_INDEX> z, VecFloatRef<WSRC_INDEX> w) 
    : mV(_asmCreateVectorAxis<XSRC_INDEX, YSRC_INDEX, ZSRC_INDEX, WSRC_INDEX>(
        x.mV, y.mV, z.mV, w.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Quaternion::Quaternion( Vector3::InParam v3, VecFloatRefW::InParam w ) 
    :mV(v3.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, w.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE Quaternion::Quaternion( Vector3::InParam v3, VecFloatRefIndex::InParam w ) 
    :mV(v3.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisW, w.mV, w.mIndex );
}

RW_MATH_FORCE_INLINE Quaternion::Quaternion( Vector3::InParam v3, VecFloat::InParam w ) 
    :mV(v3.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, w.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Quaternion::Set(VecFloat::InParam x, VecFloat::InParam y, VecFloat::InParam z, VecFloat::InParam w) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, x.mV, y.mV, z.mV, w.mV);
}

RW_MATH_FORCE_INLINE void Quaternion::Set(VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y, VecFloatRefIndex::InParam z, VecFloatRefIndex::InParam w) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic xV, yV, zV, wV;
    RW_MATH_VPU_Splat( xV, x.mV, x.mIndex );
    RW_MATH_VPU_Splat( yV, y.mV, y.mIndex );
    RW_MATH_VPU_Splat( zV, z.mV, z.mIndex );
    RW_MATH_VPU_Splat( wV, w.mV, w.mIndex );
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, xV, yV, zV, wV);
}

template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
RW_MATH_FORCE_INLINE void Quaternion::Set(VecFloatRef<XSRC_INDEX> x, VecFloatRef<YSRC_INDEX> y, VecFloatRef<ZSRC_INDEX> z, VecFloatRef<WSRC_INDEX> w) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorAxisA4_I(mV, x.mV, XSRC_INDEX, y.mV, YSRC_INDEX, z.mV, ZSRC_INDEX, w.mV, WSRC_INDEX);    
}

RW_MATH_FORCE_INLINE void Quaternion::SetComponent(int i, float value) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    detail::VectorIntrinsicUnion temp;
    temp.vector = mV;
    temp.scalarFloat[i] = value;
    mV = temp.vector;
}

RW_MATH_FORCE_INLINE void Quaternion::SetComponent(int i, VecFloat::InParam value) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Quaternion::SetComponent(int i, VecFloatRefIndex::InParam value) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_V( mV, mV, i, value.mV, value.mIndex );
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE void Quaternion::SetComponent(int i, VecFloatRef<INDEX> value) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Quaternion::SetX(float x) 
{
    SetComponent(0, x);
}

RW_MATH_FORCE_INLINE void Quaternion::SetX(VecFloat::InParam x) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Quaternion::SetX(VecFloatRefIndex::InParam x) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisX, x.mV, x.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Quaternion::SetX(VecFloatRef<INDEX> x) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Quaternion::SetY(float y) 
{
    SetComponent(1, y);
}

RW_MATH_FORCE_INLINE void Quaternion::SetY(VecFloat::InParam y) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, VectorAxisY );
}

RW_MATH_FORCE_INLINE void Quaternion::SetY(VecFloatRefIndex::InParam y) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisY, y.mV, y.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Quaternion::SetY(VecFloatRef<INDEX> y) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Quaternion::SetZ(float z) 
{
    SetComponent(2, z);
}

RW_MATH_FORCE_INLINE void Quaternion::SetZ(VecFloat::InParam z) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, z.mV, VectorAxisZ );
}

RW_MATH_FORCE_INLINE void Quaternion::SetZ(VecFloatRefIndex::InParam z) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisZ, z.mV, z.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Quaternion::SetZ(VecFloatRef<INDEX> z) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, z.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Quaternion::SetW(float w) 
{
    SetComponent(3, w);
}

RW_MATH_FORCE_INLINE void Quaternion::SetW(VecFloat::InParam w) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, w.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Quaternion::SetW(VecFloatRefIndex::InParam w) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisW, w.mV, w.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Quaternion::SetW(VecFloatRef<INDEX> w) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, w.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Quaternion::SetZero() 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_GetZero(mV);
}

RW_MATH_FORCE_INLINE void Quaternion::SetIdentity() 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_GetL(mV);
}

template<class T>
RW_MATH_FORCE_INLINE Quaternion::Quaternion(const fpu::QuaternionTemplate<T> &v) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, (float)v.x, (float)v.y, (float)v.z, (float)v.w);
}

RW_MATH_FORCE_INLINE Quaternion::Quaternion() 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

RW_MATH_FORCE_INLINE Quaternion::Quaternion( float x, float y, float z, float w ) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, x, y, z, w);
}

RW_MATH_FORCE_INLINE Quaternion::Quaternion( const float xyzw[4] ) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*xyzw, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, xyzw[0], xyzw[1], xyzw[2], xyzw[3]);
}

RW_MATH_FORCE_INLINE Quaternion::Quaternion( VectorIntrinsicInParam v ) 
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


RW_MATH_FORCE_INLINE Quaternion::Quaternion(const Quaternion &rhs) 
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


RW_MATH_FORCE_INLINE Quaternion& Quaternion::operator = (Quaternion::InParam q) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(q.mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(q.mV);
#endif
    mV = q.mV;
    return *this;
}

#if RW_MATH_ENABLE_DEPRECATED_API
    RW_MATH_FORCE_INLINE Quaternion& Quaternion::operator = (VectorIntrinsicInParam v) 
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
        mV = v;
        return *this;
    }

    RW_MATH_FORCE_INLINE Quaternion::operator VectorIntrinsic&() 
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }

    RW_MATH_FORCE_INLINE Quaternion::operator const VectorIntrinsic&() const 
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }
#endif

RW_MATH_FORCE_INLINE void Quaternion::SetVector(VectorIntrinsicInParam v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    mV = v;
}

RW_MATH_FORCE_INLINE void Quaternion::Set(float x, float y, float z, float w) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, x, y, z, w );
}

RW_MATH_FORCE_INLINE VectorIntrinsic& Quaternion::GetVector() 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE const VectorIntrinsic& Quaternion::GetVector() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE VecFloatRefX Quaternion::X() 
{
    return VecFloatRefX(mV);
}

RW_MATH_FORCE_INLINE const VecFloatRefX Quaternion::X() const 
{
    return VecFloatRefX(const_cast<Quaternion*>(this)->mV);
}

RW_MATH_FORCE_INLINE VecFloatRefY Quaternion::Y() 
{
    return VecFloatRefY(mV);
}

RW_MATH_FORCE_INLINE const VecFloatRefY Quaternion::Y() const 
{
    return VecFloatRefY(const_cast<Quaternion*>(this)->mV);
}

RW_MATH_FORCE_INLINE VecFloatRefZ Quaternion::Z() 
{
    return VecFloatRefZ(mV);
}

RW_MATH_FORCE_INLINE const VecFloatRefZ Quaternion::Z() const 
{
    return VecFloatRefZ(const_cast<Quaternion*>(this)->mV);
}

RW_MATH_FORCE_INLINE VecFloatRefW Quaternion::W() 
{
    return VecFloatRefW(mV);
}

RW_MATH_FORCE_INLINE const VecFloatRefW Quaternion::W() const 
{
    return VecFloatRefW(const_cast<Quaternion*>(this)->mV);
}

RW_MATH_FORCE_INLINE VecFloatRefX Quaternion::GetX() 
{
    return VecFloatRefX(mV);
}

RW_MATH_FORCE_INLINE const VecFloatRefX Quaternion::GetX() const 
{
    return VecFloatRefX(const_cast<Quaternion*>(this)->mV);
}

RW_MATH_FORCE_INLINE VecFloatRefY Quaternion::GetY() 
{
    return VecFloatRefY(mV);
}

RW_MATH_FORCE_INLINE const VecFloatRefY Quaternion::GetY() const 
{
    return VecFloatRefY(const_cast<Quaternion*>(this)->mV);
}

RW_MATH_FORCE_INLINE VecFloatRefZ Quaternion::GetZ() 
{
    return VecFloatRefZ(mV);
}

RW_MATH_FORCE_INLINE const VecFloatRefZ Quaternion::GetZ() const 
{
    return VecFloatRefZ(const_cast<Quaternion*>(this)->mV);
}

RW_MATH_FORCE_INLINE VecFloatRefW Quaternion::GetW() 
{
    return VecFloatRefW(mV);
}

RW_MATH_FORCE_INLINE const VecFloatRefW Quaternion::GetW() const 
{
    return VecFloatRefW(const_cast<Quaternion*>(this)->mV);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Quaternion::GetComponent(int i) 
{
    return VecFloatRefIndex(mV, i);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Quaternion::GetComponent(int i) const 
{
    return VecFloatRefIndex(const_cast<Quaternion*>(this)->mV, i);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Quaternion::operator [] (int i) 
{
    return VecFloatRefIndex( mV, i );
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Quaternion::operator [] (int i) const 
{
    return VecFloatRefIndex(const_cast<Quaternion*>(this)->mV, i);
}

}

}

RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Quaternion& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::vpu::Quaternion), sizeof(float));
}

}

#endif
