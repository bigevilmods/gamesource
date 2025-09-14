// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_VECTOR3PLUS_H
#define RW_MATH_VPU_VECTOR3PLUS_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vec_float.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/fpu/vector3plus.h"

namespace rw {
namespace math {
namespace vpu {

/*!
    \brief Represents a vector in three-dimensional (3D) space
    with the fourth element set to your own data.

    The Vector3Plus class is implemented using hardware efficient vector types.
    Therefore, the Vector3Plus should always be aligned on a 16 byte boundary.

    This vector class is only used as a container
    and there are no functions (i.e. Mult, Add) for it.
*/
#if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
    __declspec(passinreg) 
#endif
class Vector3Plus
{
public:
    enum { IsAligned = 1 };
    enum { Alignment = 16 };
    enum { NumberOfElement = 4 };
    
    typedef float FloatType;
    typedef VecFloat FastFloatType;

    #ifdef RW_MATH_VECTOR_PASS_BY_VALUE
        typedef const Vector3Plus InParam;
    #else
        #if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
            typedef const Vector3Plus InParam; 
        #else
            typedef const Vector3Plus& InParam; 
        #endif
    #endif 
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Vector3Plus& OutParam;
        typedef Vector3Plus& InOutParam;
    #endif

public:

    template<class T>
    Vector3Plus(const math::fpu::Vector3PlusTemplate<T> &v);
    
    RW_MATH_FORCE_INLINE Vector3Plus();
    RW_MATH_FORCE_INLINE Vector3Plus( float x, float y, float z, float plus );
    RW_MATH_FORCE_INLINE Vector3Plus(float xyz, float plus);
    #ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
        RW_MATH_FORCE_INLINE Vector3Plus(const Vector3Plus& rhs);
        RW_MATH_FORCE_INLINE Vector3Plus& operator = (Vector3Plus::InParam v); 
    #endif
    RW_MATH_FORCE_INLINE Vector3Plus(Vector3::InParam v, VecFloat::InParam userData);
    //! Construct a Vector3Plus from an array of floats.
    //!
    //! The array must be aligned to a 16byte boundary for hardware efficiency reasons.
    RW_MATH_FORCE_INLINE explicit Vector3Plus( const float xyzplus[4]);
    RW_MATH_FORCE_INLINE RW_MATH_FUTURE_EXPLICIT Vector3Plus( VectorIntrinsicInParam v);
    RW_MATH_FORCE_INLINE Vector3Plus( VecFloat::InParam x, VecFloat::InParam y, VecFloat::InParam z, VecFloat::InParam plus );
    RW_MATH_FORCE_INLINE Vector3Plus(VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y, VecFloatRefIndex::InParam z, VecFloatRefIndex::InParam plus);
    
    template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
    RW_MATH_FORCE_INLINE Vector3Plus(VecFloatRef<XSRC_INDEX> x, VecFloatRef<YSRC_INDEX> y, VecFloatRef<ZSRC_INDEX> z, VecFloatRef<WSRC_INDEX> plus); 
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        RW_MATH_FORCE_INLINE Vector3Plus& operator = (VectorIntrinsicInParam v);    
        RW_MATH_FORCE_INLINE operator       VectorIntrinsic&();
        RW_MATH_FORCE_INLINE operator const VectorIntrinsic&() const;
    #endif
    
    RW_MATH_FORCE_INLINE void          SetVector(VectorIntrinsicInParam v);
    
    RW_MATH_FORCE_INLINE       VectorIntrinsic& GetVector();
    RW_MATH_FORCE_INLINE const VectorIntrinsic& GetVector() const;
    
    template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
    RW_MATH_FORCE_INLINE void Set(VecFloatRef<XSRC_INDEX> x, VecFloatRef<YSRC_INDEX> y, VecFloatRef<ZSRC_INDEX> z, VecFloatRef<WSRC_INDEX> plus);
    RW_MATH_FORCE_INLINE void Set(float x, float y, float z, float plus);
    RW_MATH_FORCE_INLINE void Set(VecFloat::InParam x, VecFloat::InParam y, VecFloat::InParam z, VecFloat::InParam plus);
    RW_MATH_FORCE_INLINE void Set(VecFloatRefIndex::InParam x, VecFloatRefIndex::InParam y, VecFloatRefIndex::InParam z, VecFloatRefIndex::InParam plus);
    RW_MATH_FORCE_INLINE void Set(Vector3::InParam v3, VecFloat::InParam userData);

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

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetZ(VecFloatRef<INDEX> z);
    RW_MATH_FORCE_INLINE void SetZ(float z);
    RW_MATH_FORCE_INLINE void SetZ(VecFloat::InParam z);
    RW_MATH_FORCE_INLINE void SetZ(VecFloatRefIndex::InParam z);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetPlus(VecFloatRef<INDEX> w);
    RW_MATH_FORCE_INLINE void SetPlus(float w);
    RW_MATH_FORCE_INLINE void SetPlus(VecFloat::InParam w);
    RW_MATH_FORCE_INLINE void SetPlus(VecFloatRefIndex::InParam w);
    
    RW_MATH_FORCE_INLINE void SetZero();
    
    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetComponent(int i);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetComponent(int i) const;
    
    RW_MATH_FORCE_INLINE       VecFloatRefIndex operator [] (int i);    
    RW_MATH_FORCE_INLINE const VecFloatRefIndex operator [] (int i) const;

    RW_MATH_FORCE_INLINE void SetVector3(Vector3::InParam v3);

    RW_MATH_FORCE_INLINE Vector3 GetVector3() const;
    
    #ifdef RW_MATH_RETURN_FLOAT
        RW_MATH_FORCE_INLINE const float& GetX() const{ return mV.x; }
        RW_MATH_FORCE_INLINE       float& GetX(){ return mV.x; }
        RW_MATH_FORCE_INLINE const float& GetY() const{ return mV.y; }
        RW_MATH_FORCE_INLINE       float& GetY(){ return mV.y; }
        RW_MATH_FORCE_INLINE const float& GetZ() const{ return mV.z; }
        RW_MATH_FORCE_INLINE       float& GetZ(){ return mV.z; }
        RW_MATH_FORCE_INLINE const float& GetPlus() const{ return mV.w; }
        RW_MATH_FORCE_INLINE       float& GetPlus(){ return mV.w; }
        
        RW_MATH_FORCE_INLINE const float& X() const{ return mV.x; }
        RW_MATH_FORCE_INLINE       float& X(){ return mV.x; }
        RW_MATH_FORCE_INLINE const float& Y() const{ return mV.y; }
        RW_MATH_FORCE_INLINE       float& Y(){ return mV.y; }
        RW_MATH_FORCE_INLINE const float& Z() const{ return mV.z; }
        RW_MATH_FORCE_INLINE       float& Z(){ return mV.z; }
        RW_MATH_FORCE_INLINE const float& UserData() const{ return mV.w; }
        RW_MATH_FORCE_INLINE       float& UserData(){ return mV.w; }
    #else
        RW_MATH_FORCE_INLINE const VecFloatRefX GetX() const;
        RW_MATH_FORCE_INLINE       VecFloatRefX GetX();
        RW_MATH_FORCE_INLINE const VecFloatRefY GetY() const;
        RW_MATH_FORCE_INLINE       VecFloatRefY GetY();
        RW_MATH_FORCE_INLINE const VecFloatRefZ GetZ() const;
        RW_MATH_FORCE_INLINE       VecFloatRefZ GetZ();
        RW_MATH_FORCE_INLINE const VecFloatRefW GetPlus() const;
        RW_MATH_FORCE_INLINE       VecFloatRefW GetPlus();
        
        RW_MATH_FORCE_INLINE const VecFloatRefX X() const;
        RW_MATH_FORCE_INLINE       VecFloatRefX X();
        RW_MATH_FORCE_INLINE const VecFloatRefY Y() const;
        RW_MATH_FORCE_INLINE       VecFloatRefY Y();
        RW_MATH_FORCE_INLINE const VecFloatRefZ Z() const;
        RW_MATH_FORCE_INLINE       VecFloatRefZ Z();
        RW_MATH_FORCE_INLINE const VecFloatRefW UserData() const;
        RW_MATH_FORCE_INLINE       VecFloatRefW UserData();
    #endif

public:

    VectorIntrinsic  mV;
};

}

using namespace ::rw::math::vpu;

}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Vector3Plus& t);
}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/vector3plus_type_inline.h"
#endif

namespace rw {
namespace math {
namespace fpu {

    // NOTE defining this constructor here vs. fpu/quatpos.h avoids ps3 compiler error.
    // This also allow us to remove the dependency of fpu header on vpu header
    template<class T>
    Vector3PlusTemplate<T>::Vector3PlusTemplate(const rw::math::vpu::Vector3Plus &v)
    {
        this->x = static_cast<T>(v.X());
        this->y = static_cast<T>(v.Y());
        this->z = static_cast<T>(v.Z());
        this->w = static_cast<T>(v.UserData());
    }
}}}

#endif
