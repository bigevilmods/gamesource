// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VECUTIL_OPERATION_INLINE_H
#define RW_MATH_VECUTIL_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{
namespace math
{
namespace vpl
{

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UInt(uint32_t value)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarUInt[0] = value;
        r.scalarUInt[1] = value;
        r.scalarUInt[2] = value;
        r.scalarUInt[3] = value;
        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UShort(uint32_t value)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        unsigned int v = value * 0x00010001U;
        r.scalarUInt[0] = v;
        r.scalarUInt[1] = v;
        r.scalarUInt[2] = v;
        r.scalarUInt[3] = v;
        return r.vector;
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecUtilCreate4Bit_UChar(uint32_t value)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        unsigned int v = value * 0x01010101U;
        r.scalarUInt[0] = v;
        r.scalarUInt[1] = v;
        r.scalarUInt[2] = v;
        r.scalarUInt[3] = v;
        return r.vector;
    }

} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_VECUTIL_OPERATION_INLINE_H
