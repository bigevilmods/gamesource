// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VPU_SQT_H
#define RW_MATH_VPU_SQT_H

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
    \brief Represents a combination of scale, quaternion and translation

    The sqt class is implemented using hardware efficient vector types.
    Therefore, the sqt should always be aligned on a 16byte boundary.
*/
class Sqt
{
public:
    enum { IsAligned = 1 };
    enum { Alignment = 16 };
    enum { NumberOfElement = 10 };

    #ifdef RW_MATH_MATRIX_PASS_BY_VALUE
        typedef const Sqt InParam;    
    #else
        typedef const Sqt&  InParam;            
    #endif

public:
    
    template<class T>
    explicit RW_MATH_FORCE_INLINE Sqt(const fpu::SqtTemplate<T> &v);
    RW_MATH_FORCE_INLINE Sqt();    
    RW_MATH_FORCE_INLINE Sqt(const Sqt& rhs);    
    RW_MATH_FORCE_INLINE Sqt(Vector3::InParam s, Quaternion::InParam q, Vector3::InParam t);    
    RW_MATH_FORCE_INLINE Sqt(VectorIntrinsicInParam s, VectorIntrinsicInParam q, VectorIntrinsicInParam t);    
    RW_MATH_FORCE_INLINE Sqt(float scaleX, float scaleY, float scaleZ, float quatX, float quatY, float quatZ, float quatW, float transX, float transY, float transZ);
    //! Construct a Sqt from an array of floats.
    //!
    //! The array must be aligned to a 16byte boundary for hardware efficiency reasons.
    RW_MATH_FORCE_INLINE explicit Sqt( const float xyz_xyzw_xyz[10] );
    RW_MATH_FORCE_INLINE Sqt(VecFloat::InParam scaleX, VecFloat::InParam scaleY, VecFloat::InParam scaleZ, VecFloat::InParam quatX, VecFloat::InParam quatY, VecFloat::InParam quatZ, VecFloat::InParam quatW, VecFloat::InParam transX, VecFloat::InParam transY, VecFloat::InParam transZ);
    RW_MATH_FORCE_INLINE Sqt(VecFloatRefIndex::InParam scaleX, VecFloatRefIndex::InParam scaleY, VecFloatRefIndex::InParam scaleZ, VecFloatRefIndex::InParam quatX, VecFloatRefIndex::InParam quatY, VecFloatRefIndex::InParam quatZ, VecFloatRefIndex::InParam quatW, VecFloatRefIndex::InParam transX, VecFloatRefIndex::InParam transY, VecFloatRefIndex::InParam transZ);

    template<VectorAxis S_X_INDEX, VectorAxis S_Y_INDEX, VectorAxis S_Z_INDEX, VectorAxis R_X_INDEX, VectorAxis R_Y_INDEX, VectorAxis R_Z_INDEX, VectorAxis R_W_INDEX, VectorAxis T_X_INDEX, VectorAxis T_Y_INDEX, VectorAxis T_Z_INDEX>
    RW_MATH_FORCE_INLINE Sqt(VecFloatRef<S_X_INDEX> scaleX, VecFloatRef<S_Y_INDEX> scaleY, VecFloatRef<S_Z_INDEX> scaleZ, VecFloatRef<R_X_INDEX> quatX, VecFloatRef<R_Y_INDEX> quatY, VecFloatRef<R_Z_INDEX> quatZ, VecFloatRef<R_W_INDEX> quatW, VecFloatRef<T_X_INDEX> transX, VecFloatRef<T_Y_INDEX> transY, VecFloatRef<T_Z_INDEX> transZ);

    RW_MATH_FORCE_INLINE Sqt& operator = (Sqt::InParam s);
    
    RW_MATH_FORCE_INLINE void Set(float scaleX, float scaleY, float scaleZ, float quatX, float quatY, float quatZ, float quatW, float transX, float transY, float transZ);    
    RW_MATH_FORCE_INLINE void Set(VecFloat::InParam scaleX, VecFloat::InParam scaleY, VecFloat::InParam scaleZ, VecFloat::InParam quatX, VecFloat::InParam quatY, VecFloat::InParam quatZ, VecFloat::InParam quatW, VecFloat::InParam transX, VecFloat::InParam transY, VecFloat::InParam transZ);    
    RW_MATH_FORCE_INLINE void Set(VecFloatRefIndex::InParam scaleX, VecFloatRefIndex::InParam scaleY, VecFloatRefIndex::InParam scaleZ, VecFloatRefIndex::InParam quatX, VecFloatRefIndex::InParam quatY, VecFloatRefIndex::InParam quatZ, VecFloatRefIndex::InParam quatW, VecFloatRefIndex::InParam transX, VecFloatRefIndex::InParam transY, VecFloatRefIndex::InParam transZ);
    
    template<VectorAxis S_X_INDEX, VectorAxis S_Y_INDEX, VectorAxis S_Z_INDEX, VectorAxis R_X_INDEX, VectorAxis R_Y_INDEX, VectorAxis R_Z_INDEX, VectorAxis R_W_INDEX, VectorAxis T_X_INDEX, VectorAxis T_Y_INDEX, VectorAxis T_Z_INDEX>
    RW_MATH_FORCE_INLINE void Set(VecFloatRef<S_X_INDEX> scaleX, VecFloatRef<S_Y_INDEX> scaleY, VecFloatRef<S_Z_INDEX> scaleZ, VecFloatRef<R_X_INDEX> quatX, VecFloatRef<R_Y_INDEX> quatY, VecFloatRef<R_Z_INDEX> quatZ, VecFloatRef<R_W_INDEX> quatW, VecFloatRef<T_X_INDEX> transX, VecFloatRef<T_Y_INDEX> transY, VecFloatRef<T_Z_INDEX> transZ);
    RW_MATH_FORCE_INLINE void SetScale(Vector3::InParam v);
    RW_MATH_FORCE_INLINE void SetQuaternion(Quaternion::InParam q);    
    RW_MATH_FORCE_INLINE void SetTranslation(Vector3::InParam v);    
    RW_MATH_FORCE_INLINE void SetZero();
    RW_MATH_FORCE_INLINE void SetIdentity();

    RW_MATH_FORCE_INLINE       Vector3& GetScale();    
    RW_MATH_FORCE_INLINE const Vector3& GetScale() const;

    RW_MATH_FORCE_INLINE       Quaternion& GetQuaternion();
    RW_MATH_FORCE_INLINE const Quaternion& GetQuaternion() const;
    
    RW_MATH_FORCE_INLINE       Vector3& GetTranslation();    
    RW_MATH_FORCE_INLINE const Vector3& GetTranslation() const;

public:

    Vector3     scale;
    Quaternion  quat;
    Vector3     trans;
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
RW_MATH_FORCE_INLINE void EndianSwap(rw::math::vpu::Sqt& t);

}

#ifndef RW_MATH_VPU_NO_INLINE_FUNCTIONS
    #include "rw/math/vpu/detail/sqt_type_inline.h"
#endif

namespace rw {
namespace math {
namespace fpu {

    // NOTE defining this constructor here vs. fpu/sqt.h avoids ps3 compiler error.
    // This also allow us to remove the dependency of fpu header on vpu header
    template<class T>
    SqtTemplate<T>::SqtTemplate(const rw::math::vpu::Sqt &v)
    {
        this->scale = Vector3Template<T>(v.scale);
        this->quat = QuaternionTemplate<T>(v.quat);
        this->trans = Vector3Template<T>(v.trans);
    }

}}}

#endif
