// (c) Electronic Arts. All Rights Reserved.


#ifdef EXPLICIT_NAMESPACE
#undef EXPLICIT_NAMESPACE
#endif

#ifdef GENERIC_TESTING

    typedef ::rw::math::fpu::MaskScalarTemplate<FloatType>           MaskScalar;
    typedef ::rw::math::fpu::Mask2Template<FloatType>                Mask2;
    typedef ::rw::math::fpu::Mask3Template<FloatType>                Mask3;
    typedef ::rw::math::fpu::Mask4Template<FloatType>                Mask4;
    typedef ::rw::math::fpu::Vector2Template<FloatType>              Vector2;
    typedef ::rw::math::fpu::Vector3Template<FloatType>              Vector3;
    typedef ::rw::math::fpu::Vector4Template<FloatType>              Vector4;
    typedef ::rw::math::fpu::QuaternionTemplate<FloatType>           Quaternion;
    typedef ::rw::math::fpu::QuatPosTemplate<FloatType>              QuatPos;
    typedef ::rw::math::fpu::Matrix33Template<FloatType>             Matrix33;
    typedef ::rw::math::fpu::Matrix44Template<FloatType>             Matrix44;
    typedef ::rw::math::fpu::PlaneTemplate<FloatType>                Plane;
    typedef ::rw::math::fpu::Vector3PlusTemplate<FloatType>          Vector3Plus;
    typedef ::rw::math::fpu::Matrix44AffineTemplate<FloatType>       Matrix44Affine;
    typedef ::rw::math::fpu::Matrix44AffinePackedTemplate<FloatType> Matrix44AffinePacked;
    typedef ::rw::math::fpu::SqtTemplate<FloatType>                  Sqt;

    MaskScalar GetMaskScalar_True()
    {
        return ::rw::math::fpu::GetMaskScalar_TrueTemplate<FloatType>();
    }

    Mask2 GetMask2_True()
    {
        return ::rw::math::fpu::GetMask2_TrueTemplate<FloatType>();
    }

    Mask3 GetMask3_True()
    {
        return ::rw::math::fpu::GetMask3_TrueTemplate<FloatType>();
    }

    Mask4 GetMask4_True()
    {
        return ::rw::math::fpu::GetMask4_TrueTemplate<FloatType>();
    }

    MaskScalar GetMaskScalar_False()
    {
        return ::rw::math::fpu::GetMaskScalar_FalseTemplate<FloatType>();
    }

    Mask2 GetMask2_False()
    {
        return ::rw::math::fpu::GetMask2_FalseTemplate<FloatType>();
    }

    Mask3 GetMask3_False()
    {
        return ::rw::math::fpu::GetMask3_FalseTemplate<FloatType>();
    }

    Mask4 GetMask4_False()
    {
        return ::rw::math::fpu::GetMask4_FalseTemplate<FloatType>();
    }

    FloatType GetVecFloat_NegativeTwo()
    {
        return ::rw::math::fpu::GetVecFloat_NegativeTwoTemplate<FloatType>();
    }

    FloatType GetVecFloat_NegativeOne()
    {
        return ::rw::math::fpu::GetVecFloat_NegativeOneTemplate<FloatType>();
    }

    FloatType GetVecFloat_NegativeHalf()
    {
        return ::rw::math::fpu::GetVecFloat_NegativeHalfTemplate<FloatType>();
    }

    FloatType GetVecFloat_NegativeZero()
    {
        return ::rw::math::fpu::GetVecFloat_NegativeZeroTemplate<FloatType>();
    }

    FloatType GetVecFloat_Zero()
    {
        return ::rw::math::fpu::GetVecFloat_ZeroTemplate<FloatType>();
    }

    FloatType GetVecFloat_Half()
    {
        return ::rw::math::fpu::GetVecFloat_HalfTemplate<FloatType>();
    }

    FloatType GetVecFloat_One()
    {
        return ::rw::math::fpu::GetVecFloat_OneTemplate<FloatType>();
    }

    FloatType GetVecFloat_Two()
    {
        return ::rw::math::fpu::GetVecFloat_TwoTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector2Template<FloatType> GetVector2_Zero()
    {
        return ::rw::math::fpu::GetVector2_ZeroTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector2Template<FloatType> GetVector2_One()
    {
        return ::rw::math::fpu::GetVector2_OneTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector3Template<FloatType> GetVector3_Zero()
    {
        return ::rw::math::fpu::GetVector3_ZeroTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector3Template<FloatType> GetVector3_One()
    {
        return ::rw::math::fpu::GetVector3_OneTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector3Template<FloatType> GetVector3_XAxis()
    {
        return ::rw::math::fpu::GetVector3_XAxisTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector3Template<FloatType> GetVector3_YAxis()
    {
        return ::rw::math::fpu::GetVector3_YAxisTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector3Template<FloatType> GetVector3_ZAxis()
    {
        return ::rw::math::fpu::GetVector3_ZAxisTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector4Template<FloatType> GetVector4_Zero()
    {
        return ::rw::math::fpu::GetVector4_ZeroTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector4Template<FloatType> GetVector4_One()
    {
        return ::rw::math::fpu::GetVector4_OneTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector4Template<FloatType> GetVector4_Origin()
    {
        return ::rw::math::fpu::GetVector4_OriginTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector4Template<FloatType> GetVector4_XAxis()
    {
        return ::rw::math::fpu::GetVector4_XAxisTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector4Template<FloatType> GetVector4_YAxis()
    {
        return ::rw::math::fpu::GetVector4_YAxisTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector4Template<FloatType> GetVector4_ZAxis()
    {
        return ::rw::math::fpu::GetVector4_ZAxisTemplate<FloatType>();
    }

    ::rw::math::fpu::Vector4Template<FloatType> GetVector4_WAxis()
    {
        return ::rw::math::fpu::GetVector4_WAxisTemplate<FloatType>();
    }

    ::rw::math::fpu::QuaternionTemplate<FloatType> GetQuaternion_Zero()
    {
        return ::rw::math::fpu::GetQuaternion_ZeroTemplate<FloatType>();
    }

    ::rw::math::fpu::QuaternionTemplate<FloatType> GetQuaternion_One()
    {
        return ::rw::math::fpu::GetQuaternion_OneTemplate<FloatType>();
    }

    ::rw::math::fpu::QuaternionTemplate<FloatType> GetQuaternion_Identity()
    {
        return ::rw::math::fpu::GetQuaternion_IdentityTemplate<FloatType>();
    }

    ::rw::math::fpu::QuatPosTemplate<FloatType> GetQuatPos_Zero()
    {
        return ::rw::math::fpu::GetQuatPos_ZeroTemplate<FloatType>();
    }

    ::rw::math::fpu::QuatPosTemplate<FloatType> GetQuatPos_Identity()
    {
        return ::rw::math::fpu::GetQuatPos_IdentityTemplate<FloatType>();
    }

    ::rw::math::fpu::Matrix33Template<FloatType> GetMatrix33_Identity()
    {
        return ::rw::math::fpu::GetMatrix33_IdentityTemplate<FloatType>();
    }

    ::rw::math::fpu::Matrix33Template<FloatType> GetMatrix33_Zero()
    {
        return ::rw::math::fpu::GetMatrix33_ZeroTemplate<FloatType>();
    }

    ::rw::math::fpu::Matrix44Template<FloatType> GetMatrix44_Zero()
    {
        return ::rw::math::fpu::GetMatrix44_ZeroTemplate<FloatType>();
    }

    ::rw::math::fpu::Matrix44AffineTemplate<FloatType> GetMatrix44Affine_Zero()
    {
        return ::rw::math::fpu::GetMatrix44Affine_ZeroTemplate<FloatType>();
    }

    ::rw::math::fpu::Matrix44AffinePackedTemplate<FloatType> GetMatrix44AffinePacked_Zero()
    {
        return ::rw::math::fpu::GetMatrix44AffinePacked_ZeroTemplate<FloatType>();
    }

    ::rw::math::fpu::Matrix44Template<FloatType> GetMatrix44_Identity()
    {
        return ::rw::math::fpu::GetMatrix44_IdentityTemplate<FloatType>();
    }

    ::rw::math::fpu::Matrix44AffineTemplate<FloatType> GetMatrix44Affine_Identity()
    {
        return ::rw::math::fpu::GetMatrix44Affine_IdentityTemplate<FloatType>();
    }

    ::rw::math::fpu::Matrix44AffinePackedTemplate<FloatType> GetMatrix44AffinePacked_Identity()
    {
        return ::rw::math::fpu::GetMatrix44AffinePacked_IdentityTemplate<FloatType>();
    }

    typedef FloatType float_test_type;
    typedef FloatType float_basic_type;

    #define EXPLICIT_NAMESPACE(class_name) ::rw::math::fpu::class_name ## Template<FloatType>
#endif


#ifdef HOST_TESTING

    typedef ::rw::math::vpu::VecFloat float_test_type;
    typedef float float_basic_type;

    #define EXPLICIT_NAMESPACE(class_name) ::rw::math::vpu::class_name
#endif
