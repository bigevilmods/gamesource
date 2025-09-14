// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_DETAIL_TEMPLATES_H
#define RW_MATH_VPU_DETAIL_TEMPLATES_H

namespace rw
{

namespace math
{

namespace vpu
{

namespace detail
{

    template<int constValue>
    struct IntSource
    {    
        static const int value;
    };

    template<int constValue>
    const int IntSource<constValue>::value = constValue;

} // namespace detail

} // namespace vpu

} // namespace math

} // namespace rw

#endif