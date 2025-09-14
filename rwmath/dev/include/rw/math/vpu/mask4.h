// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_MASK4_H
#define RW_MATH_VPU_MASK4_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/maskscalar.h"

namespace rw
{
namespace math
{
namespace vpu
{

#if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
    __declspec(passinreg) 
#endif
class Mask4
{
public:
    #ifdef RW_MATH_VECTOR_PASS_BY_VALUE
        typedef const Mask4 InParam;
    #else
        #if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
            typedef const Mask4 InParam;
        #else
            typedef const Mask4& InParam;
        #endif
    #endif

public:
    RW_MATH_FORCE_INLINE Mask4();
    #ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
        RW_MATH_FORCE_INLINE Mask4(const Mask4&);
        RW_MATH_FORCE_INLINE Mask4& operator =(const Mask4&);
    #endif
    RW_MATH_FORCE_INLINE explicit Mask4(VectorIntrinsicInParam);
    RW_MATH_FORCE_INLINE explicit Mask4(bool, bool, bool, bool);
    //! Constructs a mask with a bit patterns determined by the input parameters
    //! 
    //! Note: This constructor is only available for VPU MaskScalar types.  It is not available on the FPU for performance reasons.
    //!
    //! \param[in] xMask A 32 bit bit mask to initialize the x component of the mask with.
    //! \param[in] yMask A 32 bit bit mask to initialize the y component of the mask with.
    //! \param[in] zMask A 32 bit bit mask to initialize the z component of the mask with.
    //! \param[in] wMask A 32 bit bit mask to initialize the w component of the mask with.
    RW_MATH_FORCE_INLINE explicit Mask4(uint32_t xMask, uint32_t yMask, uint32_t zMask, uint32_t wMask);
    RW_MATH_FORCE_INLINE explicit Mask4(MaskScalar::InParam s);
    RW_MATH_FORCE_INLINE explicit Mask4(MaskScalar::InParam, MaskScalar::InParam, MaskScalar::InParam, MaskScalar::InParam);
    RW_MATH_FORCE_INLINE MaskScalar GetX() const;
    RW_MATH_FORCE_INLINE MaskScalar GetY() const;
    RW_MATH_FORCE_INLINE MaskScalar GetZ() const;
    RW_MATH_FORCE_INLINE MaskScalar GetW() const;
    RW_MATH_FORCE_INLINE void SetX(MaskScalar::InParam);
    RW_MATH_FORCE_INLINE void SetY(MaskScalar::InParam);
    RW_MATH_FORCE_INLINE void SetZ(MaskScalar::InParam);
    RW_MATH_FORCE_INLINE void SetW(MaskScalar::InParam);
    RW_MATH_FORCE_INLINE void SetTrue();
    RW_MATH_FORCE_INLINE void SetFalse();

public:
    VectorIntrinsic mV;
};

} // namespace vpu
} // namespace math
} // namespace rw

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/mask4_type_inline.h"
#endif

#endif // RW_MATH_VPU_MASK4_H
