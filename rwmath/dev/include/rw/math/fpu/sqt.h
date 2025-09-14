// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_SQT_H
#define RW_MATH_FPU_SQT_H

#if defined _MSC_VER
    #pragma once
#endif


#include "rw/math/fpu/quaternion.h"
#include "rw/math/fpu/vector3.h"
#include "rw/math/fpu/vec_float.h"
#include "rw/math/types_fwd.h"

namespace rw {
namespace math {
namespace fpu {

template <class T> 
class SqtTemplate
{
public:
#ifdef RWCROSS
    // To avoid any potential impact on Renderware ACP code, the FPU types
    // claim to be unaligned.  On standard builds, the FPU types are aligned
    // according to the required alignement specified by the compiler.
    enum { IsAligned = 0 };
    enum { Alignment = 1 };
#else
    enum { IsAligned = 1 };
    enum { Alignment = EA_ALIGN_OF(T) };
#endif
    enum { NumberOfElement = 10 };
    typedef const SqtTemplate& InParam;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef SqtTemplate& OutParam;
        typedef SqtTemplate& InOutParam;
    #endif

    RW_MATH_INLINE SqtTemplate(const SqtTemplate<float> &v)
    {
        this->scale = Vector3Template<T>(v.scale);
        this->quat = QuaternionTemplate<T>(v.quat);
        this->trans = Vector3Template<T>(v.trans);
    }

    RW_MATH_INLINE SqtTemplate(const SqtTemplate<double> &v)
    {
        this->scale = Vector3Template<T>(v.scale);
        this->quat = QuaternionTemplate<T>(v.quat);
        this->trans = Vector3Template<T>(v.trans);
    }

    explicit RW_MATH_INLINE SqtTemplate(const rw::math::vpu::Sqt &v);

    RW_MATH_INLINE SqtTemplate()
    {
    }

    RW_MATH_INLINE SqtTemplate(const Vector3Template<T>& s, const QuaternionTemplate<T>& q, const Vector3Template<T>& t)
        :scale(s)
        ,quat(q)
        ,trans(t)
    {
    }

    RW_MATH_INLINE SqtTemplate(T scaleX, T scaleY, T scaleZ, T quatX, T quatY, T quatZ, T quatW, T transX, T transY, T transZ)
        :scale(scaleX, scaleY, scaleZ)
        ,quat( quatX, quatY, quatZ, quatW )
        ,trans(transX, transY, transZ)
    {
    }

    explicit RW_MATH_INLINE SqtTemplate(T* xyz_xyzw_xyz )
        :scale(xyz_xyzw_xyz)
        ,quat(&xyz_xyzw_xyz[3])
        ,trans(&xyz_xyzw_xyz[7])
    {
    }

    RW_MATH_INLINE void Set(T scaleX, T scaleY, T scaleZ, T quatX, T quatY, T quatZ, T quatW, T transX, T transY, T transZ)
    {
        scale.Set(scaleX, scaleY, scaleZ);
        quat.Set( quatX, quatY, quatZ, quatW );
        trans.Set( transX, transY, transZ );
    }

    RW_MATH_INLINE void SetScale(const Vector3Template<T>& s)
    {
        scale = s;
    }

    RW_MATH_INLINE void SetQuaternion(const QuaternionTemplate<T>& q)
    {
        quat = q;
    }

    RW_MATH_INLINE void SetTranslation(const Vector3Template<T>& t)
    {
        trans = t;
    }

    RW_MATH_INLINE void SetZero() { Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));}

    RW_MATH_INLINE void SetIdentity() { Set(static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));}

    RW_MATH_INLINE Vector3Template<T>& GetScale()
    {
        return scale;
    }

    RW_MATH_INLINE const Vector3Template<T>& GetScale() const
    {
        return scale;
    }

    RW_MATH_INLINE QuaternionTemplate<T>& GetQuaternion()
    {
        return quat;
    }

    RW_MATH_INLINE const QuaternionTemplate<T>& GetQuaternion() const
    {
        return quat;
    }

    RW_MATH_INLINE Vector3Template<T>& GetTranslation()
    {
        return trans;
    }

    RW_MATH_INLINE const Vector3Template<T>& GetTranslation() const
    {
        return trans;
    }

public:

    Vector3Template<T>           scale;
    QuaternionTemplate<T>        quat;
    Vector3Template<T>           trans;
};


typedef SqtTemplate<float>    Sqt;

typedef SqtTemplate<float>    Sqt_32;
typedef SqtTemplate<double>   Sqt_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef SqtTemplate<float>   SqtU_32;
    //! \internal
    typedef SqtTemplate<double>  SqtU_64;
#endif

}
}

/*!
 *  Warning: This function \e conditionally endian swaps the class's data.
 *
 *  Normally this function is a no-op.  See \ref p_endian_swapping for details
 *  regarding when endian swapping will actually occur.
 */
template <class T>
inline void EndianSwap(rw::math::fpu::SqtTemplate<T>& t)
{
    EndianSwap(t.scale);
    EndianSwap(t.quat);
    EndianSwap(t.trans);
}

}

#endif
