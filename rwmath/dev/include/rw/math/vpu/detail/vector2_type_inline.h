// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR2_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR2_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/fpu/vector2.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"

namespace rw {
namespace math {
namespace vpu {

template<class T>
RW_MATH_FORCE_INLINE Vector2::Vector2(const fpu::Vector2Template<T> &v)     
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV2(mV, (float)v.X(), (float)v.Y());
}

RW_MATH_FORCE_INLINE Vector2::Vector2() 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RWCROSS
    RW_MATH_VPU_GetZero(mV);
#elif defined RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

RW_MATH_FORCE_INLINE Vector2::Vector2(VecFloat::InParam scalar)
    :mV(scalar.mV)
{
    rwMathAlignmentFunctionCheckAssert(scalar.mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Vector2::Vector2( float x, float y ) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV2(mV, x, y);
}

RW_MATH_FORCE_INLINE Vector2::Vector2( const float xy[2] ) 
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    :mV(RW_MATH_VPU_LoadFromAlignedFloatPointer(xy))
#endif
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(*xy, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = RW_MATH_VPU_LoadFromAlignedFloatPointer(xy);
#endif 
}

RW_MATH_FORCE_INLINE Vector2::Vector2( VectorIntrinsicInParam v ) 
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    :mV(v)
#endif
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = v;
#endif 
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Vector2::Vector2(const Vector2 &rhs) 
#ifndef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
: mV( rhs.mV )
#endif
{
    // for alignment check, we must check before we do assignment
    rwMathAlignmentConstructorCheckAssert(rhs.mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifndef RWCROSS
	#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2
		RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2(rhs.mV);
	#endif
#endif
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    mV = rhs.mV;
#endif 
}
#endif


RW_MATH_FORCE_INLINE Vector2::Vector2( VecFloat::InParam x, VecFloat::InParam y ) 
: mV(_asmCreateVectorAxis<VectorAxisX, VectorAxisY>(x.mV, y.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}

RW_MATH_FORCE_INLINE Vector2::Vector2(VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic xV, yV;
    RW_MATH_VPU_Splat( xV, x.mV, x.mIndex );
    RW_MATH_VPU_Splat( yV, y.mV, y.mIndex );
    RW_MATH_VPU_CreateVectorAxisA2_I(mV, xV, VectorAxisX, yV, VectorAxisY);
}

template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX>
RW_MATH_FORCE_INLINE Vector2::Vector2(VecFloatRef<XSRC_INDEX> x, VecFloatRef<YSRC_INDEX> y) 
: mV(_asmCreateVectorAxis<XSRC_INDEX, YSRC_INDEX>(x.mV, y.mV))
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE Vector2& Vector2::operator = (Vector2::InParam v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));
#ifndef RWCROSS
	#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2
		RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_2(v.mV);
	#endif
#endif
    mV = v.mV;
    return *this;
}
#endif

#if RW_MATH_ENABLE_DEPRECATED_API
    RW_MATH_FORCE_INLINE Vector2& Vector2::operator = (VectorIntrinsicInParam v) 
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
        mV = v;
        return *this;
    }

    RW_MATH_FORCE_INLINE Vector2::operator VectorIntrinsic&() 
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }

    RW_MATH_FORCE_INLINE Vector2::operator const VectorIntrinsic&() const 
    {
        rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
        return mV;
    }
#endif

RW_MATH_FORCE_INLINE void Vector2::SetVector(VectorIntrinsicInParam v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    mV = v;
}

RW_MATH_FORCE_INLINE VectorIntrinsic& Vector2::GetVector() 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE const VectorIntrinsic& Vector2::GetVector() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE void Vector2::Set(VecFloat::InParam x, VecFloat::InParam y) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorAxisA2_I( mV, x.mV, VectorAxisX, y.mV, VectorAxisY );
}

RW_MATH_FORCE_INLINE void Vector2::Set(VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    VectorIntrinsic xV, yV;
    RW_MATH_VPU_Splat( xV, x.mV, x.mIndex );
    RW_MATH_VPU_Splat( yV, y.mV, y.mIndex );
    RW_MATH_VPU_CreateVectorAxisA2_I(mV, xV, VectorAxisX, yV, VectorAxisY);
}

template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX>
RW_MATH_FORCE_INLINE void Vector2::Set(VecFloatRef<XSRC_INDEX> x, VecFloatRef<YSRC_INDEX> y) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorAxisA2_I( mV, x.mV, XSRC_INDEX, y.mV, YSRC_INDEX );
}

RW_MATH_FORCE_INLINE void Vector2::SetComponent(int i, float value) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    detail::VectorIntrinsicUnion temp;
    temp.vector = mV;
    temp.scalarFloat[i] = value;
    mV = temp.vector;
}

RW_MATH_FORCE_INLINE void Vector2::SetComponent(int i, VecFloat::InParam value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, VectorAxisX );
}


RW_MATH_FORCE_INLINE void Vector2::SetComponent(int i, VecFloatRefIndex::InParam value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_V( mV, mV, i, value.mV, value.mIndex );
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE void Vector2::SetComponent(int i, VecFloatRef<INDEX> value)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_V_I( mV, mV, i, value.mV, INDEX );
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Vector2::GetComponent(int i) 
{
    return VecFloatRefIndex(mV, i);
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Vector2::GetComponent(int i) const 
{
    return VecFloatRefIndex(const_cast<Vector2*>(this)->mV, i);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex Vector2::operator [] (int i) 
{
    return VecFloatRefIndex( mV, i );
}

RW_MATH_FORCE_INLINE const VecFloatRefIndex Vector2::operator [] (int i) const 
{
    return VecFloatRefIndex(const_cast<Vector2*>(this)->mV, i);
}

RW_MATH_FORCE_INLINE void Vector2::Set(float x, float y) 
{
    RW_MATH_VPU_CreateVectorV2(mV, x, y);
}

#if !defined RW_MATH_RETURN_FLOAT
    RW_MATH_FORCE_INLINE       VecFloatRefX Vector2::X()       { return VecFloatRefX(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefX Vector2::X() const { return VecFloatRefX(const_cast<Vector2*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefY Vector2::Y()       { return VecFloatRefY(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefY Vector2::Y() const { return VecFloatRefY(const_cast<Vector2*>(this)->mV); }
    
    RW_MATH_FORCE_INLINE       VecFloatRefX Vector2::GetX()       { return VecFloatRefX(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefX Vector2::GetX() const { return VecFloatRefX(const_cast<Vector2*>(this)->mV); }
    RW_MATH_FORCE_INLINE       VecFloatRefY Vector2::GetY()       { return VecFloatRefY(mV); }
    RW_MATH_FORCE_INLINE const VecFloatRefY Vector2::GetY() const { return VecFloatRefY(const_cast<Vector2*>(this)->mV); }
#endif

RW_MATH_FORCE_INLINE void Vector2::SetX(float x)
{
    SetComponent(0, x);
}

RW_MATH_FORCE_INLINE void Vector2::SetX(VecFloat::InParam x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, VectorAxisX );
}


RW_MATH_FORCE_INLINE void Vector2::SetX(VecFloatRefIndex::InParam x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisX, x.mV, x.mIndex );
}


template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector2::SetX(VecFloatRef<INDEX> x)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisX, x.mV, INDEX );
}


RW_MATH_FORCE_INLINE void Vector2::SetY(float y)
{
    SetComponent(1, y);
}


RW_MATH_FORCE_INLINE void Vector2::SetY(VecFloat::InParam y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, VectorAxisY );
}


RW_MATH_FORCE_INLINE void Vector2::SetY(VecFloatRefIndex::InParam y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_V( mV, mV, VectorAxisY, y.mV, y.mIndex );
}


template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void Vector2::SetY(VecFloatRef<INDEX> y)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, VectorAxisY, y.mV, INDEX );
}


RW_MATH_FORCE_INLINE void Vector2::SetZero()
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_GetZero(mV);
}

}}}

#endif
