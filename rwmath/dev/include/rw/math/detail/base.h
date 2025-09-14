// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_DETAIL_BASE_H
#define RW_MATH_DETAIL_BASE_H

#if defined _MSC_VER
    #pragma once
#endif

// Don't include any platform specific headers when parsing the API
#ifndef RW_MATH_TAG_API_DURING_PREPROCESSING

#if defined(__SNC__)
    #pragma control %push diag=1
#endif

#include "EABase/eabase.h"

// TODO remove this logic once the trailing edge has moved passed EABase 2.0.24
#if EABASE_VERSION_N < 20024
    #ifdef EA_PLATFORM_UNIX
        #define EA_PLATFORM_POSIX
    #endif
#endif

#if defined(__SNC__)
    #pragma control %pop diag
#endif

#include <math.h>
#include "rw/math/version.h"

// For the time being, the math library ships with deprecated parts of the API enabled
#ifndef RW_MATH_ENABLE_DEPRECATED_API
    //! This controls whether or not the deprecated parts of the API are 
    //! exposed. Define this to zero in your code and the deprecated parts of 
    //! the API will be disabled
    #define RW_MATH_ENABLE_DEPRECATED_API 1
#endif

#if RW_MATH_ENABLE_DEPRECATED_API
    #define RW_MATH_FUTURE_EXPLICIT
#else
    #define RW_MATH_FUTURE_EXPLICIT explicit
#endif

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
#elif defined EA_PLATFORM_PS3
    #define RW_MATH_VECTOR_PASS_BY_VALUE
    #define RW_MATH_MATRIX_PASS_BY_VALUE

    #include <sdk_version.h>
    #include "rw/math/vpu/detail/ps3/ppu/vmx_macros.h"
    
#elif defined EA_PLATFORM_PS3_SPU
    #define RW_MATH_VECTOR_PASS_BY_VALUE
    #define RW_MATH_MATRIX_PASS_BY_VALUE
    #include <limits.h>
    #include "rw/math/detail/ps3/spu/spu_macros.h"    
    
#elif defined EA_PLATFORM_PSP2
    #define RW_MATH_VECTOR_PASS_BY_VALUE
    #define RW_MATH_MATRIX_PASS_BY_VALUE

#elif defined EA_PLATFORM_XENON
    #include <ppcintrinsics.h>
    #include <vectorintrinsics.h>
    #include "rw/math/vpu/detail/xbox2/vmx_macros.h"
#elif defined EA_PLATFORM_REVOLUTION
    #include <stddef.h>
    #include "rw/math/detail/wii/ps_util.h"
#endif

// Old versions of EABase don't properly set EA_SSE on x64 platforms.
// If EA_SSE less than 2 on x64 we will override the EA_SSE macro to be at
// least 2 since all x64 platforms supports at least SSE2.
#if (EA_SSE < 2) && defined(EA_PROCESSOR_X86_64)
    #undef EA_SSE
    #define EA_SSE 2
#endif

#if EA_SSE >= 2
    // This header should be available for SSE2 and greater.
    #include <emmintrin.h>
#endif

#if EA_SSE || (defined(EA_PROCESSOR_X86) && defined(EA_PLATFORM_WINDOWS))
    // This is an internal define which indicates if at least SSE1 is supported on the target.
    // We assume we can use SSE on x86 windows or any platform for which EA_SSE is non-zero.
    #define RW_MATH_TARGET_SUPPORTS_SSE
#endif

#if !defined(RWMATH_VPU_IS_OPTIMAL) && !defined(RWMATH_FPU_IS_OPTIMAL)
    // The user has not specified whether the optimal namespace is fpu or vpu so
    // the rwmath will choose a default optimal namespace
    #if defined(EA_PLATFORM_REVOLUTION) || defined(EA_PROCESSOR_ARM)
        #define RWMATH_FPU_IS_OPTIMAL
    #else
        #define RWMATH_VPU_IS_OPTIMAL
    #endif
#endif


#ifdef RWMATH_VPU_IS_OPTIMAL
    #define RWMATH_DETAIL_INCLUDE_OPTIMAL \
        namespace rw { namespace math { namespace optimal { \
         using namespace ::rw::math::vpu; \
        } } }             
#else
    #define RWMATH_DETAIL_INCLUDE_OPTIMAL \
        namespace rw { namespace math { namespace optimal { \
         using namespace ::rw::math::fpu; \
        } } } 
#endif




#ifdef RW_MATH_ENABLE_ASSERTS
    #ifndef RW_MATH_ASSERT_MSG
        #include <assert.h>
        #define RW_MATH_ASSERT_MSG(condition, message) assert(condition)
    #endif    
    #ifndef RW_MATH_ASSERT
        #include <assert.h>
        #define RW_MATH_ASSERT(condition)        assert(condition)
    #endif
#else
    #define RW_MATH_ASSERT_MSG(condition, message)
    #define RW_MATH_ASSERT(condition)
#endif


// Note RW_MATH_VECTORINTRINSIC_ALIGNMENT may change depending on the target platform and compiler being used.
// It is not always possible for rwmath to enforce the alignment of a vector type due to target platform or compiler constraints.
#if defined EA_PLATFORM_PSP2 || defined EA_PLATFORM_CTR
    #define RW_MATH_VECTORINTRINSIC_ALIGNMENT   8
#else
    #define RW_MATH_VECTORINTRINSIC_ALIGNMENT   16
#endif

// Alignment constant defines for each class.
// In addition to those here, there's also enum for every classes, see <class>::Alignment.
#define RW_MATH_VECTOR2_ALIGNMENT           RW_MATH_VECTORINTRINSIC_ALIGNMENT
#define RW_MATH_VECTOR3_ALIGNMENT           RW_MATH_VECTORINTRINSIC_ALIGNMENT
#define RW_MATH_VECTOR3PLUS_ALIGNMENT       RW_MATH_VECTORINTRINSIC_ALIGNMENT
#define RW_MATH_VECTOR4_ALIGNMENT           RW_MATH_VECTORINTRINSIC_ALIGNMENT

#define RW_MATH_QUATERNION_ALIGNMENT        RW_MATH_VECTORINTRINSIC_ALIGNMENT
#define RW_MATH_PLANE_ALIGNMENT             RW_MATH_VECTORINTRINSIC_ALIGNMENT
#define RW_MATH_QUATPOS_ALIGNMENT           RW_MATH_VECTORINTRINSIC_ALIGNMENT

#define RW_MATH_MATRIX33_ALIGNMENT          RW_MATH_VECTORINTRINSIC_ALIGNMENT
#define RW_MATH_MATRIX44_ALIGNMENT          RW_MATH_VECTORINTRINSIC_ALIGNMENT
#define RW_MATH_MATRIX44AFFINE_ALIGNMENT    RW_MATH_VECTORINTRINSIC_ALIGNMENT


//! \internal math type alignment assertion, user can use this to check memory alignment
#define rwMathAlignmentCheckAssert(var, alignment) RW_MATH_ASSERT_MSG(::rw::math::detail::IsAlignedTo(&(var), (alignment)), ("math type alignment check failed, data passed in is not aligned to %d byte.", alignment));

// Enable this define to turn on alignment check in functions
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_CONSTRUCTORS
    #define rwMathAlignmentConstructorCheckAssert(var, alignment) RW_MATH_ASSERT_MSG(::rw::math::detail::IsAlignedTo(&(var), (alignment)), ("math type alignment check failed, data passed in is not aligned to %d byte.", alignment));
#else
    #define rwMathAlignmentConstructorCheckAssert(var, alignment)
#endif

// Enable this define to turn on alignment check in constructors
#ifdef RW_MATH_ENABLE_ALIGNMENT_CHECK_FUNCTIONS
    #define rwMathAlignmentFunctionCheckAssert(var, alignment) RW_MATH_ASSERT_MSG(::rw::math::detail::IsAlignedTo(&(var), (alignment)), ("math type alignment check failed, data passed in is not aligned to %d byte.", alignment));
#else
    #define rwMathAlignmentFunctionCheckAssert(var, alignment)
#endif

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    #define RWMATH_VECTOR2_ALIGNMENT        RW_MATH_VECTOR2_ALIGNMENT       
    //! \internal
    #define RWMATH_VECTOR3_ALIGNMENT        RW_MATH_VECTOR3_ALIGNMENT       
    //! \internal
    #define RWMATH_VECTOR3PLUS_ALIGNMENT    RW_MATH_VECTOR3PLUS_ALIGNMENT   
    //! \internal
    #define RWMATH_VECTOR4_ALIGNMENT        RW_MATH_VECTOR4_ALIGNMENT                                                
    //! \internal
    #define RWMATH_QUATERNION_ALIGNMENT     RW_MATH_QUATERNION_ALIGNMENT    
    //! \internal
    #define RWMATH_PLANE_ALIGNMENT          RW_MATH_PLANE_ALIGNMENT         
    //! \internal
    #define RWMATH_QUATPOS_ALIGNMENT        RW_MATH_QUATPOS_ALIGNMENT                                                
    //! \internal
    #define RWMATH_MATRIX33_ALIGNMENT       RW_MATH_MATRIX33_ALIGNMENT      
    //! \internal
    #define RWMATH_MATRIX44_ALIGNMENT       RW_MATH_MATRIX44_ALIGNMENT      
    //! \internal
    #define RWMATH_MATRIX44AFFINE_ALIGNMENT RW_MATH_MATRIX44AFFINE_ALIGNMENT
    //! \internal
    #define RWMATRIXALIGNMENT(M) ::rw::math::detail::IsAlignedTo((M), rwMATRIXALIGNMENT)
#endif

// For the math library, proper inlining contributes most to optimization.
// Further, inlined functions allow secondary optimizations such as
// loop unrolling, and register reallocation. As result, for some functions that
// directly call intrinsic functions, the math library will use platform specific
// force inline to make sure the result is what is intended.

#define RW_MATH_INLINE inline


///////////////////////////////////////////////////////////////////////////
// Determine if force inlining should be enabled or not
///////////////////////////////////////////////////////////////////////////

// First check to see if the user has specified that force inlining should be
// enabled or disabled explicitly.
#if defined(RW_MATH_ENABLE_FORCE_INLINING) || defined(RW_MATH_DISABLE_FORCE_INLINING)

    #if defined(RW_MATH_ENABLE_FORCE_INLINING) && defined(RW_MATH_DISABLE_FORCE_INLINING)
        #error The RW_MATH_ENABLE_FORCE_INLINING and RW_MATH_DISABLE_FORCE_INLINING macros are mutually exclusive.
    #endif

    #ifdef RW_MATH_ENABLE_FORCE_INLINING
        #define RW_MATH_DETAIL_FORCE_INLINING 1
    #else
        #define RW_MATH_DETAIL_FORCE_INLINING 0
    #endif
#else
    // The user has not explicitly specified that force inlining should be
    // enabled or disabled.  So, enable/disable force inlining based on
    // the build configuration.
    #if defined EA_DEBUG || defined RWDEBUG
        #define RW_MATH_DETAIL_FORCE_INLINING 0
    #else
        #define RW_MATH_DETAIL_FORCE_INLINING 1
    #endif

#endif


#if defined( RW_MATH_TAG_API_DURING_PREPROCESSING )
    // Don't define RW_MATH_FORCE_INLINE because we don't want the pre-processor to expand this string
#elif !RW_MATH_DETAIL_FORCE_INLINING
    
    #define RW_MATH_FORCE_INLINE inline
    
#else

    #if defined _MSC_VER

        #define RW_MATH_FORCE_INLINE __forceinline

    #elif defined __SNC__ && (defined EA_PLATFORM_PS3 || defined EA_PLATFORM_PSP2)
        
        #define RW_MATH_FORCE_INLINE inline __attribute__ ((always_inline))

    #elif (__GNUC__ < 4) || defined EA_PLATFORM_PALM

        #define RW_MATH_FORCE_INLINE inline
        
    #elif (__GNUC__ >= 4)
    
        #define RW_MATH_FORCE_INLINE inline __attribute__ ((always_inline))
        
    #else
    
        #define RW_MATH_FORCE_INLINE inline
        
    #endif
#endif

#ifdef __ARM_NEON__
#define RWMATH_USE_NEON
#ifdef EA_PLATFORM_IPHONE
#define RWMATH_USE_NEON_INLINE_ASSEMBLY
#endif
#endif

// rwmath normally performs some pointer operations that are not strictly standards compliant.
// Enabling this define forces rwmath to perform standard compliant pointer indexing at the
// cost of some additional performance overhead.
// #define RW_MATH_ENABLE_COMPLIANT_POINTER_ARITHMETIC

namespace rw
{
namespace math
{

//! \internal
namespace detail
{

    //! \internal
    inline bool IsAlignedTo(const void* ptr, uint32_t align)
    {
        return 0 == (reinterpret_cast<intptr_t>(ptr) & static_cast<intptr_t>(align - 1));
    }

    //! \internal
    union Flt32UInt32Union
    {
        float f;
        unsigned int i;
    };
    
    //! \internal
    union Flt64UInt64Union
    {
        double f;
        uint64_t i;
    };
    

} // namespace detail
} // namespace math
} // namespace rw

// The endian swapping functions are only available in RenderWare Cross
// builds. Endian swapping in math only exists for supporting legacy
// code. In any other configuration (that probably means you) they're
// no-ops.
#if defined RWCROSS
    //
    // Determine whether an endian swap is required to convert to the target's endianness.
    //

    // First check to see if the target is already defined.
    #if (defined rwTARGET_WIN || defined rwTARGET_X86LINUX || \
          defined RWTARGETOS_WIN)
        #define RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED 0
    #elif (defined rwTARGET_XBOX2 || defined rwTARGET_PS3 || defined rwTARGET_PS3CEB || defined rwTARGET_REVOLUTION || \
            defined RWTARGETOS_XBOX2 || defined RWTARGETOS_PS3 || defined RWTARGETOS_REVOLUTION)
        #define RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED 1
    #endif

    // If the endianness has not been determined, include ostypes.h.
    #ifndef RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED
        // Include ostypes.h
        //  - ostypes is only included if RWCROSS is defined, and it the
        //    target has not been defined using a rwTARGET_XX macro.
        //    In this way, normally (i.e. in non cross builds) there is no dependency on
        //    other RenderWare headers.
        //  - ostypes must be included here to ensure the RenderWare target defines
        //    are defined.

        // Note: If your compilation fails to include ostypes.h, there are
        // two ways you may fix the problem.
        // 1. Define rwTARGET_XX for your platform before including rwmath.
        //    For example, pass a target define as a compiler command line
        //    option.
        // 2. Add rwcore's include directories to your compiler include
        //    paths so that ostypes.h can be found.
        #include "rw/core/base/ostypes.h"

        #if rwHOST_ENDIAN != rwTARGET_ENDIAN
            #define RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED 1
        #else
            #define RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED 0
        #endif
    #endif
#else
    // This is a native build, so an endian swap is not required.
    #define RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED 0
#endif  // RWCROSS
        
namespace rw
{
namespace math
{
namespace detail
{
    #if RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED
        //! \internal
        inline void EndianSwapArray(char *memory, uint32_t arraySize, uint32_t elementSize)
        {
            const uint32_t swapMask = elementSize - 1;
            for (uint32_t i = 0; i < arraySize; i += 2)
            {
                char temp = memory[i];
                memory[i] = memory[i ^ swapMask];
                memory[i ^ swapMask] = temp;
            }
        }
    #else
        
        //! \internal
        inline void EndianSwapArray(char *, uint32_t, uint32_t) {}
    #endif // RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED

} // namespace detail
} // namespace math
} // namespace rw

// Undefine RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED since the define is no longer needed.
#undef RW_MATH_DETAIL_ENDIAN_SWAP_IS_REQUIRED

#ifdef RW_MATH_FPU_MEMBER_INIT_FUNCTION
template<class T>
void RW_MATH_FPU_MEMBER_INIT_FUNCTION(T& f);
template<>
void RW_MATH_FPU_MEMBER_INIT_FUNCTION(float& f);
template<>
void RW_MATH_FPU_MEMBER_INIT_FUNCTION(double& f);
#endif

#ifdef RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION
template<class T>
void RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(const T& f);
template<>
void RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(const float& f);
template<>
void RW_MATH_FPU_MEMBER_VALIDATE_FUNCTION(const double& f);

#endif

#endif
#endif
