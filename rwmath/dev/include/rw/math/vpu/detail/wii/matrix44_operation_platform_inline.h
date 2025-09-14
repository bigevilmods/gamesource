// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_MATRIX44_OPERATION_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_MATRIX44_OPERATION_PLATFORM_INLINE_H

namespace rw
{

namespace math
{

namespace vpu
{

    RW_MATH_FORCE_INLINE Matrix44 Matrix44FromScaleTranslation(Vector3::InParam scale, Vector3::InParam trans) 
    {
        //scale.X(), zero, zero, zero,
        //zero, scale.Y(), zero, zero,
        //zero, zero, scale.Z(), zero,
        //trans.X(), trans.Y(), trans.Z(), one
        VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
        VectorIntrinsic row0 = zero; 
        VectorIntrinsic row1 = zero; 
        VectorIntrinsic row2 = zero; 
        VectorIntrinsic row3 = trans.mV; 

        row0 = _asmSwizzleStore<VectorAxisX, VectorAxisX>(row0, scale.mV);
        row1 = _asmSwizzleStore<VectorAxisY, VectorAxisY>(row1, scale.mV);
        row2 = _asmSwizzleStore<VectorAxisZ, VectorAxisZ>(row2, scale.mV);
        row3 = _asmSwizzleStore<VectorAxisW, VectorAxisW>(row3, RW_MATH_VPU_GetOne_R());

        return Matrix44(Vector4(row0), Vector4(row1), Vector4(row2), Vector4(row3));
    }

    RW_MATH_FORCE_INLINE Matrix44 Matrix44FromDiagonal(VecFloat::InParam a) 
    {
        //a,0,0,0,
        //0,a,0,0,
        //0,0,a,0,
        //0,0,0,a
        VectorIntrinsic zero = RW_MATH_VPU_GetZero_R();
        VectorIntrinsic row0 = zero; 
        VectorIntrinsic row1 = zero; 
        VectorIntrinsic row2 = zero; 
        VectorIntrinsic row3 = zero;

        row0 = _asmSwizzleStore<VectorAxisX, VectorAxisX>(row0, a.mV);
        row1 = _asmSwizzleStore<VectorAxisY, VectorAxisX>(row1, a.mV);
        row2 = _asmSwizzleStore<VectorAxisZ, VectorAxisX>(row2, a.mV);
        row3 = _asmSwizzleStore<VectorAxisW, VectorAxisX>(row3, a.mV);

        return Matrix44(Vector4(row0), Vector4(row1), Vector4(row2), Vector4(row3));
    }

    RW_MATH_FORCE_INLINE VecFloat Determinant(Matrix44::InParam m) 
    {
        VectorIntrinsic d;
        _asmDeterminant(d, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, m.wAxis.mV);
        return VecFloat(d);
    }

    namespace detail
    {
        Vector4 Mult(Vector4::InParam v, Matrix44::InParam m);
        Matrix44 Mult(Matrix44::InParam m, Matrix44::InParam b);
    }

    RW_MATH_FORCE_INLINE Vector4 Mult(Vector4::InParam v, Matrix44::InParam m) 
    {
        return detail::Mult(v, m);
    }

    RW_MATH_FORCE_INLINE Matrix44 Mult(Matrix44::InParam m, Matrix44::InParam b) 
    {
        return detail::Mult(m,b);
    }

    RW_MATH_FORCE_INLINE Matrix44 Mult(Matrix44Affine::InParam m, Matrix44::InParam b) 
    {
        // Last columm of m is treated as if it were 0, 0, 0, 1
        VectorIntrinsic vs[4], temp[4], result[4];

        RW_MATH_VPU_Splat_I(vs[0], m.xAxis.mV, VectorAxisX);
        RW_MATH_VPU_Splat_I(vs[1], m.yAxis.mV, VectorAxisX);
        RW_MATH_VPU_Splat_I(vs[2], m.zAxis.mV, VectorAxisX);
        RW_MATH_VPU_Splat_I(vs[3], m.wAxis.mV, VectorAxisX);

        RW_MATH_VPU_Mult(   temp[0], vs[0], b.xAxis.mV);
        RW_MATH_VPU_Mult(   temp[1], vs[1], b.xAxis.mV);
        RW_MATH_VPU_Mult(   temp[2], vs[2], b.xAxis.mV);
        RW_MATH_VPU_MultAdd(temp[3], vs[3], b.xAxis.mV, b.wAxis.mV);

        RW_MATH_VPU_Splat_I(vs[0], m.xAxis.mV, VectorAxisY);
        RW_MATH_VPU_Splat_I(vs[1], m.yAxis.mV, VectorAxisY);
        RW_MATH_VPU_Splat_I(vs[2], m.zAxis.mV, VectorAxisY);
        RW_MATH_VPU_Splat_I(vs[3], m.wAxis.mV, VectorAxisY);

        RW_MATH_VPU_MultAdd(temp[0], vs[0], b.yAxis.mV, temp[0]);
        RW_MATH_VPU_MultAdd(temp[1], vs[1], b.yAxis.mV, temp[1]);
        RW_MATH_VPU_MultAdd(temp[2], vs[2], b.yAxis.mV, temp[2]);
        RW_MATH_VPU_MultAdd(temp[3], vs[3], b.yAxis.mV, temp[3]);

        RW_MATH_VPU_Splat_I(vs[0], m.xAxis.mV, VectorAxisZ);
        RW_MATH_VPU_Splat_I(vs[1], m.yAxis.mV, VectorAxisZ);
        RW_MATH_VPU_Splat_I(vs[2], m.zAxis.mV, VectorAxisZ);
        RW_MATH_VPU_Splat_I(vs[3], m.wAxis.mV, VectorAxisZ);

        RW_MATH_VPU_MultAdd(result[0], vs[0], b.zAxis.mV, temp[0]);
        RW_MATH_VPU_MultAdd(result[1], vs[1], b.zAxis.mV, temp[1]);
        RW_MATH_VPU_MultAdd(result[2], vs[2], b.zAxis.mV, temp[2]);
        RW_MATH_VPU_MultAdd(result[3], vs[3], b.zAxis.mV, temp[3]);

        return Matrix44(result[0], result[1], result[2], result[3]);
    }

    RW_MATH_FORCE_INLINE Matrix44 Transpose(Matrix44::InParam m) 
    {
        Matrix44 r;
        RW_MATH_VPU_TransposeA4V4(r.xAxis.mV, r.yAxis.mV, r.zAxis.mV, r.wAxis.mV,
                                  m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, m.wAxis.mV);
        return r;
    }

    RW_MATH_INLINE Matrix44 Inverse(Matrix44::InParam m, VecFloat& determinant)
    {
        Matrix44 r;
        _asmInverse(determinant.mV, r.xAxis.mV, r.yAxis.mV, r.zAxis.mV, r.wAxis.mV, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV, m.wAxis.mV);

        return r;
    }


    RW_MATH_FORCE_INLINE Matrix44 Matrix44FromAxisRotationAngle(Vector3::InParam axis,           VecFloat::InParam angle) 
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

        const VecFloat zero(RW_MATH_VPU_GetZero_R());
        const VecFloat one(RW_MATH_VPU_GetOne_R());
        return Matrix44(
            tx * axis.X() + c ,   
            tx * axis.Y() + sz,   
            tx * axis.Z() - sy,
            zero,

            ty * axis.X() - sz,   
            ty * axis.Y() + c ,   
            ty * axis.Z() + sx,   
            zero,

            tz * axis.X() + sy,   
            tz * axis.Y() - sx,   
            tz * axis.Z() + c,
            zero,

            zero,
            zero,
            zero,
            one
            );
    }


    RW_MATH_FORCE_INLINE Matrix44 Matrix44FromXRotationAngle(VecFloat::InParam angle) 
    {
        VecFloat s,c;
        SinCos(angle,s,c);
        const VecFloat zero(RW_MATH_VPU_GetZero_R());
        const VecFloat one(RW_MATH_VPU_GetOne_R());
        return Matrix44(
            one,   zero,   zero,  zero,
            zero,   c,   s,  zero, 
            zero,  -s,   c,  zero,  
            zero,   zero,   zero,  one
            );
    }

    RW_MATH_FORCE_INLINE Matrix44 Matrix44FromYRotationAngle(VecFloat::InParam angle) 
    {
        VecFloat s,c;
        SinCos(angle,s,c);
        const VecFloat zero(RW_MATH_VPU_GetZero_R());
        const VecFloat one(RW_MATH_VPU_GetOne_R());
        return Matrix44(
            c,   zero,  -s,  zero,
            zero,   one,   zero,  zero, 
            s,   zero,   c,  zero,  
            zero,   zero,   zero,  one );
    }

    RW_MATH_FORCE_INLINE Matrix44 Matrix44FromZRotationAngle(VecFloat::InParam angle) 
    {
        VecFloat s,c;
        SinCos(angle,s,c);
        const VecFloat zero(RW_MATH_VPU_GetZero_R());
        const VecFloat one(RW_MATH_VPU_GetOne_R());
        return Matrix44(
            c,   s,   zero,  zero,
            -s,   c,   zero,  zero, 
            zero,   zero,   one,  zero,  
            zero,   zero,   zero,  one );
    }


    // Inverts the upper left 3x3 matrix and leaves all other matrix entries unchanged.
    RW_MATH_INLINE Matrix44 Inverse3x3(Matrix44::InParam m, VecFloat& determinant)
    {
        Matrix44 r;

        RW_MATH_VPU_InverseA3V3(determinant.mV, r.xAxis.mV, r.yAxis.mV, r.zAxis.mV, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV);

        RW_MATH_VPU_SwizzleStore_I_I(r.xAxis.mV, r.xAxis.mV, VectorAxisW, m.xAxis.mV, VectorAxisW);
        RW_MATH_VPU_SwizzleStore_I_I(r.yAxis.mV, r.yAxis.mV, VectorAxisW, m.yAxis.mV, VectorAxisW);
        RW_MATH_VPU_SwizzleStore_I_I(r.zAxis.mV, r.zAxis.mV, VectorAxisW, m.zAxis.mV, VectorAxisW);
        r.wAxis.mV = m.wAxis.mV;
        
        return r;
    }

    RW_MATH_FORCE_INLINE Matrix44  Matrix44TransposeRotationalComponent(Matrix44::InParam m) 
    {
        Matrix44 r;
        RW_MATH_VPU_TransposeA3V3(r.xAxis.mV, r.yAxis.mV, r.zAxis.mV, m.xAxis.mV, m.yAxis.mV, m.zAxis.mV);

        RW_MATH_VPU_SwizzleStore_I_I(r.xAxis.mV, r.xAxis.mV, VectorAxisW, m.xAxis.mV, VectorAxisW);
        RW_MATH_VPU_SwizzleStore_I_I(r.yAxis.mV, r.yAxis.mV, VectorAxisW, m.yAxis.mV, VectorAxisW);
        RW_MATH_VPU_SwizzleStore_I_I(r.zAxis.mV, r.zAxis.mV, VectorAxisW, m.zAxis.mV, VectorAxisW);
        r.wAxis.mV = m.wAxis.mV;

        return r;
    }

    RW_MATH_FORCE_INLINE Matrix44 Matrix44Translate(Matrix44::InParam m, Vector3::InParam trans) 
    {
        // multiply translations by current matrix
        Vector4 translationVector(
            (trans.X() * m.GetElem( 0)) + (trans.Y() * m.GetElem( 4)) + (trans.Z() * m.GetElem( 8)),
            (trans.X() * m.GetElem( 1)) + (trans.Y() * m.GetElem( 5)) + (trans.Z() * m.GetElem( 9)),
            (trans.X() * m.GetElem( 2)) + (trans.Y() * m.GetElem( 6)) + (trans.Z() * m.GetElem(10)),
            GetVecFloat_Zero()
            );

        // and translate current matrix by rotated translation    
        return Matrix44(
            m.GetRow(0),
            m.GetRow(1),
            m.GetRow(2),
            m.GetRow(3) + translationVector
            );
    }


    RW_MATH_INLINE void QueryRotate(Matrix44::InParam matrix, Vector3& unitAxis, VecFloat& angle, Vector3& center)
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
        Vector3 axisXpos = Cross(unitAxis, GetXYZ(matrix.wAxis));
        center = Cross(axisXpos, unitAxis) * GetVecFloat_Half();
        center += axisXpos * (GetVecFloat_Half() * twoSinTheta / (GetVecFloat_Two() - twoCosTheta));
    }

    RW_MATH_INLINE Matrix44 SLerp(Matrix44::InParam lFrom, Matrix44::InParam lTo, VecFloat::InParam lrParam, VecFloat& lprAngle)
    {
        Matrix44 lReturnmatrix;
        Matrix44 lScratch;
        Vector3  axis;
        Vector3  lCentre;
        VecFloat lrAngle;

        // Do we need to interpolate?
        if(lrParam <= GetVecFloat_Zero())
        {
            lprAngle = AngleBetweenVectors( GetXYZ(lFrom.zAxis), GetXYZ(lTo.zAxis) );
            return lFrom;
        }
        else if(lrParam >= GetVecFloat_One())
        {
            lprAngle = GetVecFloat_Zero();
            return lTo;
        }

        // Interpolate - lerp or slerp?
        lScratch = Mult( Transpose(lFrom), lTo );

        QueryRotate(lScratch, axis, lrAngle, lCentre );

        // Lerp? (faster than slerp)
        if( lrAngle < VecFloat(MATRIX_MIN_SLERP_ANGLE) )
        {
            // Get lerp matrix
            Matrix44 lLerpmat;

            lLerpmat.xAxis = lTo.xAxis - lFrom.xAxis;
            lLerpmat.yAxis = lTo.yAxis - lFrom.yAxis;
            lLerpmat.zAxis = lTo.zAxis - lFrom.zAxis;
            lLerpmat.wAxis = lTo.wAxis - lFrom.wAxis;

            // Do lerp
            lLerpmat *= lrParam;

            lReturnmatrix.xAxis = Normalize(lFrom.xAxis + lLerpmat.xAxis);
            lReturnmatrix.yAxis = Normalize(lFrom.yAxis + lLerpmat.yAxis);
            lReturnmatrix.zAxis = Normalize(lFrom.zAxis + lLerpmat.zAxis);
            lReturnmatrix.wAxis = lFrom.wAxis + lLerpmat.wAxis;
        }
        else
            // SLerp
        {
            Matrix44 lRotatematrix;

            Vector4 lStartPos = lFrom.wAxis;
            Vector4 lEndPos = lTo.wAxis;

            // Remove the translation for now
            lReturnmatrix = lFrom;
            lReturnmatrix.wAxis = GetVector4_WAxis() ;

            // Rotate the new matrix
            lRotatematrix = Matrix44FromAxisRotationAngle( axis, ( lrAngle * lrParam ) );
            lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

            // Do linear interpolation on position
            lReturnmatrix.wAxis = (lEndPos - lStartPos) * lrParam + lStartPos;
        }

        lprAngle = lrAngle - (lrAngle * lrParam);
        return lReturnmatrix;

    }

    RW_MATH_INLINE Matrix44 SLerpByAngle(Matrix44::InParam lFrom, Matrix44::InParam lTo, VecFloat::InParam lrMaxAngle,  VecFloat& lprAngle)
    {
        Matrix44 lReturnmatrix;
        Matrix44 lRotatematrix;
        Vector3  axis;
        Vector3  lCentre;
        VecFloat lrAngle;
        VecFloat lrDeltaAngle;

        // Get axis and angle between From and To
        Matrix44 lScratch = Transpose(lFrom);
        lScratch = Mult( lScratch, lTo );

        QueryRotate(lScratch, axis, lrAngle, lCentre );
        if( IsZero(axis) )
        {
            lprAngle = 0.0f;
            return lTo;
        }

        // Copy from and remove translation
        Vector4 lPosBackup = lFrom.wAxis;
        lReturnmatrix = lFrom;
        lReturnmatrix.wAxis = GetVector4_WAxis() ;

        // Rotate about axis by no more than maximum specified angle
        lrDeltaAngle = Min(lrAngle, lrMaxAngle);

        lRotatematrix = Matrix44FromAxisRotationAngle( axis, lrDeltaAngle );
        lReturnmatrix = Mult( lReturnmatrix, lRotatematrix );

        // Put back original translation
        lReturnmatrix.wAxis = lPosBackup ;

        lprAngle = lrAngle - lrDeltaAngle;

        return lReturnmatrix;
    }

    RW_MATH_INLINE Matrix44 OrthoNormalize3x3(Matrix44::InParam matrix)
    {
        Vector3 *vpU, *vpV, *vpW;

        Vector3 right        = Normalize(GetXYZ(matrix.xAxis));
        Vector3 up           = Normalize(GetXYZ(matrix.yAxis));
        Vector3 at           = Normalize(GetXYZ(matrix.zAxis));
        const Vector4 &pos   = matrix.wAxis;

        Vector3 vInner( Magnitude(GetXYZ(matrix.xAxis)),
            Magnitude(GetXYZ(matrix.yAxis)),
            Magnitude(GetXYZ(matrix.zAxis)) );

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
        Matrix44 result(
            right.mV,
            up.mV,
            at.mV,
            pos.mV
            );
        // Set the last column to [0,0,0,1]
        result.xAxis.W() = 0.0f;
        result.yAxis.W() = 0.0f;
        result.zAxis.W() = 0.0f;
        result.wAxis.W() = 1.0f;
        return result;
    }

}

}

}

#endif
