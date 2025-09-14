// (c) Electronic Arts. All Rights Reserved.
#include <stdlib.h>
#include <rw/math/random.h>
#include <rw/math/fpu/scalar.h>

namespace rw
{
namespace math
{

const float max_uint_as_float = 4.294967295e+009f;
const float c0 = 2.515517f;
const float c1 = 0.802853f;
const float c2 = 0.010328f;
const float d1 = 1.432788f;
const float d2 = 0.189269f;
const float d3 = 0.001308f; 


static unsigned int sSeedArray[6] =
{
    0xf22d0e56,
    0x883126e9,
    0xc624dd2f,
    0x0702c49c,
    0x9e353f7d,
    0x6fdf3b64 
};

// Add with carry. SUM is replaced with A + B + C, C is replaced with 1 if
// there was a carry, 0 if there wasn't. A carry occurred if the sum is less
// than one of the inputs. This is addition, so carry can never be more than
// one.

#define ADC(SUM, A, B, C)                                                                   \
    {                                                                                       \
        (SUM) = (A) + (B) + (C);                                                            \
        (C) = static_cast<unsigned int>(((SUM) < (A)) || (((SUM) == (A)) && ((C) != 0)) );  \
    }

// Special case ADC macro that doesn't take a carry flag.  This macro is necessary to avoid
// a lint warning that C!=0 is always false for the first invocation of the ADC macro.
#define ADC_INITIAL(SUM, A, B, C)                                                           \
    {                                                                                       \
        (SUM) = (A) + (B);                                                                  \
        (C) = static_cast<unsigned int>((SUM) < (A));                                       \
    }

#ifdef EA_PLATFORM_PS3_SPU
unsigned int RandomImpl(unsigned int seedArray[6]) __attribute__((noinline));
#else
unsigned int RandomImpl(unsigned int seedArray[6]);
#endif

unsigned int RandomImpl(unsigned int seedArray[6])
{
    unsigned int ax;
    unsigned int c = 0;

    ADC_INITIAL(ax, seedArray[5], seedArray[4], c);
    seedArray[4] = ax;

    ADC(ax, seedArray[4], seedArray[3], c);
    seedArray[3] = ax;

    ADC(ax, seedArray[3], seedArray[2], c);
    seedArray[2] = ax;

    ADC(ax, seedArray[2], seedArray[1], c);
    seedArray[1] = ax;

    ADC(ax, seedArray[1], seedArray[0], c);
    seedArray[0] = ax;

    // Increment seed array, bubbling up the carries.
    if (++seedArray[5] == 0)
    {
        if (++seedArray[4] == 0)
        {
            if (++seedArray[3] == 0)
            {
                if (++seedArray[2] == 0)
                {
                    if (++seedArray[1] == 0)
                    {
                        ++seedArray[0];
                        ++ax;
                    }
                }
            }
        }
    }

    return(ax);
}

#ifdef EA_PLATFORM_PS3_SPU
void SeedRandomImpl(unsigned int *seedArray, unsigned int s) __attribute__((noinline));
#else
void SeedRandomImpl(unsigned int *seedArray, unsigned int s);
#endif

void SeedRandomImpl(unsigned int *seedArray, unsigned int s)
{
    seedArray[0] = s + 0xf22d0e56;
    seedArray[1] = s + 0x883126e9;
    seedArray[2] = s + 0xc624dd2f;
    seedArray[3] = s + 0x0702c49c;
    seedArray[4] = s + 0x9e353f7d;
    seedArray[5] = s + 0x6fdf3b64;
}

//! Generate a pseudo-random 32 bit unsigned integer.
unsigned int Random()
{
    return RandomImpl( sSeedArray );
}

//! Seed the random number generator
void SeedRandom(unsigned int s)
{
    SeedRandomImpl( sSeedArray, s );
}

/**
    Gaussian random generator with mean = 0 and standard deviation = 1
    You can transform the return value to a dfferent mean and standard deviation by 
    the following formula: 
	mean + return * standard deviation

    x is a uniform distributed float in the range of 0.0f and 1.0f
**/
float GaussianTransform(float x)
{
	bool needNegate = false;
	if(x > 0.5)
	{
		x = 1.0f - x;
		needNegate = true;
	}
	float t = ::rw::math::fpu::Sqrt( ::rw::math::fpu::Log( 1.0f / ( x * x) ) );
	float tt = t * t;
	float ttt = tt * t;
	float ret = t - ((c0 + c1 * t + c2 * tt) / (1 + d1 * t + d2 * tt + d3 * ttt));

	return needNegate ? -ret : ret;
}

/**
    This Poisson Random generator only works for any mean that is <= 100

	x is a uniform distributed float in the range of 0.0f and 1.0f.
	mean is the expected number of occurrences that occur during a given interval.
**/
int PoissonTransform(float x, float mean)
{
#ifdef RW_MATH_ENABLE_ASSERTS
	RW_MATH_ASSERT_MSG(mean <= 100.0f, "RWMath poisson random generator only works for means that are <= 100.0f");
#endif
	int k=0;                          //Counter
	const int max_k = 1000;           //k upper limit
	float P = (float)exp(-mean);      //probability
	float sum=P;                      //cumulant
	if (sum>=x) return 0;             //done allready
	for (k=1; k<max_k; ++k) 
	{         //Loop over all k:s
		P*=mean/(float)k;            //Calc next prob
		sum+=P;                      //Increase cumulant
		if (sum>=x) break;           //Leave loop
	}

	return k;                         //return random number
}

RandomNumberGenerator::RandomNumberGenerator() 
{
    SeedRandomImpl( mSeedArray, 0 );
}

//! Generate a pseudo-random 32 bit unsigned integer.
unsigned int RandomNumberGenerator::Random()
{
    return RandomImpl( mSeedArray );
}

//! Generate a pseudo-random 32 bit float between 0.0f and 1.0f.
float RandomNumberGenerator::RandomFloat()
{
	float r = static_cast<float>(RandomImpl( mSeedArray ));
	return r / max_uint_as_float;
}

//! Seed the random number generator
void RandomNumberGenerator::Seed(unsigned int s)
{
    SeedRandomImpl( mSeedArray, s );
}

} // namespace math
} // namespace rw
