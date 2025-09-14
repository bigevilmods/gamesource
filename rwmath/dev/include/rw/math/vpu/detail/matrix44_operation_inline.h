// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MATRIX44_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_MATRIX44_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/constants_operation.h"
#include "rw/math/vpu/trig.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/matrix33_operation.h"
#include "rw/math/vpu/vector3_operation.h"
#include "rw/math/vpu/vector4_operation.h"

#ifdef EA_PLATFORM_REVOLUTION
    #include "rw/math/vpu/detail/wii/matrix44_operation_platform_inline.h"
#else
    #include "rw/math/vpu/detail/matrix44_operation_platform_inline.h"
#endif

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE bool operator ==(Matrix44::InParam a, Matrix44::InParam b) 
{
    return  a.xAxis == b.xAxis &&
            a.yAxis == b.yAxis &&
            a.zAxis == b.zAxis &&
            a.wAxis == b.wAxis;
}

RW_MATH_FORCE_INLINE bool operator !=(Matrix44::InParam a, Matrix44::InParam b) 
{
    return  a.xAxis != b.xAxis ||
            a.yAxis != b.yAxis ||
            a.zAxis != b.zAxis ||
            a.wAxis != b.wAxis;
}

RW_MATH_FORCE_INLINE bool IsSimilar(Matrix44::InParam a, Matrix44::InParam b, VecFloat::InParam epsilon ) 
{
    return _asmIsSimilarV4( a.xAxis.mV, a.yAxis.mV, a.zAxis.mV, a.wAxis.mV,
                          b.xAxis.mV, b.yAxis.mV, b.zAxis.mV, b.wAxis.mV,
                          epsilon.mV );
}

RW_MATH_FORCE_INLINE bool IsAnyNaN(Matrix44::InParam m)
{
    return (bool)(IsAnyNaN(m.xAxis) | IsAnyNaN(m.yAxis) | IsAnyNaN(m.zAxis) | IsAnyNaN(m.wAxis));
}

RW_MATH_FORCE_INLINE bool IsValid(Matrix44::InParam a) 
{
    return IsValid(a.xAxis) &&
           IsValid(a.yAxis) &&
           IsValid(a.zAxis) &&
           IsValid(a.wAxis);
}

RW_MATH_FORCE_INLINE VecFloat GetNormalError(Matrix44::InParam matrix) 
{
    VecFloat one(RW_MATH_VPU_GetOne_R());
    Vector4 errorVec(   MagnitudeSquared( matrix.xAxis ) - one,
                        MagnitudeSquared( matrix.yAxis ) - one,
                        MagnitudeSquared( matrix.zAxis ) - one,
                        MagnitudeSquared( matrix.wAxis ) - one );
    VecFloat error = MagnitudeSquared(errorVec);
    return error;
}

RW_MATH_FORCE_INLINE bool IsNormal(Matrix44::InParam a, VecFloat::InParam tolerance) 
{
    return IsZero( GetNormalError(a), tolerance );
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError(Matrix44::InParam matrix) 
{
    Matrix44 identity;
    identity.SetIdentity();

    Matrix44 result = Mult(matrix, Transpose(matrix));
    SelfSubtract(result, identity);

    Vector4 errorVec( MagnitudeSquared( result.XAxis() ),
                      MagnitudeSquared( result.YAxis() ),
                      MagnitudeSquared( result.ZAxis() ),
                      MagnitudeSquared( result.WAxis() ) );

    VecFloat error = MagnitudeSquared(errorVec);
    return error;
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError(Matrix44::InParam matrix) 
{
    // Calculate error vectors that "compare" all possible pairs of elements.
    // i.e. 4 choose 2 combinations.
    Vector3 errorVec1(  Dot( matrix.xAxis, matrix.yAxis ),
                        Dot( matrix.xAxis, matrix.zAxis ),
                        Dot( matrix.xAxis, matrix.wAxis ) );
    Vector3 errorVec2(  Dot( matrix.yAxis, matrix.zAxis ),
                        Dot( matrix.yAxis, matrix.wAxis ),
                        Dot( matrix.zAxis, matrix.wAxis ) );
    VecFloat error = MagnitudeSquared(errorVec1) + MagnitudeSquared(errorVec2);
    return error;
}

RW_MATH_FORCE_INLINE bool IsOrthogonal(Matrix44::InParam a, VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalError(a), tolerance );
}

RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet(Matrix44::InParam a, VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalError(a), tolerance );
}

RW_MATH_FORCE_INLINE bool IsRotation(Matrix44::InParam a, VecFloat::InParam tolerance)
{
    return IsOrthogonal(a, tolerance) && IsSimilar(Determinant(a), GetVecFloat_One(), tolerance);
}

RW_MATH_FORCE_INLINE bool IsValid3x3(Matrix44::InParam a) 
{
    return IsValid(Vector3(a.xAxis.mV)) &&
           IsValid(Vector3(a.yAxis.mV)) &&
           IsValid(Vector3(a.zAxis.mV));
}

// operations that return by value

// scalar 
RW_MATH_FORCE_INLINE Matrix44 Negate(Matrix44::InParam m) 
{
    return Matrix44( Negate(m.xAxis), Negate(m.yAxis), Negate(m.zAxis), Negate(m.wAxis) );
}

RW_MATH_FORCE_INLINE Matrix44 Mult(Matrix44::InParam m, VecFloat::InParam a) 
{
    return Matrix44(
            a * m.xAxis,
            a * m.yAxis,
            a * m.zAxis,
            a * m.wAxis
            );
}

template<VectorAxis INDEX> Matrix44 Mult(Matrix44::InParam m, VecFloatRef<INDEX> a)
{
    return Mult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix44 Mult(Matrix44::InParam m, VecFloatRefIndex::InParam a) 
{
    return Mult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix44 Divide(Matrix44::InParam m, VecFloat::InParam a) 
{
#ifdef EA_PLATFORM_REVOLUTION
    // Slightly different code is used on the Wii to work around a compiler/hardware bug
    // that occurs in debug builds when the optimized vector macros are used.
    return Mult(m, 1.0f/a.GetFloat());
#else
    VecFloat recipA = Reciprocal(a);
    return Mult(m, recipA);
#endif
}

template<VectorAxis INDEX> Matrix44 Divide(Matrix44::InParam m, VecFloatRef<INDEX> a)
{
    return Divide(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix44 Divide(Matrix44::InParam m, VecFloatRefIndex::InParam a) 
{
    return Divide(m, VecFloat(a));
}


// matrix
RW_MATH_FORCE_INLINE Matrix44 Add(Matrix44::InParam a, Matrix44::InParam b) 
{
    return Matrix44(
                Add(a.xAxis, b.xAxis),
                Add(a.yAxis, b.yAxis),
                Add(a.zAxis, b.zAxis),
                Add(a.wAxis, b.wAxis)
            );
}

RW_MATH_FORCE_INLINE Matrix44 Subtract(Matrix44::InParam a, Matrix44::InParam b) 
{
    return Matrix44(
        Subtract(a.xAxis, b.xAxis),
        Subtract(a.yAxis, b.yAxis),
        Subtract(a.zAxis, b.zAxis),
        Subtract(a.wAxis, b.wAxis)
        );
}

RW_MATH_FORCE_INLINE void SelfAdd(Matrix44& a, Matrix44::InParam b) 
{
    a = Add(a, b);
}

RW_MATH_FORCE_INLINE void SelfSubtract(Matrix44& a, Matrix44::InParam b) 
{
    a = Subtract(a, b);
}


RW_MATH_FORCE_INLINE void SelfMult(Matrix44& m, VecFloat::InParam a) 
{
    m = Mult(m, a);
}

template<VectorAxis INDEX> void SelfMult(Matrix44& m, VecFloatRef<INDEX> a)
{
    m = Mult(m, a);
}

RW_MATH_FORCE_INLINE void SelfMult(Matrix44& m, VecFloatRefIndex::InParam a) 
{
    m = Mult(m, a);
}

RW_MATH_FORCE_INLINE void SelfDivide(Matrix44& m, VecFloat::InParam a) 
{
    m = Divide(m, a);
}

template<VectorAxis INDEX> void SelfDivide(Matrix44& m, VecFloatRef<INDEX> a)
{
    m = Divide(m, a);
}

RW_MATH_FORCE_INLINE void SelfDivide(Matrix44& m, VecFloatRefIndex::InParam a) 
{
    m = Divide(m, a);
}


RW_MATH_INLINE Matrix44 OrthoNormalize(Matrix44::InParam m)
{
    // Note: This implementation is using the Gram-Schmidt orthonormalization
    // algorithm.  The current implementation is numerically unstable. Also,
    // this algorithm differs from the algorithm implemented for matrix 3x3
    // orthonormalization.
    Matrix44 result;
    const int numRows = 4;
    for( int row = 0; row < numRows; ++row )
    {
        // Initialize the row to the same row from the input matrix.
        const Vector4 &inputRow = m.GetRow(row);
        Vector4 tempResult = inputRow;
        // Subtract the components of the vector that are not orthogonal to the basis created thus far.
        for( int i = 0; i < row; ++i )
        {
            tempResult -= result.GetRow(i) * Dot(result.GetRow(i), inputRow );
        }
        result.SetRow( row, Normalize(tempResult) );
    }
    return result;
}

RW_MATH_FORCE_INLINE Matrix44 InverseOrthonormal(Matrix44::InParam m) 
{
    RW_MATH_ASSERT_MSG(IsOrthogonal(m), ("Input matrix is non orthonormal."));
    return Transpose(m);
}

RW_MATH_FORCE_INLINE Matrix44 Inverse(Matrix44::InParam m) 
{
    VecFloat determinant;
    return Inverse(m, determinant);
}

RW_MATH_FORCE_INLINE Vector3 TransformPoint(Vector3::InParam pt, Matrix44::InParam matrix) 
{
    VectorIntrinsic v = pt.mV;
    VectorIntrinsic mat_x = matrix.xAxis.mV;
    VectorIntrinsic mat_y = matrix.yAxis.mV;
    VectorIntrinsic mat_z = matrix.zAxis.mV;
    VectorIntrinsic mat_w = matrix.wAxis.mV;
    
    VectorIntrinsic ret, xSplat, ySplat, zSplat;
    RW_MATH_VPU_Splat_I( xSplat, v, 0 );
    RW_MATH_VPU_Splat_I( ySplat, v, 1 );
    RW_MATH_VPU_Splat_I( zSplat, v, 2 );
    RW_MATH_VPU_MultAdd( ret, mat_x, xSplat, mat_w );
    RW_MATH_VPU_MultAdd( ret, mat_y, ySplat, ret );
    RW_MATH_VPU_MultAdd( ret, mat_z, zSplat, ret );
    return Vector3(ret);
}

RW_MATH_FORCE_INLINE void TransformPoints(const Vector3* const pts, int numPoints, Matrix44::InParam matrix, Vector3* transformedPts) 
{
    for( int point = 0; point < numPoints; point++ )
    {
        transformedPts[point] = TransformPoint(pts[point], matrix);
    }
}

RW_MATH_FORCE_INLINE Vector3 TransformVector(Vector3::InParam vec, Matrix44::InParam matrix) 
{
    VectorIntrinsic result, xSplat, ySplat, zSplat;
    VectorIntrinsic inVec = vec.mV;

    RW_MATH_VPU_Splat_I( xSplat, inVec, 0 );
    RW_MATH_VPU_Splat_I( ySplat, inVec, 1 );
    RW_MATH_VPU_Splat_I( zSplat, inVec, 2 );
       RW_MATH_VPU_Mult( result, matrix.xAxis.mV, xSplat );
    RW_MATH_VPU_MultAdd( result, matrix.yAxis.mV, ySplat, result );
    RW_MATH_VPU_MultAdd( result, matrix.zAxis.mV, zSplat, result );

    return Vector3(result);
}

RW_MATH_FORCE_INLINE void TransformVectors(const Vector3* const vecs, int numVectors, Matrix44::InParam matrix, Vector3 * transformedVecs) 
{
    for( int vecIndex = 0; vecIndex < numVectors; vecIndex++ )
    {
        transformedVecs[vecIndex] = TransformVector(vecs[vecIndex], matrix);
    }
}

RW_MATH_FORCE_INLINE Vector3 Transform(Vector4::InParam vec, Matrix44::InParam matrix) 
{
    VectorIntrinsic result, xSplat, ySplat, zSplat, wSplat;
    VectorIntrinsic inVec = vec.mV;

    RW_MATH_VPU_Splat_I( xSplat, inVec, VectorAxisX );
    RW_MATH_VPU_Splat_I( ySplat, inVec, VectorAxisY );
    RW_MATH_VPU_Splat_I( zSplat, inVec, VectorAxisZ );
    RW_MATH_VPU_Splat_I( wSplat, inVec, VectorAxisW );
       RW_MATH_VPU_Mult( result, matrix.xAxis.mV, xSplat );
    RW_MATH_VPU_MultAdd( result, matrix.yAxis.mV, ySplat, result );
    RW_MATH_VPU_MultAdd( result, matrix.zAxis.mV, zSplat, result );
    RW_MATH_VPU_MultAdd( result, matrix.wAxis.mV, wSplat, result );

    return Vector3(result);
}

RW_MATH_FORCE_INLINE void Transform(const Vector4* const vecs, int numVectors, Matrix44::InParam matrix, Vector3 * transformedVecs) 
{
    for( int vecIndex = 0; vecIndex < numVectors; vecIndex++ )
    {
        transformedVecs[vecIndex] = Transform(vecs[vecIndex], matrix);
    }
}


RW_MATH_FORCE_INLINE Matrix44 Matrix44FromQuaternion(Quaternion::InParam q) 
{
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic x;
    VectorIntrinsic y;
    VectorIntrinsic z;
    VectorIntrinsic w = zero;

    RW_MATH_VPU_MatrixFromQuaternion(x, y, z, q.mV);

    x = _asmSwizzleStore<VectorAxisW, VectorAxisX>(x, zero);
    y = _asmSwizzleStore<VectorAxisW, VectorAxisX>(y, zero);
    z = _asmSwizzleStore<VectorAxisW, VectorAxisX>(z, zero);
    w = _asmSwizzleStore<VectorAxisW, VectorAxisX>(w, RW_MATH_VPU_GetOne_R());

    return Matrix44(x, y, z, w);
}

#if !defined RW_MATH_USE_REFERENCE_IMPLEMENTATION && defined EA_PLATFORM_PS3
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromEulerXYZ(Vector3::InParam euler) 
{
    VectorIntrinsic sinVec = sinf4(euler.mV);
    VectorIntrinsic cosVec = cosf4(euler.mV);

    VectorIntrinsic signMask;
    RW_MATH_VPU_GetSignMaskXYZW(signMask);
    VectorIntrinsic zeroV = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic negOneV = RW_MATH_VPU_GetNegOne_R();
    VectorIntrinsic permuteX0Y0Z0X1 = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::X1);

    // Create vectors with all sin/cos values as well as zeros in the W component.  The zero will be useful later.
    VectorIntrinsic cosZeroV = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cosVec, zeroV, permuteX0Y0Z0X1 ));
    VectorIntrinsic sinZeroV = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(sinVec, zeroV, permuteX0Y0Z0X1 ));

    VectorIntrinsic ci_ci_si_si = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cosVec, sinVec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X1, ::rw::math::vpu::detail::X1) ));
    VectorIntrinsic ch_sh_ch_sh = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cosVec, sinVec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::Z1, ::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::Z1) ));
    VectorIntrinsic cc_cs_sc_ss;
    // zeroNegOneV = (0, -1, -1, -1)
    VectorIntrinsic zeroNegOneV = RWMATH_VPU_DETAIL_VecSld( zeroV, negOneV, sizeof(float)*3);
    RW_MATH_VPU_MultAdd( cc_cs_sc_ss, ci_ci_si_si, ch_sh_ch_sh, zeroV );

    // X
    // cj * ch
    // cj * sh
    // -1 * sj
    VectorIntrinsic xP0 = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cosVec, zeroNegOneV, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::Y1, ::rw::math::vpu::detail::X1) ));
    VectorIntrinsic xP1 = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cosVec, sinVec, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::Z1, ::rw::math::vpu::detail::Y1, ::rw::math::vpu::detail::X0/*dummy arg*/) ));
    VectorIntrinsic xRow;
    RW_MATH_VPU_MultAdd( xRow, xP0, xP1, zeroV );

    // Y
    // sj * sc - cs 
    // sj * ss + cc
    // cj * si + 0
    VectorIntrinsic yP0 = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cosZeroV, sinZeroV, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Y1, ::rw::math::vpu::detail::Y1, ::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::W0) ));
    VectorIntrinsic yP1 = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cc_cs_sc_ss, sinZeroV, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::W0, ::rw::math::vpu::detail::X1, ::rw::math::vpu::detail::W1) ));
    VectorIntrinsic yA = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cc_cs_sc_ss, zeroV, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::X1, ::rw::math::vpu::detail::X1) ));
    VectorIntrinsic negateX = RWMATH_VPU_DETAIL_VecSld( signMask, zeroV, sizeof(float)*3);
    yA = RW_MATH_VPU_Xor_R( yA, negateX );
    VectorIntrinsic yRow;
    RW_MATH_VPU_MultAdd( yRow, yP0, yP1, yA );

    // Z
    // sj * cc + ss
    // sj * cs - sc
    // cj * ci + 0
    VectorIntrinsic zP0 = yP0;
    VectorIntrinsic zP1 = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cc_cs_sc_ss, cosZeroV, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::X1, ::rw::math::vpu::detail::W1) ));
    VectorIntrinsic zA = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(cc_cs_sc_ss, zeroV, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::W0, ::rw::math::vpu::detail::Z0, ::rw::math::vpu::detail::X1, ::rw::math::vpu::detail::X1) ));
    VectorIntrinsic negateYZW = RWMATH_VPU_DETAIL_VecSld( zeroV, signMask, sizeof(float)*3);
    zA = RW_MATH_VPU_Xor_R( zA, negateYZW );
    VectorIntrinsic zRow;
    RW_MATH_VPU_MultAdd( zRow, zP0, zP1, zA );

    VectorIntrinsic wRow = RW_MATH_VPU_GetL_R();

    return Matrix44(
        Vector4( xRow ),
        Vector4( yRow ),
        Vector4( zRow ),
        Vector4( wRow )
        );
}
#else
RW_MATH_FORCE_INLINE Matrix44 Matrix44FromEulerXYZ(Vector3::InParam euler) 
{
    VecFloat   ci, cj, ch, si, sj, sh;

    SinCos(euler.X(), si, ci);
    SinCos(euler.Y(), sj, cj);
    SinCos(euler.Z(), sh, ch);

    VecFloat cc = ci * ch;
    VecFloat cs = ci * sh;
    VecFloat sc = si * ch;
    VecFloat ss = si * sh;

    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix44(
        cj * ch     ,  cj * sh     , -sj      , zero,
        sj * sc - cs,  sj * ss + cc,  cj * si , zero,
        sj * cc + ss,  sj * cs - sc,  cj * ci , zero,
        zero, zero, zero, one
        );
}
#endif

RW_MATH_FORCE_INLINE Vector3 EulerXYZFromMatrix44(Matrix44::InParam m) 
{
    return EulerXYZFromMatrix33( Matrix33(m.xAxis.mV, m.yAxis.mV, m.zAxis.mV) );
}

// operating as Matrix33
RW_MATH_FORCE_INLINE VecFloat     Determinant3x3(Matrix44::InParam m) 
{
    return Determinant( Matrix33(m.xAxis.mV, m.yAxis.mV, m.zAxis.mV) );
}

RW_MATH_FORCE_INLINE Vector3 Mult3x3(Vector3::InParam v, Matrix44::InParam m) 
{
    return Mult(v, Matrix33(m.xAxis.mV, m.yAxis.mV, m.zAxis.mV));
}

RW_MATH_FORCE_INLINE Vector4 Mult3x3(Vector4::InParam v, Matrix44::InParam m) 
{
    Vector3 v3 = Mult3x3(Vector3(v.mV), m);
    return Vector4( v3, v.W() );
}

RW_MATH_FORCE_INLINE Matrix44 Inverse3x3(Matrix44::InParam m) 
{
    VecFloat determinant;
    return Inverse3x3(m, determinant);
}


RW_MATH_FORCE_INLINE Matrix44 Matrix44InverseRotationalComponent(Matrix44::InParam src) 
{
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    VectorIntrinsic xRow, yRow, zRow, wRow;
    VectorIntrinsic negatePos;
    RW_MATH_VPU_Negate( negatePos, src.wAxis.mV );

    // Transpose the upper 3x3 and fill the last column with zeros.
    RW_MATH_VPU_TransposeA4V3( xRow, yRow, zRow,
                               src.xAxis.mV, src.yAxis.mV, src.zAxis.mV, zero);
    RW_MATH_VPU_TransformVectorV3( wRow, negatePos, xRow, yRow, zRow );
    RW_MATH_VPU_SwizzleStore_I_I( wRow, wRow, VectorAxisW, one, VectorAxisW );

    return Matrix44( xRow, yRow, zRow, wRow );
}

RW_MATH_FORCE_INLINE Matrix44   Matrix44FromScale(Vector3::InParam scale) 
{
    //       X(),  zero, zero, zero,
    //       zero,  Y(), zero, zero,
    //       zero, zero,  Z(), zero,
    //       zero, zero, zero, one

    VectorIntrinsic row0 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row1 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row2 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row3 = RW_MATH_VPU_GetZero_R(); 

    row0 = _asmSwizzleStore<VectorAxisX, VectorAxisX>(row0, scale.mV);
    row1 = _asmSwizzleStore<VectorAxisY, VectorAxisY>(row1, scale.mV);
    row2 = _asmSwizzleStore<VectorAxisZ, VectorAxisZ>(row2, scale.mV);
    row3 = _asmSwizzleStore<VectorAxisW, VectorAxisW>(row3, RW_MATH_VPU_GetOne_R());

    return Matrix44(Vector4(row0), Vector4(row1), Vector4(row2), Vector4(row3));
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromTranslation(Vector3::InParam trans) 
{
    //  one,       zero,       zero,       zero,
    // zero,        one,       zero,       zero,
    // zero,       zero,        one,       zero,
    //  X(),        Y(),        Z(),        one
    VectorIntrinsic row0 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row1 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row2 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row3 = trans.mV; 

    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();

    row0 = _asmSwizzleStore<VectorAxisX, VectorAxisX>(row0, one);
    row1 = _asmSwizzleStore<VectorAxisY, VectorAxisY>(row1, one);
    row2 = _asmSwizzleStore<VectorAxisZ, VectorAxisZ>(row2, one);
    row3 = _asmSwizzleStore<VectorAxisW, VectorAxisW>(row3, one);

    return Matrix44(Vector4(row0), Vector4(row1), Vector4(row2), Vector4(row3));
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromScaleQuaternion(Vector3::InParam scale, Quaternion::InParam q) 
{
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic x;
    VectorIntrinsic y;
    VectorIntrinsic z;
    VectorIntrinsic w = zero;

    RW_MATH_VPU_MatrixFromQuaternion(x, y, z, q.mV);

    x = _asmSwizzleStore<VectorAxisW, VectorAxisX>(x, zero);
    y = _asmSwizzleStore<VectorAxisW, VectorAxisX>(y, zero);
    z = _asmSwizzleStore<VectorAxisW, VectorAxisX>(z, zero);
    w = _asmSwizzleStore<VectorAxisW, VectorAxisX>(w, RW_MATH_VPU_GetOne_R());

    return Matrix44(Vector4(x) * scale.X(), Vector4(y) * scale.Y(), Vector4(z) * scale.Z(), Vector4(w));
}

RW_MATH_FORCE_INLINE Matrix44 RotateAroundXAxis(Matrix44::InParam m, VecFloat::InParam angle) 
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labelled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //    [         e00                e01                e02                e03       ]
    //    [ e20 SIN + e10 COS  e21 SIN + e11 COS  e22 SIN + e12 COS  e23 SIN + e13 COS ]
    //    [ e20 COS - e10 SIN  e21 COS - e11 SIN  e22 COS - e12 SIN  e23 COS - e13 SIN ]
    //    [         e30                e31                e32                e33       ]
    VecFloat s,c;
    SinCos(angle, s, c);

    // rotate incrementally about x
    return Matrix44(
        m.xAxis,
        m.zAxis * s + m.yAxis * c,
        m.zAxis * c - m.yAxis * s,
        m.wAxis        
    );                     
}

RW_MATH_FORCE_INLINE Matrix44 RotateAroundYAxis(Matrix44::InParam m, VecFloat::InParam angle) 
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e00 COS - e20 SIN  e01 COS - e21 SIN  e02 COS - e22 SIN  e03 COS - e23 SIN ]
    //  [         e10                e11                e12                e13       ]
    //  [ e00 SIN + e20 COS  e01 SIN + e21 COS  e02 SIN + e22 COS  e03 SIN + e23 COS ]
    //  [         e30                e31                e32                e33       ]
    VecFloat s,c;

    SinCos(angle, s, c);

    // rotate incrementally about y
    return Matrix44(
        m.xAxis * c - m.zAxis * s,
        m.yAxis,           
        m.xAxis * s + m.zAxis * c,
        m.wAxis        
    );
}

RW_MATH_FORCE_INLINE Matrix44 RotateAroundZAxis(Matrix44::InParam m, VecFloat::InParam angle) 
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e10 SIN + e00 COS  e11 SIN + e01 COS  e12 SIN + e02 COS  e13 SIN + e03 COS ]
    //  [ e10 COS - e00 SIN  e11 COS - e01 SIN  e12 COS - e02 SIN  e13 COS - e03 SIN ]
    //  [         e20                e21                e22                e23       ]
    //  [         e30                e31                e32                e33       ]
    VecFloat s,c;

    SinCos(angle, s, c);

    // rotate incrementally about z
    return Matrix44(
        m.yAxis * s + m.xAxis * c,
        m.yAxis * c - m.xAxis * s,
        m.zAxis,
        m.wAxis
    );
}


RW_MATH_FORCE_INLINE Matrix44 ScaleRow(Matrix44::InParam m, VecFloat::InParam r0, VecFloat::InParam r1, VecFloat::InParam r2) 
{
    Vector4 row0Scale = Vector4( r0, r0, r0, GetVecFloat_One() );
    Vector4 row1Scale = Vector4( r1, r1, r1, GetVecFloat_One() );
    Vector4 row2Scale = Vector4( r2, r2, r2, GetVecFloat_One() );
    return Matrix44(
        Mult( row0Scale, m.xAxis ),
        Mult( row1Scale, m.yAxis ),
        Mult( row2Scale, m.zAxis ),
        m.wAxis             
        );
}

RW_MATH_FORCE_INLINE Matrix44 ScaleColumn(Matrix44::InParam m, VecFloat::InParam c0, VecFloat::InParam c1, VecFloat::InParam c2 ) 
{
    Vector4 rowMultiplier(c0, c1, c2, GetVecFloat_One());
    return Matrix44(
        Mult( m.xAxis, rowMultiplier ),
        Mult( m.yAxis, rowMultiplier ),
        Mult( m.zAxis, rowMultiplier ),
        m.wAxis  
        );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveLH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44(
        two*zn/w,      zero,          zero,     zero,
            zero,  two*zn/h,          zero,     zero,
            zero,      zero,    zf/(zf-zn),      one,
            zero,      zero, zn*zf/(zn-zf),     zero
          );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveRH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat negOne(GetVecFloat_NegativeOne());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44(
        two*zn/w,      zero,             zero,     zero,
            zero, two*zn/h,             zero,     zero, 
            zero,      zero,    zf/(zn-zf), negOne,  
            zero,      zero, zn*zf/(zn-zf),     zero );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspective(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    return Matrix44FromPerspectiveRH(w, h, zn, zf);
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoLH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44(
       two/w,   zero,          zero,    zero,
        zero,  two/h,          zero,    zero,
        zero,   zero,   one/(zf-zn),    zero,
        zero,   zero,   -zn/(zf-zn),     one
        );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoRH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44(
       two/w,   zero,          zero,    zero,
        zero,  two/h,          zero,    zero,
        zero,   zero,   one/(zn-zf),    zero,
        zero,   zero,    zn/(zn-zf),     one
        );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrtho(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf)
{
    return Matrix44FromOrthoRH(w, h, zn, zf);
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoOffCenterLH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44(
        two/(r-l),           zero,          zero,    zero,
             zero,      two/(t-b),          zero,    zero,
             zero,           zero,   one/(zf-zn),    zero,
      (l+r)/(l-r),    (t+b)/(b-t),    zn/(zn-zf),     one
      );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoOffCenterRH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44(
        two/(r-l),           zero,          zero,    zero,
             zero,      two/(t-b),          zero,    zero,
             zero,           zero,   one/(zn-zf),    zero,
      (l+r)/(l-r),    (t+b)/(b-t),    zn/(zn-zf),     one
      );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromOrthoOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    return Matrix44FromOrthoOffCenterRH(l, r, b, t, zn, zf);
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromQuaternionTranslation(Quaternion::InParam q, Vector3::InParam trans) 
{
    VectorIntrinsic xRow, yRow, zRow, wRow;
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    RW_MATH_VPU_MatrixFromQuaternion( xRow, yRow, zRow, q.mV );
    RW_MATH_VPU_SwizzleStore_I_I( wRow, trans.mV, VectorAxisW, one, VectorAxisW);
#if !defined( EA_PLATFORM_PS3_SPU )
    // The SPU automatically ensures the w component of the values is zero, so
    // there is no need to clear them here.
    RW_MATH_VPU_SetWToZero( xRow, xRow );
    RW_MATH_VPU_SetWToZero( yRow, yRow );
    RW_MATH_VPU_SetWToZero( zRow, zRow );
#endif
    return Matrix44( xRow, yRow, zRow, wRow );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromScaleQuaternionTranslation(Vector3::InParam scale, Quaternion::InParam q, Vector3::InParam trans) 
{
    VectorIntrinsic xRow, yRow, zRow, wRow;
    VectorIntrinsic xScale, yScale, zScale;
    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();
    RW_MATH_VPU_Splat_I( xScale, scale.mV, 0 );
    RW_MATH_VPU_Splat_I( yScale, scale.mV, 1 );
    RW_MATH_VPU_Splat_I( zScale, scale.mV, 2 );
    RW_MATH_VPU_MatrixFromQuaternion( xRow, yRow, zRow, q.mV );
    RW_MATH_VPU_Mult( xRow, xRow, xScale );
    RW_MATH_VPU_Mult( yRow, yRow, yScale );
    RW_MATH_VPU_Mult( zRow, zRow, zScale );
    RW_MATH_VPU_SwizzleStore_I_I( wRow, trans.mV, VectorAxisW, one, VectorAxisW);
#if !defined( EA_PLATFORM_PS3_SPU )
    // The SPU automatically ensures the w component of the values is zero, so
    // there is no need to clear them here.
    RW_MATH_VPU_SetWToZero( xRow, xRow );
    RW_MATH_VPU_SetWToZero( yRow, yRow );
    RW_MATH_VPU_SetWToZero( zRow, zRow );
#endif
    return Matrix44( xRow, yRow, zRow, wRow );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveFovLH(VecFloat::InParam fov, VecFloat::InParam aspect, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    // same as gluperspective except our fov is already in radians whereas opengl's parameter is in degrees
    VecFloat h = Tan(fov/VecFloat(GetVecFloat_Two()));
    VecFloat w = h * aspect;

    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix44(
         one/w,   zero,            zero,       zero,
          zero,  one/h,            zero,       zero,
          zero,   zero,      zf/(zf-zn),        one,
          zero,   zero,  -zn*zf/(zf-zn),       zero
          );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveFovRH(VecFloat::InParam fov, VecFloat::InParam aspect, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    // same as gluperspective except our fov is already in radians whereas opengl's parameter is in degrees
    VecFloat h = Tan(fov/VecFloat(GetVecFloat_Two()));
    VecFloat w = h * aspect;

    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat negOne(GetVecFloat_NegativeOne());
    return Matrix44(
          one/w,  zero,     zero,           zero,
          zero,   one/h,    zero,           zero, 
          zero,   zero,     zf/(zn-zf),     negOne,  
          zero,   zero,     zn*zf/(zn-zf),  zero
          );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveFov(VecFloat::InParam fov, VecFloat::InParam aspect, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    return Matrix44FromPerspectiveFovRH(fov, aspect, zn, zf);
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromLookAtLH(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition) 
{
    // calculate the 3 axes
    Vector3 zaxis = Normalize(Subtract(lookAt, eyePosition));
    Vector3 xaxis = Normalize(Cross(up, zaxis));
    Vector3 yaxis = Cross(zaxis, xaxis);

    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix44(
        xaxis.X(), yaxis.X(), zaxis.X(), zero,
        xaxis.Y(), yaxis.Y(), zaxis.Y(), zero,
        xaxis.Z(), yaxis.Z(), zaxis.Z(), zero,
        -Dot(xaxis, eyePosition), -Dot(yaxis, eyePosition), -Dot(zaxis, eyePosition), one
        );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromLookAtRH(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition) 
{
    // calculate the 3 axes
    Vector3 zaxis = Normalize(Subtract(eyePosition, lookAt));
    Vector3 xaxis = Normalize(Cross(up, zaxis));
    Vector3 yaxis = Cross(zaxis, xaxis);

    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix44(
        xaxis.X(), yaxis.X(), zaxis.X(), zero,
        xaxis.Y(), yaxis.Y(), zaxis.Y(), zero,
        xaxis.Z(), yaxis.Z(), zaxis.Z(), zero,
        -Dot(xaxis, eyePosition), -Dot(yaxis, eyePosition), -Dot(zaxis, eyePosition), one
        );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromLookAt(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition)
{
    return Matrix44FromLookAtRH(lookAt, up, eyePosition);
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveOffCenterLH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44(
        two*zn/(r-l),           zero,             zero,    zero,
                zero,   two*zn/(t-b),             zero,    zero,
        (l+r)/(l-r),     (t+b)/(b-t),       zf/(zf-zn),     one,
               zero,            zero,    zn*zf/(zn-zf),    zero
        );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveOffCenterRH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat negOne(GetVecFloat_NegativeOne());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44(
        two*zn/(r-l),           zero,            zero,    zero,
                zero,   two*zn/(t-b),            zero,    zero,
         (l+r)/(r-l),    (t+b)/(t-b),      zf/(zn-zf),  negOne,
                zero,           zero,   zn*zf/(zn-zf),    zero
        );
}

RW_MATH_FORCE_INLINE Matrix44 Matrix44FromPerspectiveOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    return Matrix44FromPerspectiveOffCenterRH(l, r, b, t, zn, zf);
}

RW_MATH_FORCE_INLINE VecFloat GetNormalError3x3(Matrix44::InParam matrix) 
{
    return GetNormalError( Matrix33(matrix.xAxis.mV, matrix.yAxis.mV, matrix.zAxis.mV) );
}

RW_MATH_FORCE_INLINE bool IsNormal3x3(Matrix44::InParam matrix,        VecFloat::InParam tolerance) 
{
    return IsZero( GetNormalError3x3(matrix), tolerance );
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError3x3(Matrix44::InParam matrix) 
{
    return GetOrthogonalError( Matrix33(matrix.xAxis.mV, matrix.yAxis.mV, matrix.zAxis.mV) );
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError3x3(Matrix44::InParam matrix) 
{
    return GetOrthogonalRowSetError( Matrix33(matrix.xAxis.mV, matrix.yAxis.mV, matrix.zAxis.mV) );
}


RW_MATH_FORCE_INLINE Vector3 QueryRotateDegenerateUnitAxis(Matrix44::InParam matrix) 
{
    return QueryRotateDegenerateUnitAxis( Matrix33(matrix.xAxis.mV, matrix.yAxis.mV, matrix.zAxis.mV) );
}


RW_MATH_FORCE_INLINE bool IsOrthogonal3x3(Matrix44::InParam matrix, VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalError3x3(matrix), tolerance );
}

RW_MATH_FORCE_INLINE bool IsRotation3x3(Matrix44::InParam a, VecFloat::InParam tolerance)
{
    return IsOrthogonal3x3(a, tolerance) && IsSimilar(Determinant3x3(a), GetVecFloat_One(), tolerance);
}

RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet3x3(Matrix44::InParam matrix, VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalRowSetError3x3(matrix), tolerance );
}

RW_MATH_INLINE Matrix33 Get3x3(Matrix44::InParam m)
{
    return Matrix33(GetXYZ(m.xAxis), GetXYZ(m.yAxis), GetXYZ(m.zAxis));
}

RW_MATH_INLINE Matrix44 Set3x3(Matrix44::InParam m, Matrix33::InParam m33)
{
    return Matrix44(Vector4(m33.xAxis.X(), m33.xAxis.Y(), m33.xAxis.Z(), m.xAxis.W()),
                    Vector4(m33.yAxis.X(), m33.yAxis.Y(), m33.yAxis.Z(), m.yAxis.W()),
                    Vector4(m33.zAxis.X(), m33.zAxis.Y(), m33.zAxis.Z(), m.zAxis.W()),
                    Vector4(m.wAxis.X(), m.wAxis.Y(), m.wAxis.Z(), m.wAxis.W()));
}

RW_MATH_FORCE_INLINE Matrix44 Select(MaskScalar::InParam mask, Matrix44::InParam trueValue, Matrix44::InParam falseValue )
{
    VectorIntrinsic xRow, yRow, zRow, wRow;
    RW_MATH_VPU_Select( xRow, falseValue.xAxis.mV, trueValue.xAxis.mV, mask.mV );
    RW_MATH_VPU_Select( yRow, falseValue.yAxis.mV, trueValue.yAxis.mV, mask.mV );
    RW_MATH_VPU_Select( zRow, falseValue.zAxis.mV, trueValue.zAxis.mV, mask.mV );
    RW_MATH_VPU_Select( wRow, falseValue.wAxis.mV, trueValue.wAxis.mV, mask.mV );
    return Matrix44(xRow, yRow, zRow, wRow);
}

// operations that return by reference
#if RW_MATH_ENABLE_DEPRECATED_API

RW_MATH_FORCE_INLINE void Negate(Matrix44::InParam m, Matrix44& result) 
{
    result = Negate(m);
}

RW_MATH_FORCE_INLINE void Mult(Matrix44::InParam m, VecFloat::InParam a, Matrix44& result) 
{
    result = Mult(m, a);
}

template<VectorAxis INDEX> void Mult(Matrix44::InParam m, VecFloatRef<INDEX> a, Matrix44& result)
{
    result = Mult(m, a);
}

RW_MATH_FORCE_INLINE void Mult(Matrix44::InParam m, VecFloatRefIndex::InParam a, Matrix44& result) 
{
    result = Mult(m, a);
}

RW_MATH_FORCE_INLINE void Divide(Matrix44::InParam m, VecFloat::InParam a, Matrix44& result) 
{
    result = Divide(m, a);
}

template<VectorAxis INDEX> void Divide(Matrix44::InParam m, VecFloatRef<INDEX> a, Matrix44& result)
{
    result = Divide(m, a);
}

RW_MATH_FORCE_INLINE void Divide(Matrix44::InParam m, VecFloatRefIndex::InParam a, Matrix44& result) 
{
    result = Divide(m, a);
}


// vector
RW_MATH_FORCE_INLINE void Mult(Vector4::InParam v, Matrix44::InParam m, Vector4& result) 
{
    result = Mult(v, m);
}

// matrix
RW_MATH_FORCE_INLINE void Mult(Matrix44::InParam m, Matrix44::InParam b, Matrix44& result) 
{
    result = Mult(m, b);
}

RW_MATH_FORCE_INLINE void Add(Matrix44::InParam a, Matrix44::InParam b, Matrix44& result) 
{
    result = Add(a, b);
}

RW_MATH_FORCE_INLINE void Subtract(Matrix44::InParam a, Matrix44::InParam b, Matrix44& result) 
{
    result = Subtract(a, b);
}


RW_MATH_FORCE_INLINE void Transpose(Matrix44::InParam m, Matrix44& result) 
{
    result = Transpose(m);
}

RW_MATH_FORCE_INLINE VecFloat Inverse(Matrix44::InParam m, Matrix44& result) 
{
    VecFloat determinant;
    result = Inverse(m, determinant);
    return determinant;
}


RW_MATH_FORCE_INLINE void Matrix44FromQuaternion(Quaternion::InParam q, Matrix44& result) 
{
    result = Matrix44FromQuaternion(q);
}

RW_MATH_FORCE_INLINE void Matrix44FromAxisAngle(Vector3::InParam axis, VecFloat::InParam angle, Matrix44& result) 
{
    result = Matrix44FromAxisRotationAngle(axis, angle);
}

RW_MATH_FORCE_INLINE void Matrix44FromEulerXYZ(Vector3::InParam euler, Matrix44& result) 
{
    result = Matrix44FromEulerXYZ(euler);
}

RW_MATH_FORCE_INLINE void EulerXYZFromMatrix44(Matrix44::InParam m, Vector3& result) 
{
    result = EulerXYZFromMatrix44(m);
}

// use as Matrix33
RW_MATH_FORCE_INLINE void Mult3x3(Vector4::InParam v, Matrix44::InParam m, Vector4& result) 
{
    result = Mult3x3(v, m);
}

RW_MATH_FORCE_INLINE void Mult3x3(Vector3::InParam v, Matrix44::InParam m, Vector3& result) 
{
    result = Mult3x3(v, m);
}

RW_MATH_FORCE_INLINE VecFloat Matrix44Inverse3x3(Matrix44::InParam src, Matrix44& result) 
{
    VecFloat determinant;
    result = Inverse3x3(src, determinant);
    return determinant;
}

// matrix44 specific function
RW_MATH_FORCE_INLINE void Matrix44InverseRotTrans(Matrix44::InParam src, Matrix44& result) 
{
    result = Matrix44InverseRotationalComponent(src);
}

RW_MATH_FORCE_INLINE void Matrix44FromScale(Vector3::InParam scale, Matrix44& result) 
{
    result = Matrix44FromScale(scale);
}

RW_MATH_FORCE_INLINE void Matrix44FromTranslation(Vector3::InParam trans, Matrix44& result) 
{
    result = Matrix44FromTranslation(trans);
}

RW_MATH_FORCE_INLINE void Matrix44FromScaleQuaternion(Vector3::InParam scale, Quaternion::InParam q, Matrix44& result) 
{
    result = Matrix44FromScaleQuaternion(scale, q);
}

RW_MATH_FORCE_INLINE void Matrix44FromScaleTranslation(Vector3::InParam scale, Vector3::InParam trans, Matrix44& result) 
{
    result = Matrix44FromScaleTranslation(scale, trans);
}

RW_MATH_FORCE_INLINE void Matrix44Diagonal(VecFloat::InParam a, Matrix44& result) 
{
    result = Matrix44FromDiagonal(a);
}

RW_MATH_FORCE_INLINE void Matrix44RotationX(VecFloat::InParam angle, Matrix44& result) 
{
    result = Matrix44FromXRotationAngle(angle);
}

RW_MATH_FORCE_INLINE void Matrix44RotationY(VecFloat::InParam angle, Matrix44& result) 
{
    result = Matrix44FromYRotationAngle(angle);
}

RW_MATH_FORCE_INLINE void Matrix44RotationZ(VecFloat::InParam angle, Matrix44& result) 
{
    result = Matrix44FromZRotationAngle(angle);
}

RW_MATH_FORCE_INLINE void Matrix44RotateX(VecFloat::InParam angle, Matrix44& m) 
{
    m = RotateAroundXAxis(m, angle);
}

RW_MATH_FORCE_INLINE void Matrix44RotateY(VecFloat::InParam angle, Matrix44& m) 
{
    m = RotateAroundYAxis(m, angle);
}

RW_MATH_FORCE_INLINE void Matrix44RotateZ(VecFloat::InParam angle, Matrix44& m) 
{
    m = RotateAroundZAxis(m, angle);
}

RW_MATH_FORCE_INLINE void Matrix44TransposeRotation(Matrix44& m) 
{
    m = Matrix44TransposeRotationalComponent(m);
}

RW_MATH_FORCE_INLINE void Matrix44Translate(Vector3::InParam trans,          Matrix44& m) 
{
    m = Matrix44Translate(m, trans);
}

RW_MATH_FORCE_INLINE void Matrix44ScaleRow(VecFloat::InParam r0,            VecFloat::InParam r1,           VecFloat::InParam r2,           Matrix44& m) 
{
    m = ScaleRow(m, r0, r1, r2);
}

RW_MATH_FORCE_INLINE void Matrix44ScaleColumn(VecFloat::InParam c0,            VecFloat::InParam c1,           VecFloat::InParam c2,           Matrix44& m) 
{
    m = ScaleColumn(m, c0, c1, c2);
}

RW_MATH_FORCE_INLINE void Matrix44Perspective(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result) 
{
    result = Matrix44FromPerspective( w, h, zn, zf );
}

RW_MATH_FORCE_INLINE void Matrix44Ortho(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result) 
{
    result = Matrix44FromOrtho(w, h, zn, zf);
}

RW_MATH_FORCE_INLINE void Matrix44OrthoOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result) 
{
    result = Matrix44FromOrthoOffCenter(l, r, b, t, zn, zf);
}

RW_MATH_FORCE_INLINE void Matrix44FromQuaternionTranslation(Quaternion::InParam q, Vector3::InParam trans, Matrix44& result) 
{
    result = Matrix44FromQuaternionTranslation(q, trans);
}

RW_MATH_FORCE_INLINE void Matrix44FromScaleQuaternionTranslation(Vector3::InParam scale, Quaternion::InParam q, Vector3::InParam trans, Matrix44& result) 
{
    result = Matrix44FromScaleQuaternionTranslation(scale, q, trans);
}

RW_MATH_FORCE_INLINE void Matrix44PerspectiveFov(VecFloat::InParam fov, VecFloat::InParam aspect, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result) 
{
    result = Matrix44FromPerspectiveFov(fov, aspect, zn, zf);
}

RW_MATH_FORCE_INLINE void Matrix44LookAt(Vector3::InParam eye, Vector3::InParam focal, Vector3::InParam up, Matrix44& result) 
{
    result = Matrix44FromLookAt(focal, up, eye);
}

RW_MATH_FORCE_INLINE void Matrix44PerspectiveOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf, Matrix44& result) 
{
    result = Matrix44FromPerspectiveOffCenter(l, r, b, t, zn, zf);
}


#endif

RW_MATH_FORCE_INLINE Matrix44& operator *=(Matrix44& m, VecFloat::InParam a) 
{
    m = Mult(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44& operator /=(Matrix44& m, VecFloat::InParam a) 
{
    m = Divide(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44 operator * (Matrix44::InParam m, VecFloat::InParam a) 
{
    return Mult(m, a);
}

RW_MATH_FORCE_INLINE Matrix44 operator * (VecFloat::InParam a, Matrix44::InParam b) 
{
    return Mult(b, a);
}

RW_MATH_FORCE_INLINE Matrix44 operator / (Matrix44::InParam m, VecFloat::InParam a) 
{
    return Divide(m, a);
}

// float
Matrix44&   operator *=(Matrix44& m,          float a)
{
    m = Mult(m, VecFloat(a));
    return m;
}

Matrix44&   operator /=(Matrix44& m,          float a)
{
    m = Divide(m, VecFloat(a));
    return m;
}

Matrix44    operator * (Matrix44::InParam m,             float a)
{
    return Mult(m, VecFloat(a));
}

Matrix44    operator * (float a,            Matrix44::InParam m)
{
    return Mult(m, VecFloat(a));
}

Matrix44    operator / (Matrix44::InParam m,             float a)
{
    return Divide(m, VecFloat(a));
}



// VecFloatRef
template<VectorAxis INDEX> Matrix44&   operator *=(Matrix44& m,          VecFloatRef<INDEX> a)
{
    m = Mult(m, a);
    return m;
}

template<VectorAxis INDEX> Matrix44&   operator /=(Matrix44& m,          VecFloatRef<INDEX> a)
{
    m = Divide(m, a);
    return m;
}

template<VectorAxis INDEX> Matrix44    operator * (Matrix44::InParam m,             VecFloatRef<INDEX> a)
{
    return Mult(m, a);
}

template<VectorAxis INDEX> Matrix44    operator * (VecFloatRef<INDEX> a,            Matrix44::InParam m)
{
    return Mult(m, a);
}

template<VectorAxis INDEX> Matrix44    operator / (Matrix44::InParam m,             VecFloatRef<INDEX> a)
{
    return Divide(m, a);
}


// VecFloatRefIndex
RW_MATH_FORCE_INLINE Matrix44&   operator *=(Matrix44& m,          VecFloatRefIndex::InParam a) 
{
    m = Mult(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44&   operator /=(Matrix44& m,          VecFloatRefIndex::InParam a) 
{
    m = Divide(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44    operator * (Matrix44::InParam m,             VecFloatRefIndex::InParam a) 
{
    return Mult(m, a);
}

RW_MATH_FORCE_INLINE Matrix44    operator * (VecFloatRefIndex::InParam a,     Matrix44::InParam b) 
{
    return Mult(b, a);
}

RW_MATH_FORCE_INLINE Matrix44    operator / (Matrix44::InParam m,             VecFloatRefIndex::InParam a) 
{
    return Divide(m, a);
}


//
// Vector4 operations
//
RW_MATH_FORCE_INLINE Vector4 operator * (Vector4::InParam v, Matrix44::InParam m) 
{
    return Mult(v, m);
}

//
// Matrix44 operations
//
RW_MATH_FORCE_INLINE Matrix44& operator *=(Matrix44& m, Matrix44::InParam b) 
{
    m = Mult(m, b);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44& operator +=(Matrix44& m, Matrix44::InParam b) 
{
    m = Add(m, b);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44& operator -=(Matrix44& m, Matrix44::InParam b) 
{
    m = Subtract(m, b);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44 operator - (Matrix44::InParam m)  
{
    return Negate(m);
}


RW_MATH_FORCE_INLINE Matrix44 operator + (Matrix44::InParam a, Matrix44::InParam b) 
{
    return Add(a, b);
}

RW_MATH_FORCE_INLINE Matrix44 operator - (Matrix44::InParam a, Matrix44::InParam b) 
{
    return Subtract(a, b);
}

RW_MATH_FORCE_INLINE Matrix44 operator * (Matrix44::InParam a, Matrix44::InParam b) 
{
    return Mult(a, b);
}

}

}

}

#endif
