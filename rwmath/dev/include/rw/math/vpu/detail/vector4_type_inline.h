// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR4_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR4_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif


#include "rw/math/fpu/vector4.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Vector4::Vector4(VecFloat::InParam scalar)
    :mV(scalar.mV)
{
    rwMathAlignmentFunctionCheckAssert(scalar.mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE void Vector4::Set(float x, float y, float z, float w)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, x, y, z, w);
}


RW_MATH_FORCE_INLINE void Vector4::Set(VecFloat::InParam x, VecFloat::InParam y, VecFloat::InParam z, VecFloat::InParam w)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, x.mV, y.mV, z.mV, w.mV);
}

RW_MATH_FORCE_INLINE void Vector4::Set(VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y, VecFloatRefIndex::InParam z, VecFloatRefIndex::InParam w)
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
RW_MATH_FORCE_INLINE void Vector4::Set(VecFloatRef<XSRC_INDEX> x, VecFloatRef<YSRC_INDEX> y, VecFloatRef<ZSRC_INDEX> z, VecFloatRef<WSRC_INDEX> w)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorAxisA4_I(mV, x.mV, XSRC_INDEX, y.mV, YSRC_INDEX, z.mV, ZSRC_INDEX, w.mV, WSRC_INDEX);    
}

RW_MATH_FORCE_INLINE void Vector4::SetComponent(int i, float value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    detail::VectorIntrinsicUnion temp;
    temp.vector = mV;
    temp.scalarFloat[i] = value;
    mV = temp.vector;
}

RW_MATH_FORCE_INLINE void Vector4::SetComponent(int i, VecFloat::InParam value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Vector4::SetComponent(int i, VecFloatRefIndex::InParam value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_V( mV, mV, i, value.mV, value.mIndex );
}

template<VectorAxis INDEX>
void Vector4::SetComponent(int i, VecFloatRef<INDEX> value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector4::SetX(float x)
{
    SetComponent(0, x);
}

RW_MATH_FORCE_INLINE void Vector4::SetX(VecFloat::InParam x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Vector4::SetX(VecFloatRefIndex::InParam x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisX, x.mV, x.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector4::SetX(VecFloatRef<INDEX> x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector4::SetY(float y)
{
    SetComponent(1, y);
}

RW_MATH_FORCE_INLINE void Vector4::SetY(VecFloat::InParam y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, VectorAxisY );
}

RW_MATH_FORCE_INLINE void Vector4::SetY(VecFloatRefIndex::InParam y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisY, y.mV, y.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector4::SetY(VecFloatRef<INDEX> y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector4::SetZ(float z)
{
    SetComponent(2, z);
}

RW_MATH_FORCE_INLINE void Vector4::SetZ(VecFloat::InParam z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, z.mV, VectorAxisZ );
}

RW_MATH_FORCE_INLINE void Vector4::SetZ(VecFloatRefIndex::InParam z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisZ, z.mV, z.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector4::SetZ(VecFloatRef<INDEX> z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, z.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector4::SetW(float w)
{
    SetComponent(3, w);
}

RW_MATH_FORCE_INLINE void Vector4::SetW(VecFloat::InParam w)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, w.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Vector4::SetW(VecFloatRefIndex::InParam w)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisW, w.mV, w.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector4::SetW(VecFloatRef<INDEX> w)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, w.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector4::SetZero()
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_GetZero(mV);
}

RW_MATH_FORCE_INLINE Vector4::Vector4(VecFloat::InParam x,
                                           VecFloat::InParam y,
                                           VecFloat::InParam z,
                                           VecFloat::InParam w)
: mV(_asmCreateVectorAxis<VectorAxisX, VectorAxisY, VectorAxisZ, VectorAxisW>(
     x.mV, y.mV, z.mV, w.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}


RW_MATH_FORCE_INLINE Vector4::Vector4(VecFloatRefIndex::InParam x,
                                           VecFloatRefIndex::InParam y,
                                           VecFloatRefIndex::InParam z,
                                           VecFloatRefIndex::InParam w)
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
RW_MATH_FORCE_INLINE Vector4::Vector4(VecFloatRef<XSRC_INDEX> x,
                                           VecFloatRef<YSRC_INDEX> y,
                                           VecFloatRef<ZSRC_INDEX> z,
                                           VecFloatRef<WSRC_INDEX> w)
    : mV(_asmCreateVectorAxis<XSRC_INDEX, YSRC_INDEX, ZSRC_INDEX, WSRC_INDEX>(
        x.mV, y.mV, z.mV, w.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Vector4::Vector4( Vector3::InParam v3, VecFloatRefW::InParam w ) 
    :mV(v3.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, w.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE Vector4::Vector4( Vector3::InParam v3, VecFloatRefIndex::InParam w ) 
    :mV(v3.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisW, w.mV, w.mIndex );
}

RW_MATH_FORCE_INLINE Vector4::Vector4( Vector3::InParam v3, VecFloat::InParam w ) 
    :mV(v3.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, w.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE Vector4::Vector4( Vector3::InParam v3, const float w ) 
    :mV(v3.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    SetComponent(3, w);
}


template<class T>
RW_MATH_FORCE_INLINE Vector4::Vector4(const fpu::Vector4Template<T> &v)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, (float)v.X(), (float)v.Y(), (float)v.Z(), (float)v.W());
}

RW_MATH_FORCE_INLINE Vector4::Vector4()
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

RW_MATH_FORCE_INLINE Vector4::Vector4(float x, float y, float z, float w)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, x, y, z, w);
}

RW_MATH_FORCE_INLINE Vector4::Vector4(const float xyzw[4])
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    :mV(RW_MATH_VPU_LoadFromAlignedFloatPointer(xyzw))
#endif
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*xyzw, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = RW_MATH_VPU_LoadFromAlignedFloatPointer(xyzw);
#endif 
}

RW_MATH_FORCE_INLINE Vector4::Vector4(VectorIntrinsicInParam v)
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
: mV(v)
#endif
{
    // for alignment check, we must check before we do a assignment
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = v;
#endif 
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Vector4::Vector4(const Vector4 &rhs) 
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
#endif


// Operators
#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Vector4& Vector4::operator = (Vector4::InParam v) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(v.mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
    mV = v.mV;
    return *this;
}
#endif

#if RW_MATH_ENABLE_DEPRECATED_API
    RW_MATH_FORCE_INLINE Vector4& Vector4::operator = (VectorIntrinsicInParam v)
    {
        rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
        rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
        mV = v;
        return *this;
    }

    RW_MATH_FORCE_INLINE Vector4::operator VectorIntrinsic&()
    {
        rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }

    RW_MATH_FORCE_INLINE Vector4::operator const VectorIntrinsic&() const
    {
        rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }
#endif

RW_MATH_FORCE_INLINE void Vector4::SetVector(VectorIntrinsicInParam v)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
    mV = v;
}

RW_MATH_FORCE_INLINE VectorIntrinsic& Vector4::GetVector()
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE const VectorIntrinsic& Vector4::GetVector() const
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

#if !defined RW_MATH_RETURN_FLOAT
    RW_MATH_FORCE_INLINE       VecFloatRefX Vector4::X()       { return VecFloatRefX(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefX Vector4::X() const { return VecFloatRefX(const_cast<Vector4*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefY Vector4::Y()       { return VecFloatRefY(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefY Vector4::Y() const { return VecFloatRefY(const_cast<Vector4*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefZ Vector4::Z()       { return VecFloatRefZ(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefZ Vector4::Z() const { return VecFloatRefZ(const_cast<Vector4*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefW Vector4::W()       { return VecFloatRefW(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefW Vector4::W() const { return VecFloatRefW(const_cast<Vector4*>(this)->mV); }
    
    RW_MATH_FORCE_INLINE       VecFloatRefX Vector4::GetX()       { return VecFloatRefX(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefX Vector4::GetX() const { return VecFloatRefX(const_cast<Vector4*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefY Vector4::GetY()       { return VecFloatRefY(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefY Vector4::GetY() const { return VecFloatRefY(const_cast<Vector4*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefZ Vector4::GetZ()       { return VecFloatRefZ(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefZ Vector4::GetZ() const { return VecFloatRefZ(const_cast<Vector4*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefW Vector4::GetW()       { return VecFloatRefW(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefW Vector4::GetW() const { return VecFloatRefW(const_cast<Vector4*>(this)->mV); }
#endif

RW_MATH_FORCE_INLINE VecFloatRefIndex Vector4::GetComponent(int i)
{ 
    return VecFloatRefIndex(mV, i); 
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Vector4::GetComponent(int i) const
{
    return VecFloatRefIndex(const_cast<Vector4*>(this)->mV, i);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Vector4::operator [] (int i)
{
    return VecFloatRefIndex(mV, i);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Vector4::operator [] (int i) const
{
    return VecFloatRefIndex(const_cast<Vector4*>(this)->mV, i);
}

} // namespace vpu
} // namespace math

RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Vector4& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::vpu::Vector4), sizeof(float));
}

} // namespace rw

#endif
