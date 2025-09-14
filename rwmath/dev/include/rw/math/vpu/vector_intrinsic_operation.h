// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_VECTOR_INTRINSIC_OPERATION_H
#define RW_MATH_VPU_VECTOR_INTRINSIC_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vector_axis.h"

namespace rw {
namespace math {
namespace vpu {

#if RW_MATH_ENABLE_DEPRECATED_API

RW_MATH_FORCE_INLINE bool _asmIsEqualV4(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);
RW_MATH_FORCE_INLINE bool _asmIsEqualV3(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);
RW_MATH_FORCE_INLINE bool _asmIsEqualV2(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

RW_MATH_FORCE_INLINE VectorIntrinsic _asmIsGreaterEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSelect(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1, VectorIntrinsicInParam mask);

RW_MATH_FORCE_INLINE bool _asmIsZeroV4(VectorIntrinsicInParam v,
                                VectorIntrinsicInParam tolerance);
RW_MATH_FORCE_INLINE bool _asmIsZeroV4(VectorIntrinsicInParam x,
                                VectorIntrinsicInParam y,
                                VectorIntrinsicInParam z,
                                VectorIntrinsicInParam w,
                                VectorIntrinsicInParam tolerance);
RW_MATH_FORCE_INLINE bool _asmIsZeroV2(VectorIntrinsicInParam v,
                                VectorIntrinsicInParam tolerance);
RW_MATH_FORCE_INLINE bool _asmIsZeroV3(VectorIntrinsicInParam x,
                                VectorIntrinsicInParam y,
                                VectorIntrinsicInParam z,
                                VectorIntrinsicInParam tolerance);
RW_MATH_FORCE_INLINE bool _asmIsSimilarV4(VectorIntrinsicInParam v0, 
                                   VectorIntrinsicInParam v1,
                                   VectorIntrinsicInParam tolerance);
RW_MATH_FORCE_INLINE bool _asmIsSimilarV2(VectorIntrinsicInParam v0,
                                   VectorIntrinsicInParam v1,
                                   VectorIntrinsicInParam tolerance);
RW_MATH_FORCE_INLINE bool _asmIsSimilarV3(VectorIntrinsicInParam v0,
                                   VectorIntrinsicInParam v1,
                                   VectorIntrinsicInParam tolerance);
RW_MATH_FORCE_INLINE bool _asmIsSimilarV3(VectorIntrinsicInParam x0,
                                   VectorIntrinsicInParam y0,
                                   VectorIntrinsicInParam z0,
                                   VectorIntrinsicInParam x1,
                                   VectorIntrinsicInParam y1,
                                   VectorIntrinsicInParam z1,
                                   VectorIntrinsicInParam tolerance);
RW_MATH_FORCE_INLINE bool _asmIsSimilarV3(VectorIntrinsicInParam x0,
                                   VectorIntrinsicInParam y0,
                                   VectorIntrinsicInParam z0,
                                   VectorIntrinsicInParam w0,
                                   VectorIntrinsicInParam x1,
                                   VectorIntrinsicInParam y1,
                                   VectorIntrinsicInParam z1,
                                   VectorIntrinsicInParam w1,
                                   VectorIntrinsicInParam tolerance);
template<VectorAxis INDEX_DEST, VectorAxis INDEX_SRC>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSwizzleStore(VectorIntrinsicInParam dest, VectorIntrinsicInParam src);
template<VectorAxis INDEX_DEST>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSwizzleStore(VectorIntrinsicInParam dest, VectorIntrinsicInParam src, VectorAxis srcIndex);
template<VectorAxis INDEX_SRC>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSwizzleStore(VectorIntrinsicInParam dest, VectorAxis destIndex, VectorIntrinsicInParam src);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSwizzleStore(VectorIntrinsicInParam dest, VectorAxis destIndex, VectorIntrinsicInParam src, VectorAxis srcIndex);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmMultAdd(VectorIntrinsicInParam a, VectorIntrinsicInParam b,  VectorIntrinsicInParam c);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmMult(VectorIntrinsicInParam a, VectorIntrinsicInParam b);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSubtract(VectorIntrinsicInParam a, VectorIntrinsicInParam b);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmGetZero();
RW_MATH_FORCE_INLINE VectorIntrinsic _asmGetOne();
template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSplat(VectorIntrinsicInParam v );
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSplat(VectorIntrinsicInParam v, VectorAxis index );
RW_MATH_FORCE_INLINE VectorIntrinsic _asmReciprocalSquareRootEstimate(VectorIntrinsicInParam v);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmReciprocalEstimate(VectorIntrinsicInParam in);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmReciprocalFast(VectorIntrinsicInParam in);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmReciprocal(VectorIntrinsicInParam in);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmDot(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmDotV3(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);
RW_MATH_FORCE_INLINE VectorIntrinsic _asmDotV2(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);
#endif // RW_MATH_ENABLE_DEPRECATED_API

}

using namespace rw::math::vpu;

}}


#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/vector_intrinsic_operation_inline.h"
#endif

#endif
