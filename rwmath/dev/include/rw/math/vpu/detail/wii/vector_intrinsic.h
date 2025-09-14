// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_WII_VECTOR_INTRINSIC_H
#define RW_MATH_VPU_DETAIL_WII_VECTOR_INTRINSIC_H

#include "rw/math/detail/base.h"


namespace rw
{
namespace math
{
namespace vpu
{

    /*!
        \brief Platform specific vector intrinsic type
        
        This is the generic definition, on other platforms this type
        maps to hardware registers.
    */
    struct EA_PREFIX_ALIGN(16) VectorIntrinsic
    {
        union
        {
            struct
            {
                ::rw::math::ps::detail::mw_ps ps0;
                ::rw::math::ps::detail::mw_ps ps1;
            };
            float f[4];
            unsigned int i[4];
        };
    } EA_POSTFIX_ALIGN(16);
    
    //! Ideal function passing type for platform specific vector intrisics
    typedef const VectorIntrinsic & VectorIntrinsicInParam;

} // namespace vpu
} // namespace math
} // namespace rw

#endif
