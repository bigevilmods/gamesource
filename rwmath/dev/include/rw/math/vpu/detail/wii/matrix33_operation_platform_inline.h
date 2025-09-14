// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MATRIX33_WII_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_MATRIX33_WII_OPERATION_PLATFORM_INLINE_H

namespace rw
{
namespace math
{
namespace vpu
{

namespace detail
{
    Matrix33 OrthoNormalize(Matrix33::InParam matrix);
    Vector3 Mult(Vector3::InParam v, Matrix33::InParam m);
    Matrix33 Mult(Matrix33::InParam m, Matrix33::InParam b);
    VecFloat GetNormalError(Matrix33::InParam matrix);
}

RW_MATH_FORCE_INLINE VecFloat GetNormalError(Matrix33::InParam matrix) 
{
    return detail::GetNormalError(matrix);
}

RW_MATH_FORCE_INLINE Vector3 Mult(Vector3::InParam v, Matrix33::InParam m) 
{
    return detail::Mult(v, m);
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
    return detail::Mult(m, b);
}

RW_MATH_FORCE_INLINE Matrix33 Transpose(Matrix33::InParam m) 
{
    VectorIntrinsic x, y, z;
    RW_MATH_VPU_TransposeA3V3(x, y, z, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV);
    return Matrix33(x, y, z);
}

RW_MATH_INLINE Matrix33 OrthoNormalize(Matrix33::InParam matrix)
{
    return detail::OrthoNormalize(matrix);
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
    Vector3 twoSinThetaAxis;
    VecFloat twoCosTheta = matrix.xAxis.X() + matrix.yAxis.Y() + matrix.zAxis.Z() - GetVecFloat_One();
    twoSinThetaAxis.X() =matrix.yAxis.Z() - matrix.zAxis.Y();
    twoSinThetaAxis.Y() =matrix.zAxis.X() - matrix.xAxis.Z();
    twoSinThetaAxis.Z() =matrix.xAxis.Y() - matrix.yAxis.X();
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
    if(lrParam <= GetVecFloat_Zero())
    {
        lprAngle = AngleBetweenVectors( lFrom.zAxis, lTo.zAxis );
        return lFrom;
    }
    else if(lrParam >= GetVecFloat_One())
    {
        lprAngle = GetVecFloat_Zero();
        return lTo;
    }

    // Interpolate - lerp or slerp?
    lScratch = Mult( Transpose(lFrom), lTo );

    QueryRotate(lScratch, axis, lrAngle );

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

#ifdef EA_PLATFORM_PS3
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
        return Vector3(
            ATan2( m.yAxis.Z(), m.zAxis.Z()),
            ATan2(-m.xAxis.Z(), cy),
            ATan2( m.xAxis.Y(), m.xAxis.X())
            );
    }
    else
    {
        return Vector3(
            ATan2(-m.zAxis.Y(), m.yAxis.Y()),
            ATan2(-m.xAxis.Z(), cy),
            VecFloat(RW_MATH_VPU_GetZero_R())
            );
    }
}
#endif

} // namespace vpu
} // namespace math
} // namespace rw

#endif
