// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_VEC_FLOAT_H
#define RW_MATH_VPU_VEC_FLOAT_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/detail/base.h"

#include "rw/math/types_fwd.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vector_intrinsic_operation.h"
#include "rw/math/vpu/vector_axis.h"

namespace rw {
namespace math {
namespace vpu {

    class VecFloat;
    class VecFloatRefIndex;
    template<VectorAxis INDEX> class VecFloatRef;

    typedef VecFloatRef<VectorAxisX> VecFloatRefX;
    typedef VecFloatRef<VectorAxisY> VecFloatRefY;
    typedef VecFloatRef<VectorAxisZ> VecFloatRefZ;
    typedef VecFloatRef<VectorAxisW> VecFloatRefW;

/*!
    \brief The VecFloat class represents a float stored in intrinsic vector type.

    The VecFloat class is implemented using hardware efficient vector types.
    Therefore, the VecFloat should always be aligned on a 16 byte boundary.
    This is optimal for vector calculation where memory access from the float
    register from/to the vector register is expensive.
*/
#if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
    __declspec(passinreg) 
#endif
class VecFloat
{
public:

    #ifdef RW_MATH_VECTOR_PASS_BY_VALUE
        typedef const VecFloat InParam;
    #else
        #if defined EA_PLATFORM_XENON && defined RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
            typedef const VecFloat InParam;
        #else
            typedef const VecFloat& InParam;
        #endif
    #endif
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef VecFloat& OutParam;
        typedef VecFloat& InOutParam;
    #endif    

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE VecFloat(VecFloatRef<INDEX> v);
    RW_MATH_FORCE_INLINE VecFloat(VecFloatRefIndex v);
    RW_MATH_FORCE_INLINE VecFloat();
    #ifndef RW_MATH_DISABLE_VECTOR_COPY_CTOR_AND_ASSIGNMENT_OPERATOR
        RW_MATH_FORCE_INLINE VecFloat(const VecFloat& v);
        RW_MATH_FORCE_INLINE VecFloat& operator =(const VecFloat& f);
    #endif
    RW_MATH_FORCE_INLINE explicit VecFloat(VectorIntrinsicInParam v);
    RW_MATH_FORCE_INLINE VecFloat(VectorIntrinsicInParam v, int index);

    #ifndef RW_MATH_DISABLE_VECFLOAT_IMPLICIT_FLOAT_CTOR
        RW_MATH_FORCE_INLINE VecFloat(const float &f);
    #else
        RW_MATH_FORCE_INLINE explicit VecFloat(const float &f);
    #endif

    RW_MATH_FORCE_INLINE float GetFloat() const;
    RW_MATH_FORCE_INLINE void SetFloat(const float& f);
    RW_MATH_FORCE_INLINE const VectorIntrinsic& GetVector() const;
    #ifndef RW_MATH_DISABLE_VECFLOAT_IMPLICIT_FLOAT_CONVERSION_OPERATOR
        RW_MATH_FORCE_INLINE operator float() const;
    #endif

    // This method may be useful in avoiding LHS's on some platforms.
    RW_MATH_FORCE_INLINE void WriteToFloat(float* f) const;

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE VecFloat& operator =(const VecFloatRef<INDEX>& v);
    RW_MATH_FORCE_INLINE VecFloat& operator =(const float& f);
    RW_MATH_FORCE_INLINE VecFloat& operator =(const VecFloatRefIndex& v);

public:

    // member data is public accessible because
    // 1) it is less likely it will change
    // 2) it make end user access to the data without function call overhead
    VectorIntrinsic  mV;
};

typedef VecFloat::InParam VecFloatInParam;
typedef VecFloat::OutParam VecFloatOutParam;
 


/*!
    \brief The VecFloatRef class represents a float reference stored in the original vector register.
    
    The VecFloatRef class refers to a specific floating point element stored
    in a vector.  VecFloatRef is a proxy class that allows compile time
    reference to a specific element of a vector register.
*/
template<VectorAxis INDEX>
class VecFloatRef
{
public:

    #ifdef RW_MATH_VECTOR_PASS_BY_VALUE
        typedef const VecFloatRef InParam;
    #else
        typedef const VecFloatRef&      InParam;
    #endif

    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef VecFloatRef& OutParam;
        typedef VecFloatRef& InOutParam;
    #endif

    RW_MATH_FORCE_INLINE VecFloatRef(VectorIntrinsic& v);
    RW_MATH_FORCE_INLINE VecFloatRef(const VecFloatRef<INDEX> &v);

    RW_MATH_FORCE_INLINE float GetFloat() const;
    RW_MATH_FORCE_INLINE void SetFloat(float f);
    RW_MATH_FORCE_INLINE const VectorIntrinsic& GetVector() const;
    RW_MATH_FORCE_INLINE int GetIndex() const;
    
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator =(VecFloatRefX rhs);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator =(VecFloatRefY rhs);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator =(VecFloatRefZ rhs);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator =(VecFloatRefW rhs);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator =(VecFloatRefIndex rhs);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator =(VecFloat::InParam rhs);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator =(float f);

    #ifndef RW_MATH_DISABLE_VECFLOAT_IMPLICIT_FLOAT_CONVERSION_OPERATOR
        RW_MATH_FORCE_INLINE operator float() const;
    #endif
    
    template<VectorAxis INDEX_OTHER>
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator += (VecFloatRef<INDEX_OTHER> b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator += (VecFloat::InParam b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator += (const VecFloatRefIndex& b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator += (const float& b);
    
    template<VectorAxis INDEX_OTHER>
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator /= (VecFloatRef<INDEX_OTHER> b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator /= (VecFloat::InParam b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator /= (const VecFloatRefIndex& b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator /= (const float& b);
    
    template<VectorAxis INDEX_OTHER>
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator -= (VecFloatRef<INDEX_OTHER> b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator -= (VecFloat::InParam b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator -= (const VecFloatRefIndex& b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator -= (const float& b);
    
    template<VectorAxis INDEX_OTHER>
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator *= (VecFloatRef<INDEX_OTHER> b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator *= (VecFloat::InParam b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator *= (const VecFloatRefIndex& b);
    RW_MATH_FORCE_INLINE VecFloatRef<INDEX>& operator *= (const float& b);

public:

    VectorIntrinsic &mV;

    enum 
    {
        mIndex = INDEX
    };

};

/*!

    \brief The VecFloatRefIndex class represents a float reference stored in the original vector register.

    This proxy class allows runtime reference to a specific element of
    vector register via an index. If this class is used properly, compiler
    should optimize out the reference of the vector and only uses the
    index integer.  If you are able to determine the index at compile
    time you should use the VecFloat<INDEX> class instead for better
    performance.
*/
class VecFloatRefIndex
{
public:

    typedef const VecFloatRefIndex& InParam;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef VecFloatRefIndex& OutParam;
        typedef VecFloatRefIndex& InOutParam;
    #endif

    RW_MATH_FORCE_INLINE VecFloatRefIndex(VectorIntrinsic& v, int index);
    
    RW_MATH_FORCE_INLINE VecFloatRefIndex(const VecFloatRefIndex &v);

    RW_MATH_FORCE_INLINE float GetFloat() const;
    RW_MATH_FORCE_INLINE void SetFloat(float f);
    RW_MATH_FORCE_INLINE const VectorIntrinsic& GetVector() const;
    RW_MATH_FORCE_INLINE int GetIndex() const;

    template<VectorAxis INDEX>
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator =(VecFloatRef<INDEX> rhs);
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator =(VecFloatRefIndex rhs);
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator =(VecFloat::InParam rhs);
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator =(float f);

    #ifndef RW_MATH_DISABLE_VECFLOAT_IMPLICIT_FLOAT_CONVERSION_OPERATOR
        RW_MATH_FORCE_INLINE operator float() const;
    #endif
    
    template<VectorAxis INDEX1>
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator += (VecFloatRef<INDEX1> b);    
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator += (VecFloat::InParam b);
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator += (VecFloatRefIndex::InParam b);    
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator += (const float& b);
    
    template<VectorAxis INDEX1>
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator -= (VecFloatRef<INDEX1> b);    
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator -= (VecFloat::InParam b);
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator -= (VecFloatRefIndex::InParam b);    
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator -= (const float& b);
    
    template<VectorAxis INDEX1>
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator *= (VecFloatRef<INDEX1> b);    
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator *= (VecFloat::InParam b);
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator *= (VecFloatRefIndex::InParam b);    
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator *= (const float& b);
    
    template<VectorAxis INDEX1>
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator /= (VecFloatRef<INDEX1> b);    
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator /= (VecFloat::InParam b);
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator /= (VecFloatRefIndex::InParam b);    
    RW_MATH_FORCE_INLINE VecFloatRefIndex& operator /= (const float& b);

public:

    VectorIntrinsic &mV;
    VectorAxis mIndex;
};

template<VectorAxis AXIS>
RW_MATH_FORCE_INLINE float GetFloat(const VecFloatRef<AXIS>& v);
RW_MATH_FORCE_INLINE float GetFloat(const VecFloatRefIndex& v);
RW_MATH_FORCE_INLINE float GetFloat(const VecFloat& v);
RW_MATH_FORCE_INLINE float GetFloat(float f);

template<VectorAxis AXIS>
RW_MATH_FORCE_INLINE float GetFloat(const VecFloatRef<AXIS>& v) { return v.GetFloat(); }
RW_MATH_FORCE_INLINE float GetFloat(const VecFloatRefIndex& v) { return v.GetFloat(); }
RW_MATH_FORCE_INLINE float GetFloat(const VecFloat& v) { return v.GetFloat(); }
RW_MATH_FORCE_INLINE float GetFloat(float f) { return f; }

}}}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/vec_float_type_inline.h"
#endif

#endif
