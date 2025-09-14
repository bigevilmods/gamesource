// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CROSS_VEC_FLOAT_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_CROSS_ VEC_FLOAT_OPERATION_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{

namespace math
{

namespace vpu
{

//----------------------------------------------------------------------------
// Negate
//----------------------------------------------------------------------------

RW_MATH_FORCE_INLINE VecFloat Negate(VecFloat::InParam a) 
{
    return VecFloat(-a.GetFloat());
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat Negate(VecFloatRef<INDEX> a) 
{
    return VecFloat(-a.GetFloat());
}

RW_MATH_FORCE_INLINE VecFloat Negate(VecFloatRefIndex::InParam a) 
{
    return VecFloat(-a.GetFloat());
}

//----------------------------------------------------------------------------
// SelfAdd
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat& SelfAdd(VecFloat& a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat& SelfAdd(VecFloat& a, VecFloatRef<INDEX1> b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfAdd(VecFloat& a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfAdd(VecFloat& a, const float& b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfAdd(VecFloatRef<INDEX1> &a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfAdd(VecFloatRef<INDEX1> &a, VecFloatRef<INDEX2> b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfAdd(VecFloatRef<INDEX1> &a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfAdd(VecFloatRef<INDEX1> &a, const float& b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfAdd(VecFloatRefIndex& a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfAdd(VecFloatRefIndex& a, VecFloatRef<INDEX1> b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}


RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfAdd(VecFloatRefIndex& a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfAdd(VecFloatRefIndex& a, const float& b) 
{
    a.SetFloat(GetFloat(a) + GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE float& SelfAdd(float& a, VecFloat::InParam b) 
{
    a += b.GetFloat();
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE float& SelfAdd(float& a, VecFloatRef<INDEX1> b) 
{
    a += b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfAdd(float& a, VecFloatRefIndex::InParam b) 
{
    a += b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfAdd(float& a, const float& b) 
{
    a += b;
    return a;
}

//----------------------------------------------------------------------------
// SelfSubtract
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat& SelfSubtract(VecFloat& a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat& SelfSubtract(VecFloat& a, VecFloatRef<INDEX1> b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfSubtract(VecFloat& a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfSubtract(VecFloat& a, const float& b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfSubtract(VecFloatRef<INDEX1> &a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfSubtract(VecFloatRef<INDEX1> &a, VecFloatRef<INDEX2> b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfSubtract(VecFloatRef<INDEX1> &a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfSubtract(VecFloatRef<INDEX1> &a, const float& b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfSubtract(VecFloatRefIndex& a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfSubtract(VecFloatRefIndex& a, VecFloatRef<INDEX1> b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfSubtract(VecFloatRefIndex& a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfSubtract(VecFloatRefIndex& a, const float& b) 
{
    a.SetFloat(GetFloat(a) - GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE float& SelfSubtract(float& a, VecFloat::InParam b) 
{
    a -= b.GetFloat();
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE float& SelfSubtract(float& a, VecFloatRef<INDEX1> b) 
{
    a -= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfSubtract(float& a, VecFloatRefIndex::InParam b) 
{
    a -= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfSubtract(float& a, const float& b) 
{
    a -= b;
    return a;
}

//----------------------------------------------------------------------------
// SelfMult
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat& SelfMult(VecFloat& a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat& SelfMult(VecFloat& a, VecFloatRef<INDEX1> b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfMult(VecFloat& a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfMult(VecFloat& a, const float& b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfMult(VecFloatRef<INDEX1> &a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfMult(VecFloatRef<INDEX1> &a, VecFloatRef<INDEX2> b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfMult(VecFloatRef<INDEX1> &a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfMult(VecFloatRef<INDEX1> &a, const float& b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfMult(VecFloatRefIndex& a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfMult(VecFloatRefIndex& a, VecFloatRef<INDEX1> b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfMult(VecFloatRefIndex& a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfMult(VecFloatRefIndex& a, const float& b) 
{
    a.SetFloat(GetFloat(a) * GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE float& SelfMult(float& a, VecFloat::InParam b) 
{
    a *= b.GetFloat();
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE float& SelfMult(float& a, VecFloatRef<INDEX1> b) 
{
    a *= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfMult(float& a, VecFloatRefIndex::InParam b) 
{
    a *= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfMult(float& a, const float& b) 
{
    a *= b;
    return a;
}


//----------------------------------------------------------------------------
// SelfDivide
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat& SelfDivide(VecFloat& a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat& SelfDivide(VecFloat& a, VecFloatRef<INDEX1> b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfDivide(VecFloat& a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloat& SelfDivide(VecFloat& a, const float& b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfDivide(VecFloatRef<INDEX1> &a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfDivide(VecFloatRef<INDEX1> &a, VecFloatRef<INDEX2> b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfDivide(VecFloatRef<INDEX1> &a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfDivide(VecFloatRef<INDEX1> &a, const float& b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfDivide(VecFloatRefIndex& a, VecFloat::InParam b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfDivide(VecFloatRefIndex& a, VecFloatRef<INDEX1> b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfDivide(VecFloatRefIndex& a, VecFloatRefIndex::InParam b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE VecFloatRefIndex& SelfDivide(VecFloatRefIndex& a, const float& b) 
{
    a.SetFloat(GetFloat(a) / GetFloat(b));
    return a;
}

RW_MATH_FORCE_INLINE float& SelfDivide(float& a, VecFloat::InParam b) 
{
    a /= b.GetFloat();
    return a;
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE float& SelfDivide(float& a, VecFloatRef<INDEX1> b) 
{
    a /= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfDivide(float& a, VecFloatRefIndex::InParam b) 
{
    a /= b.GetFloat();
    return a;
}

RW_MATH_FORCE_INLINE float& SelfDivide(float& a, const float& b) 
{
    a /= b;
    return a;
}


#define RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOAT(op, a, b) \
    return VecFloat(a.GetFloat() op b.GetFloat()); \

#define RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREF(op, a, b) \
    return VecFloat(a.GetFloat() op b.GetFloat()); \

#define RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREFINDEX(op, a, b) \
    return VecFloat(a.GetFloat() op b.GetFloat()); \

#define RWMATH_DETAIL_OPERATION_VECFLOAT_FLOAT(op, a, b) \
    return VecFloat(a.GetFloat() op b); \

#define RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOAT(op, a, b) \
    return VecFloat(a.GetFloat() op b.GetFloat()); \

#define RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREF(op, a, b) \
    return VecFloat(a.GetFloat() op b.GetFloat()); \

#define RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREFINDEX(op, a, b) \
    return VecFloat(a.GetFloat() op b.GetFloat()); \

#define RWMATH_DETAIL_OPERATION_VECFLOATREF_FLOAT(op, a, b) \
    return VecFloat(a.GetFloat() op b); \

#define RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOAT(op, a, b) \
    return VecFloat(a.GetFloat() op b.GetFloat()); \

#define RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREF(op, a, b) \
    return VecFloat(a.GetFloat() op b.GetFloat()); \

#define RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(op, a, b) \
    return VecFloat(a.GetFloat() op b.GetFloat()); \

#define RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_FLOAT(op, a, b) \
    return VecFloat(a.GetFloat() op b); \



//----------------------------------------------------------------------------
// Add
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOAT(+, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, VecFloatRef<INDEX1> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREF(+, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREFINDEX(+, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_FLOAT(+, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX1> a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOAT(+, a, b);
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREF(+, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREFINDEX(+, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX1> a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_FLOAT(+, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOAT(+, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREF(+, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(+, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_FLOAT(+, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Add(const float& a, VecFloat::InParam b) 
{
    return VecFloat(a + b.GetFloat());
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Add(const float& a, VecFloatRef<INDEX1> b) 
{
    return VecFloat(a + b.GetFloat());
}

RW_MATH_FORCE_INLINE VecFloat Add(const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a + b.GetFloat());
}

RW_MATH_FORCE_INLINE VecFloat Add(const float& a, const float& b) 
{
    return VecFloat(a + b);
}

//----------------------------------------------------------------------------
// Subtract
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOAT(-, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, VecFloatRef<INDEX1> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREF(-, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREFINDEX(-, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_FLOAT(-, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX1> a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOAT(-, a, b);
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREF(-, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREFINDEX(-, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX1> a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_FLOAT(-, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOAT(-, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREF(-, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(-, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_FLOAT(-, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, VecFloat::InParam b) 
{
    return VecFloat(a - b.GetFloat());
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, VecFloatRef<INDEX1> b) 
{
    return VecFloat(a - b.GetFloat());
}

RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a - b.GetFloat());
}

RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, const float& b) 
{
    return VecFloat(a - b);
}

//----------------------------------------------------------------------------
// Mult
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOAT(*, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, VecFloatRef<INDEX1> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREF(*, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREFINDEX(*, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_FLOAT(*, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX1> a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOAT(*, a, b);
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREF(*, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREFINDEX(*, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX1> a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_FLOAT(*, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOAT(*, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREF(*, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(*, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_FLOAT(*, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, VecFloat::InParam b) 
{
    return VecFloat(a * b.GetFloat());
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, VecFloatRef<INDEX1> b) 
{
    return VecFloat(a * b.GetFloat());
}

RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a * b.GetFloat());
}

RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, const float& b) 
{
    return VecFloat(a * b);
}

//----------------------------------------------------------------------------
// operator /
//----------------------------------------------------------------------------
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOAT(/, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, VecFloatRef<INDEX1> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREF(/, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_VECFLOATREFINDEX(/, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOAT_FLOAT(/, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX1> a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOAT(/, a, b);
}

template<VectorAxis INDEX1, VectorAxis INDEX2>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREF(/, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_VECFLOATREFINDEX(/, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX1> a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREF_FLOAT(/, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, VecFloat::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOAT(/, a, b);
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREF(/, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_VECFLOATREFINDEX(/, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, const float& b) 
{
    RWMATH_DETAIL_OPERATION_VECFLOATREFINDEX_FLOAT(/, a, b);
}

RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, VecFloat::InParam b) 
{
    return VecFloat(a / b.GetFloat());
}

template<VectorAxis INDEX1>
RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, VecFloatRef<INDEX1> b) 
{
    return VecFloat(a / b.GetFloat());
}

RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, VecFloatRefIndex::InParam b) 
{
    return VecFloat(a / b.GetFloat());
}

RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, const float& b) 
{
    return VecFloat(a / b);
}


}

}

}

#endif
