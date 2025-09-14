// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR3_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR3_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/vector3.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Vector3::Vector3(VecFloat::InParam scalar)
    :mV(scalar.mV)
{
    rwMathAlignmentFunctionCheckAssert(scalar.mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE void Vector3::Set(float x, float y, float z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV3(mV, x, y, z);
}

RW_MATH_FORCE_INLINE void Vector3::Set(VecFloat::InParam x, VecFloat::InParam y, VecFloat::InParam z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorAxisA3_I( mV, x.mV, VectorAxisX, y.mV, VectorAxisY, z.mV, VectorAxisZ );
}

RW_MATH_FORCE_INLINE void Vector3::Set(VecFloatRefIndex::InParam x,
                                            VecFloatRefIndex::InParam y,
                                            VecFloatRefIndex::InParam z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic xV, yV, zV;
    RW_MATH_VPU_Splat( xV, x.mV, x.mIndex );
    RW_MATH_VPU_Splat( yV, y.mV, y.mIndex );
    RW_MATH_VPU_Splat( zV, z.mV, z.mIndex );
    RW_MATH_VPU_CreateVectorAxisA3_I(mV, xV, VectorAxisX, yV, VectorAxisY, zV, VectorAxisZ);
}

template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX>
RW_MATH_FORCE_INLINE void Vector3::Set(VecFloatRef<XSRC_INDEX> x,
                                            VecFloatRef<YSRC_INDEX> y,
                                            VecFloatRef<ZSRC_INDEX> z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorAxisA3_I( mV, x.mV, XSRC_INDEX, y.mV, YSRC_INDEX, z.mV, ZSRC_INDEX );
}


RW_MATH_FORCE_INLINE void Vector3::SetComponent(int i, float value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    detail::VectorIntrinsicUnion temp;
    temp.vector = mV;
    temp.scalarFloat[i] = value;
    mV = temp.vector;
}


RW_MATH_FORCE_INLINE void Vector3::SetComponent(int i, VecFloat::InParam value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, VectorAxisX );
}


RW_MATH_FORCE_INLINE void Vector3::SetComponent(int i, VecFloatRefIndex::InParam value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_V( mV, mV, i, value.mV, value.mIndex );
}


template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE void Vector3::SetComponent(int i, VecFloatRef<INDEX> value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, INDEX );
}


RW_MATH_FORCE_INLINE void Vector3::SetX(float x)
{
    SetComponent(0, x);
}


RW_MATH_FORCE_INLINE void Vector3::SetX(VecFloat::InParam x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, VectorAxisX );
}


RW_MATH_FORCE_INLINE void Vector3::SetX(VecFloatRefIndex::InParam x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisX, x.mV, x.mIndex );
}


template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector3::SetX(VecFloatRef<INDEX> x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, INDEX );
}


RW_MATH_FORCE_INLINE void Vector3::SetY(float y)
{
    SetComponent(1, y);
}


RW_MATH_FORCE_INLINE void Vector3::SetY(VecFloat::InParam y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, VectorAxisY );
}


RW_MATH_FORCE_INLINE void Vector3::SetY(VecFloatRefIndex::InParam y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisY, y.mV, y.mIndex );
}


template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector3::SetY(VecFloatRef<INDEX> y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, INDEX );
}


RW_MATH_FORCE_INLINE void Vector3::SetZ(float z)
{
    SetComponent(2, z);
}


RW_MATH_FORCE_INLINE void Vector3::SetZ(VecFloat::InParam z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, z.mV, VectorAxisZ );
}


RW_MATH_FORCE_INLINE void Vector3::SetZ(VecFloatRefIndex::InParam z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisZ, z.mV, z.mIndex );
}


template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector3::SetZ(VecFloatRef<INDEX> z)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisZ, z.mV, INDEX );
}


RW_MATH_FORCE_INLINE void Vector3::SetZero()
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_GetZero(mV);
}


// Constructors
RW_MATH_FORCE_INLINE Vector3::Vector3(VecFloat::InParam x, VecFloat::InParam y, VecFloat::InParam z)
: mV(_asmCreateVectorAxis<VectorAxisX, VectorAxisY, VectorAxisZ>(x.mV, y.mV, z.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}


RW_MATH_FORCE_INLINE Vector3::Vector3(VecFloatRefIndex::InParam x,
                                            VecFloatRefIndex::InParam y,
                                            VecFloatRefIndex::InParam z)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic xV, yV, zV;
    RW_MATH_VPU_Splat( xV, x.mV, x.mIndex );
    RW_MATH_VPU_Splat( yV, y.mV, y.mIndex );
    RW_MATH_VPU_Splat( zV, z.mV, z.mIndex );
    RW_MATH_VPU_CreateVectorAxisA3_I(mV, xV, VectorAxisX, yV, VectorAxisY, zV, VectorAxisZ);
}

RW_MATH_FORCE_INLINE Vector3::Vector3(Vector2::InParam v2, VecFloatRefZ::InParam z)
    :mV(v2.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I(mV, mV, VectorAxisZ, z.mV, VectorAxisZ);
}

RW_MATH_FORCE_INLINE Vector3::Vector3(Vector2::InParam v2, VecFloat::InParam z)
    :mV(v2.mV)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I(mV, mV, VectorAxisZ, z.mV, VectorAxisZ);
}

template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX>
RW_MATH_FORCE_INLINE Vector3::Vector3(VecFloatRef<XSRC_INDEX> x,
                                           VecFloatRef<YSRC_INDEX> y,
                                           VecFloatRef<ZSRC_INDEX> z)
: mV(_asmCreateVectorAxis<XSRC_INDEX, YSRC_INDEX, ZSRC_INDEX>(x.mV, y.mV, z.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}


template<class T>
RW_MATH_FORCE_INLINE Vector3::Vector3(const fpu::Vector3Template<T> &v)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV3(mV, static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z));
}


RW_MATH_FORCE_INLINE Vector3::Vector3()
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RWCROSS
    RW_MATH_VPU_GetZero(mV);
#elif defined RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Vector3::Vector3(const Vector3 &rhs) 
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
: mV( rhs.mV )
#endif
{
    // for alignment check, we must check before we do assignment
    rwMathAlignmentConstructorCheckAssert(rhs.mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifndef RWCROSS
	#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_3
		RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_3(rhs.mV);
	#endif
#endif
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = rhs.mV;
#endif 
}
#endif


RW_MATH_FORCE_INLINE Vector3::Vector3(float x, float y, float z)
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV3(mV, x, y, z);
}

RW_MATH_FORCE_INLINE Vector3::Vector3(const float xyz[3])
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    :mV(RW_MATH_VPU_LoadFromAlignedFloatPointer(xyz))
#endif
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*xyz, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = RW_MATH_VPU_LoadFromAlignedFloatPointer(xyz);
#endif 
}


RW_MATH_FORCE_INLINE Vector3::Vector3(VectorIntrinsicInParam v)
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    :mV(v)
#endif
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = v;
#endif 
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Vector3& Vector3::operator=(Vector3::InParam v)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));
#ifndef RWCROSS
	#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_3
		RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_3(v.mV);
	#endif
#endif
    mV = v.mV;
    return *this;
}
#endif

#if RW_MATH_ENABLE_DEPRECATED_API
    RW_MATH_FORCE_INLINE Vector3& Vector3::operator=(VectorIntrinsicInParam v)
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
        mV = v;
        return *this;
    }

    RW_MATH_FORCE_INLINE Vector3::operator VectorIntrinsic&()
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }


    RW_MATH_FORCE_INLINE Vector3::operator const VectorIntrinsic&() const
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }
#endif


RW_MATH_FORCE_INLINE void Vector3::SetVector(VectorIntrinsicInParam v)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
    mV = v;
}


RW_MATH_FORCE_INLINE VectorIntrinsic& Vector3::GetVector()
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}


RW_MATH_FORCE_INLINE const VectorIntrinsic& Vector3::GetVector() const
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

#if !defined RW_MATH_RETURN_FLOAT
    RW_MATH_FORCE_INLINE       VecFloatRefX Vector3::X()       { return VecFloatRefX(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefX Vector3::X() const { return VecFloatRefX(const_cast<Vector3*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefY Vector3::Y()       { return VecFloatRefY(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefY Vector3::Y() const { return VecFloatRefY(const_cast<Vector3*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefZ Vector3::Z()       { return VecFloatRefZ(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefZ Vector3::Z() const { return VecFloatRefZ(const_cast<Vector3*>(this)->mV); }
        
    RW_MATH_FORCE_INLINE       VecFloatRefX Vector3::GetX()       { return VecFloatRefX(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefX Vector3::GetX() const { return VecFloatRefX(const_cast<Vector3*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefY Vector3::GetY()       { return VecFloatRefY(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefY Vector3::GetY() const { return VecFloatRefY(const_cast<Vector3*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefZ Vector3::GetZ()       { return VecFloatRefZ(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefZ Vector3::GetZ() const { return VecFloatRefZ(const_cast<Vector3*>(this)->mV); }
#endif

RW_MATH_FORCE_INLINE VecFloatRefIndex Vector3::GetComponent(int i)
{
    return VecFloatRefIndex(mV, i);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Vector3::GetComponent(int i) const
{
    return VecFloatRefIndex(const_cast<Vector3*>(this)->mV, i);
}


RW_MATH_FORCE_INLINE VecFloatRefIndex Vector3::operator[](int i)
{
    return VecFloatRefIndex(mV, i);
}


RW_MATH_FORCE_INLINE const VecFloatRefIndex Vector3::operator[](int i) const
{
    return VecFloatRefIndex(const_cast<Vector3*>(this)->mV, i);
}

} // namespace vpu
} // namespace math


RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Vector3& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::vpu::Vector3), sizeof(float));
}


} // namespace rw

#endif
