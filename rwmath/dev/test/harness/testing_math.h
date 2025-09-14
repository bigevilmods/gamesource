// (c) Electronic Arts. All Rights Reserved.
#ifndef TESTING_MATH_INCLUDED
#define TESTING_MATH_INCLUDED

#include <string.h>

namespace testing
{
    // Some math functions are duplicated here to avoid pulling in extra headers when testing specific math types/operations.

    inline float Abs(float a)
    {
        return (a<0.0f) ? -a : a;
    }

    inline bool IsZero(float a, float epsilon)
    {
        return Abs(a) < epsilon;
    }

    inline bool IsSimilar(float a, float b, float epsilon)
    {
        return IsZero(a-b, epsilon);
    }

    inline bool IsSimilarAngle( float a, float b, float epsilon )
    {
        float difference = a - b;
        float quotient = difference / (2.0f*3.14159265359f);
        float frac = quotient - (float)(int)quotient;
        return testing::IsZero(frac, epsilon) || testing::IsSimilar(frac, 1.0f, epsilon);
    }

    double GetNaN();
}

#endif
