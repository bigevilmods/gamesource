// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MATRIX44AFFINE_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_MATRIX44AFFINE_OPERATION_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE VecFloat Determinant(Matrix44Affine::InParam m) 
{
    return  
        m.xAxis.X() * (
        m.yAxis.Y() * m.zAxis.Z() -
        m.yAxis.Z() * m.zAxis.Y() 
        ) + 
        m.xAxis.Y() * (
        m.yAxis.Z() * m.zAxis.X() -
        m.yAxis.X() * m.zAxis.Z()
        ) + 
        m.xAxis.Z() * (
        m.yAxis.X() * m.zAxis.Y() -
        m.yAxis.Y() * m.zAxis.X()
        );
}


RW_MATH_FORCE_INLINE Vector3 TransformVector(Vector3::InParam vec, Matrix44Affine::InParam matrix) 
{
    VectorIntrinsic tmp = vec.mV;
    VectorIntrinsic xSplat, ySplat, zSplat, result;

    RW_MATH_VPU_Splat_I( xSplat, tmp, 0 );
    RW_MATH_VPU_Splat_I( ySplat, tmp, 1 );
    RW_MATH_VPU_Splat_I( zSplat, tmp, 2 );

    RW_MATH_VPU_Mult( result, matrix.xAxis.mV, xSplat );
    RW_MATH_VPU_MultAdd( result, matrix.yAxis.mV, ySplat, result );
    RW_MATH_VPU_MultAdd( result, matrix.zAxis.mV, zSplat, result );

    return Vector3(result);
}


RW_MATH_FORCE_INLINE Vector3 TransformPoint(Vector3::InParam pt, Matrix44Affine::InParam matrix) 
{
    VectorIntrinsic tmp = pt.mV;
    VectorIntrinsic xSplat, ySplat, zSplat, result;

    RW_MATH_VPU_Splat_I( xSplat, tmp, 0 );
    RW_MATH_VPU_Splat_I( ySplat, tmp, 1 );
    RW_MATH_VPU_Splat_I( zSplat, tmp, 2 );

    RW_MATH_VPU_MultAdd( result, matrix.xAxis.mV, xSplat, matrix.wAxis.mV);
    RW_MATH_VPU_MultAdd( result, matrix.yAxis.mV, ySplat, result );
    RW_MATH_VPU_MultAdd( result, matrix.zAxis.mV, zSplat, result );

    return Vector3(result);
}


RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v, Matrix44Affine::InParam m) 
{
    VectorIntrinsic tmp = v.mV;
    VectorIntrinsic splatX, splatY, splatZ, splatW;
    RW_MATH_VPU_Splat_I( splatX, tmp, 0 );
    RW_MATH_VPU_Splat_I( splatY, tmp, 1 );
    RW_MATH_VPU_Splat_I( splatZ, tmp, 2 );
    RW_MATH_VPU_Splat_I( splatW, tmp, 3 );

    VectorIntrinsic result;
    RW_MATH_VPU_Mult( result, m.xAxis.mV, splatX );
    RW_MATH_VPU_MultAdd( result, m.yAxis.mV, splatY, result);
    RW_MATH_VPU_MultAdd( result, m.zAxis.mV, splatZ, result);
    RW_MATH_VPU_MultAdd( result, m.wAxis.mV, splatW, result);

    VectorIntrinsic ret;
    RW_MATH_VPU_SwizzleStore_I_V( ret, result, VectorAxisW, tmp, VectorAxisW );
    return Vector4(ret);
}

#if defined EA_PLATFORM_REVOLUTION

namespace detail { Matrix44Affine Mult(Matrix44Affine::InParam m, Matrix44Affine::InParam b); }

RW_MATH_FORCE_INLINE Matrix44Affine Mult(Matrix44Affine::InParam m, Matrix44Affine::InParam b) 
{
    return detail::Mult( m, b );
}

#else

RW_MATH_FORCE_INLINE Matrix44Affine Mult(Matrix44Affine::InParam m, Matrix44Affine::InParam b) 
{
    VectorIntrinsic sp0, sp1, sp2, sp3;
    VectorIntrinsic ma0, ma1, ma2, ma3;
    VectorIntrinsic bx = b.xAxis.mV;
    VectorIntrinsic by = b.yAxis.mV;
    VectorIntrinsic bz = b.zAxis.mV;

    RW_MATH_VPU_Splat_I( sp0, m.xAxis.mV, 0);
    RW_MATH_VPU_Splat_I( sp1, m.yAxis.mV, 0);
    RW_MATH_VPU_Splat_I( sp2, m.zAxis.mV, 0);
    RW_MATH_VPU_Splat_I( sp3, m.wAxis.mV, 0);

    RW_MATH_VPU_Mult(ma0, sp0, bx );
    RW_MATH_VPU_Mult(ma1, sp1, bx );
    RW_MATH_VPU_Mult(ma2, sp2, bx );
    RW_MATH_VPU_MultAdd(ma3, sp3, bx, b.wAxis.mV);

    RW_MATH_VPU_Splat_I( sp0, m.xAxis.mV, 1);
    RW_MATH_VPU_Splat_I( sp1, m.yAxis.mV, 1);
    RW_MATH_VPU_Splat_I( sp2, m.zAxis.mV, 1);
    RW_MATH_VPU_Splat_I( sp3, m.wAxis.mV, 1);

    RW_MATH_VPU_MultAdd(ma0, sp0, by, ma0);
    RW_MATH_VPU_MultAdd(ma1, sp1, by, ma1);
    RW_MATH_VPU_MultAdd(ma2, sp2, by, ma2);
    RW_MATH_VPU_MultAdd(ma3, sp3, by, ma3);

    RW_MATH_VPU_Splat_I( sp0, m.xAxis.mV, 2);
    RW_MATH_VPU_Splat_I( sp1, m.yAxis.mV, 2);
    RW_MATH_VPU_Splat_I( sp2, m.zAxis.mV, 2);
    RW_MATH_VPU_Splat_I( sp3, m.wAxis.mV, 2);

    RW_MATH_VPU_MultAdd(sp0, sp0, bz, ma0);
    RW_MATH_VPU_MultAdd(sp1, sp1, bz, ma1);
    RW_MATH_VPU_MultAdd(sp2, sp2, bz, ma2);
    RW_MATH_VPU_MultAdd(sp3, sp3, bz, ma3);

    return Matrix44Affine(
        Vector3(sp0),
        Vector3(sp1),
        Vector3(sp2),
        Vector3(sp3)
        );
}

#endif

RW_MATH_FORCE_INLINE Matrix44Affine Transpose(Matrix44Affine::InParam m) 
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_TransposeA4V3( x, y, z, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, m.wAxis.mV );
    return Matrix44Affine( x, y, z, RW_MATH_VPU_GetZero_R() );
}


RW_MATH_FORCE_INLINE Matrix44Affine InverseOfMatrixWithOrthonormal3x3(Matrix44Affine::InParam in) 
{
    RW_MATH_ASSERT(IsNormal3x3(in, VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.01f))) && IsOrthogonal3x3(in, VecFloat(RW_MATH_VPU_CreateVectorFromScalar_R(0.01f))));

    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();

    VectorIntrinsic negatePos = RW_MATH_VPU_Subtract_R(zero, in.wAxis.mV);

    VectorIntrinsic tempT[4];
    tempT[2] = RWMATH_VPU_DETAIL_VecMergeH(in.xAxis.mV, in.zAxis.mV);
    tempT[3] = RWMATH_VPU_DETAIL_VecMergeH(in.yAxis.mV, zero);
    tempT[0] = RWMATH_VPU_DETAIL_VecMergeL(in.xAxis.mV, in.zAxis.mV);
    tempT[1] = RWMATH_VPU_DETAIL_VecMergeL(in.yAxis.mV, zero);

    VectorIntrinsic transposedTransform[4];
    transposedTransform[0] = (VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(tempT[2], tempT[3]);
    transposedTransform[1] = (VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeL(tempT[2], tempT[3]);
    transposedTransform[2] = (VectorIntrinsic)RWMATH_VPU_DETAIL_VecMergeH(tempT[0], tempT[1]);

    VectorIntrinsic X, Y, Z;
    RW_MATH_VPU_Splat_I( X, negatePos, 0 );
    RW_MATH_VPU_Splat_I( Y, negatePos, 1 );
    RW_MATH_VPU_Splat_I( Z, negatePos, 2 );

    RW_MATH_VPU_Mult(transposedTransform[3], Z, transposedTransform[2]);
    RW_MATH_VPU_MultAdd(transposedTransform[3], Y, transposedTransform[1], transposedTransform[3]);
    RW_MATH_VPU_MultAdd(transposedTransform[3], X, transposedTransform[0], transposedTransform[3]);
    return Matrix44Affine(transposedTransform[0], transposedTransform[1], transposedTransform[2], transposedTransform[3]);
}


RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromAxisRotationAngle(Vector3::InParam axis, VecFloat::InParam angle) 
{
    VecFloat s, c;

    SinCos(angle, s, c);
    VecFloat t = GetVecFloat_One() - c;
    VecFloat tx = t * axis.X();    
    VecFloat ty = t * axis.Y();    
    VecFloat tz = t * axis.Z();    
    VecFloat sx = s * axis.X();
    VecFloat sy = s * axis.Y();
    VecFloat sz = s * axis.Z();

    const VecFloat zero(GetVecFloat_Zero());
    return Matrix44Affine(
        tx * axis.X() + c ,   
        tx * axis.Y() + sz,   
        tx * axis.Z() - sy,

        ty * axis.X() - sz,   
        ty * axis.Y() + c ,   
        ty * axis.Z() + sx,   

        tz * axis.X() + sy,   
        tz * axis.Y() - sx,   
        tz * axis.Z() + c,

        zero,
        zero,
        zero
        );
}

    
// Note: This function clears the last row to (0, 0, 0, 1)
RW_MATH_INLINE Matrix44Affine Inverse3x3(Matrix44Affine::InParam m, VecFloat& determinant)
{
    determinant = Determinant(m);
    RW_MATH_ASSERT_MSG((determinant != GetVecFloat_Zero()), ("Matrix is singular."));
    if(determinant==GetVecFloat_Zero()) 
    {
        return Matrix44Affine();
    }
    else
    {
        Matrix44Affine result(GetMatrix44Affine_Zero());
        for(int i=0; i<3; ++i)
        {
            for(int j=0; j<3; ++j)
            {
                int32_t i1=(i+1)%3;
                int32_t i2=(i+2)%3;
                int32_t j1=(j+1)%3;
                int32_t j2=(j+2)%3;
                result.SetElem(j,i,
                    (m.GetElem(i1, j1)*m.GetElem(i2, j2)-m.GetElem(i2, j1)*m.GetElem(i1, j2)) / determinant
                    );
            } // for(int j=0; j<3; ++j)
        } // for(int i=0; i<3; ++i)
        return result;
    }
}

    
RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromXRotationAngle(VecFloat::InParam angle) 
{
    VecFloat s,c;
    SinCos(angle,s,c);
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix44Affine(
        one,    zero,   zero,
        zero,  c,       s,
        zero,  -s,      c,
        zero,  zero,   zero
        );
}


RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromYRotationAngle(VecFloat::InParam angle) 
{
    VecFloat s,c;
    SinCos(angle,s,c);
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix44Affine(
        c,      zero,   -s,
        zero,  one, zero,
        s,      zero,   c,
        zero,  zero,   zero );
}


RW_MATH_FORCE_INLINE Matrix44Affine Matrix44AffineFromZRotationAngle(VecFloat::InParam angle) 
{
    VecFloat s,c;
    SinCos(angle,s,c);
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix44Affine(
        c,      s,      zero,
        -s,     c,      zero,
        zero,  zero,   one,
        zero,  zero,   zero);
}


RW_MATH_INLINE void QueryRotate(Matrix44Affine::InParam matrix, Vector3& unitAxis, VecFloat& angle, Vector3& center)
{
    VecFloat twoCosTheta = matrix.xAxis.X() + matrix.yAxis.Y() + matrix.zAxis.Z() - GetVecFloat_One();
    Vector3 twoSinThetaAxis(
                         matrix.yAxis.Z() - matrix.zAxis.Y(),
                         matrix.zAxis.X() - matrix.xAxis.Z(),
                         matrix.xAxis.Y() - matrix.yAxis.X());
    VecFloat twoSinTheta = Magnitude(twoSinThetaAxis);
    if(twoSinTheta > GetVecFloat_Zero())
    {
        const VecFloat lrRecipLength = GetVecFloat_One() / twoSinTheta;
        unitAxis = twoSinThetaAxis * lrRecipLength;
    }
    else
    {
        unitAxis = GetVector3_Zero();
    }
    angle = ATan2(twoSinTheta, twoCosTheta);
    if((twoSinTheta <= VecFloat(MINIMUM_RECIPROCAL)) && (twoCosTheta <= GetVecFloat_Zero()))
    {
        unitAxis = QueryRotateDegenerateUnitAxis(matrix);
    }
    Vector3 axisXpos = Cross(unitAxis, matrix.wAxis);
    center = Cross(axisXpos, unitAxis) * GetVecFloat_Half();
    center += axisXpos * (GetVecFloat_Half() * twoSinTheta / (GetVecFloat_Two() - twoCosTheta));
}


RW_MATH_INLINE Matrix44Affine SLerp(Matrix44Affine::InParam lFrom, Matrix44Affine::InParam lTo, VecFloat::InParam lrParam, VecFloat& lprAngle)
{
    Matrix44Affine lReturnmatrix;
    Matrix44Affine lScratch;
    Vector3  axis;
    Vector3  lCentre;
    VecFloat lrAngle;

    // Do we need to interpolate?
    if(lrParam >= GetVecFloat_One())
    {
        lprAngle = GetVecFloat_Zero();
        return lTo;
    }

    // Interpolate - lerp or slerp?
    lScratch = Mult( InverseOrthonormal(lFrom), lTo );

    QueryRotate(lScratch, axis, lrAngle, lCentre );

    if( lrParam <= GetVecFloat_Zero() )
    {
        lprAngle = lrAngle;
        return lFrom;
    }

    // Lerp? (faster than slerp)
    if( lrAngle < VecFloat(MATRIX_MIN_SLERP_ANGLE) )
    {
        // Get lerp matrix
        Matrix44Affine lLerpmat;

        lLerpmat.xAxis = lTo.xAxis - lFrom.xAxis;
        lLerpmat.yAxis = lTo.yAxis - lFrom.yAxis;
        lLerpmat.zAxis = lTo.zAxis - lFrom.zAxis;
        lLerpmat.wAxis = lTo.wAxis - lFrom.wAxis;

        // Do lerp
        lLerpmat = Mult( lLerpmat, lrParam);

        lReturnmatrix.xAxis = Normalize(lFrom.xAxis + lLerpmat.xAxis);
        lReturnmatrix.yAxis = Normalize(lFrom.yAxis + lLerpmat.yAxis);
        lReturnmatrix.zAxis = Normalize(lFrom.zAxis + lLerpmat.zAxis);
        lReturnmatrix.wAxis = lFrom.wAxis + lLerpmat.wAxis;
    }
    else
        // SLerp
    {
        Matrix44Affine lRotatematrix;

        Vector3 lStartPos = lFrom.wAxis;
        Vector3 lEndPos = lTo.wAxis;

        // Remove the translation for now
        lReturnmatrix = lFrom;
        lReturnmatrix.wAxis = GetVector3_Zero() ;

        // Rotate the new matrix
        lRotatematrix = Matrix44AffineFromAxisRotationAngle( axis, ( lrAngle * lrParam ) );
        lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

        // Do linear interpolation on position
        lReturnmatrix.wAxis = (lEndPos - lStartPos) * lrParam + lStartPos;
    }

    lprAngle = lrAngle - (lrAngle * lrParam);
    return lReturnmatrix;
}


RW_MATH_INLINE Matrix44Affine SLerpByAngle(Matrix44Affine::InParam lFrom, Matrix44Affine::InParam lTo, VecFloat::InParam lrMaxAngle,  VecFloat& lprAngle)
{
    Matrix44Affine lReturnmatrix;
    Matrix44Affine lRotatematrix;
    Vector3  axis;
    Vector3  lCentre;
    VecFloat lrAngle;
    VecFloat lrDeltaAngle;

    // Get axis and angle between From and To
    Matrix44Affine lScratch = InverseOrthonormal(lFrom);
    lScratch = Mult( lScratch, lTo );

    QueryRotate(lScratch, axis, lrAngle, lCentre );
    if( IsZero(axis) )
    {
        lprAngle = GetVecFloat_Zero();
        return lTo;
    }

    // Copy from and remove translation
    Vector3 lPosBackup = lFrom.wAxis;
    lReturnmatrix = lFrom;
    lReturnmatrix.wAxis = GetVector3_Zero() ;

    // Rotate about axis by no more than maximum specified angle
    lrDeltaAngle = Min(lrAngle, lrMaxAngle);

    lRotatematrix = Matrix44AffineFromAxisRotationAngle( axis, lrDeltaAngle );
    lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

    // Put back original translation
    lReturnmatrix.wAxis = lPosBackup ;

    lprAngle = lrAngle - lrDeltaAngle;

    return lReturnmatrix;
}


RW_MATH_INLINE Matrix44Affine OrthoNormalize3x3(Matrix44Affine::InParam matrix)
{
    Vector3 *vpU, *vpV, *vpW;

    Vector3 right        = Normalize(matrix.xAxis);
    Vector3 up           = Normalize(matrix.yAxis);
    Vector3 at           = Normalize(matrix.zAxis);
    const Vector3 &pos   = matrix.wAxis;

    Vector3 vInner( Magnitude(matrix.xAxis),
        Magnitude(matrix.yAxis),
        Magnitude(matrix.zAxis) );

    if (vInner.X() > GetVecFloat_Zero())
    {
        if (vInner.Y() > GetVecFloat_Zero())
        {
            if (vInner.Z() > GetVecFloat_Zero())
            {
                Vector3 vOuter(Abs(Dot(up, at)), Abs(Dot(at,right)), Abs(Dot(right, up)));
                if (vOuter.X() < vOuter.Y())
                {
                    if (vOuter.X() < vOuter.Z())
                    {
                        vpU = &up;
                        vpV = &at;
                        vpW = &right;
                    }
                    else
                    {
                        vpU = &right;
                        vpV = &up;
                        vpW = &at;
                    }
                }
                else
                {
                    if (vOuter.Y() < vOuter.Z())
                    {
                        vpU = &at;
                        vpV = &right;
                        vpW = &up;
                    }
                    else
                    {
                        vpU = &right;
                        vpV = &up;
                        vpW = &at;
                    }
                }
            }
            else
            {
                vpU = &right;
                vpV = &up;
                vpW = &at;
            }
        }
        else
        {
            vpU = &at;
            vpV = &right;
            vpW = &up;
        }
    }
    else
    {
        vpU = &up;
        vpV = &at;
        vpW = &right;
    }
    *vpW = Normalize(Cross(*vpU, *vpV));
    *vpV = Normalize(Cross(*vpW, *vpU));
    Matrix44Affine result(
        right.mV,
        up.mV,
        at.mV,
        pos.mV
        );
    return result;
}


RW_MATH_INLINE Matrix44Affine InverseGeneric3x3(Matrix44Affine::InParam in)
{
#define rwMATRIXDET(MAA, MAB, MBA, MBB)  \
( (MAA) * (MBB)                        \
- (MAB) * (MBA)                        \
)
    Matrix44Affine out;
    out.xAxis.X() =rwMATRIXDET(in.yAxis.Y(), in.yAxis.Z(), in.zAxis.Y(), in.zAxis.Z());
    out.xAxis.Y() =-rwMATRIXDET(in.xAxis.Y(), in.xAxis.Z(), in.zAxis.Y(), in.zAxis.Z());
    out.xAxis.Z() =rwMATRIXDET(in.xAxis.Y(), in.xAxis.Z(), in.yAxis.Y(), in.yAxis.Z());
    VecFloat determinant = ((out.xAxis.X()) * (in.xAxis.X()) +
        (out.xAxis.Y()) * (in.yAxis.X()) +
        (out.xAxis.Z()) * (in.zAxis.X()) );
    VecFloat normalize;
    if (IsZero(determinant))
    {
        normalize = GetVecFloat_One();
    }
    else
    {
        normalize = GetVecFloat_One() / determinant;
    }

    out.xAxis *= normalize;
    out.yAxis.X() =-rwMATRIXDET(in.yAxis.X(), in.yAxis.Z(), in.zAxis.X(), in.zAxis.Z()) * normalize;
    out.yAxis.Y() = rwMATRIXDET(in.xAxis.X(), in.xAxis.Z(), in.zAxis.X(), in.zAxis.Z()) * normalize;
    out.yAxis.Z() =-rwMATRIXDET(in.xAxis.X(), in.xAxis.Z(), in.yAxis.X(), in.yAxis.Z()) * normalize;
    out.zAxis.X() = rwMATRIXDET(in.yAxis.X(), in.yAxis.Y(), in.zAxis.X(), in.zAxis.Y()) * normalize;
    out.zAxis.Y() =-rwMATRIXDET(in.xAxis.X(), in.xAxis.Y(), in.zAxis.X(), in.zAxis.Y()) * normalize;
    out.zAxis.Z() = rwMATRIXDET(in.xAxis.X(), in.xAxis.Y(), in.yAxis.X(), in.yAxis.Y()) * normalize;
    out.wAxis.X() =-((in.wAxis.X() * out.xAxis.X()) +
        (in.wAxis.Y() * out.xAxis.Y()) +
        (in.wAxis.Z() * out.xAxis.Z()));
    out.wAxis.Y() =-((in.wAxis.X() * out.yAxis.X()) +
        (in.wAxis.Y() * out.yAxis.Y()) +
        (in.wAxis.Z() * out.yAxis.Z()));
    out.wAxis.Z() =-((in.wAxis.X() * out.zAxis.X()) +
        (in.wAxis.Y() * out.zAxis.Y()) +
        (in.wAxis.Z() * out.zAxis.Z()));
    return out;
#undef rwMATRIXDET
}


}}}

#endif
