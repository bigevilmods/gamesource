// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_RANDOM_H
#define RW_MATH_RANDOM_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

namespace rw {
namespace math {

    /*!
    Retrieves a random number from the static random number generator.

    This is an old API. It's strongly recommended to use the Random() function
    from RandomNumberGenerator class

    \return The generated random number.
    */
    unsigned int Random();

    /*!
    Seeds the static random number generator.

    \param seed The number to seed the generator.
    */
    void SeedRandom(unsigned int seed);

	/*!
    Transforms uniformly distributed float numbers to Gaussian numbers with mean = 0 and standard deviation = 1
    You can convert the return value to a dfferent set of mean and standard deviation by 
    the following formula: 
	mean + return * standard deviation

    x is a uniform distributed float in the range of 0.0f and 1.0f

    \param x: input random number
	\return Gaussian float
    */
    float GaussianTransform(float x);

	/*!
    Transforms uniformly distributed float numbers to Poisson numbers.
    This function only works for means that are less than or equal to 100. 

	x is a uniform distributed float in the range of 0.0f and 1.0f.
	mean is the expected number of occurrences that occur during a given interval.

    \param x: input random number
	\param mean: mean
	\return Poisson int
    */
    int PoissonTransform(float x, float mean);

    /*! 

    \brief Random number generator.

    This class was created to support multiple instances of the
    random number generator.
    */
    class RandomNumberGenerator
    {
    public:

        /*!
        Constructs the random number generator and seeds it with a default
        seed.
        */
        RandomNumberGenerator();

        /*!
        Retrieves a random number from the random number generator.

        \return The generated random number.
        */
        unsigned int Random();

		/*!
		Return a random float between 0 and 1
		*/
		float RandomFloat();

        /*!
        Seeds the random number generator.

        \param seed The number to seed the generator.
        */
        void Seed(unsigned int seed);

    private:
        unsigned int mSeedArray[6];
    };


}}

#endif
