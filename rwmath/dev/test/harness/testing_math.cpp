// (c) 2008 Criterion Software Limited and its licensors. All Rights Reserved
#include "testing_math.h"


namespace testing
{

double GetNaN()
{
    // zero is made volatile to eliminate a warning about dividing by zero on PC
    volatile double zero = 0.0;
    return zero/zero;
}

} // namespace testing
