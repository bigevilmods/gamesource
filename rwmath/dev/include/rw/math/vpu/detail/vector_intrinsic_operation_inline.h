// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VECTOR_INTRINSIC_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_VECTOR_INTRINSIC_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/vpu/constants.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"

#ifndef RW_MATH_TAG_API_DURING_PREPROCESSING
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"
#endif

namespace rw
{
namespace math
{
namespace vpu
{

#if RW_MATH_ENABLE_DEPRECATED_API


RW_MATH_FORCE_INLINE VectorIntrinsic _asmIsGreaterEqual(VectorIntrinsicInParam a, VectorIntrinsicInParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_IsGreaterEqual(ret, a, b);
    return ret;

}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmSelect(VectorIntrinsicInParam a, VectorIntrinsicInParam b, VectorIntrinsicInParam mask)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Select(ret, a, b, mask);
    return ret;

}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmMultAdd(VectorIntrinsicInParam a, VectorIntrinsicInParam b,  VectorIntrinsicInParam c)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_MultAdd(ret, a, b, c);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmMult(VectorIntrinsicInParam a, VectorIntrinsicInParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Mult(ret, a, b);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmSubtract(VectorIntrinsicInParam a, VectorIntrinsicInParam b)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Subtract(ret, a, b);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmGetZero()
{
    VectorIntrinsic ret;
    RW_MATH_VPU_GetZero(ret);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmGetOne()
{
    VectorIntrinsic ret;
    RW_MATH_VPU_GetOne(ret);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSplat(VectorIntrinsicInParam v )
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Splat_I(ret, v, INDEX);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmSplat(VectorIntrinsicInParam v, VectorAxis index )
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Splat(ret, v, index);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmSwizzleStore(VectorIntrinsicInParam dest, VectorAxis destIndex, VectorIntrinsicInParam src, VectorAxis srcIndex) 
{
    rwMathAlignmentFunctionCheckAssert(dest, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(src, sizeof(VectorIntrinsic));
    VectorIntrinsic ret;
    RW_MATH_VPU_SwizzleStore_V_V(ret, dest, destIndex, src, srcIndex);
    return ret;
}

template<VectorAxis INDEX_DEST, VectorAxis INDEX_SRC>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSwizzleStore(VectorIntrinsicInParam dest, VectorIntrinsicInParam src) 
{
    rwMathAlignmentFunctionCheckAssert(dest, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(src, sizeof(VectorIntrinsic));
    VectorIntrinsic ret;
    RW_MATH_VPU_SwizzleStore_I_I(ret, dest, INDEX_DEST, src, INDEX_SRC);
    return ret;
}

template<VectorAxis INDEX_DEST>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSwizzleStore(VectorIntrinsicInParam dest, VectorIntrinsicInParam src, VectorAxis srcIndex) 
{
    rwMathAlignmentFunctionCheckAssert(dest, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(src, sizeof(VectorIntrinsic));
    VectorIntrinsic ret;
    RW_MATH_VPU_SwizzleStore_I_V(ret, dest, INDEX_DEST, src, srcIndex);
    return ret;
}

template<VectorAxis INDEX_SRC>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmSwizzleStore(VectorIntrinsicInParam dest, VectorAxis destIndex, VectorIntrinsicInParam src) 
{
    rwMathAlignmentFunctionCheckAssert(dest, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(src, sizeof(VectorIntrinsic));
    VectorIntrinsic ret;
    RW_MATH_VPU_SwizzleStore_V_I(ret, dest, destIndex, src, INDEX_SRC);
    return ret;
}


RW_MATH_FORCE_INLINE VectorIntrinsic _asmReciprocalSquareRootEstimate(VectorIntrinsicInParam v)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_ReciprocalSquareRootEstimate(ret, v);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmReciprocalEstimate(VectorIntrinsicInParam in)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_ReciprocalEstimate(ret, in);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmReciprocalFast(VectorIntrinsicInParam in)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_ReciprocalFast(ret, in);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmReciprocal(VectorIntrinsicInParam v)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_Reciprocal(ret, v);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmDot(VectorIntrinsicInParam v0,
                                                      VectorIntrinsicInParam v1)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV4(ret, v0, v1);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmDotV3(VectorIntrinsicInParam v0,
                                                        VectorIntrinsicInParam v1)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV3(ret, v0, v1);
    return ret;
}

RW_MATH_FORCE_INLINE VectorIntrinsic _asmDotV2(VectorIntrinsicInParam v0,
                                                        VectorIntrinsicInParam v1)
{
    VectorIntrinsic ret;
    RW_MATH_VPU_DotV2(ret, v0, v1);
    return ret;
}

#endif // RW_MATH_ENABLE_DEPRECATED_API

template<VectorAxis XAXIS>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmCreateVectorAxis(VectorIntrinsicInParam x) 
{
    rwMathAlignmentFunctionCheckAssert(x, sizeof(VectorIntrinsic));

    VectorIntrinsic ret;
    RW_MATH_VPU_CreateVectorAxisA1_I(ret, x, XAXIS);
    return ret;
}

template<>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmCreateVectorAxis<VectorAxisX>(VectorIntrinsicInParam x) 
{
    rwMathAlignmentFunctionCheckAssert(x, sizeof(VectorIntrinsic));

    return x;
}

template<VectorAxis XAXIS, VectorAxis YAXIS>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmCreateVectorAxis(VectorIntrinsicInParam x,
                                                                  VectorIntrinsicInParam y) 
{
    rwMathAlignmentFunctionCheckAssert(x, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y, sizeof(VectorIntrinsic));

    VectorIntrinsic ret;
    RW_MATH_VPU_CreateVectorAxisA2_I(ret, x, XAXIS, y, YAXIS);
    return ret;
}

template<VectorAxis XAXIS, VectorAxis YAXIS, VectorAxis ZAXIS>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmCreateVectorAxis(VectorIntrinsicInParam x,
                                                                  VectorIntrinsicInParam y,
                                                                  VectorIntrinsicInParam z) 
{
    rwMathAlignmentFunctionCheckAssert(x, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z, sizeof(VectorIntrinsic));
    VectorIntrinsic ret;
    RW_MATH_VPU_CreateVectorAxisA3_I(ret, x, XAXIS, y, YAXIS, z, ZAXIS);
    return ret;
}

template<VectorAxis XAXIS, VectorAxis YAXIS, VectorAxis ZAXIS, VectorAxis WAXIS>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmCreateVectorAxis(VectorIntrinsicInParam x,
                                                                  VectorIntrinsicInParam y,
                                                                  VectorIntrinsicInParam z,
                                                                  VectorIntrinsicInParam w) 
{
    rwMathAlignmentFunctionCheckAssert(x, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(w, sizeof(VectorIntrinsic));
    VectorIntrinsic ret;
    RW_MATH_VPU_CreateVectorAxisA4_I(ret, x, XAXIS, y, YAXIS, z, ZAXIS, w, WAXIS);
    return ret;
}

//------------------------------------------------------------------------------------------------------------
// Axis extraction

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmDivideAxis(VectorIntrinsicInParam v0,
                                                             VectorIntrinsicInParam v1) 
{
    rwMathAlignmentFunctionCheckAssert(v0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v1, sizeof(VectorIntrinsic));
    VectorIntrinsic ret;
    RW_MATH_VPU_DivideAxisA1_I(ret, v0, v1, INDEX);
    return ret;
}

template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE void _asmDivideAxis(VectorIntrinsic &o0,
                                              VectorIntrinsic &o1,
                                              VectorIntrinsic &o2,
                                              VectorIntrinsicInParam i0,
                                              VectorIntrinsicInParam i1,
                                              VectorIntrinsicInParam i2,
                                              VectorIntrinsicInParam scale) 
{
    rwMathAlignmentFunctionCheckAssert(o0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(o1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(o2, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(i0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(i1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(i2, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(scale, sizeof(VectorIntrinsic));
    RW_MATH_VPU_DivideAxisA3_I(o0, o1, o2, i0, i1, i2, scale, INDEX);
}


template<VectorAxis INDEX>
RW_MATH_FORCE_INLINE VectorIntrinsic _asmMultAxis(VectorIntrinsicInParam v0,
                                                             VectorIntrinsicInParam v1) 
{
    rwMathAlignmentFunctionCheckAssert(v0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v1, sizeof(VectorIntrinsic));
    VectorIntrinsic ret;
    RW_MATH_VPU_MultAxisA1_I(ret, v0, v1, INDEX);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsEqualV4(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1) 
{
    rwMathAlignmentFunctionCheckAssert(v0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v1, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsEqualV4(ret, v0, v1);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsEqualV3(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1) 
{
    rwMathAlignmentFunctionCheckAssert(v0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v1, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsEqualV3(ret, v0, v1);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsEqualV2(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1) 
{
    rwMathAlignmentFunctionCheckAssert(v0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v1, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsEqualV2(ret, v0, v1);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsZeroV4(VectorIntrinsicInParam v,
                                         VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsZeroA1V4(ret, v, tolerance);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsZeroV4(VectorIntrinsicInParam x,
                                          VectorIntrinsicInParam y,
                                          VectorIntrinsicInParam z,
                                          VectorIntrinsicInParam w,
                                          VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(x, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(w, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsZeroA4V4(ret, x, y, z, w, tolerance);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsZeroV2(VectorIntrinsicInParam v,
                                            VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsZeroA1V2(ret, v, tolerance);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsZeroV3(VectorIntrinsicInParam v,
                                            VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(v, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsZeroA1V3(ret, v, tolerance);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsZeroV3(VectorIntrinsicInParam x,
                                           VectorIntrinsicInParam y,
                                           VectorIntrinsicInParam z,
                                           VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(x, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsZeroA3V3(ret, x, y, z, tolerance);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsZeroV3(VectorIntrinsicInParam x,
                                            VectorIntrinsicInParam y,
                                            VectorIntrinsicInParam z,
                                            VectorIntrinsicInParam w,
                                            VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(x, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(w, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsZeroA4V3(ret, x, y, z, w, tolerance);
    return ret;
}


RW_MATH_FORCE_INLINE bool _asmIsSimilarV4(VectorIntrinsicInParam v0,
                                              VectorIntrinsicInParam v1,
                                              VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(v0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsSimilarA1V4(ret, v0, v1, tolerance);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsSimilarV4(VectorIntrinsicInParam x0,
                                             VectorIntrinsicInParam y0,
                                             VectorIntrinsicInParam z0,
                                             VectorIntrinsicInParam w0,
                                             VectorIntrinsicInParam x1,
                                             VectorIntrinsicInParam y1,
                                             VectorIntrinsicInParam z1,
                                             VectorIntrinsicInParam w1,
                                             VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(x0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(w0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(x1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(w1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsSimilarA4V4(ret, x0, y0, z0, w0, x1, y1, z1, w1, tolerance);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsSimilarV2(VectorIntrinsicInParam v0,
                                              VectorIntrinsicInParam v1,
                                              VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(v0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsSimilarA1V2(ret, v0, v1, tolerance);
    return ret;
}

RW_MATH_FORCE_INLINE bool _asmIsSimilarV3(VectorIntrinsicInParam v0,
                                               VectorIntrinsicInParam v1,
                                               VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(v0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(v1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsSimilarA1V3(ret, v0, v1, tolerance);
    return ret;
}


RW_MATH_FORCE_INLINE bool _asmIsSimilarV3(VectorIntrinsicInParam x0,
                                               VectorIntrinsicInParam y0,
                                               VectorIntrinsicInParam z0,
                                               VectorIntrinsicInParam x1,
                                               VectorIntrinsicInParam y1,
                                               VectorIntrinsicInParam z1,
                                               VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(x0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(x1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsSimilarA3V3(ret, x0, y0, z0, x1, y1, z1, tolerance);
    return ret;
}


RW_MATH_FORCE_INLINE bool _asmIsSimilarV3(VectorIntrinsicInParam x0,
                                               VectorIntrinsicInParam y0,
                                               VectorIntrinsicInParam z0,
                                               VectorIntrinsicInParam w0,
                                               VectorIntrinsicInParam x1,
                                               VectorIntrinsicInParam y1,
                                               VectorIntrinsicInParam z1,
                                               VectorIntrinsicInParam w1,
                                               VectorIntrinsicInParam tolerance) 
{
    rwMathAlignmentFunctionCheckAssert(x0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(w0, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(x1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(y1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(z1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(w1, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(tolerance, sizeof(VectorIntrinsic));
    bool ret;
    RW_MATH_VPU_IsSimilarA4V3(ret, x0, y0, z0, w0, x1, y1, z1, w1, tolerance);
    return ret;
}

//------------------------------------------------------------------------------------------------------------
RW_MATH_FORCE_INLINE void _asmDeterminant(VectorIntrinsic &od,
                                               VectorIntrinsicInParam ix,
                                               VectorIntrinsicInParam iy,
                                               VectorIntrinsicInParam iz,
                                               VectorIntrinsicInParam iw) 
{
    rwMathAlignmentFunctionCheckAssert(od, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(ix, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(iy, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(iz, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(iw, sizeof(VectorIntrinsic));

    VectorIntrinsic shift1Y, shift1Z, shift1W, shift2Y, shift2Z, shift2W, shift3Y, shift3Z, shift3W;

    RW_MATH_VPU_SwizzleYZWX(shift1Y, iy);
    RW_MATH_VPU_SwizzleYZWX(shift1Z, iz);
    RW_MATH_VPU_SwizzleYZWX(shift1W, iw);
    RW_MATH_VPU_SwizzleYZWX(shift2Y, shift1Y);
    RW_MATH_VPU_SwizzleYZWX(shift2Z, shift1Z);
    RW_MATH_VPU_SwizzleYZWX(shift2W, shift1W);
    RW_MATH_VPU_SwizzleYZWX(shift3Y, shift2Y);
    RW_MATH_VPU_SwizzleYZWX(shift3Z, shift2Z);
    RW_MATH_VPU_SwizzleYZWX(shift3W, shift2W);

    VectorIntrinsic productYZW;
    VectorIntrinsic tmp0, tmp1, tmp2;
    RW_MATH_VPU_ProductDifference(tmp0, shift2Z, shift3W, shift3Z, shift2W);
    RW_MATH_VPU_ProductDifference(tmp1, shift3Z, shift1W, shift1Z, shift3W);
    RW_MATH_VPU_ProductDifference(tmp2, shift1Z, shift2W, shift2Z, shift1W);

    RW_MATH_VPU_ProductSum3(productYZW, shift1Y, tmp0, shift2Y, tmp1, shift3Y, tmp2);
    RW_MATH_VPU_NegateYW(productYZW, productYZW);

    // Calculate determinant
    RW_MATH_VPU_DotV4(od, ix, productYZW);
}

RW_MATH_FORCE_INLINE void _asmInverse(VectorIntrinsic &od,
                                           VectorIntrinsic &ox,
                                           VectorIntrinsic &oy,
                                           VectorIntrinsic &oz,
                                           VectorIntrinsic &ow,
                                           VectorIntrinsicInParam ix,
                                           VectorIntrinsicInParam iy,
                                           VectorIntrinsicInParam iz,
                                           VectorIntrinsicInParam iw) 
{
    // See "Adjoint method" here http://www.mathwords.com/i/inverse_of_a_matrix.htm for details.
    rwMathAlignmentFunctionCheckAssert(od, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(ox, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(oy, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(oz, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(ow, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(ix, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(iy, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(iz, sizeof(VectorIntrinsic));
    rwMathAlignmentFunctionCheckAssert(iw, sizeof(VectorIntrinsic));

    VectorIntrinsic shift1X, shift1Y, shift1Z, shift1W, shift2X, shift2Y, shift2Z, shift2W, shift3X, shift3Y, shift3Z, shift3W;

    // Generate all possible rotations of the input vectors
    RW_MATH_VPU_SwizzleYZWX(shift1X, ix);
    RW_MATH_VPU_SwizzleYZWX(shift1Y, iy);
    RW_MATH_VPU_SwizzleYZWX(shift1Z, iz);
    RW_MATH_VPU_SwizzleYZWX(shift1W, iw);
    RW_MATH_VPU_SwizzleYZWX(shift2X, shift1X);
    RW_MATH_VPU_SwizzleYZWX(shift2Y, shift1Y);
    RW_MATH_VPU_SwizzleYZWX(shift2Z, shift1Z);
    RW_MATH_VPU_SwizzleYZWX(shift2W, shift1W);
    RW_MATH_VPU_SwizzleYZWX(shift3X, shift2X);
    RW_MATH_VPU_SwizzleYZWX(shift3Y, shift2Y);
    RW_MATH_VPU_SwizzleYZWX(shift3Z, shift2Z);
    RW_MATH_VPU_SwizzleYZWX(shift3W, shift2W);

    // Generate products of 3 adjacent rows
    // productYZW, productZWX, productWXY, productXYZ are the x, y, z, w rows (respectively) of a cofactor matrix.
    VectorIntrinsic productYZW, productZWX, productWXY, productXYZ;

    {
        VectorIntrinsic tmp0, tmp1, tmp2;
        RW_MATH_VPU_ProductDifference(tmp0, shift2Z, shift3W, shift3Z, shift2W);
        RW_MATH_VPU_ProductDifference(tmp1, shift3Z, shift1W, shift1Z, shift3W);
        RW_MATH_VPU_ProductDifference(tmp2, shift1Z, shift2W, shift2Z, shift1W);

        RW_MATH_VPU_ProductSum3(productYZW, shift1Y, tmp0, shift2Y, tmp1, shift3Y, tmp2);
        RW_MATH_VPU_NegateYW(productYZW, productYZW);
    }


    {
        VectorIntrinsic tmp0, tmp1, tmp2;
        RW_MATH_VPU_ProductDifference(tmp0, shift2W, shift3X, shift3W, shift2X);
        RW_MATH_VPU_ProductDifference(tmp1, shift3W, shift1X, shift1W, shift3X);
        RW_MATH_VPU_ProductDifference(tmp2, shift1W, shift2X, shift2W, shift1X);

        RW_MATH_VPU_ProductSum3(productZWX, shift1Z, tmp0, shift2Z, tmp1, shift3Z, tmp2);
        RW_MATH_VPU_NegateXZ(productZWX, productZWX);
    }

    {
        VectorIntrinsic tmp0, tmp1, tmp2;
        RW_MATH_VPU_ProductDifference(tmp0, shift2X, shift3Y, shift3X, shift2Y);
        RW_MATH_VPU_ProductDifference(tmp1, shift3X, shift1Y, shift1X, shift3Y);
        RW_MATH_VPU_ProductDifference(tmp2, shift1X, shift2Y, shift2X, shift1Y);

        RW_MATH_VPU_ProductSum3(productWXY, shift1W, tmp0, shift2W, tmp1, shift3W, tmp2);
        RW_MATH_VPU_NegateYW(productWXY, productWXY);
    }

    {
        VectorIntrinsic tmp0, tmp1, tmp2;
        RW_MATH_VPU_ProductDifference(tmp0, shift2Y, shift3Z, shift3Y, shift2Z);
        RW_MATH_VPU_ProductDifference(tmp1, shift3Y, shift1Z, shift1Y, shift3Z);
        RW_MATH_VPU_ProductDifference(tmp2, shift1Y, shift2Z, shift2Y, shift1Z);

        RW_MATH_VPU_ProductSum3(productXYZ, shift1X, tmp0, shift2X, tmp1, shift3X, tmp2);
        RW_MATH_VPU_NegateXZ(productXYZ, productXYZ);
    }

    // Calculate determinant
    RW_MATH_VPU_DotV4(od, ix, productYZW);

    // Calculate inverse
    VectorIntrinsic x, y, z, w;
    // Transpose the cofactor matrix to find the adjoint matrix
    RW_MATH_VPU_TransposeA4V4(x, y, z, w, productYZW, productZWX, productWXY, productXYZ);
    // Divide by od to find the inverse of the original matrix
    RW_MATH_VPU_Divide(ox, x, od);
    RW_MATH_VPU_Divide(oy, y, od);
    RW_MATH_VPU_Divide(oz, z, od);
    RW_MATH_VPU_Divide(ow, w, od);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
