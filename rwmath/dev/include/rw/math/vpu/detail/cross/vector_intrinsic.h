// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_CROSS_GENERIC_VECTOR_INTRINSIC_H
#define RW_MATH_VPU_DETAIL_CROSS_GENERIC_VECTOR_INTRINSIC_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"


namespace rw {
namespace math {
namespace vpu {

#if defined _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4324)  // structure was padded due to __declspec(align())
#endif
    
    /*!
        \brief Platform specific vector intrinsic type
        
        This is the generic definition, on other platforms this type
        maps to hardware registers.
    */
    struct 
#ifndef _MANAGED
    EA_PREFIX_ALIGN(RW_MATH_VECTORINTRINSIC_ALIGNMENT)
#endif
    VectorIntrinsic
    {
        union
        {
            float f[4];
            unsigned int i[4];
        };
        
    }
#ifndef _MANAGED
    EA_POSTFIX_ALIGN(RW_MATH_VECTORINTRINSIC_ALIGNMENT)
#endif
    ;
    
#if defined _MSC_VER
    #pragma warning(pop)
#endif

//! Ideal function passing type for platform specific vector intrisics
typedef const VectorIntrinsic & VectorIntrinsicInParam;

namespace detail
{
    extern const VectorIntrinsic gCosMinSlerpAngle;
}

}}}

#endif
