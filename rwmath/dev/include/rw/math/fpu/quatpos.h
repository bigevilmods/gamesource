// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_QUATPOS_H
#define RW_MATH_FPU_QUATPOS_H

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
class QuatPosTemplate
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
    enum { NumberOfElement = 7 };
    typedef const QuatPosTemplate& InParam;
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef QuatPosTemplate& OutParam;
        typedef QuatPosTemplate& InOutParam;
    #endif

    RW_MATH_INLINE QuatPosTemplate(const QuatPosTemplate<float> &v)
    {
        this->quat = QuaternionTemplate<T>(v.quat);
        this->pos = Vector3Template<T>(v.pos);
    }

    RW_MATH_INLINE QuatPosTemplate(const QuatPosTemplate<double> &v)
    {
        this->quat = QuaternionTemplate<T>(v.quat);
        this->pos = Vector3Template<T>(v.pos);
    }

    explicit RW_MATH_INLINE QuatPosTemplate(const rw::math::vpu::QuatPos &v);

    RW_MATH_INLINE QuatPosTemplate()
    {
    }

    RW_MATH_INLINE QuatPosTemplate(const QuaternionTemplate<T>& q, const Vector3Template<T>& p)
        :quat(q)
        ,pos(p)
    {
    }

    RW_MATH_INLINE QuatPosTemplate(T quatX, T quatY, T quatZ, T quatW, T posX, T posY, T posZ)
        :quat( quatX, quatY, quatZ, quatW )
        ,pos(posX, posY, posZ)
    {
    }

    explicit RW_MATH_INLINE QuatPosTemplate(T* xyzw )
        :quat(xyzw)
        ,pos(&xyzw[4])
    {
    }

    RW_MATH_INLINE void Set(T quatX, T quatY, T quatZ, T quatW, T posX, T posY, T posZ)
    {
        quat.Set( quatX, quatY, quatZ, quatW );
        pos.Set( posX, posY, posZ );
    }

    RW_MATH_INLINE void SetQuaternion(const QuaternionTemplate<T>& q)
    {
        quat = q;
    }

    RW_MATH_INLINE void SetPos(const Vector3Template<T>& v)
    {
        pos = v;
    }

    RW_MATH_INLINE void SetZero() { Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));}

    RW_MATH_INLINE void SetIdentity() { Set(static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(1.0f), static_cast<T>(0.0f), static_cast<T>(0.0f), static_cast<T>(0.0f));}

    RW_MATH_INLINE QuaternionTemplate<T>& GetQuaternion()
    {
        return quat;
    }

    RW_MATH_INLINE const QuaternionTemplate<T>& GetQuaternion() const
    {
        return quat;
    }

    RW_MATH_INLINE Vector3Template<T> GetPos()
    {
        return pos;
    }

    RW_MATH_INLINE const Vector3Template<T> GetPos() const
    {
        return pos;
    }

public:

    QuaternionTemplate<T>        quat;
    Vector3Template<T>           pos;
};


typedef QuatPosTemplate<float>    QuatPos;

typedef QuatPosTemplate<float>    QuatPos_32;
typedef QuatPosTemplate<double>   QuatPos_64;

#if RW_MATH_ENABLE_DEPRECATED_API
    //! \internal
    typedef QuatPosTemplate<float>   QuatPosU_32;
    //! \internal
    typedef QuatPosTemplate<double>  QuatPosU_64;
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
inline void EndianSwap(rw::math::fpu::QuatPosTemplate<T>& t)
{
    EndianSwap(t.quat);
    EndianSwap(t.pos);
}

}

#endif
