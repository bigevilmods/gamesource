// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_VEC_FLOAT_OPERATION_H
#define RW_MATH_VPU_VEC_FLOAT_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/vpu/vec_float.h"

namespace rw {
namespace math {
namespace vpu {

// ------------------------------------------------------
template<VectorAxis INDEX> RW_MATH_FORCE_INLINE VecFloat Negate(VecFloatRef<INDEX> a);
                           RW_MATH_FORCE_INLINE VecFloat Negate(VecFloat::InParam a);
                           RW_MATH_FORCE_INLINE VecFloat Negate(VecFloatRefIndex::InParam a);

// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2> RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfAdd(VecFloatRef<INDEX1> &a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfAdd(VecFloatRef<INDEX1> &a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfAdd(VecFloatRef<INDEX1> &a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfAdd(VecFloatRef<INDEX1> &a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat&            SelfAdd(VecFloat& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfAdd(VecFloat& a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfAdd(VecFloat& a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfAdd(VecFloat& a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfAdd(VecFloatRefIndex& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfAdd(VecFloatRefIndex& a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfAdd(VecFloatRefIndex& a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfAdd(VecFloatRefIndex& a, const float& b);
                                               RW_MATH_FORCE_INLINE float&               SelfAdd(float& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE float&               SelfAdd(float& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE float&               SelfAdd(float& a, VecFloatRefIndex::InParam b);

// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2> RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfSubtract(VecFloatRef<INDEX1> &a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfSubtract(VecFloatRef<INDEX1> &a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfSubtract(VecFloatRef<INDEX1> &a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfSubtract(VecFloatRef<INDEX1> &a, const float& b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfSubtract(VecFloat& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat&            SelfSubtract(VecFloat& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfSubtract(VecFloat& a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfSubtract(VecFloat& a, const float& b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfSubtract(VecFloatRefIndex& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfSubtract(VecFloatRefIndex& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfSubtract(VecFloatRefIndex& a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfSubtract(VecFloatRefIndex& a, const float& b);
                                               RW_MATH_FORCE_INLINE float&               SelfSubtract(float& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE float&               SelfSubtract(float& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE float&               SelfSubtract(float& a, VecFloatRefIndex::InParam b);

// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2> RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfMult(VecFloatRef<INDEX1> &a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfMult(VecFloatRef<INDEX1> &a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfMult(VecFloatRef<INDEX1> &a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfMult(VecFloatRef<INDEX1> &a, const float& b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfMult(VecFloat& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat&            SelfMult(VecFloat& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfMult(VecFloat& a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfMult(VecFloat& a, const float& b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfMult(VecFloatRefIndex& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfMult(VecFloatRefIndex& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfMult(VecFloatRefIndex& a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfMult(VecFloatRefIndex& a, const float& b);
                                               RW_MATH_FORCE_INLINE float&               SelfMult(float& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE float&               SelfMult(float& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE float&               SelfMult(float& a, VecFloatRefIndex::InParam b);

// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2> RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfDivide(VecFloatRef<INDEX1> &a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfDivide(VecFloatRef<INDEX1> &a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfDivide(VecFloatRef<INDEX1> &a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRef<INDEX1>& SelfDivide(VecFloatRef<INDEX1> &a, const float& b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfDivide(VecFloat& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat&            SelfDivide(VecFloat& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfDivide(VecFloat& a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat&            SelfDivide(VecFloat& a, const float& b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfDivide(VecFloatRefIndex& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfDivide(VecFloatRefIndex& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfDivide(VecFloatRefIndex& a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloatRefIndex&    SelfDivide(VecFloatRefIndex& a, const float& b);
                                               RW_MATH_FORCE_INLINE float&               SelfDivide(float& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE float&               SelfDivide(float& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE float&               SelfDivide(float& a, VecFloatRefIndex::InParam b);

// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2> RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX1> a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRef<INDEX1> a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Add(VecFloat::InParam a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Add(VecFloatRefIndex::InParam a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Add(const float& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Add(const float& a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Add(const float& a, VecFloatRefIndex::InParam b);
                                               
// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2> RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX1> a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRef<INDEX1> a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloat::InParam a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Subtract(VecFloatRefIndex::InParam a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Subtract(const float& a, VecFloatRefIndex::InParam b);
                                               
// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2> RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX1> a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRef<INDEX1> a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Mult(VecFloat::InParam a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Mult(VecFloatRefIndex::InParam a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Mult(const float& a, VecFloatRefIndex::InParam b);


// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2> RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX1> a, VecFloat::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRef<INDEX1> a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Divide(VecFloat::InParam a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Divide(VecFloatRefIndex::InParam a, const float& b);
template<VectorAxis INDEX1>                    RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, VecFloatRef<INDEX1> b);
                                               RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, VecFloat::InParam b);
                                               RW_MATH_FORCE_INLINE VecFloat Divide(const float& a, VecFloatRefIndex::InParam b);

// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE bool operator ==(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
                                                RW_MATH_FORCE_INLINE bool operator ==(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator ==(VecFloat::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator ==(VecFloat::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator ==(VecFloatRef<INDEX> a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator ==(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator ==(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator ==(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator ==(VecFloatRefIndex::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator ==(const float& a, VecFloatRef<INDEX> b);
                                                RW_MATH_FORCE_INLINE bool operator ==(const float& a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator ==(const float& a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator ==(VecFloatRef<INDEX> a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator ==(VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator ==(VecFloat::InParam a, const float& b);

// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE bool operator !=(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
                                                RW_MATH_FORCE_INLINE bool operator !=(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator !=(VecFloat::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator !=(VecFloat::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator !=(VecFloatRef<INDEX> a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator !=(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator !=(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator !=(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator !=(VecFloatRefIndex::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator !=(const float& a, VecFloatRef<INDEX> b);
                                                RW_MATH_FORCE_INLINE bool operator !=(const float& a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator !=(const float& a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator !=(VecFloatRef<INDEX> a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator !=(VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator !=(VecFloat::InParam a, const float& b);
                                                
// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE bool operator >(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
                                                RW_MATH_FORCE_INLINE bool operator >(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator >(VecFloat::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >(VecFloat::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >(VecFloatRef<INDEX> a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator >(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator >(VecFloatRefIndex::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >(const float& a, VecFloatRef<INDEX> b);
                                                RW_MATH_FORCE_INLINE bool operator >(const float& a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator >(const float& a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >(VecFloatRef<INDEX> a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator >(VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator >(VecFloat::InParam a, const float& b);
                                                
// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE bool operator <(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
                                                RW_MATH_FORCE_INLINE bool operator <(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator <(VecFloat::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <(VecFloat::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <(VecFloatRef<INDEX> a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator <(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator <(VecFloatRefIndex::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <(const float& a, VecFloatRef<INDEX> b);
                                                RW_MATH_FORCE_INLINE bool operator <(const float& a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator <(const float& a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <(VecFloatRef<INDEX> a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator <(VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator <(VecFloat::InParam a, const float& b);

// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE bool operator >=(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
                                                RW_MATH_FORCE_INLINE bool operator >=(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator >=(VecFloat::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >=(VecFloat::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >=(VecFloatRef<INDEX> a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >=(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >=(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator >=(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator >=(VecFloatRefIndex::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >=(const float& a, VecFloatRef<INDEX> b);
                                                RW_MATH_FORCE_INLINE bool operator >=(const float& a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator >=(const float& a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator >=(VecFloatRef<INDEX> a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator >=(VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator >=(VecFloat::InParam a, const float& b);

// ------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE bool operator <=(VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
                                                RW_MATH_FORCE_INLINE bool operator <=(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator <=(VecFloat::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <=(VecFloat::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <=(VecFloatRef<INDEX> a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <=(VecFloatRefIndex::InParam a, VecFloatRef<INDEX> b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <=(VecFloatRef<INDEX> a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator <=(VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator <=(VecFloatRefIndex::InParam a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <=(const float& a, VecFloatRef<INDEX> b);
                                                RW_MATH_FORCE_INLINE bool operator <=(const float& a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE bool operator <=(const float& a, VecFloat::InParam b);
template<VectorAxis INDEX>                      RW_MATH_FORCE_INLINE bool operator <=(VecFloatRef<INDEX> a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator <=(VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE bool operator <=(VecFloat::InParam a, const float& b);

//----------------------------------------------------------------------------
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX> a);
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a);
RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a);


//----------------------------------------------------------------------------
template<VectorAxis INDEX1> RW_MATH_FORCE_INLINE VecFloat& operator += (VecFloat& a, VecFloatRef<INDEX1> b);
                            RW_MATH_FORCE_INLINE VecFloat& operator += (VecFloat& a, VecFloat::InParam b);
                            RW_MATH_FORCE_INLINE VecFloat& operator += (VecFloat& a, VecFloatRefIndex::InParam b);
                            RW_MATH_FORCE_INLINE VecFloat& operator += (VecFloat& a, const float& b);
                            RW_MATH_FORCE_INLINE float&    operator += (float& a, VecFloat::InParam b);
template<VectorAxis INDEX1> RW_MATH_FORCE_INLINE float&    operator += (float& a, VecFloatRef<INDEX1> b);
                            RW_MATH_FORCE_INLINE float&    operator += (float& a, VecFloatRefIndex::InParam b);
                            
//----------------------------------------------------------------------------
template<VectorAxis INDEX1> RW_MATH_FORCE_INLINE VecFloat& operator -= (VecFloat& a, VecFloatRef<INDEX1> b);
                            RW_MATH_FORCE_INLINE VecFloat& operator -= (VecFloat& a, VecFloat::InParam b);
                            RW_MATH_FORCE_INLINE VecFloat& operator -= (VecFloat& a, VecFloatRefIndex::InParam b);
                            RW_MATH_FORCE_INLINE VecFloat& operator -= (VecFloat& a, const float& b);
                            RW_MATH_FORCE_INLINE float&    operator -= (float& a, VecFloat::InParam b);
template<VectorAxis INDEX1> RW_MATH_FORCE_INLINE float&    operator -= (float& a, VecFloatRef<INDEX1> b);
                            RW_MATH_FORCE_INLINE float&    operator -= (float& a, VecFloatRefIndex::InParam b);

//----------------------------------------------------------------------------
template<VectorAxis INDEX1> RW_MATH_FORCE_INLINE VecFloat& operator *= (VecFloat& a, VecFloatRef<INDEX1> b);
                            RW_MATH_FORCE_INLINE VecFloat& operator *= (VecFloat& a, VecFloat::InParam b);
                            RW_MATH_FORCE_INLINE VecFloat& operator *= (VecFloat& a, VecFloatRefIndex::InParam b);
                            RW_MATH_FORCE_INLINE VecFloat& operator *= (VecFloat& a, const float& b);
                            RW_MATH_FORCE_INLINE float&    operator *= (float& a, VecFloat::InParam b);
template<VectorAxis INDEX1> RW_MATH_FORCE_INLINE float&    operator *= (float& a, VecFloatRef<INDEX1> b);
                            RW_MATH_FORCE_INLINE float&    operator *= (float& a, VecFloatRefIndex::InParam b);
                            
//----------------------------------------------------------------------------
template<VectorAxis INDEX1> RW_MATH_FORCE_INLINE VecFloat& operator /= (VecFloat& a, VecFloatRef<INDEX1> b);
                            RW_MATH_FORCE_INLINE VecFloat& operator /= (VecFloat& a, VecFloat::InParam b);
                            RW_MATH_FORCE_INLINE VecFloat& operator /= (VecFloat& a, VecFloatRefIndex::InParam b);
                            RW_MATH_FORCE_INLINE VecFloat& operator /= (VecFloat& a, const float& b);
                            RW_MATH_FORCE_INLINE float&    operator /= (float& a, VecFloat::InParam b);
template<VectorAxis INDEX1> RW_MATH_FORCE_INLINE float&    operator /= (float& a, VecFloatRef<INDEX1> b);
                            RW_MATH_FORCE_INLINE float&    operator /= (float& a, VecFloatRefIndex::InParam b);

//----------------------------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRef<INDEX1> a, VecFloat::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRef<INDEX1> a, const float& b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator + (VecFloat::InParam a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator + (VecFloat::InParam a, VecFloat::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator + (VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator + (VecFloat::InParam a, const float& b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRefIndex::InParam a, VecFloat::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator + (VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE VecFloat operator + (const float& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator + (const float& a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator + (const float& a, VecFloatRefIndex::InParam b);

//----------------------------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX1> a, VecFloat::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRef<INDEX1> a, const float& b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a, VecFloat::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator - (VecFloat::InParam a, const float& b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a, VecFloat::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator - (VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE VecFloat operator - (const float& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator - (const float& a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator - (const float& a, VecFloatRefIndex::InParam b);
                                                

//----------------------------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRef<INDEX1> a, VecFloat::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRef<INDEX1> a, const float& b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator * (VecFloat::InParam a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator * (VecFloat::InParam a, VecFloat::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator * (VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator * (VecFloat::InParam a, const float& b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRefIndex::InParam a, VecFloat::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator * (VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE VecFloat operator * (const float& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator * (const float& a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator * (const float& a, VecFloatRefIndex::InParam b);
                                                

//----------------------------------------------------------------------------
template<VectorAxis INDEX1, VectorAxis INDEX2>  RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRef<INDEX1> a, VecFloatRef<INDEX2> b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRef<INDEX1> a, VecFloat::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRef<INDEX1> a, VecFloatRefIndex::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRef<INDEX1> a, const float& b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator / (VecFloat::InParam a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator / (VecFloat::InParam a, VecFloat::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator / (VecFloat::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator / (VecFloat::InParam a, const float& b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRefIndex::InParam a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRefIndex::InParam a, VecFloat::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b);
                                                RW_MATH_FORCE_INLINE VecFloat operator / (VecFloatRefIndex::InParam a, const float& b);
                                                RW_MATH_FORCE_INLINE VecFloat operator / (const float& a, VecFloat::InParam b);
template<VectorAxis INDEX1>                     RW_MATH_FORCE_INLINE VecFloat operator / (const float& a, VecFloatRef<INDEX1> b);
                                                RW_MATH_FORCE_INLINE VecFloat operator / (const float& a, VecFloatRefIndex::InParam b);


}}}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/vec_float_operation_inline.h"
#endif

#endif
