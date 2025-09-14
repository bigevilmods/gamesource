// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_Matrix44Affine_OPERATION_INLINE_H
#define RW_MATH_VPU_DETAIL_Matrix44Affine_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#include "rw/math/vpu/constants_operation.h"
#include "rw/math/vpu/trig.h"
#include "rw/math/vpu/scalar.h"
#include "rw/math/vpu/vec_float_operation.h"
#include "rw/math/vpu/matrix33_operation.h"
#include "rw/math/vpu/vector3_operation.h"
#include "rw/math/vpu/vector4_operation.h"
#include "rw/math/vpu/matrix44affine.h"
#include "rw/math/vpu/detail/matrix44affine_operation_platform_inline.h"

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Matrix44Affine& operator *=(Matrix44Affine& m, VecFloat::InParam a) 
{
    m = Mult(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44Affine& operator /=(Matrix44Affine& m, VecFloat::InParam a) 
{
    m = Divide(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44Affine operator * (Matrix44Affine::InParam m, VecFloat::InParam a) 
{
    return Mult(m, a);
}

RW_MATH_FORCE_INLINE Matrix44Affine operator * (VecFloat::InParam a, Matrix44Affine::InParam b) 
{
    return Mult(b, a);
}

RW_MATH_FORCE_INLINE Matrix44Affine operator / (Matrix44Affine::InParam m, VecFloat::InParam a) 
{
    return Divide(m, a);
}


// float
Matrix44Affine& operator *=(Matrix44Affine& m, float a)
{
    m = Mult(m, VecFloat(a));
    return m;
}

Matrix44Affine& operator /=(Matrix44Affine& m, float a)
{
    m = Divide(m, VecFloat(a));
    return m;
}

Matrix44Affine operator * (Matrix44Affine::InParam m, float a)
{
    return Mult(m, VecFloat(a));
}

Matrix44Affine operator * (float a, Matrix44Affine::InParam b)
{
    return Mult(b, VecFloat(a));
}

Matrix44Affine operator / (Matrix44Affine::InParam m, float a)
{
    return Divide(m, VecFloat(a));
}


// VecFloatRef
template<VectorAxis INDEX> Matrix44Affine&   operator *=(Matrix44Affine& m,          VecFloatRef<INDEX> a)
{
    m = Mult(m, a);
    return m;
}

template<VectorAxis INDEX> Matrix44Affine&   operator /=(Matrix44Affine& m,          VecFloatRef<INDEX> a)
{
    m = Divide(m, a);
    return m;
}

template<VectorAxis INDEX> Matrix44Affine    operator * (Matrix44Affine::InParam m,             VecFloatRef<INDEX> a)
{
    return Mult(m, a);
}

template<VectorAxis INDEX> Matrix44Affine    operator * (VecFloatRef<INDEX> a,            Matrix44Affine::InParam b)
{
    return Mult(b, a);
}

template<VectorAxis INDEX> Matrix44Affine    operator / (Matrix44Affine::InParam m,             VecFloatRef<INDEX> a)
{
    return Divide(m, a);
}


// VecFloatRefIndex
RW_MATH_FORCE_INLINE Matrix44Affine&   operator *=(Matrix44Affine& m,          VecFloatRefIndex::InParam a) 
{
    m = Mult(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44Affine&   operator /=(Matrix44Affine& m,          VecFloatRefIndex::InParam a) 
{
    m = Divide(m, a);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44Affine    operator * (Matrix44Affine::InParam m,             VecFloatRefIndex::InParam a) 
{
    return Mult(m, a);
}

RW_MATH_FORCE_INLINE Matrix44Affine    operator * (VecFloatRefIndex::InParam a,     Matrix44Affine::InParam b) 
{
    return Mult(b, a);
}

RW_MATH_FORCE_INLINE Matrix44Affine    operator / (Matrix44Affine::InParam m,             VecFloatRefIndex::InParam a) 
{
    return Divide(m, a);
}


//
// Vector4 operations
//
RW_MATH_FORCE_INLINE Vector4 operator * (Vector4::InParam v, Matrix44Affine::InParam m) 
{
    return Mult(v, m);
}

//
// Matrix44Affine operations
//
RW_MATH_FORCE_INLINE Matrix44Affine& operator *=(Matrix44Affine& m, Matrix44Affine::InParam b) 
{
    m = Mult(m, b);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44Affine& operator +=(Matrix44Affine& m, Matrix44Affine::InParam b) 
{
    m = Add(m, b);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44Affine& operator -=(Matrix44Affine& m, Matrix44Affine::InParam b) 
{
    m = Subtract(m, b);
    return m;
}

RW_MATH_FORCE_INLINE Matrix44Affine operator - (Matrix44Affine::InParam m)  
{
    return Negate(m);
}


RW_MATH_FORCE_INLINE Matrix44Affine operator + (Matrix44Affine::InParam a, Matrix44Affine::InParam b) 
{
    return Add(a, b);
}

RW_MATH_FORCE_INLINE Matrix44Affine operator - (Matrix44Affine::InParam a, Matrix44Affine::InParam b) 
{
    return Subtract(a, b);
}

RW_MATH_FORCE_INLINE Matrix44Affine operator * (Matrix44Affine::InParam a, Matrix44Affine::InParam b) 
{
    return Mult(a, b);
}

RW_MATH_FORCE_INLINE bool operator ==(Matrix44Affine::InParam a, Matrix44Affine::InParam b) 
{
    return (bool)( (a.xAxis == b.xAxis) &
                   (a.yAxis == b.yAxis) &
                   (a.zAxis == b.zAxis) &
                   (a.wAxis == b.wAxis) );
}

RW_MATH_FORCE_INLINE bool operator !=(Matrix44Affine::InParam a, Matrix44Affine::InParam b) 
{
    return !(a==b);
}

RW_MATH_FORCE_INLINE bool IsSimilar(Matrix44Affine::InParam a, Matrix44Affine::InParam b, VecFloat::InParam epsilon ) 
{
    return _asmIsSimilarV3( a.xAxis.mV, a.yAxis.mV, a.zAxis.mV, a.wAxis.mV,
                            b.xAxis.mV, b.yAxis.mV, b.zAxis.mV, b.wAxis.mV,
                            epsilon.mV );
}

RW_MATH_FORCE_INLINE bool IsAnyNaN(Matrix44Affine::InParam m)
{
    return (bool)(IsAnyNaN(m.xAxis) | IsAnyNaN(m.yAxis) | IsAnyNaN(m.zAxis) | IsAnyNaN(m.wAxis));
}

RW_MATH_FORCE_INLINE bool IsValid(Matrix44Affine::InParam a) 
{
    return (bool)( IsValid(a.xAxis) &
                   IsValid(a.yAxis) &
                   IsValid(a.zAxis) &
                   IsValid(a.wAxis) );
}

RW_MATH_FORCE_INLINE VecFloat GetNormalError(Matrix44Affine::InParam matrix) 
{
    VecFloat one(RW_MATH_VPU_GetOne_R());
    Vector4 errorVec(   MagnitudeSquared( matrix.xAxis ) - one,
                        MagnitudeSquared( matrix.yAxis ) - one,
                        MagnitudeSquared( matrix.zAxis ) - one,
                        MagnitudeSquared( matrix.wAxis ) );
    VecFloat error = MagnitudeSquared(errorVec);
    return error;
}

RW_MATH_FORCE_INLINE bool IsNormal(Matrix44Affine::InParam a, VecFloat::InParam tolerance) 
{
    return IsZero( GetNormalError(a), tolerance );
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError(Matrix44Affine::InParam matrix) 
{
    Matrix44Affine identity;
    identity.SetIdentity();

    Matrix44Affine result = Mult(matrix, Transpose(matrix));
    SelfSubtract(result, identity);

    Vector4 errorVec( MagnitudeSquared( result.XAxis() ),
                      MagnitudeSquared( result.YAxis() ),
                      MagnitudeSquared( result.ZAxis() ),
                      MagnitudeSquared( result.WAxis() ) );

    VecFloat error = MagnitudeSquared(errorVec);
    return error;
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError(Matrix44Affine::InParam matrix) 
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

RW_MATH_FORCE_INLINE bool IsOrthogonal(Matrix44Affine::InParam a, VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalError(a), tolerance );
}

RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet(Matrix44Affine::InParam a, VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalRowSetError(a), tolerance );
}

RW_MATH_FORCE_INLINE bool IsValid3x3(Matrix44Affine::InParam a) 
{
    return IsValid(a.xAxis) &&
           IsValid(a.yAxis) &&
           IsValid(a.zAxis);
}

RW_MATH_FORCE_INLINE void SelfAdd(Matrix44Affine& a, Matrix44Affine::InParam b) 
{
    a = Add(a, b);
}

RW_MATH_FORCE_INLINE void SelfSubtract(Matrix44Affine& a, Matrix44Affine::InParam b) 
{
    a = Subtract(a, b);
}


RW_MATH_FORCE_INLINE void SelfMult(Matrix44Affine& m, VecFloat::InParam a) 
{
    m = Mult(m, a);
}

template<VectorAxis INDEX> void SelfMult(Matrix44Affine& m, VecFloatRef<INDEX> a)
{
    m = Mult(m, a);
}

RW_MATH_FORCE_INLINE void SelfMult(Matrix44Affine& m, VecFloatRefIndex::InParam a) 
{
    m = Mult(m, a);
}

RW_MATH_FORCE_INLINE void SelfDivide(Matrix44Affine& m, VecFloat::InParam a) 
{
    m = Divide(m, a);
}

template<VectorAxis INDEX> void SelfDivide(Matrix44Affine& m, VecFloatRef<INDEX> a)
{
    m = Divide(m, a);
}

RW_MATH_FORCE_INLINE void SelfDivide(Matrix44Affine& m, VecFloatRefIndex::InParam a) 
{
    m = Divide(m, a);
}


// operations that return by value

// scalar 
RW_MATH_FORCE_INLINE Matrix44Affine Negate(Matrix44Affine::InParam m) 
{
    return Matrix44Affine( Negate(m.xAxis), Negate(m.yAxis), Negate(m.zAxis), Negate(m.wAxis) );
}

RW_MATH_FORCE_INLINE Matrix44Affine Mult(Matrix44Affine::InParam m, VecFloat::InParam a) 
{
    return Matrix44Affine(
            a * m.xAxis,
            a * m.yAxis,
            a * m.zAxis,
            a * m.wAxis
            );
}

template<VectorAxis INDEX> Matrix44Affine Mult(Matrix44Affine::InParam m, VecFloatRef<INDEX> a)
{
    return Mult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix44Affine Mult(Matrix44Affine::InParam m, VecFloatRefIndex::InParam a) 
{
    return Mult(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix44Affine Divide(Matrix44Affine::InParam m, VecFloat::InParam a) 
{
    VecFloat recipA = Reciprocal(a);
    return Mult(m, recipA);
}

template<VectorAxis INDEX> Matrix44Affine Divide(Matrix44Affine::InParam m, VecFloatRef<INDEX> a)
{
    return Divide(m, VecFloat(a));
}

RW_MATH_FORCE_INLINE Matrix44Affine Divide(Matrix44Affine::InParam m, VecFloatRefIndex::InParam a) 
{
    return Divide(m, VecFloat(a));
}

// matrix
RW_MATH_FORCE_INLINE Matrix44Affine Add(Matrix44Affine::InParam a, Matrix44Affine::InParam b) 
{
    return Matrix44Affine(
                Add(a.xAxis, b.xAxis),
                Add(a.yAxis, b.yAxis),
                Add(a.zAxis, b.zAxis),
                Add(a.wAxis, b.wAxis)
            );
}

RW_MATH_FORCE_INLINE Matrix44Affine Subtract(Matrix44Affine::InParam a, Matrix44Affine::InParam b) 
{
    return Matrix44Affine(
                Subtract(a.xAxis, b.xAxis),
                Subtract(a.yAxis, b.yAxis),
                Subtract(a.zAxis, b.zAxis),
                Subtract(a.wAxis, b.wAxis)
            );
}

/*
    \summary This function computes an orthonormal matrix given an input matrix.

    The function accepts a matrix as input and returns an orthonormalized
    matrix that is "similar".  How similar the resulting matrix is to the input
    matrix depends heavily on how close the input matrix is to being
    orthonormal.

    \par Preconditions
    The input matrix must consist of four row vectors that are linearly
    independent.  (Note: Each row vector consists of three user modifiable
    elements, and one assumed element.)  Put another way, the input matrix must
    have a rank of four.
    See: mathworld.wolfram.com/LinearlyIndependent.html

    \par Notes regarding the result
    In order to construct an orthonormal affine matrix, the last row of the
    result \b must be (0, 0, 0, 1).  That is the only vector that has unit
    length and preserves the assumptions for an affine matrix.  For many 
    applications it is more useful to orthonormalize the upper-left 3x3
    sub-matrix instead of orthonormalizing the entire matrix.


    \param m The matrix that should be orthonormalized.
    \result The orthonormalized result.
*/
RW_MATH_INLINE Matrix44Affine OrthoNormalize(Matrix44Affine::InParam m)
{
    // Note: This implementation is using the Gram-Schmidt orthonormalization
    // algorithm.  The current implementation is numerically unstable. Also,
    // this algorithm differs from the algorithm implemented for matrix 3x3
    // orthonormalization.
    Matrix44Affine result;

    // First, set the last row to (0,0,0,1).  This is the only value for the last row that will
    // allow us to create an ortho-normal matrix that is also Affine.
    result.wAxis = GetVector3_Zero();

    // Note: We could call OrthoNormalize3x3 on the sub-matrix and get an
    // orthonormal affine matrix.

    // Loop over rows 2, 1, and 0
    for( int row = 2; row >= 0; --row )
    {
        // Initialize the row to the same row from the input matrix.
        const Vector3 &inputRow = m.GetRow(row);
        Vector3 tempResult = inputRow;
        // Subtract the components of the vector that are not orthogonal to the basis created thus far.
        // Note: By only operating on Vector3's, we eliminate the need to remove the contribution
        // of the last row in the matrix.
        //
        // "Handwaving Proof"
        //   by Dave Cope
        // let a = (x y z w)
        // Now consider what happens when we subtract the component of 'a' that is not orthogonal to
        // the last row (0 0 0 1).
        //
        // result = a - (0 0 0 1) * Dot(a, (0 0 0 1))
        //        = (x y z w) - (0 0 0 1) * Dot((x y z w), (0 0 0 1))
        //        = (x y z w) - (0 0 0 1) * w
        //        = (x y z w) - (0 0 0 w)
        //        = (x y z 0)
        // Since all other row vectors have their w components fixed at 0, and normalizing a vector4
        // with w=0 is equivalent to normalizing a vector3 and appending a zero, we can operate on Vector3's
        // instead of Vector4's.
        for( int i = 2; i > row; --i )
        {
            tempResult -= result.GetRow(i) * Dot(result.GetRow(i), inputRow );
        }
        result.SetRow( row, Normalize(tempResult) );
    }
    return result;
}

RW_MATH_FORCE_INLINE Matrix44Affine InverseOrthonormal(Matrix44Affine::InParam m) 
{
    RW_MATH_ASSERT_MSG(IsOrthogonal(m), ("Input matrix is non orthonormal."));
    return Transpose(m);
}

#ifdef EA_PLATFORM_REVOLUTION
namespace detail
{
    Matrix44Affine Inverse(Matrix44Affine::InParam m, VecFloat& determinant);
}
#endif

RW_MATH_FORCE_INLINE Matrix44Affine Inverse(Matrix44Affine::InParam m, VecFloat& determinant) 
{
#ifdef EA_PLATFORM_REVOLUTION
    return detail::Inverse(m, determinant);
#else
    RW_MATH_ASSERT_MSG((Determinant(m)!= GetVecFloat_Zero()), ("Matrix is singular."));
    VectorIntrinsic det, xAxisResult, yAxisResult, zAxisResult, wAxisResult;
    RW_MATH_VPU_InverseA4V3( det, xAxisResult, yAxisResult, zAxisResult, wAxisResult, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, m.wAxis.mV );

    determinant.mV = det;
    return Matrix44Affine(xAxisResult, yAxisResult, zAxisResult, wAxisResult);
#endif
}

RW_MATH_FORCE_INLINE Matrix44Affine Inverse(Matrix44Affine::InParam m) 
{
    VecFloat determinant;
    return Inverse(m, determinant);
}

RW_MATH_FORCE_INLINE void TransformPoints(const Vector3* const pts, int numPoints, Matrix44Affine::InParam matrix, Vector3* transformedPts) 
{
    for( int point = 0; point < numPoints; point++ )
    {
        transformedPts[point] = TransformPoint(pts[point], matrix);
    }
}

RW_MATH_FORCE_INLINE void TransformVectors(const Vector3* const vecs, int numVectors, Matrix44Affine::InParam matrix, Vector3 * transformedVecs) 
{
    for( int vecIndex = 0; vecIndex < numVectors; vecIndex++ )
    {
        transformedVecs[vecIndex] = TransformVector(vecs[vecIndex], matrix);
    }
}

RW_MATH_FORCE_INLINE Vector3 Transform(Vector4::InParam vec, Matrix44Affine::InParam matrix) 
{
    Vector3 transformedVector = matrix.xAxis * vec.X();
    transformedVector += matrix.yAxis * vec.Y();
    transformedVector += matrix.zAxis * vec.Z();
    transformedVector += matrix.wAxis * vec.W();;
    return transformedVector;
}

RW_MATH_FORCE_INLINE void Transform(const Vector4* const vecs, int numVectors, Matrix44Affine::InParam matrix, Vector3 * transformedVecs) 
{
    for( int vecIndex = 0; vecIndex < numVectors; vecIndex++ )
    {
        transformedVecs[vecIndex] = Transform(vecs[vecIndex], matrix);
    }
}


RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromQuaternion(Quaternion::InParam q) 
{
    VectorIntrinsic x;
    VectorIntrinsic y;
    VectorIntrinsic z;

    RW_MATH_VPU_MatrixFromQuaternion(x, y, z, q.mV);

    return Matrix44Affine(x, y, z, RW_MATH_VPU_GetZero_R());
}


RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromEulerXYZ(Vector3::InParam euler) 
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
    return Matrix44Affine(
        cj * ch     ,  cj * sh     , -sj      ,
        sj * sc - cs,  sj * ss + cc,  cj * si ,
        sj * cc + ss,  sj * cs - sc,  cj * ci ,
        zero, zero, zero
        );
}

RW_MATH_FORCE_INLINE Vector3 EulerXYZFromMatrix44Affine(Matrix44Affine::InParam m) 
{
    return EulerXYZFromMatrix33( Matrix33(m.xAxis, m.yAxis, m.zAxis) );
}

// operating as Matrix33
RW_MATH_FORCE_INLINE VecFloat     Determinant3x3(Matrix44Affine::InParam m) 
{
    VectorIntrinsic cross;
    RW_MATH_VPU_CrossV3(cross, m.xAxis.mV, m.yAxis.mV)
    VectorIntrinsic dot;    
    RW_MATH_VPU_DotV3(dot, cross, m.zAxis.mV);
    return VecFloat(dot);
}

RW_MATH_FORCE_INLINE Vector4 Mult3x3(Vector4::InParam v, Matrix44Affine::InParam m) 
{
    Vector3 v3 = Mult3x3(Vector3(v.mV), m);
    return Vector4( v3, v.W() );
}

RW_MATH_FORCE_INLINE Vector3 Mult3x3(Vector3::InParam v, Matrix44Affine::InParam m) 
{
    return Mult(v,Matrix33(m.xAxis, m.yAxis, m.zAxis));
}

RW_MATH_FORCE_INLINE Matrix44Affine   Matrix44AffineFromScale(Vector3::InParam scale) 
{
    //       X(),  zero, zero
    //       zero,  Y(), zero
    //       zero, zero,  Z()
    //       zero, zero, zero

    VectorIntrinsic row0 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row1 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row2 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row3 = RW_MATH_VPU_GetZero_R(); 

    row0 = _asmSwizzleStore<VectorAxisX, VectorAxisX>(row0, scale.mV);
    row1 = _asmSwizzleStore<VectorAxisY, VectorAxisY>(row1, scale.mV);
    row2 = _asmSwizzleStore<VectorAxisZ, VectorAxisZ>(row2, scale.mV);

    return Matrix44Affine(Vector3(row0), Vector3(row1), Vector3(row2), Vector3(row3));
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromTranslation(Vector3::InParam trans) 
{
    //  one,       zero,       zero,
    // zero,        one,       zero,
    // zero,       zero,        one,
    //  X(),        Y(),        Z()
    VectorIntrinsic row0 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row1 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row2 = RW_MATH_VPU_GetZero_R(); 
    VectorIntrinsic row3 = trans.mV; 

    VectorIntrinsic one = RW_MATH_VPU_GetOne_R();

    row0 = _asmSwizzleStore<VectorAxisX, VectorAxisX>(row0, one);
    row1 = _asmSwizzleStore<VectorAxisY, VectorAxisY>(row1, one);
    row2 = _asmSwizzleStore<VectorAxisZ, VectorAxisZ>(row2, one);
    return Matrix44Affine(Vector3(row0), Vector3(row1), Vector3(row2), Vector3(row3));
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromScaleQuaternion(Vector3::InParam scale, Quaternion::InParam q) 
{
    Vector3 zero(RW_MATH_VPU_GetZero_R());
    VectorIntrinsic x;
    VectorIntrinsic y;
    VectorIntrinsic z;

    RW_MATH_VPU_MatrixFromQuaternion(x, y, z, q.mV);

    return Matrix44Affine(Vector3(x) * scale.X(), Vector3(y) * scale.Y(), Vector3(z) * scale.Z(), zero);
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromScaleTranslation(Vector3::InParam scale, Vector3::InParam trans) 
{
    // scale.X(), zero, zero,
    // zero, scale.Y(), zero,
    // zero, zero, scale.Z(),
    // trans.X(), trans.Y(), trans.Z()
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic row0 = zero; 
    VectorIntrinsic row1 = zero; 
    VectorIntrinsic row2 = zero; 
    VectorIntrinsic row3 = trans.mV; 

    row0 = _asmSwizzleStore<VectorAxisX, VectorAxisX>(row0, scale.mV);
    row1 = _asmSwizzleStore<VectorAxisY, VectorAxisY>(row1, scale.mV);
    row2 = _asmSwizzleStore<VectorAxisZ, VectorAxisZ>(row2, scale.mV);

    return Matrix44Affine(Vector3(row0), Vector3(row1), Vector3(row2), Vector3(row3));
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromDiagonal(VecFloat::InParam a) 
{
    // a,0,0,
    // 0,a,0,
    // 0,0,a,
    // 0,0,0
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic row0 = zero; 
    VectorIntrinsic row1 = zero; 
    VectorIntrinsic row2 = zero; 

    row0 = _asmSwizzleStore<VectorAxisX, VectorAxisX>(row0, a.mV);
    row1 = _asmSwizzleStore<VectorAxisY, VectorAxisX>(row1, a.mV);
    row2 = _asmSwizzleStore<VectorAxisZ, VectorAxisX>(row2, a.mV);

    return Matrix44Affine(Vector3(row0), Vector3(row1), Vector3(row2), Vector3(zero));
}

RW_MATH_FORCE_INLINE Matrix44Affine RotateAroundXAxis(Matrix44Affine::InParam m, VecFloat::InParam angle) 
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labeled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //    [         e00                e01                e02        0 ]
    //    [ e20 SIN + e10 COS  e21 SIN + e11 COS  e22 SIN + e12 COS  0 ]
    //    [ e20 COS - e10 SIN  e21 COS - e11 SIN  e22 COS - e12 SIN  0 ]
    //    [         e30                e31                e32        1 ]
    VecFloat s,c;
    SinCos(angle, s, c);

    // rotate incrementally about x
    return Matrix44Affine(
        m.xAxis,
        m.zAxis * s + m.yAxis * c,
        m.zAxis * c - m.yAxis * s,
        m.wAxis        
    );                     
}

RW_MATH_FORCE_INLINE Matrix44Affine RotateAroundYAxis(Matrix44Affine::InParam m, VecFloat::InParam angle) 
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labelled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e00 COS - e20 SIN  e01 COS - e21 SIN  e02 COS - e22 SIN  0 ]
    //  [         e10                e11                e12        0 ]
    //  [ e00 SIN + e20 COS  e01 SIN + e21 COS  e02 SIN + e22 COS  0 ]
    //  [         e30                e31                e32        1 ]
    VecFloat s,c;

    SinCos(angle, s, c);

    // rotate incrementally about y
    return Matrix44Affine(
        m.xAxis * c - m.zAxis * s,
        m.yAxis,           
        m.xAxis * s + m.zAxis * c,
        m.wAxis        
    );
}

RW_MATH_FORCE_INLINE Matrix44Affine RotateAroundZAxis(Matrix44Affine::InParam m, VecFloat::InParam angle) 
{
    // Below is the product of a rotation matrix and a generic matrix where
    // each element has been labelled eij (i'th row index, j'th column index).
    // This matrix can be easily expressed as row vector operations.
    //
    //  [ e10 SIN + e00 COS  e11 SIN + e01 COS  e12 SIN + e02 COS  0 ]
    //  [ e10 COS - e00 SIN  e11 COS - e01 SIN  e12 COS - e02 SIN  0 ]
    //  [         e20                e21                e22        0 ]
    //  [         e30                e31                e32        1 ]
    VecFloat s,c;

    SinCos(angle, s, c);

    // rotate incrementally about z
    return Matrix44Affine(
        m.yAxis * s + m.xAxis * c,
        m.yAxis * c - m.xAxis * s,
        m.zAxis,           
        m.wAxis        
    );
}

RW_MATH_FORCE_INLINE Matrix44Affine  Matrix44AffineTransposeRotationalComponent(Matrix44Affine::InParam m) 
{
    Matrix44Affine result = m;

    // transpose rotation component of matrix
    result.SetElem(1, m.GetElem(3));
    result.SetElem(3, m.GetElem(1));
    
    result.SetElem(2, m.GetElem(6));
    result.SetElem(6, m.GetElem(2));

    result.SetElem(5, m.GetElem(7));
    result.SetElem(7, m.GetElem(5));
    return result;
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineTranslate(Matrix44Affine::InParam m, Vector3::InParam trans) 
{
    // multiply translations by current matrix
    Vector3 translationVector(
        (trans.X() * m.GetElem(0)) + (trans.Y() * m.GetElem(3)) + (trans.Z() * m.GetElem(6)),
        (trans.X() * m.GetElem(1)) + (trans.Y() * m.GetElem(4)) + (trans.Z() * m.GetElem(7)),
        (trans.X() * m.GetElem(2)) + (trans.Y() * m.GetElem(5)) + (trans.Z() * m.GetElem(8))
        );

    // and translate current matrix by rotated translation    
    return Matrix44Affine(
        m.GetRow(0),
        m.GetRow(1),
        m.GetRow(2),
        m.GetRow(3) + translationVector
        );
}

RW_MATH_FORCE_INLINE Matrix44Affine ScaleRow(Matrix44Affine::InParam m, VecFloat::InParam r0, VecFloat::InParam r1, VecFloat::InParam r2) 
{
    Vector3 row0Scale( r0, r0, r0 );
    Vector3 row1Scale( r1, r1, r1 );
    Vector3 row2Scale( r2, r2, r2 );
    return Matrix44Affine(
        Mult( row0Scale, m.xAxis ),
        Mult( row1Scale, m.yAxis ),
        Mult( row2Scale, m.zAxis ),
        m.wAxis             
        );
}

RW_MATH_FORCE_INLINE Matrix44Affine ScaleColumn(Matrix44Affine::InParam m, VecFloat::InParam c0, VecFloat::InParam c1, VecFloat::InParam c2 ) 
{
    Vector3 rowMultiplier(c0, c1, c2);
    return Matrix44Affine(
        Mult( m.xAxis, rowMultiplier ),
        Mult( m.yAxis, rowMultiplier ),
        Mult( m.zAxis, rowMultiplier ),
        m.wAxis  
        );
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoLH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44Affine(
       two/w,   zero,          zero,
        zero,  two/h,          zero,
        zero,   zero,   one/(zf-zn),
        zero,   zero,   -zn/(zf-zn)
        );
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoRH(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44Affine(
        two/w,  zero,   zero,
        zero,   two/h,  zero,
        zero,   zero,   one/(zn-zf),
        zero,   zero,   zn/(zn-zf)
        );
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrtho(VecFloat::InParam w, VecFloat::InParam h, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    return Matrix44AffineFromOrthoRH(w, h, zn, zf );
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoOffCenterLH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44Affine(
        two/(r-l),           zero,          zero,
             zero,      two/(t-b),          zero,
             zero,           zero,   one/(zf-zn),
      (l+r)/(l-r),    (t+b)/(b-t),    zn/(zn-zf)
      );
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoOffCenterRH(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    const VecFloat two(GetVecFloat_Two());
    return Matrix44Affine(
        two/(r-l),      zero,           zero,
        zero,           two/(t-b),      zero,
        zero,           zero,           one/(zn-zf),
        (l+r)/(l-r),    (t+b)/(b-t),    zn/(zn-zf)
      );
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromOrthoOffCenter(VecFloat::InParam l, VecFloat::InParam r, VecFloat::InParam b, VecFloat::InParam t, VecFloat::InParam zn, VecFloat::InParam zf) 
{
    return Matrix44AffineFromOrthoOffCenterRH(l, r, b, t, zn, zf);
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromQuaternionTranslation(Quaternion::InParam q, Vector3::InParam trans) 
{
    VectorIntrinsic xRow, yRow, zRow;
    RW_MATH_VPU_MatrixFromQuaternion( xRow, yRow, zRow, q.mV );
    return Matrix44Affine( xRow, yRow, zRow, trans.mV );
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromScaleQuaternionTranslation(Vector3::InParam scale, Quaternion::InParam q, Vector3::InParam trans) 
{
    VectorIntrinsic xRow, yRow, zRow;
    VectorIntrinsic xScale, yScale, zScale;
    RW_MATH_VPU_Splat_I( xScale, scale.mV, 0 );
    RW_MATH_VPU_Splat_I( yScale, scale.mV, 1 );
    RW_MATH_VPU_Splat_I( zScale, scale.mV, 2 );
    RW_MATH_VPU_MatrixFromQuaternion( xRow, yRow, zRow, q.mV );
    RW_MATH_VPU_Mult( xRow, xRow, xScale );
    RW_MATH_VPU_Mult( yRow, yRow, yScale );
    RW_MATH_VPU_Mult( zRow, zRow, zScale );
    return Matrix44Affine( xRow, yRow, zRow, trans.mV );
}

RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromLookAt(Vector3::InParam lookAt, Vector3::InParam up, Vector3::InParam eyePosition) 
{
    // calculate the 3 axes
    Vector3 zaxis = Normalize(Subtract(eyePosition, lookAt));
    Vector3 xaxis = Normalize(Cross(up, zaxis));
    Vector3 yaxis = Cross(zaxis, xaxis);

    return Matrix44Affine(
        xaxis.X(), yaxis.X(), zaxis.X(),
        xaxis.Y(), yaxis.Y(), zaxis.Y(),
        xaxis.Z(), yaxis.Z(), zaxis.Z(),
        -Dot(xaxis, eyePosition), -Dot(yaxis, eyePosition), -Dot(zaxis, eyePosition)
        );
}

RW_MATH_FORCE_INLINE VecFloat GetNormalError3x3(Matrix44Affine::InParam matrix) 
{
    return GetNormalError( Matrix33(matrix.xAxis, matrix.yAxis, matrix.zAxis) );
}

RW_MATH_FORCE_INLINE bool IsNormal3x3(Matrix44Affine::InParam matrix,        VecFloat::InParam tolerance) 
{
    return IsZero( GetNormalError3x3(matrix), tolerance );
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalError3x3(Matrix44Affine::InParam matrix) 
{
    return GetOrthogonalError( Matrix33(matrix.xAxis, matrix.yAxis, matrix.zAxis) );
}

RW_MATH_FORCE_INLINE VecFloat GetOrthogonalRowSetError3x3(Matrix44Affine::InParam matrix) 
{
    return GetOrthogonalRowSetError( Matrix33(matrix.xAxis, matrix.yAxis, matrix.zAxis) );
}

RW_MATH_FORCE_INLINE Vector3 QueryRotateDegenerateUnitAxis(Matrix44Affine::InParam matrix) 
{
    return QueryRotateDegenerateUnitAxis( Matrix33(matrix.xAxis, matrix.yAxis, matrix.zAxis) );
}

RW_MATH_FORCE_INLINE bool IsOrthogonal3x3(Matrix44Affine::InParam matrix,        VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalError3x3(matrix), tolerance );
}

RW_MATH_FORCE_INLINE bool IsRotation3x3(Matrix44Affine::InParam a, VecFloat::InParam tolerance)
{
    return IsOrthogonal3x3(a, tolerance) && IsSimilar(Determinant3x3(a), GetVecFloat_One(), tolerance);
}

RW_MATH_FORCE_INLINE bool IsOrthogonalRowSet3x3(Matrix44Affine::InParam matrix,        VecFloat::InParam tolerance) 
{
    return IsZero( GetOrthogonalRowSetError3x3(matrix), tolerance );
}

RW_MATH_INLINE Matrix33 Get3x3(Matrix44Affine::InParam m)
{
    return Matrix33(m.xAxis, m.yAxis, m.zAxis);
}

RW_MATH_INLINE Matrix44Affine Set3x3(Matrix44Affine::InParam m, Matrix33::InParam m33)
{
    return Matrix44Affine(m33.xAxis, m33.yAxis, m33.zAxis, m.wAxis);
}

RW_MATH_FORCE_INLINE bool IsRotation(Matrix44Affine::InParam a, VecFloat::InParam tolerance)
{
    return IsOrthogonal(a, tolerance) && IsSimilar(Determinant(a), GetVecFloat_One(), tolerance);
}

RW_MATH_FORCE_INLINE Matrix44Affine Select(MaskScalar::InParam mask, Matrix44Affine::InParam trueValue, Matrix44Affine::InParam falseValue )
{
    VectorIntrinsic xRow, yRow, zRow, wRow;
    RW_MATH_VPU_Select( xRow, falseValue.xAxis.mV, trueValue.xAxis.mV, mask.mV );
    RW_MATH_VPU_Select( yRow, falseValue.yAxis.mV, trueValue.yAxis.mV, mask.mV );
    RW_MATH_VPU_Select( zRow, falseValue.zAxis.mV, trueValue.zAxis.mV, mask.mV );
    RW_MATH_VPU_Select( wRow, falseValue.wAxis.mV, trueValue.wAxis.mV, mask.mV );
    return Matrix44Affine(xRow, yRow, zRow, wRow);
}

} // namespace vpu
} // namespace math
} // namespace rw

#endif
