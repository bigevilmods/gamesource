// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_VEC_FLOAT_TYPE_PLATFORM_INLINE_H
#define RW_MATH_VPU_DETAIL_VEC_FLOAT_TYPE_PLATFORM_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#if defined RWCROSS
    #include "rw/math/vpu/detail/cross/vec_float_type_platform_inline.h"
#else

namespace rw {
namespace math {
namespace vpu {

RW_MATH_FORCE_INLINE VecFloat::VecFloat(VectorIntrinsicInParam v) 
{
    // for alignment check, we must check before we do a assignment
    rwMathAlignmentConstructorCheckAssert(mV, sizeof(VectorIntrinsic));
    rwMathAlignmentConstructorCheckAssert(v, sizeof(VectorIntrinsic));
    
    #ifdef RW_MATH_ENABLE_ASSERTS
        
        detail::VectorIntrinsicUnion unionVector;    
        unionVector.vector = v;

        // We do a comparison using floats, and using ints.
        // We need to do a comparison using ints to handle NaN's (i.e. if a = NaN then a!=a using float compares.)
        // We also do a float compare because apparently some vector unit
        // computations were giving slightly different values for elements even
        // though the inputs values were all identical.  However, a float compare
        // shows they are equivalent.  It may be possible to drop the float
        // compares on some platforms if more investigation is done.
        RW_MATH_ASSERT_MSG((unionVector.scalarFloat[0] == unionVector.scalarFloat[1]) &
                        (unionVector.scalarFloat[0] == unionVector.scalarFloat[2]) &
                        (unionVector.scalarFloat[0] == unionVector.scalarFloat[3])
                        || (unionVector.scalarUInt[0] == unionVector.scalarUInt[1]) &
                        (unionVector.scalarUInt[0] == unionVector.scalarUInt[2]) &
                        (unionVector.scalarUInt[0] == unionVector.scalarUInt[3]),
                        ("All element of input vector must be the same value to use this constructor."));                       
    #endif
    
    mV = v;
}

RW_MATH_FORCE_INLINE const VectorIntrinsic& VecFloat::GetVector() const 
{
    rwMathAlignmentFunctionCheckAssert(mV, sizeof(VectorIntrinsic));
    return mV;
}

}}}

#endif
#endif
