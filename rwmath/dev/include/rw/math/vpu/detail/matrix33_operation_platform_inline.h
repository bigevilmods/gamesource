// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MATRIX33_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_MATRIX33_OPERATION_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#if defined EA_PLATFORM_PS3_SPU
    #include "rw/math/vpu/detail/ps3/spu/matrix33_operation_platform_inline.h"
#elif defined EA_PLATFORM_REVOLUTION
    #include "rw/math/vpu/detail/wii/matrix33_operation_platform_inline.h"
#else

namespace rw
{
namespace math
{
namespace vpu
{

RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam v, Matrix33::InParam m) 
{
    VectorIntrinsic tmp = v.mV;
    VectorIntrinsic xSplat, ySplat, zSplat, result;

    RW_MATH_VPU_Splat_I( xSplat, tmp, 0 );
    RW_MATH_VPU_Splat_I( ySplat, tmp, 1 );
    RW_MATH_VPU_Splat_I( zSplat, tmp, 2 );

       RW_MATH_VPU_Mult( result, m.xAxis.mV, xSplat );
    RW_MATH_VPU_MultAdd( result, m.yAxis.mV, ySplat, result );
    RW_MATH_VPU_MultAdd( result, m.zAxis.mV, zSplat, result );

    return Vector3(result);
}

RW_MATH_FORCE_INLINE VecFloat GetNormalError(Matrix33::InParam matrix) 
{
    VecFloat one(RW_MATH_VPU_GetOne_R());
    Vector3 errorVec(   MagnitudeSquared( matrix.xAxis ) - one,
                        MagnitudeSquared( matrix.yAxis ) - one,
                        MagnitudeSquared( matrix.zAxis ) - one );
    VecFloat error = MagnitudeSquared(errorVec);
    return error;
}

RW_MATH_FORCE_INLINE VecFloat Determinant(Matrix33::InParam m) 
{
    VectorIntrinsic cross;
    RW_MATH_VPU_CrossV3(cross, m.xAxis.mV, m.yAxis.mV)
    VectorIntrinsic dot;    
    RW_MATH_VPU_DotV3(dot, cross, m.zAxis.mV);
    return VecFloat(dot);
}

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromQuaternion(Quaternion::InParam q) 
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_MatrixFromQuaternion(x, y, z, q.mV);
    return Matrix33(x, y, z);
}

RW_MATH_FORCE_INLINE Matrix33 Mult(Matrix33::InParam m, Matrix33::InParam b) 
{
    VectorIntrinsic bX = b.xAxis.mV;
    VectorIntrinsic bY = b.yAxis.mV;
    VectorIntrinsic bZ = b.zAxis.mV;
    VectorIntrinsic row0, row1, row2;

    RW_MATH_VPU_TransformVectorV3(row0, m.xAxis.mV, bX, bY, bZ);
    RW_MATH_VPU_TransformVectorV3(row1, m.yAxis.mV, bX, bY, bZ);
    RW_MATH_VPU_TransformVectorV3(row2, m.zAxis.mV, bX, bY, bZ);
    return Matrix33(row0, row1, row2);
}

RW_MATH_FORCE_INLINE Matrix33 Transpose(Matrix33::InParam m) 
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_TransposeA3V3(x, y, z, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV);
    return Matrix33(x, y, z);
}

RW_MATH_INLINE Matrix33 OrthoNormalize(Matrix33::InParam matrix)
{
    Vector3 *vpU, *vpV, *vpW;

    Vector3 right        = Normalize(matrix.xAxis);
    Vector3 up           = Normalize(matrix.yAxis);
    Vector3 at           = Normalize(matrix.zAxis);

    Vector3 vInner( MagnitudeSquared(matrix.xAxis),
                    MagnitudeSquared(matrix.yAxis),
                    MagnitudeSquared(matrix.zAxis) );

    VecFloat zero(GetVecFloat_Zero());
    if (vInner.X() > zero)
    {
        if (vInner.Y() > zero)
        {
            if (vInner.Z() > zero)
            {
                Vector3 vOuter(Abs(Vector3(Dot(up, at), Dot(at,right), Dot(right, up))));
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
    return Matrix33( right, up, at );
}


RW_MATH_INLINE Matrix33 Inverse(Matrix33::InParam m, VecFloat& determinant)
{
    VectorIntrinsic d;
    VectorIntrinsic x;
    VectorIntrinsic y;
    VectorIntrinsic z;
    RW_MATH_VPU_InverseA3V3(d, x, y, z, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV);
    determinant = VecFloat(d);
    return Matrix33(x, y, z);
}

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromAxisRotationAngle(Vector3::InParam axis, VecFloat::InParam angle) 
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

    return Matrix33(

        tx * axis.X() + c ,   
        tx * axis.Y() + sz,   
        tx * axis.Z() - sy,

        ty * axis.X() - sz,   
        ty * axis.Y() + c ,   
        ty * axis.Z() + sx,   

        tz * axis.X() + sy,   
        tz * axis.Y() - sx,   
        tz * axis.Z() + c

        );
}

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromXRotationAngle(VecFloat::InParam angle) 
{
    VecFloat s,c;
    SinCos(angle,s,c);
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix33(
        one,   zero,   zero,
        zero,   c,   s,
        zero,  -s,   c
        );
}

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromYRotationAngle(VecFloat::InParam angle) 
{
    VecFloat s,c;
    SinCos(angle,s,c);
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix33(
        c,   zero,  -s,
        zero,   one,   zero,
        s,   zero,   c );
}

RW_MATH_FORCE_INLINE Matrix33 Matrix33FromZRotationAngle(VecFloat::InParam angle) 
{
    VecFloat s,c;
    SinCos(angle,s,c);
    const VecFloat zero(GetVecFloat_Zero());
    const VecFloat one(GetVecFloat_One());
    return Matrix33(
        c,   s,   zero,
        -s,   c,   zero,
        zero,   zero,   one );
}

RW_MATH_INLINE Vector3 QueryRotateDegenerateUnitAxis(Matrix33::InParam matrix)
{
    if(matrix.xAxis.X() > matrix.yAxis.Y())
    {
        if(matrix.xAxis.X() > matrix.zAxis.Z())
        {
            VecFloat xVal = GetVecFloat_One() + matrix.xAxis.X();
            return Normalize(Vector3(
                xVal*xVal,
                matrix.xAxis.Y() + matrix.yAxis.X(),
                matrix.xAxis.Z() + matrix.zAxis.X()
                ));
        }
        else
        {
            VecFloat zVal = GetVecFloat_One() + matrix.zAxis.Z();
            return Normalize(Vector3(
                matrix.zAxis.X() + matrix.xAxis.Z(),
                matrix.zAxis.Y() + matrix.yAxis.Z(),
                zVal*zVal
                ));
        }
    }
    else
    {
        if(matrix.yAxis.Y() > matrix.zAxis.Z())
        {
            VecFloat yVal = GetVecFloat_One() + matrix.yAxis.Y();
            return Normalize(Vector3(
                matrix.yAxis.X() + matrix.xAxis.Y(),
                yVal*yVal,
                matrix.yAxis.Z() + matrix.zAxis.Y()
                ));
        }
        else
        {
            VecFloat zVal = GetVecFloat_One() + matrix.zAxis.Z();
            return Normalize(Vector3(
                matrix.zAxis.X() + matrix.xAxis.Z(),
                matrix.zAxis.Y() + matrix.yAxis.Z(),
                zVal*zVal
                ));
        }
    }
}

RW_MATH_INLINE void QueryRotate(Matrix33::InParam matrix, Vector3& unitAxis, VecFloat& angle)
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
}

RW_MATH_INLINE Matrix33 SLerp(Matrix33::InParam lFrom, Matrix33::InParam lTo, VecFloat::InParam lrParam, VecFloat& lprAngle)
{
    Matrix33 lReturnmatrix;
    Matrix33 lScratch;
    Vector3  axis;
    VecFloat lrAngle;

    // Do we need to interpolate?
    if(lrParam >= GetVecFloat_One())
    {
        lprAngle = GetVecFloat_Zero();
        return lTo;
    }

    // Interpolate - lerp or slerp?
    lScratch = Mult( Transpose(lFrom), lTo );

    QueryRotate(lScratch, axis, lrAngle );

    if( lrParam <= GetVecFloat_Zero() )
    {
        lprAngle = lrAngle;
        return lFrom;
    }

    // Lerp? (faster than slerp)
    if( lrAngle < VecFloat(MATRIX_MIN_SLERP_ANGLE) )
    {
        // Get lerp matrix
        Matrix33 lLerpmat;

        lLerpmat.xAxis = (lTo.xAxis - lFrom.xAxis) * lrParam;
        lLerpmat.yAxis = (lTo.yAxis - lFrom.yAxis) * lrParam;
        lLerpmat.zAxis = (lTo.zAxis - lFrom.zAxis) * lrParam;

        lReturnmatrix.xAxis = Normalize(lFrom.xAxis + lLerpmat.xAxis);
        lReturnmatrix.yAxis = Normalize(lFrom.yAxis + lLerpmat.yAxis);
        lReturnmatrix.zAxis = Normalize(lFrom.zAxis + lLerpmat.zAxis);
    }
    else
        // SLerp
    {
        Matrix33 lRotatematrix;

        // Remove the translation for now
        lReturnmatrix = lFrom;

        // Rotate the new matrix
        lRotatematrix = Matrix33FromAxisRotationAngle( axis, ( lrAngle * lrParam ) );
        lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );
    }

    lprAngle = lrAngle - (lrAngle * lrParam);
    return lReturnmatrix;

}

RW_MATH_INLINE Matrix33 SLerpByAngle(Matrix33::InParam lFrom, Matrix33::InParam lTo, VecFloat::InParam lrMaxAngle, VecFloat& lprAngle)
{
    Matrix33 lReturnmatrix;
    Matrix33 lRotatematrix;
    Vector3  axis;
    VecFloat lrAngle;
    VecFloat lrDeltaAngle;

    // Get axis and angle between From and To
    Matrix33 lScratch = Transpose(lFrom);
    lScratch = Mult( lScratch, lTo );

    QueryRotate(lScratch, axis, lrAngle );
    if( IsZero(axis) )
    {
        lprAngle = VecFloat(RW_MATH_VPU_GetZero_R());
        return lTo;
    }

    // Copy from and remove translation
    lReturnmatrix = lFrom;

    // Rotate about axis by no more than maximum specified angle
    lrDeltaAngle = Min(lrAngle, lrMaxAngle);

    lRotatematrix = Matrix33FromAxisRotationAngle( axis, lrDeltaAngle );
    lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

    lprAngle = lrAngle - lrDeltaAngle;

    return lReturnmatrix;
}

#if !defined(RW_MATH_USE_REFERENCE_IMPLEMENTATION) && defined EA_PLATFORM_PS3
RW_MATH_INLINE Vector3 EulerXYZFromMatrix33(Matrix33::InParam m)
{
    VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
    VectorIntrinsic signBitMask;
    RW_MATH_VPU_GetSignMaskXYZW(signBitMask);
    VectorIntrinsic maskPNPN = RWMATH_VPU_DETAIL_VecMergeH(zero, signBitMask );

    VectorIntrinsic xx_yy_zz;
    RW_MATH_VPU_MultAdd( xx_yy_zz, m.xAxis.mV, m.xAxis.mV, zero );
    VectorIntrinsic yy = RWMATH_VPU_DETAIL_VecSld( xx_yy_zz, xx_yy_zz/*dummy arg*/, sizeof(float) );
    VectorIntrinsic xx_plus_yy = RW_MATH_VPU_Add_R( xx_yy_zz, yy );
    VectorIntrinsic cy; // value for cy is actually only stored in the X component
    RW_MATH_VPU_SquareRoot( cy, xx_plus_yy );


    // Build aTan2InputY
    //   (
    //     m.yAxis.Z(),
    //    -m.xAxis.Z(),
    //     m.xAxis.Y(),
    //    -m.zAxis.Y()
    //   );
    VectorIntrinsic yz_yw_zx_zy = RWMATH_VPU_DETAIL_VecSld( m.yAxis.mV, m.zAxis.mV, sizeof(float)*2 );
    VectorIntrinsic aTan2InputY = ((VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(yz_yw_zx_zy, m.xAxis.mV, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::X0, ::rw::math::vpu::detail::Z1, ::rw::math::vpu::detail::Y1, ::rw::math::vpu::detail::W0) ));
    aTan2InputY = RW_MATH_VPU_Xor_R( aTan2InputY, maskPNPN );

    // Build aTan2InputX
    //   (
    //     m.zAxis.Z(),
    //     VecFloatRefX(cy),
    //     m.xAxis.X(),
    //     m.yAxis.Y()
    //   );
    VectorIntrinsic aTan2InputX;
    RW_MATH_VPU_CreateVectorAxisA4_I(aTan2InputX, m.zAxis.mV, VectorAxisZ, cy, VectorAxisX, m.xAxis.mV, VectorAxisX, m.yAxis.mV, VectorAxisY) \

    VectorIntrinsic atanV;
    RW_MATH_VPU_ATan2( atanV, aTan2InputY, aTan2InputX );

    // Generate mask to determine what return result to use.
    VectorIntrinsic cyNotSmallMask = RW_MATH_VPU_IsGreater_R( cy, (vector float)(0.001f) );
    RW_MATH_VPU_Splat_I( cyNotSmallMask, cyNotSmallMask, 0 );   // splat x component which has the result of test

    // atanV contains these three elements in X,Y,Z
    // Vector3(
    //    ATan2( m.yAxis.Z(), m.zAxis.Z()),
    //    ATan2(-m.xAxis.Z(), cy),
    //    ATan2( m.xAxis.Y(), m.xAxis.X())
    //    );

    // Place the following elements in atanAlternateResult.
    // Vector3(
    //    ATan2(-m.zAxis.Y(), m.yAxis.Y()),
    //    ATan2(-m.xAxis.Z(), cy),
    //    0.0f
    //    );
    VectorIntrinsic atanAlternateResult = ((::rw::math::vpu::VectorIntrinsic)RWMATH_VPU_DETAIL_VecPerm(atanV, zero, RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(::rw::math::vpu::detail::W0, ::rw::math::vpu::detail::Y0, ::rw::math::vpu::detail::X1, ::rw::math::vpu::detail::X1) ));

    VectorIntrinsic result = RW_MATH_VPU_Select_R( atanAlternateResult, atanV, cyNotSmallMask );
    return Vector3(result);
}
#else
RW_MATH_INLINE Vector3 EulerXYZFromMatrix33(Matrix33::InParam m)
{
    VecFloat cy = Sqrt(m.xAxis.X() * m.xAxis.X() + m.xAxis.Y() * m.xAxis.Y());

    if (cy > VecFloat(0.001f))    // SMALL_REAL
    {
        // This can be made even more optimal, but at least this only calls ATan2 once...
        VectorIntrinsic returnValue;
        Vector3 v1(m.yAxis.Z(), -m.xAxis.Z(), m.xAxis.Y());
        Vector3 v2(m.zAxis.Z(), cy, m.xAxis.X());
        RW_MATH_VPU_ATan2( returnValue, v1.GetVector(), v2.GetVector() );
        return Vector3(returnValue);
    }
    else
    {
        // This can be made even more optimal, but at least this only calls ATan2 once...
        // The Z component is going to be overwritten by 0, so it doesn't matter what's in it for
        // the ATan2 call...
        VectorIntrinsic returnValue;
        Vector2 v1(-m.zAxis.Y(), -m.xAxis.Z());
        Vector2 v2(m.yAxis.Y(), cy);
        RW_MATH_VPU_ATan2( returnValue, v1.GetVector(), v2.GetVector() );
        Vector3 returnVec(returnValue);
        returnVec.SetZ(VecFloat(RW_MATH_VPU_GetZero_R()));
        return returnVec;
    }
}
#endif

} // namespace vpu
} // namespace math
} // namespace rw

#endif
#endif
