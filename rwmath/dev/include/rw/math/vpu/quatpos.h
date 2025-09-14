// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_QUATPOS_H
#define RW_MATH_VPU_QUATPOS_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/types_fwd.h"
#include "rw/math/detail/base.h"
#include "rw/math/vpu/vector_intrinsic.h"
#include "rw/math/vpu/vec_float.h"
#include "rw/math/vpu/quaternion.h"
#include "rw/math/vpu/vector3.h"

namespace rw {
namespace math {
namespace vpu {

/*!
    \brief Represents a quaternion with position

    The QuatPos class is implemented using hardware efficient vector types.
    Therefore, the QuatPos should always be aligned on a 16byte boundary.
*/
class QuatPos
{
public:
    enum { IsAligned = 1 };
    enum { Alignment = 16 };
    enum { NumberOfElement = 7 };

    #ifdef RW_MATH_MATRIX_PASS_BY_VALUE
        typedef const QuatPos InParam;    
    #else
        typedef const QuatPos&  InParam;            
    #endif
    
    #if RW_MATH_ENABLE_DEPRECATED_API
        typedef QuatPos& OutParam;
        typedef QuatPos& InOutParam;
    #endif

public:
    
    template<class T>
    explicit RW_MATH_FORCE_INLINE QuatPos(const fpu::QuatPosTemplate<T> &v);
    RW_MATH_FORCE_INLINE QuatPos();    
    RW_MATH_FORCE_INLINE QuatPos(const QuatPos& rhs);    
    RW_MATH_FORCE_INLINE QuatPos(Quaternion::InParam quat, Vector3::InParam pos);    
    RW_MATH_FORCE_INLINE QuatPos(VectorIntrinsicInParam quat, VectorIntrinsicInParam pos);    
    RW_MATH_FORCE_INLINE QuatPos(float quatX, float quatY, float quatZ, float quatW, float posX, float posY, float posZ);
    //! Construct a QuatPos from an array of floats.
    //!
    //! The array must be aligned to a 16byte boundary for hardware efficiency reasons.
    RW_MATH_FORCE_INLINE explicit QuatPos( const float xyzw_xyz[7] );
    RW_MATH_FORCE_INLINE QuatPos(VecFloat::InParam quatX, VecFloat::InParam quatY, VecFloat::InParam quatZ, VecFloat::InParam quatW, VecFloat::InParam posX, VecFloat::InParam posY, VecFloat::InParam posZ);
    RW_MATH_FORCE_INLINE QuatPos(VecFloatRefIndex::InParam quatX, VecFloatRefIndex::InParam quatY, VecFloatRefIndex::InParam quatZ, VecFloatRefIndex::InParam quatW, VecFloatRefIndex::InParam posX, VecFloatRefIndex::InParam posY, VecFloatRefIndex::InParam posZ);

    template<VectorAxis QUAT_XSRC_INDEX, VectorAxis QUAT_YSRC_INDEX, VectorAxis QUAT_ZSRC_INDEX, VectorAxis QUAT_WSRC_INDEX, VectorAxis POS_XSRC_INDEX, VectorAxis POS_YSRC_INDEX, VectorAxis POS_ZSRC_INDEX>
    RW_MATH_FORCE_INLINE QuatPos(VecFloatRef<QUAT_XSRC_INDEX> quatX, VecFloatRef<QUAT_YSRC_INDEX> quatY, VecFloatRef<QUAT_ZSRC_INDEX> quatZ, VecFloatRef<QUAT_WSRC_INDEX> quatW, VecFloatRef<POS_XSRC_INDEX> posX, VecFloatRef<POS_YSRC_INDEX> posY, VecFloatRef<POS_ZSRC_INDEX> posZ);

    RW_MATH_FORCE_INLINE QuatPos& operator = (QuatPos::InParam q);
    
    RW_MATH_FORCE_INLINE void Set(float quatX, float quatY, float quatZ, float quatW, float posX, float posY, float posZ);    
    RW_MATH_FORCE_INLINE void Set(VecFloat::InParam quatX, VecFloat::InParam quatY, VecFloat::InParam quatZ, VecFloat::InParam quatW, VecFloat::InParam posX, VecFloat::InParam posY, VecFloat::InParam posZ);    
    RW_MATH_FORCE_INLINE void Set(VecFloatRefIndex::InParam quatX, VecFloatRefIndex::InParam quatY, VecFloatRefIndex::InParam quatZ, VecFloatRefIndex::InParam quatW, VecFloatRefIndex::InParam posX, VecFloatRefIndex::InParam posY, VecFloatRefIndex::InParam posZ);
    
    template<VectorAxis QUAT_XSRC_INDEX, VectorAxis QUAT_YSRC_INDEX, VectorAxis QUAT_ZSRC_INDEX, VectorAxis QUAT_WSRC_INDEX, VectorAxis POS_XSRC_INDEX, VectorAxis POS_YSRC_INDEX, VectorAxis POS_ZSRC_INDEX>
    RW_MATH_FORCE_INLINE void Set(VecFloatRef<QUAT_XSRC_INDEX> quatX, VecFloatRef<QUAT_YSRC_INDEX> quatY, VecFloatRef<QUAT_ZSRC_INDEX> quatZ, VecFloatRef<QUAT_WSRC_INDEX> quatW, VecFloatRef<POS_XSRC_INDEX> posX, VecFloatRef<POS_YSRC_INDEX> posY, VecFloatRef<POS_ZSRC_INDEX> posZ);    
    RW_MATH_FORCE_INLINE void SetQuaternion(Quaternion::InParam q);    
    RW_MATH_FORCE_INLINE void SetPos(Vector3::InParam v);    
    RW_MATH_FORCE_INLINE void SetZero();
    RW_MATH_FORCE_INLINE void SetIdentity();

    RW_MATH_FORCE_INLINE       Quaternion& GetQuaternion();
    RW_MATH_FORCE_INLINE const Quaternion& GetQuaternion() const;
    
    RW_MATH_FORCE_INLINE       Vector3& GetPos();    
    RW_MATH_FORCE_INLINE const Vector3& GetPos() const;

public:

    Quaternion  quat;
    Vector3     pos;
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
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::QuatPos& t);

}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/quatpos_type_inline.h"
#endif

namespace rw {
namespace math {
namespace fpu {

    // NOTE defining this constructor here vs. fpu/quatpos.h avoids ps3 compiler error.
    // This also allow us to remove the dependency of fpu header on vpu header
    template<class T>
    QuatPosTemplate<T>::QuatPosTemplate(const rw::math::vpu::QuatPos &v)
    {
        this->quat = QuaternionTemplate<T>(v.quat);
        this->pos = Vector3Template<T>(v.pos);
    }

}}}

#endif
