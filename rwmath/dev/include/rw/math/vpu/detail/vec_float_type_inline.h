// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VEC_FLOAT_TYPE_INLINE_H
#define RW_MATH_VPU_DETAIL_VEC_FLOAT_TYPE_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"
#include "rw/math/vpu/detail/vec_float_macros.h"
#include "rw/math/vpu/detail/vec_float_type_platform_inline.h"

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE VecFloatRefIndex::VecFloatRefIndex(VectorIntrinsic& v, int index) 
:mV(v)
{
    // for alignment check, we must check before we do a assignment
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));

    mIndex = (VectorAxis)index;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex::VecFloatRefIndex(const VecFloatRefIndex &v) 
:mV(v.mV)
{
    rwMathAlignmentConstructorCheckAssert(v.mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    mIndex= v.mIndex;

}

RW_MATH_FORCE_INLINE float VecFloatRefIndex::GetFloat() const 
{
	rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	return mV.m128_f32[mIndex];
#else
	// copy to temp to avoid aliasing, should be optimized out in release mode on platform/compiler
	// that doesn't have aliasing issue.
	detail::VectorIntrinsicUnion temp;
	temp.vector = mV;
	return temp.scalarFloat[mIndex];
#endif
}

RW_MATH_FORCE_INLINE void VecFloatRefIndex::SetFloat(float f) 
{
	rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	mV.m128_f32[mIndex] = f;
#else
    VectorIntrinsic vf;
    RW_MATH_VPU_CreateVectorV1(vf, f);
    RW_MATH_VPU_SwizzleStore_V_I(mV, mV, mIndex, vf, VectorAxisX)
#endif
}

RW_MATH_FORCE_INLINE const VectorIntrinsic& VecFloatRefIndex::GetVector() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

RW_MATH_FORCE_INLINE int VecFloatRefIndex::GetIndex() const 
{
    return mIndex;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator=(VecFloatRefIndex rhs) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(rhs.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_V_V(mV, mV, mIndex, rhs.mV, rhs.mIndex);
    return *this;
}

template<VectorAxis INDEX>
VecFloatRefIndex& VecFloatRefIndex::operator=(VecFloatRef<INDEX> rhs)
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(rhs.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_V_I(mV, mV, mIndex, rhs.mV, INDEX);
    return *this;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator=(VecFloat::InParam rhs) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(rhs.mV, sizeof(VectorIntrinsic));

    RW_MATH_VPU_SwizzleStore_V_I(mV, mV, mIndex, rhs.mV, VectorAxisX);
    return *this;
}

#ifndef RW_MATH_DISABLE_VECFLOAT_IMPLICIT_FLOAT_CONVERSION_OPERATOR
RW_MATH_FORCE_INLINE VecFloatRefIndex::operator float() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	return mV.m128_f32[mIndex];
#else
    // copy to temp to avoid aliasing, should be optimized out in release mode on platform/compiler
    // that doesn't have aliasing issue.
    detail::VectorIntrinsicUnion temp;
    temp.vector = mV;
    return temp.scalarFloat[mIndex];
#endif
}
#endif

RW_MATH_FORCE_INLINE VecFloatRefIndex &VecFloatRefIndex::operator =(float f) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	mV.m128_f32[mIndex] = f;
#else
	VectorIntrinsic vf;
	RW_MATH_VPU_CreateVectorV1(vf, f);
	RW_MATH_VPU_SwizzleStore_V_I(mV, mV, mIndex, vf, VectorAxisX)
#endif
	return *this;
}

//
//  Implementation for VecFloat
//

RW_MATH_FORCE_INLINE VecFloat::VecFloat() 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VPU_MEMBER_INIT_FUNCTION
	RW_MATH_VPU_MEMBER_INIT_FUNCTION(mV);
#endif
}

RW_MATH_FORCE_INLINE VecFloat::VecFloat(const float &v) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV1_FromMemory(mV, &v);
}

RW_MATH_FORCE_INLINE VecFloat::VecFloat( VectorIntrinsicInParam v, int index ) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_Splat(mV, v, index);
}

RW_MATH_FORCE_INLINE VecFloat::VecFloat(VecFloatRefIndex v) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_Splat(mV, v.mV, v.mIndex);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat::VecFloat(VecFloatRef<INDEX> v) 
{
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_Splat_I(mV, v.mV, INDEX);
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE VecFloat::VecFloat(const VecFloat& v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(v.mV);
#endif
    mV = v.mV;
}
#endif

RW_MATH_FORCE_INLINE void VecFloat::SetFloat(const float& v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    RW_MATH_VPU_CreateVectorV1_FromMemory(mV, &v);
}

RW_MATH_FORCE_INLINE float VecFloat::GetFloat() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	return mV.m128_f32[VectorAxisX];
#else
    float ret;
    RW_MATH_VPU_GetFloatAxis_I(ret, mV, VectorAxisX);
    return ret;
#endif
}

RW_MATH_FORCE_INLINE void VecFloat::WriteToFloat(float* f) const
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_XENON)
    RWMATH_VPU_DETAIL_VecStvewx(mV, 0, f);
#else
    *f = GetFloat();
#endif
}

#ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
RW_MATH_FORCE_INLINE VecFloat &VecFloat::operator =(const VecFloat &f) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(f.mV, sizeof(VectorIntrinsic));
#ifdef RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4
	RW_MATH_VEC_INTRINSIC_MEMBER_VALIDATE_FUNCTION_4(f.mV);
#endif
    mV = f.mV;
    return *this;
}
#endif

#ifndef RW_MATH_DISABLE_VECFLOAT_IMPLICIT_FLOAT_CONVERSION_OPERATOR
RW_MATH_FORCE_INLINE VecFloat::operator float() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	return mV.m128_f32[VectorAxisX];
#else
    float ret;
    RW_MATH_VPU_GetFloatAxis_I(ret, mV, VectorAxisX);
    return ret;
#endif
}
#endif

RW_MATH_FORCE_INLINE VecFloat &VecFloat::operator =(const float& f) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    SetFloat(f);
    return *this;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat& VecFloat::operator=(const VecFloatRef<INDEX>& v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));
    *this = VecFloat(v);
    return *this;
}

RW_MATH_FORCE_INLINE VecFloat& VecFloat::operator=(const VecFloatRefIndex& v) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v.mV, sizeof(VectorIntrinsic));
    *this = VecFloat(v);
    return *this;
}

//
//  Implementation for VecFloatRef
//
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>::VecFloatRef(VectorIntrinsic& v) 
:mV(v)
{
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>::VecFloatRef(const VecFloatRef<INDEX> & v) 
:mV(v.mV)
{
  rwMathAlignmentConstructorCheckAssert(v.mV, sizeof(VectorIntrinsic));
  rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
}


template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE float VecFloatRef<INDEX>::GetFloat() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	return mV.m128_f32[INDEX];
#else
    float ret;
    RW_MATH_VPU_GetFloatAxis_I(ret, mV, INDEX);
    return ret;
#endif
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE void VecFloatRef<INDEX>::SetFloat(float f) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	mV.m128_f32[INDEX] = f;
#else
    VectorIntrinsic vec;
    RW_MATH_VPU_CreateVectorV1(vec, f);
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, INDEX, vec, VectorAxisX );
#endif
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE const VectorIntrinsic& VecFloatRef<INDEX>::GetVector() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE int VecFloatRef<INDEX>::GetIndex() const 
{
    return INDEX;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator =(VecFloatRefX rhs) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	mV.m128_f32[INDEX] = rhs.mV.m128_f32[VectorAxisX];
#else
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, INDEX, rhs.mV, VectorAxisX );
#endif
    return *this;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator =(VecFloatRefY rhs) 
{
	rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	mV.m128_f32[INDEX] = rhs.mV.m128_f32[VectorAxisY];
#else
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, INDEX, rhs.mV, VectorAxisY );
#endif
    return *this;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator =(VecFloatRefZ rhs) 
{
	rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	mV.m128_f32[INDEX] = rhs.mV.m128_f32[VectorAxisZ];
#else
    RW_MATH_VPU_SwizzleStore_I_I( mV, mV, INDEX, rhs.mV, VectorAxisZ );
#endif
    return *this;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator =(VecFloatRefW rhs) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	mV.m128_f32[INDEX] = rhs.mV.m128_f32[VectorAxisW];
#else
	RW_MATH_VPU_SwizzleStore_I_I( mV, mV, INDEX, rhs.mV, VectorAxisW );
#endif
    return *this;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator =(VecFloatRefIndex rhs) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	mV.m128_f32[INDEX] = rhs.mV.m128_f32[rhs.mIndex];
#else
	RW_MATH_VPU_SwizzleStore_I_V( mV, mV, INDEX, rhs.mV, rhs.mIndex );
#endif
    return *this;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator =(VecFloat::InParam rhs) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
#if defined(EA_PLATFORM_XBOXONE)
	mV.m128_f32[INDEX] = rhs.mV.m128_f32[VectorAxisX];
#else
	RW_MATH_VPU_SwizzleStore_I_I( mV, mV, INDEX, rhs.mV, VectorAxisX );
#endif
    return *this;
}

#ifndef RW_MATH_DISABLE_VECFLOAT_IMPLICIT_FLOAT_CONVERSION_OPERATOR
template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>::operator float() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return GetFloat();
}
#endif

template<VectorAxis INDEX> 
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator =(float f) 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    SetFloat(f);
    return *this;
}

//
//  Implementation for member operators of VecFloatRef<INDEX> and VectorRefIndex
//

// operator +=
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator += (VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(RW_MATH_VPU_Add, *this, b);
}

template<VectorAxis INDEX>
template<VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator += (VecFloatRef<INDEX_OTHER> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(RW_MATH_VPU_Add, *this, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator += (VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(RW_MATH_VPU_Add, *this, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator += (const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(RW_MATH_VPU_Add, *this, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator += (VecFloat::InParam b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(RW_MATH_VPU_Add, *this, b);
}                                           
                                            
template<VectorAxis INDEX>                 
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator += (VecFloatRef<INDEX> b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(RW_MATH_VPU_Add, *this, b);
}                                           
                                            
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator += (VecFloatRefIndex::InParam b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(RW_MATH_VPU_Add, *this, b);
}                                           
                                            
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator += (const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(RW_MATH_VPU_Add, *this, b);
}


// operator -=
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator -= (VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(RW_MATH_VPU_Subtract, *this, b);
}

template<VectorAxis INDEX>
template<VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator -= (VecFloatRef<INDEX_OTHER> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(RW_MATH_VPU_Subtract, *this, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator -= (VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(RW_MATH_VPU_Subtract, *this, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator -= (const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(RW_MATH_VPU_Subtract, *this, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator -= (VecFloat::InParam b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(RW_MATH_VPU_Subtract, *this, b);
}                                           
                                            
template<VectorAxis INDEX>                 
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator -= (VecFloatRef<INDEX> b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(RW_MATH_VPU_Subtract, *this, b);
}                                           
                                            
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator -= (VecFloatRefIndex::InParam b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(RW_MATH_VPU_Subtract, *this, b);
}                                           
                                            
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator -= (const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(RW_MATH_VPU_Subtract, *this, b);
}


// operator *=
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator *= (VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(RW_MATH_VPU_Mult, *this, b);
}

template<VectorAxis INDEX>
template<VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator *= (VecFloatRef<INDEX_OTHER> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(RW_MATH_VPU_Mult, *this, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator *= (VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(RW_MATH_VPU_Mult, *this, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator *= (const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(RW_MATH_VPU_Mult, *this, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator *= (VecFloat::InParam b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(RW_MATH_VPU_Mult, *this, b);
}                                           
                                            
template<VectorAxis INDEX>                 
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator *= (VecFloatRef<INDEX> b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(RW_MATH_VPU_Mult, *this, b);
}                                           
                                            
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator *= (VecFloatRefIndex::InParam b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(RW_MATH_VPU_Mult, *this, b);
}                                           
                                            
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator *= (const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(RW_MATH_VPU_Mult, *this, b);
}


// operator /=
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator /= (VecFloat::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOAT(RW_MATH_VPU_Divide, *this, b);
}

template<VectorAxis INDEX>
template<VectorAxis INDEX_OTHER>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator /= (VecFloatRef<INDEX_OTHER> b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREF(RW_MATH_VPU_Divide, *this, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator /= (VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_VECFLOATREFINDEX(RW_MATH_VPU_Divide, *this, b);
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& VecFloatRef<INDEX>::operator /= (const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREF_FLOAT(RW_MATH_VPU_Divide, *this, b);
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator /= (VecFloat::InParam b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOAT(RW_MATH_VPU_Divide, *this, b);
}                                           
                                            
template<VectorAxis INDEX>                 
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator /= (VecFloatRef<INDEX> b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREF(RW_MATH_VPU_Divide, *this, b);
}                                           
                                            
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator /= (VecFloatRefIndex::InParam b) 
{                                           
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(RW_MATH_VPU_Divide, *this, b);
}                                           
                                            
RW_MATH_FORCE_INLINE VecFloatRefIndex& VecFloatRefIndex::operator /= (const float& b) 
{
    RWMATH_DETAIL_SELF_OPERATION_VECFLOATREFINDEX_FLOAT(RW_MATH_VPU_Divide, *this, b);
}



}}}

#endif
