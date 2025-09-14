// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR3PLUS_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR3PLUS_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/vector3plus.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"


namespace rw
{
namespace math
{
namespace vpu
{


RW_MATH_FORCE_INLINE void Vector3Plus::Set(float x, float y, float z, float plus)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, x, y, z, plus);
}


RW_MATH_FORCE_INLINE void Vector3Plus::Set(VecFloat::InParam x,
                                           VecFloat::InParam y,
                                           VecFloat::InParam z,
                                           VecFloat::InParam plus)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, x.mV, y.mV, z.mV, plus.mV);
}


RW_MATH_FORCE_INLINE void Vector3Plus::Set(VecFloatRefIndex::InParam x,
                                                VecFloatRefIndex::InParam y,
                                                VecFloatRefIndex::InParam z,
                                                VecFloatRefIndex::InParam plus)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic xV, yV, zV, plusV;
    RW_MATH_VPU_Splat( xV, x.mV, x.mIndex );
    RW_MATH_VPU_Splat( yV, y.mV, y.mIndex );
    RW_MATH_VPU_Splat( zV, z.mV, z.mIndex );
    RW_MATH_VPU_Splat( plusV, plus.mV, plus.mIndex );
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, xV, yV, zV, plusV);
}


template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
RW_MATH_FORCE_INLINE void Vector3Plus::Set(VecFloatRef<XSRC_INDEX> x,
                                                VecFloatRef<YSRC_INDEX> y,
                                                VecFloatRef<ZSRC_INDEX> z,
                                                VecFloatRef<WSRC_INDEX> plus)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorAxisA4_I(mV, x.mV, XSRC_INDEX, y.mV, YSRC_INDEX, z.mV, ZSRC_INDEX, plus.mV, WSRC_INDEX);    
}

RW_MATH_FORCE_INLINE void Vector3Plus::Set(Vector3::InParam v3, VecFloat::InParam userData)
{
    RW_MATH_VPU_SwizzleStore_I_I( mV, v3.mV, VectorAxisW, userData.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetComponent(int i, float value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    detail::VectorIntrinsicUnion temp;
    temp.vector = mV;
    temp.scalarFloat[i] = value;
    mV = temp.vector;
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetComponent(int i, VecFloat::InParam value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetComponent(int i, VecFloatRefIndex::InParam value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_V( mV, mV, i, value.mV, value.mIndex );
}

template<VectorAxis INDEX>
void Vector3Plus::SetComponent(int i, VecFloatRef<INDEX> value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetX(float x)
{
    SetComponent(0, x);
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetX(VecFloat::InParam x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, VectorAxisX );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetX(VecFloatRefIndex::InParam x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisX, x.mV, x.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector3Plus::SetX(VecFloatRef<INDEX> x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetY(float y)
{
    SetComponent(1, y);
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetY(VecFloat::InParam y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, VectorAxisY );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetY(VecFloatRefIndex::InParam y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisY, y.mV, y.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector3Plus::SetY(VecFloatRef<INDEX> y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetZ(float z)
{
    SetComponent(2, z);
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetZ(VecFloat::InParam z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, z.mV, VectorAxisZ );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetZ(VecFloatRefIndex::InParam z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisZ, z.mV, z.mIndex );
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector3Plus::SetZ(VecFloatRef<INDEX> z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, z.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetPlus(float plus)
{
    SetComponent(3, plus);
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetPlus(VecFloat::InParam plus)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, plus.mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetPlus(VecFloatRefIndex::InParam plus)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisW, plus.mV, plus.mIndex);
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector3Plus::SetPlus(VecFloatRef<INDEX> plus)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisW, plus.mV, INDEX );
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetZero()
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_GetZero(mV);
}

template<class T>
RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(const fpu::Vector3PlusTemplate<T> &v)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, (float)v.x, (float)v.y, (float)v.z, (float)v.userData);
}


RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus()
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}


RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(float x, float y, float z, float plus)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, x, y, z, plus);
}

RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(float xyz, float plus)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, xyz, xyz, xyz, plus);
}

RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(const float xyzplus[4])
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV4(mV, xyzplus[0], xyzplus[1], xyzplus[2], xyzplus[3]);
}

RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(VectorIntrinsicInParam v)
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


RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(VecFloat::InParam x,
                                                   VecFloat::InParam y,
                                                   VecFloat::InParam z,
                                                   VecFloat::InParam plus)
: mV(_asmCreateVectorAxis<VectorAxisX, VectorAxisY, VectorAxisZ, VectorAxisW>(
     x.mV, y.mV, z.mV, plus.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}


RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(VecFloatRefIndex::InParam x,
                                              VecFloatRefIndex::InParam y,
                                              VecFloatRefIndex::InParam z,
                                              VecFloatRefIndex::InParam plus)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic xV, yV, zV, plusV;
    RW_MATH_VPU_Splat( xV, x.mV, x.mIndex );
    RW_MATH_VPU_Splat( yV, y.mV, y.mIndex );
    RW_MATH_VPU_Splat( zV, z.mV, z.mIndex );
    RW_MATH_VPU_Splat( plusV, plus.mV, plus.mIndex );
    RW_MATH_VPU_ExtractXElementsAsVectorA4V4(mV, xV, yV, zV, plusV);
}


template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(VecFloatRef<XSRC_INDEX> x,
                                                   VecFloatRef<YSRC_INDEX> y,
                                                   VecFloatRef<ZSRC_INDEX> z,
                                                   VecFloatRef<WSRC_INDEX> plus)
: mV(_asmCreateVectorAxis<XSRC_INDEX, YSRC_INDEX, ZSRC_INDEX, WSRC_INDEX>(
     x.mV, y.mV, z.mV, plus.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}


#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(const Vector3Plus &rhs) 
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

RW_MATH_FORCE_INLINE Vector3Plus::Vector3Plus(Vector3::InParam v, VecFloat::InParam userData)
{
    RW_MATH_VPU_SwizzleStore_I_I( mV, v.mV, VectorAxisW, userData.mV, VectorAxisW );
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Vector3Plus& Vector3Plus::operator=(Vector3Plus::InParam v) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
    mV = v.mV;
    return *this;
}
#endif

#if RW_MATH_ENABLE_DEPRECATED_API
    RW_MATH_FORCE_INLINE Vector3Plus& Vector3Plus::operator=(VectorIntrinsicInParam v)
    {
        rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
        rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
        mV = v;
        return *this;
    }

    RW_MATH_FORCE_INLINE Vector3Plus::operator VectorIntrinsic&()
    {
        rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }

    RW_MATH_FORCE_INLINE Vector3Plus::operator const VectorIntrinsic&() const
    {
        rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }
#endif

RW_MATH_FORCE_INLINE void Vector3Plus::SetVector(VectorIntrinsicInParam v)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
    mV = v;
}

RW_MATH_FORCE_INLINE VectorIntrinsic& Vector3Plus::GetVector()
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE const VectorIntrinsic& Vector3Plus::GetVector() const
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

#if !defined RW_MATH_RETURN_FLOAT
    RW_MATH_FORCE_INLINE       VecFloatRefX Vector3Plus::X()       { return VecFloatRefX(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefX Vector3Plus::X() const { return VecFloatRefX(const_cast<Vector3Plus*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefY Vector3Plus::Y()       { return VecFloatRefY(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefY Vector3Plus::Y() const { return VecFloatRefY(const_cast<Vector3Plus*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefZ Vector3Plus::Z()       { return VecFloatRefZ(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefZ Vector3Plus::Z() const { return VecFloatRefZ(const_cast<Vector3Plus*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefW Vector3Plus::UserData()       { return VecFloatRefW(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefW Vector3Plus::UserData() const { return VecFloatRefW(const_cast<Vector3Plus*>(this)->mV); }
    
    RW_MATH_FORCE_INLINE       VecFloatRefX Vector3Plus::GetX()       { return VecFloatRefX(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefX Vector3Plus::GetX() const { return VecFloatRefX(const_cast<Vector3Plus*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefY Vector3Plus::GetY()       { return VecFloatRefY(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefY Vector3Plus::GetY() const { return VecFloatRefY(const_cast<Vector3Plus*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefZ Vector3Plus::GetZ()       { return VecFloatRefZ(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefZ Vector3Plus::GetZ() const { return VecFloatRefZ(const_cast<Vector3Plus*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefW Vector3Plus::GetPlus()       { return VecFloatRefW(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefW Vector3Plus::GetPlus() const { return VecFloatRefW(const_cast<Vector3Plus*>(this)->mV); }
#endif

RW_MATH_FORCE_INLINE VecFloatRefIndex Vector3Plus::GetComponent(int i)
{
    return VecFloatRefIndex(mV, i);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Vector3Plus::GetComponent(int i) const
{
    return VecFloatRefIndex(const_cast<Vector3Plus*>(this)->mV, i);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Vector3Plus::operator[](int i)
{
    return VecFloatRefIndex(mV, i);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Vector3Plus::operator[](int i) const
{
    return VecFloatRefIndex(const_cast<Vector3Plus*>(this)->mV, i);
}

RW_MATH_FORCE_INLINE void Vector3Plus::SetVector3(Vector3::InParam v3)
{
    RW_MATH_VPU_SwizzleStore_I_I( mV, v3.mV, VectorAxisW, mV, VectorAxisW );
}

RW_MATH_FORCE_INLINE Vector3 Vector3Plus::GetVector3() const
{
    return Vector3(mV);
}

} // namespace vpu
} // namespace math


RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Vector3Plus& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::vpu::Vector3Plus), sizeof(float));
}

} // namespace rw


#endif
