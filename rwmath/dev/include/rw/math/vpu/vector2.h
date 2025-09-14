// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_VECTOR2_H
#define RW_MATH_VPU_VECTOR2_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vec_float.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/fpu/vector2.h"

namespace rw {
namespace math {
namespace vpu {

#if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
    __declspec(passinreg) 
#endif
//! Vector with two components
class Vector2
{
public:
    enum { IsAligned = 1 };
    enum { Alignment = 16 };
    enum { NumberOfElement = 2 };

    typedef float FloatType;
    typedef VecFloat FastFloatType;

    #ifdef RW_MATH_VECTOR_PASS_BY_VALUE
        typedef const Vector2 InParam;    
    #else
        #if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
            typedef const Vector2 InParam; 
        #else
            typedef const Vector2& InParam; 
        #endif
    #endif
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Vector2& OutParam;
        typedef Vector2& InOutParam;
    #endif
    
public:

    RW_MATH_FORCE_INLINE Vector2();  
    #ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
        RW_MATH_FORCE_INLINE Vector2(const Vector2& rhs);
        RW_MATH_FORCE_INLINE Vector2& operator = (Vector2::InParam v);
    #endif
    //! This constructor creates a vector with all its elements equal to \c scalar.
    RW_MATH_FORCE_INLINE explicit Vector2(VecFloat::InParam scalar);
    RW_MATH_FORCE_INLINE Vector2(float x, float y);
    //! Construct a Vector2 from an array of floats.
    //!
    //! The array must be aligned to a 16byte boundary for hardware efficiency reasons.
    RW_MATH_FORCE_INLINE explicit Vector2(const float xy[2]);
    RW_MATH_FORCE_INLINE RW_MATH_FUTURE_EXPLICIT Vector2(VectorIntrinsicInParam v);
    RW_MATH_FORCE_INLINE Vector2(VecFloat::InParam x, VecFloat::InParam y);
    RW_MATH_FORCE_INLINE Vector2(VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y);
    template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX>
    RW_MATH_FORCE_INLINE Vector2(VecFloatRef<XSRC_INDEX> x, VecFloatRef<YSRC_INDEX> y);
    template<class T>
    RW_MATH_FORCE_INLINE explicit Vector2(const math::fpu::Vector2Template<T> &v);    
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        RW_MATH_FORCE_INLINE Vector2& operator = (VectorIntrinsicInParam v);
        RW_MATH_FORCE_INLINE operator VectorIntrinsic&();
        RW_MATH_FORCE_INLINE operator const VectorIntrinsic&() const;
    #endif
    
    RW_MATH_FORCE_INLINE       VectorIntrinsic& GetVector();
    RW_MATH_FORCE_INLINE const VectorIntrinsic& GetVector() const;

    RW_MATH_FORCE_INLINE void SetVector(VectorIntrinsicInParam v);    
    
    template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX>
    RW_MATH_FORCE_INLINE void Set(VecFloatRef<XSRC_INDEX> x, VecFloatRef<YSRC_INDEX> y);
    RW_MATH_FORCE_INLINE void Set(float x, float y);
    RW_MATH_FORCE_INLINE void Set(VecFloat::InParam x, VecFloat::InParam y);
    RW_MATH_FORCE_INLINE void Set(VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y);
   
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetComponent(int i, VecFloatRef<INDEX> value);
    RW_MATH_FORCE_INLINE void SetComponent(int i, float value);
    RW_MATH_FORCE_INLINE void SetComponent(int i, VecFloat::InParam value);
    RW_MATH_FORCE_INLINE void SetComponent(int i, VecFloatRefIndex::InParam value);
   
    template<VectorAxis INDEX> 
    RW_MATH_FORCE_INLINE void SetX(VecFloatRef<INDEX> x);
    RW_MATH_FORCE_INLINE void SetX(float x);
    RW_MATH_FORCE_INLINE void SetX(VecFloat::InParam x);
    RW_MATH_FORCE_INLINE void SetX(VecFloatRefIndex::InParam x);
    
    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetY(VecFloatRef<INDEX> y);
    RW_MATH_FORCE_INLINE void SetY(float y);
    RW_MATH_FORCE_INLINE void SetY(VecFloat::InParam y);
    RW_MATH_FORCE_INLINE void SetY(VecFloatRefIndex::InParam y);

    RW_MATH_FORCE_INLINE void SetZero();
    
    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetComponent(int i);    
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetComponent(int i) const;    
    RW_MATH_FORCE_INLINE       VecFloatRefIndex operator [] (int i);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex operator [] (int i) const;

    #ifdef RW_MATH_RETURN_FLOAT
        RW_MATH_FORCE_INLINE const float& GetX() const{ return mV.x; }
        RW_MATH_FORCE_INLINE       float& GetX(){ return mV.x; }
        RW_MATH_FORCE_INLINE const float& GetY() const{ return mV.y; }
        RW_MATH_FORCE_INLINE       float& GetY(){ return mV.y; }
        
        RW_MATH_FORCE_INLINE const float& X() const{ return mV.x; }
        RW_MATH_FORCE_INLINE       float& X(){ return mV.x; }
        RW_MATH_FORCE_INLINE const float& Y() const{ return mV.y; }
        RW_MATH_FORCE_INLINE       float& Y(){ return mV.y; }
    #else
        RW_MATH_FORCE_INLINE const VecFloatRefX GetX() const;
        RW_MATH_FORCE_INLINE       VecFloatRefX GetX();
        RW_MATH_FORCE_INLINE const VecFloatRefY GetY() const;
        RW_MATH_FORCE_INLINE       VecFloatRefY GetY();
        
        RW_MATH_FORCE_INLINE const VecFloatRefX X() const;
        RW_MATH_FORCE_INLINE       VecFloatRefX X();
        RW_MATH_FORCE_INLINE const VecFloatRefY Y() const;
        RW_MATH_FORCE_INLINE       VecFloatRefY Y();
    #endif
    
public:

    VectorIntrinsic  mV;
};

}

using ::rw::math::vpu::Vector2;

}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Vector2& t)
{
    math::detail::EndianSwapArray(reinterpret_cast<char *>(&t), sizeof(rw::math::vpu::Vector2), sizeof(float));
}

}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/vector2_type_inline.h"
#endif

namespace rw {
namespace math {
namespace fpu {

    // Note; defining this constructor here vs. fpu/vector2.h avoids ps3 compiler error.
    // This also allow us to remove the dependency of fpu header on vpu header
    template<class T>
    Vector2Template<T>::Vector2Template(const rw::math::vpu::Vector2 &v)
    {
        this->x = static_cast<T>(v.X().GetFloat());
        this->y = static_cast<T>(v.Y().GetFloat());
    }
}}}


#endif
