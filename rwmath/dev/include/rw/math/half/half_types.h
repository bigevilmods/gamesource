// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_HALF_TYPES_H
#define RW_MATH_HALF_TYPES_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

namespace rw
{
namespace math
{
namespace half
{

    typedef uint16_t half_t;

    struct HalfVector2
    {
        typedef const HalfVector2& InParam;

        HalfVector2() 
            : x(0), y(0)
        {
        }

        HalfVector2(half_t hx, half_t hy) 
            : x(hx), y(hy)
        {
        }

        half_t x, y;
    };

    struct HalfVector3
    {
        typedef const HalfVector3& InParam;

        HalfVector3() 
            : x(0), y(0), z(0)
        {
        }

        HalfVector3(half_t hx, half_t hy, half_t hz) 
            : x(hx), y(hy), z(hz)
        {
        }

        half_t x, y, z;
    };

    struct HalfVector4
    {
        typedef const HalfVector4& InParam;

        HalfVector4() 
            : x(0), y(0), z(0), w(0)
        {
        }

        HalfVector4(half_t hx, half_t hy, half_t hz, half_t hw) 
            : x(hx), y(hy), z(hz), w(hw)
        {
        }

        half_t x, y, z, w;
    };

} // namespace half
} // namespace math
} // namespace rw

#endif
