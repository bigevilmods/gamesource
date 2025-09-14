// (c) Electronic Arts. All Rights Reserved.

#include "rw/math/detail/base.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"
#include "rw/math/vpu/detail/vector_intrinsic_operation_macros.h"

#include "rw/math/fpu/vector2.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vector3plus.h"
#include "rw/math/fpu/vector4.h"
#include "rw/math/fpu/quaternion.h"
#include "rw/math/fpu/plane.h"
#include "rw/math/fpu/matrix33.h"
#include "rw/math/fpu/matrix44.h"
#include "rw/math/fpu/matrix44affine.h"
#include "rw/math/fpu/matrix44affinepacked.h"

#include "rw/math/vpu/matrix33.h"
#include "rw/math/vpu/matrix44.h"
#include "rw/math/vpu/matrix44affine.h"
#include "rw/math/vpu/matrix44affinepacked.h"

#if defined EA_PLATFORM_IPHONE_SIMULATOR
    static inline rw::math::vpu::VectorIntrinsic CreateFloatVector(float x, float y, float z, float w)
    {
            rw::math::vpu::detail::VectorIntrinsicUnion v;
            v.scalarFloat[0] = x;
            v.scalarFloat[1] = y;
            v.scalarFloat[2] = z;
            v.scalarFloat[3] = w;
            return v.vector;
    }
#elif defined(EA_COMPILER_INTEL) && defined(RW_MATH_TARGET_SUPPORTS_SSE)
    static inline rw::math::vpu::VectorIntrinsic CreateFloatVector(float x, float y, float z, float w)
    {
        return _mm_set_ps(z, y, x, w);
    }
#else
    #define CreateFloatVector(a, b, c, d) { a, b, c, d }
#endif

namespace rw {
namespace math {

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
namespace fpu
{
    template<> float Vector2Template<float>::*  const Vector2Template<float>::sElements[3] = { &Vector2Template<float>::x, &Vector2Template<float>::y };
    template<> double Vector2Template<double>::*  const Vector2Template<double>::sElements[3] = { &Vector2Template<double>::x, &Vector2Template<double>::y };

    template<> float Vector3Template<float>::*  const Vector3Template<float>::sElements[] = { &Vector3Template<float>::x, &Vector3Template<float>::y, &Vector3Template<float>::z };
    template<> double Vector3Template<double>::*  const Vector3Template<double>::sElements[] = { &Vector3Template<double>::x, &Vector3Template<double>::y, &Vector3Template<double>::z };

    template<> float Vector3PlusTemplate<float>::*  const Vector3PlusTemplate<float>::sElements[] = { &Vector3PlusTemplate<float>::x, &Vector3PlusTemplate<float>::y, &Vector3PlusTemplate<float>::z, &Vector3PlusTemplate<float>::userData };
    template<> double Vector3PlusTemplate<double>::*  const Vector3PlusTemplate<double>::sElements[] = { &Vector3PlusTemplate<double>::x, &Vector3PlusTemplate<double>::y, &Vector3PlusTemplate<double>::z, &Vector3PlusTemplate<double>::userData };

    template<> float Vector4Template<float>::*  const Vector4Template<float>::sElements[] = { &Vector4Template<float>::x, &Vector4Template<float>::y, &Vector4Template<float>::z, &Vector4Template<float>::w };
    template<> double Vector4Template<double>::*  const Vector4Template<double>::sElements[] = { &Vector4Template<double>::x, &Vector4Template<double>::y, &Vector4Template<double>::z, &Vector4Template<double>::w };

    template<> float QuaternionTemplate<float>::*  const QuaternionTemplate<float>::sElements[] = { &QuaternionTemplate<float>::x, &QuaternionTemplate<float>::y, &QuaternionTemplate<float>::z, &QuaternionTemplate<float>::w };
    template<> double QuaternionTemplate<double>::*  const QuaternionTemplate<double>::sElements[] = { &QuaternionTemplate<double>::x, &QuaternionTemplate<double>::y, &QuaternionTemplate<double>::z, &QuaternionTemplate<double>::w };

    template<> float PlaneTemplate<float>::*  const PlaneTemplate<float>::sElements[] = { &PlaneTemplate<float>::a, &PlaneTemplate<float>::b, &PlaneTemplate<float>::c, &PlaneTemplate<float>::d };
    template<> double PlaneTemplate<double>::*  const PlaneTemplate<double>::sElements[] = { &PlaneTemplate<double>::a, &PlaneTemplate<double>::b, &PlaneTemplate<double>::c, &PlaneTemplate<double>::d };

    template<> Vector3Template<float> Matrix33Template<float>::* const Matrix33Template<float>::sElements[] = { &Matrix33Template<float>::xAxis, &Matrix33Template<float>::yAxis, &Matrix33Template<float>::zAxis };
    template<> Vector3Template<double> Matrix33Template<double>::* const Matrix33Template<double>::sElements[] = { &Matrix33Template<double>::xAxis, &Matrix33Template<double>::yAxis, &Matrix33Template<double>::zAxis };

    template<> Vector4Template<float> Matrix44Template<float>::* const Matrix44Template<float>::sElements[] = { &Matrix44Template<float>::xAxis, &Matrix44Template<float>::yAxis, &Matrix44Template<float>::zAxis, &Matrix44Template<float>::wAxis };
    template<> Vector4Template<double> Matrix44Template<double>::* const Matrix44Template<double>::sElements[] = { &Matrix44Template<double>::xAxis, &Matrix44Template<double>::yAxis, &Matrix44Template<double>::zAxis, &Matrix44Template<double>::wAxis };

    template<> Vector3Template<float> Matrix44AffineTemplate<float>::* const Matrix44AffineTemplate<float>::sElements[] = { &Matrix44AffineTemplate<float>::xAxis, &Matrix44AffineTemplate<float>::yAxis, &Matrix44AffineTemplate<float>::zAxis, &Matrix44AffineTemplate<float>::wAxis };
    template<> Vector3Template<double> Matrix44AffineTemplate<double>::* const Matrix44AffineTemplate<double>::sElements[] = { &Matrix44AffineTemplate<double>::xAxis, &Matrix44AffineTemplate<double>::yAxis, &Matrix44AffineTemplate<double>::zAxis, &Matrix44AffineTemplate<double>::wAxis };

    template<> Vector4Template<float> Matrix44AffinePackedTemplate<float>::* const Matrix44AffinePackedTemplate<float>::sElements[] = { &Matrix44AffinePackedTemplate<float>::xAxis, &Matrix44AffinePackedTemplate<float>::yAxis, &Matrix44AffinePackedTemplate<float>::zAxis };
    template<> Vector4Template<double> Matrix44AffinePackedTemplate<double>::* const Matrix44AffinePackedTemplate<double>::sElements[] = { &Matrix44AffinePackedTemplate<double>::xAxis, &Matrix44AffinePackedTemplate<double>::yAxis, &Matrix44AffinePackedTemplate<double>::zAxis };
}
#endif

namespace vpu
{

#ifdef RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC
    Vector3 Matrix33::* const Matrix33::sElements[] = { &Matrix33::xAxis, &Matrix33::yAxis, &Matrix33::zAxis };
    Vector4 Matrix44::* const Matrix44::sElements[] = { &Matrix44::xAxis, &Matrix44::yAxis, &Matrix44::zAxis, &Matrix44::wAxis };
    Vector3 Matrix44Affine::* const Matrix44Affine::sElements[] = { &Matrix44Affine::xAxis, &Matrix44Affine::yAxis, &Matrix44Affine::zAxis, &Matrix44Affine::wAxis };
    Vector4 Matrix44AffinePacked::* const Matrix44AffinePacked::sElements[] = { &Matrix44AffinePacked::xAxis, &Matrix44AffinePacked::yAxis, &Matrix44AffinePacked::zAxis };
#endif

namespace detail
{

static VectorIntrinsic CreateIntegerVector(unsigned int x, unsigned int y, unsigned int z, unsigned int w)
{
        VectorIntrinsicUnion r;
        r.scalarUInt[0] = x;
        r.scalarUInt[1] = y;
        r.scalarUInt[2] = z;
        r.scalarUInt[3] = w;
        return r.vector;
}

#ifdef _MSC_VER
    // To minimize problems where user code may attempt to use math constants (indirectly) before
    // they have been initialized, all math constants are constructed in the "lib" segment when
    // building with the MSC compiler.
    #pragma warning(disable: 4073) // warning C4073: initializers put in library initialization area
    #pragma init_seg(lib)    
#endif

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION || (defined EA_PLATFORM_POSIX && !defined(RW_MATH_TARGET_SUPPORTS_SSE) && !defined(RWMATH_USE_NEON)) || defined(EA_PLATFORM_AIRPLAY)
    const VectorIntrinsic gCosMinSlerpAngle = {{{ 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f }}};
#elif defined EA_PROCESSOR_ARM
    #ifdef RWMATH_USE_NEON
        const VectorIntrinsic gCosMinSlerpAngle = (float32x4_t){ 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f };
    #else
        const VectorIntrinsic gCosMinSlerpAngle = {{{ 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f }}};
    #endif
#elif defined RWCROSS
    const VectorIntrinsic gCosMinSlerpAngle = { 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f };
#elif defined EA_PLATFORM_XENON
    const VectorIntrinsic gQuatFromMat_xSigns = CreateIntegerVector( 0x00000000, 0x00000000, 0x80000000, 0x80000000 );
    const VectorIntrinsic gQuatFromMat_ySigns = CreateIntegerVector( 0x00000000, 0x80000000, 0x00000000, 0x80000000 );
    const VectorIntrinsic gQuatFromMat_zSigns = CreateIntegerVector( 0x00000000, 0x80000000, 0x80000000, 0x00000000 );
    const VectorIntrinsic gXYZMask  = CreateIntegerVector( 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 );
    // This permute constant effectively concatenates the most significant byte
    // of each word in a vector.  Then the 32 bit result is splatted across a
    // vector.
    const VectorIntrinsic gGatherMostSignificantBytesV4 = CreateIntegerVector( 0x0004080c, 0x0004080c, 0x0004080c, 0x0004080c );
    // This permute constant effectively concatenates the most significant byte
    // of the first, second and third element of the vector.  The third element
    // is actually concatenated twice to create a full 32bit mask.
    const VectorIntrinsic gGatherMostSignificantBytesV3 = CreateIntegerVector( 0x00040808, 0x00040808, 0x00040808, 0x00040808 );
    const VectorIntrinsic gPI = {3.14159265358979323846f, 3.14159265358979323846f, 3.14159265358979323846f, 3.14159265358979323846f};

    // This function creates a permution constant given 4 source indices.
    //
    // Note: This function should be unnecessary.  The
    // GET_VECTOR_PERMUTE_CONSTANT_I macro _should_ be able to create global constants.
    // Unfortunately, sometimes the Xenon compiler doesn't generate the
    // constant properly (i.e. all zeros are generated) so this function was
    // implemented as a workaround.
    static VectorIntrinsic CreatePermuteConstant( VectorPermSource xSource, VectorPermSource ySource, VectorPermSource zSource, VectorPermSource wSource )
    {
        VectorIntrinsicUnion result;
        result.scalarUInt[0] = _asmCreatePermuteInt32( xSource );
        result.scalarUInt[1] = _asmCreatePermuteInt32( ySource );
        result.scalarUInt[2] = _asmCreatePermuteInt32( zSource );
        result.scalarUInt[3] = _asmCreatePermuteInt32( wSource );
        return result.vector;
    }

    // sqrt(2.0f)
    const VectorIntrinsic gSqrt2s = { 1.41421356237f, 1.41421356237f, 1.41421356237f, 1.41421356237f };
    const VectorIntrinsic gIVector = { 1.0f, 0.0f, 0.0f, 0.0f };
    const VectorIntrinsic gJVector = { 0.0f, 1.0f, 0.0f, 0.0f };
    const VectorIntrinsic gKVector = { 0.0f, 0.0f, 1.0f, 0.0f };
    const VectorIntrinsic gLVector = { 0.0f, 0.0f, 0.0f, 1.0f };
    const VectorIntrinsic g2PI = { 6.28318530717958f, 6.28318530717958f, 6.28318530717958f, 6.28318530717958f };
    const VectorIntrinsic g2PIReciprocal = { 0.1591549430918953f, 0.1591549430918953f, 0.1591549430918953f, 0.1591549430918953f };
    const VectorIntrinsic gLn2 = { 0.6931471805599453f, 0.6931471805599453f, 0.6931471805599453f, 0.6931471805599453f };
    const VectorIntrinsic gLn2Reciprocal = { 1.4426950408889634f, 1.4426950408889634f, 1.4426950408889634f, 1.4426950408889634f };

    const VectorIntrinsic gSwizzleStoreConstants[NUM_SWIZZLE_STORE_CONSTANTS] =
    {
        // dest = X, src = X
        CreatePermuteConstant(X1, Y0, Z0, W0),

        // dest = X, src = Y
        CreatePermuteConstant(Y1, Y0, Z0, W0),

        // dest = X, src = Z
        CreatePermuteConstant(Z1, Y0, Z0, W0),

        // dest = X, src = W
        CreatePermuteConstant(W1, Y0, Z0, W0),

        // dest = Y, src = X
        CreatePermuteConstant(X0, X1, Z0, W0),

        // dest = Y, src = Y
        CreatePermuteConstant(X0, Y1, Z0, W0),

        // dest = Y, src = Z
        CreatePermuteConstant(X0, Z1, Z0, W0),

        // dest = Y, src = W
        CreatePermuteConstant(X0, W1, Z0, W0),

        // dest = Z, src = X
        CreatePermuteConstant(X0, Y0, X1, W0),

        // dest = Z, src = Y
        CreatePermuteConstant(X0, Y0, Y1, W0),

        // dest = Z, src = Z
        CreatePermuteConstant(X0, Y0, Z1, W0),

        // dest = Z, src = W
        CreatePermuteConstant(X0, Y0, W1, W0),

        // dest = W, src = X
        CreatePermuteConstant(X0, Y0, Z0, X1),

        // dest = W, src = Y
        CreatePermuteConstant(X0, Y0, Z0, Y1),

        // dest = W, src = Z
        CreatePermuteConstant(X0, Y0, Z0, Z1),

        // dest = W, src = W
        CreatePermuteConstant(X0, Y0, Z0, W1),
    };

    const VectorIntrinsic gBooleanConstants[2] =
    {
        CreateIntegerVector( 0x00000000, 0x00000000, 0x00000000, 0x00000000 ),
        CreateIntegerVector( 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff )
    };

    const VectorIntrinsic gCosMinSlerpAngle = { 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f };

    // --------------------------------------------------
#elif defined EA_PLATFORM_PS3
    const VectorIntrinsic gSqrt2s = { 1.41421356237f, 1.41421356237f, 1.41421356237f, 1.41421356237f };
    const VectorIntrinsic gIVector = { 1.0f, 0.0f, 0.0f, 0.0f };
    const VectorIntrinsic gJVector = { 0.0f, 1.0f, 0.0f, 0.0f };
    const VectorIntrinsic gKVector = { 0.0f, 0.0f, 1.0f, 0.0f };
    const VectorIntrinsic gLVector = { 0.0f, 0.0f, 0.0f, 1.0f };
    const VectorIntrinsic gHalfPI   = { 1.57079632679489661923f, 1.57079632679489661923f, 1.57079632679489661923f, 1.57079632679489661923f };
    const VectorIntrinsic gPI       = { 3.14159265358979323846f, 3.14159265358979323846f, 3.14159265358979323846f, 3.14159265358979323846f };
    const VectorIntrinsic g2PI      = { 6.28318530717958f, 6.28318530717958f, 6.28318530717958f, 6.28318530717958f };
    const VectorIntrinsic g2PIReciprocal = { 0.1591549430918953f, 0.1591549430918953f, 0.1591549430918953f, 0.1591549430918953f };
    const VectorIntrinsic gCosMinSlerpAngle = { 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f };
    // --------------------------------------------------
#elif defined EA_PLATFORM_PS3_SPU
    // sqrt(2.0f)
    const VectorIntrinsic gSqrt2s = { 1.41421356237f, 1.41421356237f, 1.41421356237f, 1.41421356237f };
    const VectorIntrinsic gIVector = { 1.0f, 0.0f, 0.0f, 0.0f };
    const VectorIntrinsic gJVector = { 0.0f, 1.0f, 0.0f, 0.0f };
    const VectorIntrinsic gKVector = { 0.0f, 0.0f, 1.0f, 0.0f };
    const VectorIntrinsic gLVector = { 0.0f, 0.0f, 0.0f, 1.0f };
    const VectorIntrinsic g2PI      = { 6.28318530717958f, 6.28318530717958f, 6.28318530717958f, 6.28318530717958f };
    const VectorIntrinsic g2PIReciprocal = { 0.1591549430918953f, 0.1591549430918953f, 0.1591549430918953f, 0.1591549430918953f };

    void ForceVectorsToMemory(VectorIntrinsic *)
    {
        // This function doesn't need to do anything.  Calling this function
        // will force all VectorIntrinsic's to be written to memory.
        // Note: In theory, it should never be necessary to call this function.
        // If calling this function solves a bug, make sure you haven't
        // introduced an aliasing problem by accessing data through an
        // incompatible pointer type.
        //
        // This function was added to solve an issue with the SPU compiler
        // that is believed to be caused by an error in the compiler, and
        // not an aliasing bug in the math library.
    }

    const VectorIntrinsic gCosMinSlerpAngle = { 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f };

#elif defined RW_MATH_TARGET_SUPPORTS_SSE
    const VectorIntrinsic gXYZMask  = CreateIntegerVector( 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 );
    const VectorIntrinsic gXYZWMask = CreateIntegerVector( 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff );

    const VectorIntrinsic gNegOneOverTwoPi = CreateFloatVector(-0.15915493667125702, -0.15915493667125702, -0.15915493667125702, -0.15915493667125702);
    const VectorIntrinsic gRemoveFraction  = CreateFloatVector(12582912.f, 12582912.f, 12582912.f, 12582912.f);
    const VectorIntrinsic gOneHalf         = CreateFloatVector(0.5f, 0.5f, 0.5f, 0.5f);
    const VectorIntrinsic gOneQuarter      = CreateFloatVector(0.25f, 0.25f, 0.25f, 0.25f);
    const VectorIntrinsic gOneHalfPi       = CreateFloatVector(1.5707963705062866, 1.5707963705062866, 1.5707963705062866, 1.5707963705062866);

    const VectorIntrinsic gTaylorCosXCoefficient = CreateFloatVector(6.2831854820251465, 6.2831854820251465, 6.2831854820251465, 6.2831854820251465);
    const VectorIntrinsic gTaylorCosX3Coefficient = CreateFloatVector(-41.34170150756836, -41.34170150756836, -41.34170150756836, -41.34170150756836);
    const VectorIntrinsic gTaylorCosX5Coefficient = CreateFloatVector(81.6052474975586, 81.6052474975586, 81.6052474975586, 81.6052474975586);
    const VectorIntrinsic gTaylorCosX7Coefficient = CreateFloatVector(-76.70585632324219, -76.70585632324219, -76.70585632324219, -76.70585632324219);
    const VectorIntrinsic gTaylorCosX9Coefficient = CreateFloatVector(42.058692932128906, 42.058692932128906, 42.058692932128906, 42.058692932128906);
    const VectorIntrinsic gTaylorCosX11Coefficient = CreateFloatVector(-15.094642639160156, -15.094642639160156, -15.094642639160156, -15.094642639160156);
    const VectorIntrinsic gTaylorCosX13Coefficient = CreateFloatVector(3.819952584848282f, 3.819952584848282f, 3.819952584848282f, 3.819952584848282f);
    const VectorIntrinsic gTaylorCosX15Coefficient = CreateFloatVector(-0.718122301778500f, -0.718122301778500f, -0.718122301778500f, -0.718122301778500f);
    const VectorIntrinsic gTaylorCosX17Coefficient = CreateFloatVector(0.104229162208139f, 0.104229162208139f, 0.104229162208139f, 0.104229162208139f);
    const VectorIntrinsic gTaylorCosX19Coefficient = CreateFloatVector(-0.012031585942120f, -0.012031585942120f, -0.012031585942120f, -0.012031585942120f);

    const VectorIntrinsic gCosMinSlerpAngle = CreateFloatVector(0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f, 0.99619469809174553229f);

#elif defined(EA_PLATFORM_REVOLUTION)

// 8/11/2010 JDF no constants on this platform.
// Unsure whether this is intentional, but I don't want to add one in that
// didn't have it before the Airplay changes were made, so I'm leaving them
// out.

#else

    #error Unsupported platform

#endif

} // detail
} // vpu

namespace vpl {
    namespace detail {

    extern const VectorIntrinsic vecFloatHalfConversionConstants[3];
    const VectorIntrinsic vecFloatHalfConversionConstants[3] = 
    {
        ::rw::math::vpu::detail::CreateIntegerVector(0xffff8000, 0x00007c00, 0x7f800000, (unsigned int)(-127 + 15)),
        ::rw::math::vpu::detail::CreateIntegerVector(0x80000000,0x7f800000, 0x00007c00, (unsigned int)(-15 + 127)),
        ::rw::math::vpu::detail::CreateIntegerVector(0, 0, 0, 0)
    };

#if defined RW_MATH_TARGET_SUPPORTS_SSE

    const VectorIntrinsic vecTwo31Float = CreateFloatVector(2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f);
            const VectorIntrinsic vecNegTwo31Float = CreateFloatVector(-2147483648.0f, -2147483648.0f, -2147483648.0f, -2147483648.0f);
            const VectorIntrinsic vecTwo32Float = CreateFloatVector(4294967295.0f, 4294967295.0f, 4294967295.0f, 4294967295.0f);
            const VectorIntrinsic vecZero = CreateFloatVector(0.0f, 0.0f, 0.0f, 0.0f);
            const VectorIntrinsic vecOne = CreateFloatVector( 1.0f, 1.0f, 1.0f, 1.0f );
            const VectorIntrinsic maxHalfFloatFromUnsignedInteger = CreateFloatVector(65536.0f, 65536.0f, 65536.0f, 65536.0f);
            const VectorIntrinsic vecNegNaN = CreateFloatVector(-2.5521178e38f, -2.5521178e38f, -2.5521178e38f, -2.5521178e38f);
            const VectorIntrinsic vecPosNaN = CreateFloatVector(2.5521178e38f, 2.5521178e38f, 2.5521178e38f, 2.5521178e38f);
            const FourUnsignedInt sixtyFourUInt = {0x00000040, 0x00000040, 0x00000040, 0x00000040};
            const VectorIntrinsic ScaleExponentArray[32] = 
            {
                CreateFloatVector(1.0f, 1.0f, 1.0f, 1.0f), CreateFloatVector(2.0f, 2.0f, 2.0f, 2.0f), CreateFloatVector(4.0f, 4.0f, 4.0f, 4.0f), CreateFloatVector(8.0f, 8.0f, 8.0f, 8.0f),
                CreateFloatVector(16.0f, 16.0f, 16.0f, 16.0f), CreateFloatVector(32.0f, 32.0f, 32.0f, 32.0f), CreateFloatVector(64.0f, 64.0f, 64.0f, 64.0f), CreateFloatVector(128.0f, 128.0f, 128.0f, 128.0f),
                CreateFloatVector(256.0f, 256.0f, 256.0f, 256.0f), CreateFloatVector(512.0f, 512.0f, 512.0f, 512.0f), CreateFloatVector(1024.0f, 1024.0f, 1024.0f, 1024.0f), CreateFloatVector(2048.0f, 2048.0f, 2048.0f, 2048.0f),
                CreateFloatVector(4096.0f, 4096.0f, 4096.0f, 4096.0f), CreateFloatVector(8192.0f, 8192.0f, 8192.0f, 8192.0f), CreateFloatVector(16384.0f, 16384.0f, 16384.0f, 16384.0f), CreateFloatVector(32768.0f, 32768.0f, 32768.0f, 32768.0f),
                CreateFloatVector(65536.0f, 65536.0f, 65536.0f, 65536.0f), CreateFloatVector(131072.0f, 131072.0f, 131072.0f, 131072.0f), CreateFloatVector(262144.0f, 262144.0f, 262144.0f, 262144.0f), CreateFloatVector(524288.0f, 524288.0f, 524288.0f, 524288.0f),
                CreateFloatVector(1048576.0f, 1048576.0f, 1048576.0f, 1048576.0f), CreateFloatVector(2097152.0f, 2097152.0f, 2097152.0f, 2097152.0f), CreateFloatVector(4194304.0f, 4194304.0f, 4194304.0f, 4194304.0f), CreateFloatVector(8388608.0f, 8388608.0f, 8388608.0f, 8388608.0f),
                CreateFloatVector(16777216.0f, 16777216.0f, 16777216.0f, 16777216.0f), CreateFloatVector(33554432.0f, 33554432.0f, 33554432.0f, 33554432.0f), CreateFloatVector(67108864.0f, 67108864.0f, 67108864.0f, 67108864.0f), CreateFloatVector(134217728.0f, 134217728.0f, 134217728.0f, 134217728.0f),
                CreateFloatVector(268435456.0f, 268435456.0f, 268435456.0f, 268435456.0f), CreateFloatVector(536870912.0f, 536870912.0f, 536870912.0f, 536870912.0f), CreateFloatVector(1073741824.0f, 1073741824.0f, 1073741824.0f, 1073741824.0f), CreateFloatVector(2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f)
            };
            const FourUnsignedInt InsertionMaskArray[16] =
            {
                {0x00000000, 0x00000000, 0x00000000, 0x00000000}, {0x00000000, 0x00000000, 0x00000000, 0xffffffff}, {0x00000000, 0x00000000, 0xffffffff, 0x00000000}, {0x00000000, 0x00000000, 0xffffffff, 0xffffffff},
                {0x00000000, 0xffffffff, 0x00000000, 0x00000000}, {0x00000000, 0xffffffff, 0x00000000, 0xffffffff}, {0x00000000, 0xffffffff, 0xffffffff, 0x00000000}, {0x00000000, 0xffffffff, 0xffffffff, 0xffffffff},
                {0xffffffff, 0x00000000, 0x00000000, 0x00000000}, {0xffffffff, 0x00000000, 0x00000000, 0xffffffff}, {0xffffffff, 0x00000000, 0xffffffff, 0x00000000}, {0xffffffff, 0x00000000, 0xffffffff, 0xffffffff},
                {0xffffffff, 0xffffffff, 0x00000000, 0x00000000}, {0xffffffff, 0xffffffff, 0x00000000, 0xffffffff}, {0xffffffff, 0xffffffff, 0xffffffff, 0x00000000}, {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff}
            };

#endif

#ifdef RWMATH_USE_NEON
       
        const VectorIntrinsic gShiftLeftArray[16] =
        {
            ::rw::math::vpu::detail::CreateIntegerVector(0x03020100, 0x07060504, 0x0B0A0908, 0x0F0E0D0C),
            ::rw::math::vpu::detail::CreateIntegerVector(0x04030201, 0x08070605, 0x0C0B0A09, 0xFF0F0E0D),
            ::rw::math::vpu::detail::CreateIntegerVector(0x05040302, 0x09080706, 0x0D0C0B0A, 0xFFFF0F0E),
            ::rw::math::vpu::detail::CreateIntegerVector(0x06050403, 0x07090807, 0x0E0D0C0B, 0x0FFFFF0F),
            ::rw::math::vpu::detail::CreateIntegerVector(0x07060504, 0x080A0908, 0x0F0E0D0C, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0x08070605, 0x090B0A09, 0xFF0F0E0D, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0x09080706, 0x0D0C0B0A, 0xFFFF0F0E, 0xFFFFFFFF),
            
            ::rw::math::vpu::detail::CreateIntegerVector(0x0A090807, 0x0E0D0C0B, 0xFFFFFF0F, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0x0B0A0908, 0x0F0E0D0C, 0xFFFFFFFF, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0x0C0B0A09, 0xFF0F0E0D, 0xFFFFFFFF, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0x0D0C0B0A, 0xFFFF0F0E, 0xFFFFFFFF, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0x0E0D0C0B, 0x0FFFFF0F, 0xFFFFFFFF, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0x0F0E0D0C, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFF0F0E0D, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFF0F0E, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFF0F, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)
        };

		const VectorIntrinsic gShiftRightArray[16] =
        {
            ::rw::math::vpu::detail::CreateIntegerVector(0x03020100, 0x07060504, 0x0B0A0908, 0x0F0E0D0C),
            ::rw::math::vpu::detail::CreateIntegerVector(0x020100FF, 0x06050403, 0x0A090807, 0x0E0D0C0B),
            ::rw::math::vpu::detail::CreateIntegerVector(0x0100FFFF, 0x05040302, 0x09080706, 0x0D0C0B0A),
            ::rw::math::vpu::detail::CreateIntegerVector(0x00FFFFFF, 0x04030201, 0x08070605, 0x0C0B0A09),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0x03020100, 0x07060504, 0x0B0A0908),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0x020100FF, 0x06050403, 0x0A090807),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0x0100FFFF, 0x05040302, 0x09080706),
            
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0x00FFFFFF, 0x04030201, 0x08070605),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0xFFFFFFFF, 0x03020100, 0x07060504),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0xFFFFFFFF, 0x020100FF, 0x06050403),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0xFFFFFFFF, 0x0100FFFF, 0x05040302),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0xFFFFFFFF, 0x00FFFFFF, 0x04030201),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x03020100),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x020100FF),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0100FFFF),
            ::rw::math::vpu::detail::CreateIntegerVector(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00FFFFFF)
        };
#endif

} // detail
} // vpl
} // math
} // rw
