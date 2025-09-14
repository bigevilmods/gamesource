// (c) Electronic Arts. All Rights Reserved.

#ifndef VEC_FLOAT_UTILITY_HPP_INCLUDED
#define VEC_FLOAT_UTILITY_HPP_INCLUDED

#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vec_float.h"

inline bool IsEqual(::rw::math::vpu::VectorIntrinsicInParam a, ::rw::math::vpu::VectorIntrinsicInParam b)
{
	return memcmp(&a, &b, sizeof(::rw::math::vpu::VectorIntrinsic)) == 0;
}

// This following classes and templated utility function that can be
// used for unit testing comparison operators (i.e. ==, !=, >=, <=, >, < ).

class GenericCompareE
{
public:
	bool ExpectedResultForEqual()
	{
		return true;
	}

	bool ExpectedResultForGreater()
	{
		return false;
	}

	bool ExpectedResultForLess()
	{
		return false;
	}
};

class GenericCompareNE
{
public:
	bool ExpectedResultForEqual()
	{
		return false;
	}

	bool ExpectedResultForGreater()
	{
		return true;
	}

	bool ExpectedResultForLess()
	{
		return true;
	}
};

class GenericCompareGT
{
public:
	bool ExpectedResultForEqual()
	{
		return false;
	}

	bool ExpectedResultForGreater()
	{
		return true;
	}

	bool ExpectedResultForLess()
	{
		return false;
	}

};

class GenericCompareLT
{
public:
	bool ExpectedResultForEqual()
	{
		return false;
	}

	bool ExpectedResultForGreater()
	{
		return false;
	}

	bool ExpectedResultForLess()
	{
		return true;
	}

};

class GenericCompareGTE
{
public:
	bool ExpectedResultForEqual()
	{
		return true;
	}

	bool ExpectedResultForGreater()
	{
		return true;
	}

	bool ExpectedResultForLess()
	{
		return false;
	}

};

class GenericCompareLTE
{
public:
	bool Compare(float a, float b)
	{
		return a >= b;
	}

	bool ExpectedResultForEqual()
	{
		return true;
	}

	bool ExpectedResultForGreater()
	{
		return false;
	}

	bool ExpectedResultForLess()
	{
		return true;
	}
};

template <class T>
bool TestComparisonOperator(T operatorToTest)
{
	return( 
			operatorToTest.Compare(-1.8f, 5.0f)  == operatorToTest.ExpectedResultForLess() &&
			operatorToTest.Compare(-1.8f, -1.0f) == operatorToTest.ExpectedResultForLess() &&
			operatorToTest.Compare(1.2f, 5.0f)   == operatorToTest.ExpectedResultForLess() &&

			operatorToTest.Compare(4.1f, 4.1f)   == operatorToTest.ExpectedResultForEqual() &&
			operatorToTest.Compare(-3.2f, -3.2f) == operatorToTest.ExpectedResultForEqual() &&
			operatorToTest.Compare(0.0f, 0.0f)   == operatorToTest.ExpectedResultForEqual() &&

			operatorToTest.Compare(1.8f, -5.0f)  == operatorToTest.ExpectedResultForGreater() &&
			operatorToTest.Compare(1.8f, 1.0f)   == operatorToTest.ExpectedResultForGreater() &&
			operatorToTest.Compare(-1.2f, -5.0f) == operatorToTest.ExpectedResultForGreater()
			);
}

template<class T>
class InstanceFactory
{
public:
	InstanceFactory( float value );
	T GetInstance();
};

template<>
class InstanceFactory< ::rw::math::vpu::VecFloat >
{
public:
	InstanceFactory( float value )
		:mInstance(value)
	{
	}

	::rw::math::vpu::VecFloat GetInstance()
	{
		return mInstance;
	}

private:
	::rw::math::vpu::VecFloat mInstance;
};

template<>
class InstanceFactory<float>
{
public:
	InstanceFactory( float value )
		:mInstance(value)
	{
	}

	float GetInstance()
	{
		return mInstance;
	}

private:
	float mInstance;
};

template<>
class InstanceFactory< ::rw::math::vpu::VecFloatRefIndex >
{
public:
	InstanceFactory( float value )
		:mInstance(mData, 1)
	{
        union
        {   
            rw::math::vpu::VectorIntrinsic v;
            float scalarFloat[4];
        } vecUnion;
        vecUnion.scalarFloat[0] = 0.0f;
        vecUnion.scalarFloat[1] = value;
        vecUnion.scalarFloat[2] = -1.0f;
        vecUnion.scalarFloat[3] = 1.0f;
        mData = vecUnion.v;
	}

	::rw::math::vpu::VecFloatRefIndex GetInstance()
	{
		return mInstance;
	}

private:
	::rw::math::vpu::VectorIntrinsic mData;
	::rw::math::vpu::VecFloatRefIndex mInstance;
};

template<>
class InstanceFactory< ::rw::math::vpu::VecFloatRef< ::rw::math::vpu::VectorAxisX > >
{
public:
	InstanceFactory( float value )
		:mInstance(mData)
	{
        union
        {   
            ::rw::math::vpu::VectorIntrinsic v;
            float scalarFloat[4];
        } vecUnion;
        vecUnion.scalarFloat[0] = value;
        vecUnion.scalarFloat[1] = -1.0f;
        vecUnion.scalarFloat[2] = 0.0f;
        vecUnion.scalarFloat[3] = 1.0f;
        mData = vecUnion.v;
	}

	::rw::math::vpu::VecFloatRef< ::rw::math::vpu::VectorAxisX > GetInstance()
	{
		return mInstance;
	}

private:
	::rw::math::vpu::VectorIntrinsic mData;
	::rw::math::vpu::VecFloatRef< ::rw::math::vpu::VectorAxisX > mInstance;
};

template <class T1, class T2>
class TemplatedCompareE : public GenericCompareE
{
public:
	bool Compare(float a, float b)
	{
		InstanceFactory<T1> factoryA(a);
		InstanceFactory<T2> factoryB(b);
		return factoryA.GetInstance() == factoryB.GetInstance();
	}
};

template <class T1, class T2>
class TemplatedCompareNE : public GenericCompareNE
{
public:
	bool Compare(float a, float b)
	{
		InstanceFactory<T1> factoryA(a);
		InstanceFactory<T2> factoryB(b);
		return factoryA.GetInstance() != factoryB.GetInstance();
	}
};

template <class T1, class T2>
class TemplatedCompareGT : public GenericCompareGT
{
public:
	bool Compare(float a, float b)
	{
		InstanceFactory<T1> factoryA(a);
		InstanceFactory<T2> factoryB(b);
		return factoryA.GetInstance() > factoryB.GetInstance();
	}
};

template <class T1, class T2>
class TemplatedCompareLT : public GenericCompareLT
{
public:
	bool Compare(float a, float b)
	{
		InstanceFactory<T1> factoryA(a);
		InstanceFactory<T2> factoryB(b);
		return factoryA.GetInstance() < factoryB.GetInstance();
	}
};

template <class T1, class T2>
class TemplatedCompareGTE : public GenericCompareGTE
{
public:
	bool Compare(float a, float b)
	{
		InstanceFactory<T1> factoryA(a);
		InstanceFactory<T2> factoryB(b);
		return factoryA.GetInstance() >= factoryB.GetInstance();
	}
};

template <class T1, class T2>
class TemplatedCompareLTE : public GenericCompareLTE
{
public:
	bool Compare(float a, float b)
	{
		InstanceFactory<T1> factoryA(a);
		InstanceFactory<T2> factoryB(b);
		return factoryA.GetInstance() <= factoryB.GetInstance();
	}
};

template<class T>
inline const char *GetInParamName();

template<>
inline const char *GetInParamName< ::rw::math::vpu::VecFloat >()
{
	return "VecFloat::InParam";
}

template<>
inline const char *GetInParamName<float>()
{
	return "float";
}

template<>
inline const char *GetInParamName< ::rw::math::vpu::VecFloatRefIndex >()
{
	return "VecFloatRefIndex::InParam";
}

template<>
inline const char *GetInParamName< ::rw::math::vpu::VecFloatRef< ::rw::math::vpu::VectorAxisX > >()
{
	return "VecFloatRef<VectorAxisX>::InParam";
}

#endif
