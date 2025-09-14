// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASKSCALAR_H
#define RW_MATH_VPU_MASKSCALAR_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector_intrinsic.h"

namespace rw
{
namespace math
{
namespace vpu
{

#if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
    __declspec(passinreg) 
#endif
class MaskScalar
{
public:
    #ifdef RW_MATH_VECTOR_PASS_BY_VALUE
        typedef const MaskScalar InParam;
    #else
        #if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
            typedef const MaskScalar InParam; 
        #else
            typedef const MaskScalar& InParam; 
        #endif
    #endif

public:
    RW_MATH_FORCE_INLINE MaskScalar();
    #ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
        RW_MATH_FORCE_INLINE MaskScalar(const MaskScalar&);
        RW_MATH_FORCE_INLINE MaskScalar& operator =(const MaskScalar&);
    #endif
    RW_MATH_FORCE_INLINE explicit MaskScalar(VectorIntrinsicInParam);
    RW_MATH_FORCE_INLINE explicit MaskScalar(bool);
    //! Constructs a mask with a bit pattern determined by the input parameter
    //! 
    //! Note: This constructor is only available for VPU/SoA4 MaskScalar types.  It is not available on the FPU for performance reasons.
    //!
    //! \param[in] mask A 32 bit bit mask to initialize the object with.
    RW_MATH_FORCE_INLINE explicit MaskScalar(uint32_t mask);
    RW_MATH_FORCE_INLINE bool GetBool() const;
    //! Returns the value of the mask as a 32bit integer.
    //! 
    //! Note: This function is only available for VPU MaskScalar types.  It is not available on the FPU for performance reasons.
    RW_MATH_FORCE_INLINE uint32_t ToUInt32() const;

    RW_MATH_FORCE_INLINE void SetTrue();
    RW_MATH_FORCE_INLINE void SetFalse();
public:
    VectorIntrinsic mV;
};

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/maskscalar_type_inline.h"
#endif

#endif // RW_MATH_VPU_MASKSCALAR_H
