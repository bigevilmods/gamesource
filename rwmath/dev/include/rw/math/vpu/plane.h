// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_PLANE_H
#define RW_MATH_VPU_PLANE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"
#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vec_float.h"
#include "rw/math/vpu/vector3.h"
#include "rw/math/fpu/plane.h"

namespace rw {
namespace math {
namespace vpu {

using ::rw::math::fpu::PlaneIntersection;
using ::rw::math::fpu::PI_ZERO;
using ::rw::math::fpu::PI_ONE;
using ::rw::math::fpu::PI_INFINITY;

/*!
    \brief Represents the plane equation.
    
    The A, B, C, D coefficients stored by the Plane class represent a plane equation of the form Ax + By + Cz + D = 0

    The Plane class is implemented using hardware efficient vector types.
    Therefore, the Plane should always be aligned on a 16 byte boundary.
*/
class Plane
{
public:

    enum { IsAligned = 1 };
    enum { Alignment = 16 };
    enum { NumberOfElement = 4 };
    
    #ifdef RW_MATH_VECTOR_PASS_BY_VALUE
        typedef const Plane InParam;        
    #else
        typedef const Plane& InParam;
    #endif 
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef Plane& OutParam;
        typedef Plane& InOutParam;
    #endif
    
    RW_MATH_FORCE_INLINE Plane();

    template<class T>
    explicit Plane(const math::fpu::PlaneTemplate<T> &v);    
    RW_MATH_FORCE_INLINE Plane(const Plane& rhs);
    RW_MATH_FORCE_INLINE Plane(Vector3::InParam normal, VecFloat::InParam d);
    //! Construct a Plane from an array of floats.
    //!
    //! The array must be aligned to a 16byte boundary for hardware efficiency reasons.
    RW_MATH_FORCE_INLINE explicit Plane(const float abcd[4]);
    RW_MATH_FORCE_INLINE RW_MATH_FUTURE_EXPLICIT Plane(VectorIntrinsicInParam v);
    RW_MATH_FORCE_INLINE Plane(float a, float b, float c, float d);
    RW_MATH_FORCE_INLINE Plane(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam c, VecFloat::InParam d);
    RW_MATH_FORCE_INLINE Plane(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b, VecFloatRefIndex::InParam c, VecFloatRefIndex::InParam d);
    template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
    RW_MATH_FORCE_INLINE Plane(VecFloatRef<XSRC_INDEX> a, VecFloatRef<YSRC_INDEX> b, VecFloatRef<ZSRC_INDEX> c, VecFloatRef<WSRC_INDEX> d);

    RW_MATH_FORCE_INLINE Plane& operator = (Plane::InParam v);
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        RW_MATH_FORCE_INLINE Plane& operator = (VectorIntrinsicInParam v);
        RW_MATH_FORCE_INLINE operator       VectorIntrinsic&();
        RW_MATH_FORCE_INLINE operator const VectorIntrinsic&() const;
    #endif

    RW_MATH_FORCE_INLINE void SetVector(VectorIntrinsicInParam v);

    RW_MATH_FORCE_INLINE       VectorIntrinsic& GetVector();
    RW_MATH_FORCE_INLINE const VectorIntrinsic& GetVector() const;

    RW_MATH_FORCE_INLINE void Set(Vector3::InParam normal, VecFloat::InParam d);
    
    template<VectorAxis XSRC_INDEX, VectorAxis YSRC_INDEX, VectorAxis ZSRC_INDEX, VectorAxis WSRC_INDEX>
    RW_MATH_FORCE_INLINE void Set(VecFloatRef<XSRC_INDEX> a, VecFloatRef<YSRC_INDEX> b, VecFloatRef<ZSRC_INDEX> c, VecFloatRef<WSRC_INDEX> d);
    RW_MATH_FORCE_INLINE void Set(float a, float b, float c, float d);
    RW_MATH_FORCE_INLINE void Set(VecFloat::InParam a, VecFloat::InParam b, VecFloat::InParam c, VecFloat::InParam d);
    RW_MATH_FORCE_INLINE void Set(VecFloatRefIndex::InParam a, VecFloatRefIndex::InParam b, VecFloatRefIndex::InParam c, VecFloatRefIndex::InParam d);

    RW_MATH_FORCE_INLINE void SetZero();

    RW_MATH_FORCE_INLINE void SetNormal(Vector3::InParam v);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetComponent(int i, VecFloatRef<INDEX> value);
    RW_MATH_FORCE_INLINE void SetComponent(int i, float value);
    RW_MATH_FORCE_INLINE void SetComponent(int i, VecFloat::InParam value);
    RW_MATH_FORCE_INLINE void SetComponent(int i, VecFloatRefIndex::InParam value);
   
    template<VectorAxis INDEX> 
    RW_MATH_FORCE_INLINE void SetA(VecFloatRef<INDEX> a);
    RW_MATH_FORCE_INLINE void SetA(float a);
    RW_MATH_FORCE_INLINE void SetA(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE void SetA(VecFloatRefIndex::InParam a);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetB(VecFloatRef<INDEX> b);
    RW_MATH_FORCE_INLINE void SetB(float b);
    RW_MATH_FORCE_INLINE void SetB(VecFloat::InParam b);
    RW_MATH_FORCE_INLINE void SetB(VecFloatRefIndex::InParam b);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetC(VecFloatRef<INDEX> c);
    RW_MATH_FORCE_INLINE void SetC(float c);
    RW_MATH_FORCE_INLINE void SetC(VecFloat::InParam c);
    RW_MATH_FORCE_INLINE void SetC(VecFloatRefIndex::InParam c);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void SetD(VecFloatRef<INDEX> d);
    RW_MATH_FORCE_INLINE void SetD(float d);
    RW_MATH_FORCE_INLINE void SetD(VecFloat::InParam d);
    RW_MATH_FORCE_INLINE void SetD(VecFloatRefIndex::InParam d);    
    
    template<VectorAxis INDEX> 
    RW_MATH_FORCE_INLINE void A(VecFloatRef<INDEX> a);
    RW_MATH_FORCE_INLINE void A(float a);
    RW_MATH_FORCE_INLINE void A(VecFloat::InParam a);
    RW_MATH_FORCE_INLINE void A(VecFloatRefIndex::InParam a);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void B(VecFloatRef<INDEX> b);
    RW_MATH_FORCE_INLINE void B(float b);
    RW_MATH_FORCE_INLINE void B(VecFloat::InParam b);
    RW_MATH_FORCE_INLINE void B(VecFloatRefIndex::InParam b);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void C(VecFloatRef<INDEX> c);
    RW_MATH_FORCE_INLINE void C(float c);
    RW_MATH_FORCE_INLINE void C(VecFloat::InParam c);
    RW_MATH_FORCE_INLINE void C(VecFloatRefIndex::InParam c);

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE void D(VecFloatRef<INDEX> d);
    RW_MATH_FORCE_INLINE void D(float d);
    RW_MATH_FORCE_INLINE void D(VecFloat::InParam d);
    RW_MATH_FORCE_INLINE void D(VecFloatRefIndex::InParam d);

    RW_MATH_FORCE_INLINE const VecFloatRefX GetA() const;
    RW_MATH_FORCE_INLINE       VecFloatRefX GetA();
    RW_MATH_FORCE_INLINE const VecFloatRefY GetB() const;
    RW_MATH_FORCE_INLINE       VecFloatRefY GetB();
    RW_MATH_FORCE_INLINE const VecFloatRefZ GetC() const;
    RW_MATH_FORCE_INLINE       VecFloatRefZ GetC();
    RW_MATH_FORCE_INLINE const VecFloatRefW GetD() const;
    RW_MATH_FORCE_INLINE       VecFloatRefW GetD(); 
    
    RW_MATH_FORCE_INLINE const VecFloatRefX A() const;
    RW_MATH_FORCE_INLINE       VecFloatRefX A();
    RW_MATH_FORCE_INLINE const VecFloatRefY B() const;
    RW_MATH_FORCE_INLINE       VecFloatRefY B();
    RW_MATH_FORCE_INLINE const VecFloatRefZ C() const;
    RW_MATH_FORCE_INLINE       VecFloatRefZ C();
    RW_MATH_FORCE_INLINE const VecFloatRefW D() const;
    RW_MATH_FORCE_INLINE       VecFloatRefW D();
    
    RW_MATH_FORCE_INLINE       VecFloatRefIndex GetComponent(int i);    
    RW_MATH_FORCE_INLINE const VecFloatRefIndex GetComponent(int i) const;

    RW_MATH_FORCE_INLINE       VecFloatRefIndex operator [] (int i);
    RW_MATH_FORCE_INLINE const VecFloatRefIndex operator [] (int i) const;

    RW_MATH_FORCE_INLINE Vector3 GetNormal() const;

    RW_MATH_FORCE_INLINE       VecFloatRefW SignedDistance();
    RW_MATH_FORCE_INLINE const VecFloatRefW SignedDistance() const;

    RW_MATH_FORCE_INLINE VecFloat Distance() const;

public:

    VectorIntrinsic  mV;


    #if ! RW_MATH_ENABLE_DEPRECATED_API
        private:
        Plane& operator = (const vpu::Vector4& v);
        Plane& operator = (const vpu::Vector3& v);
        Plane& operator = (const vpu::Vector2& v);
        Plane& operator = (const vpu::Quaternion& v);
        explicit Plane(const vpu::Vector4& v);
        explicit Plane(const vpu::Vector3& v);
        explicit Plane(const vpu::Vector2& v);
        explicit Plane(const vpu::Quaternion& v);
    #endif 
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
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Plane& t);

}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/plane_type_inline.h"
#endif

namespace rw {
namespace math {
namespace fpu {

    // Note, defining this constructor here vs. fpu/plane.h avoids ps3 compiler error.
    // This also allow us to remove the dependency of fpu header on vpu header
    template <class T> 
    PlaneTemplate<T>::PlaneTemplate(const rw::math::vpu::Plane &v)
    {
        this->a = static_cast<T>(v.A());
        this->b = static_cast<T>(v.B());
        this->c = static_cast<T>(v.C());
        this->d = static_cast<T>(v.D());
    }
    
}}}

#endif
