// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_SIMULATED_VEC_OPERATION_INLINE_H
#define RW_MATH_SIMULATED_VEC_OPERATION_INLINE_H

#if defined _MSC_VER
    #pragma once
#endif

namespace rw
{
namespace math
{
namespace vpl
{
namespace detail
{
namespace simulated_vecops
{

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAbs(VectorIntrinsicInParam v0)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        r.scalarFloat[0] = (v0u.scalarFloat[0] < 0.0f) ? -v0u.scalarFloat[0] : v0u.scalarFloat[0];
        r.scalarFloat[1] = (v0u.scalarFloat[1] < 0.0f) ? -v0u.scalarFloat[1] : v0u.scalarFloat[1];
        r.scalarFloat[2] = (v0u.scalarFloat[2] < 0.0f) ? -v0u.scalarFloat[2] : v0u.scalarFloat[2];
        r.scalarFloat[3] = (v0u.scalarFloat[3] < 0.0f) ? -v0u.scalarFloat[3] : v0u.scalarFloat[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarFloat[0] = v0u.scalarFloat[0] + v1u.scalarFloat[0];
        r.scalarFloat[1] = v0u.scalarFloat[1] + v1u.scalarFloat[1];
        r.scalarFloat[2] = v0u.scalarFloat[2] + v1u.scalarFloat[2];
        r.scalarFloat[3] = v0u.scalarFloat[3] + v1u.scalarFloat[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAnd(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = v0u.scalarUInt[0] & v1u.scalarUInt[0];
        r.scalarUInt[1] = v0u.scalarUInt[1] & v1u.scalarUInt[1];
        r.scalarUInt[2] = v0u.scalarUInt[2] & v1u.scalarUInt[2];
        r.scalarUInt[3] = v0u.scalarUInt[3] & v1u.scalarUInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAndComplement(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = v0u.scalarUInt[0] & (~v1u.scalarUInt[0]);
        r.scalarUInt[1] = v0u.scalarUInt[1] & (~v1u.scalarUInt[1]);
        r.scalarUInt[2] = v0u.scalarUInt[2] & (~v1u.scalarUInt[2]);
        r.scalarUInt[3] = v0u.scalarUInt[3] & (~v1u.scalarUInt[3]);
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API bool VecAllTrue_Word(VectorIntrinsicInParam v)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion vu;
        vu.vector = v;
        return vu.scalarUInt[0] && vu.scalarUInt[1] && vu.scalarUInt[2] && vu.scalarUInt[3];
    }

    RW_MATH_FORCE_INLINE VPL_API bool VecAnyTrue_Word(VectorIntrinsicInParam v)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion vu;
        vu.vector = v;
        return vu.scalarUInt[0] || vu.scalarUInt[1] || vu.scalarUInt[2] || vu.scalarUInt[3];
    }

    RW_MATH_FORCE_INLINE VPL_API bool VecAllFalse_Word(VectorIntrinsicInParam v)
    {
        return !VecAnyTrue_Word(v);
    }

    RW_MATH_FORCE_INLINE VPL_API bool VecAnyFalse_Word(VectorIntrinsicInParam v)
    {
        return !VecAllTrue_Word(v);
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarFloat[0] == v1u.scalarFloat[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarFloat[1] == v1u.scalarFloat[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarFloat[2] == v1u.scalarFloat[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarFloat[3] == v1u.scalarFloat[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarFloat[0] >= v1u.scalarFloat[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarFloat[1] >= v1u.scalarFloat[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarFloat[2] >= v1u.scalarFloat[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarFloat[3] >= v1u.scalarFloat[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarFloat[0] > v1u.scalarFloat[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarFloat[1] > v1u.scalarFloat[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarFloat[2] > v1u.scalarFloat[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarFloat[3] > v1u.scalarFloat[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarFloat[0] <= v1u.scalarFloat[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarFloat[1] <= v1u.scalarFloat[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarFloat[2] <= v1u.scalarFloat[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarFloat[3] <= v1u.scalarFloat[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarFloat[0] < v1u.scalarFloat[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarFloat[1] < v1u.scalarFloat[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarFloat[2] < v1u.scalarFloat[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarFloat[3] < v1u.scalarFloat[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetOne()
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarFloat[0] = 1.0f;
        r.scalarFloat[1] = 1.0f;
        r.scalarFloat[2] = 1.0f;
        r.scalarFloat[3] = 1.0f;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetHalf()
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarFloat[0] = 0.5f;
        r.scalarFloat[1] = 0.5f;
        r.scalarFloat[2] = 0.5f;
        r.scalarFloat[3] = 0.5f;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetTwo()
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarFloat[0] = 2.0f;
        r.scalarFloat[1] = 2.0f;
        r.scalarFloat[2] = 2.0f;
        r.scalarFloat[3] = 2.0f;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetZero()
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarFloat[0] = 0.0f;
        r.scalarFloat[1] = 0.0f;
        r.scalarFloat[2] = 0.0f;
        r.scalarFloat[3] = 0.0f;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegOne()
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarFloat[0] = -1.0f;
        r.scalarFloat[1] = -1.0f;
        r.scalarFloat[2] = -1.0f;
        r.scalarFloat[3] = -1.0f;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegHalf()
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarFloat[0] = -0.5f;
        r.scalarFloat[1] = -0.5f;
        r.scalarFloat[2] = -0.5f;
        r.scalarFloat[3] = -0.5f;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegTwo()
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarFloat[0] = -2.0f;
        r.scalarFloat[1] = -2.0f;
        r.scalarFloat[2] = -2.0f;
        r.scalarFloat[3] = -2.0f;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegZero()
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarUInt[0] = 0x80000000;
        r.scalarUInt[1] = 0x80000000;
        r.scalarUInt[2] = 0x80000000;
        r.scalarUInt[3] = 0x80000000;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadLeftAligned(const void *base, int offset)
    {
        VectorIntrinsic result;
        const char *address = reinterpret_cast<const char *>(base) + offset;
        uintptr_t amountToCopy = 16 - (reinterpret_cast<uintptr_t>(address) & 15);
        char *resultPtr = reinterpret_cast<char *>(&result);
        uintptr_t i;
        for( i=0; i<amountToCopy; i++ )
        {
            *(resultPtr++) = *(address++);
        }
        for(;i<16;i++)
        {
            *(resultPtr++) = 0;
        }
        return result;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadRightAligned(const void *base, int offset)
    {
        VectorIntrinsic result;
        const char *address = reinterpret_cast<const char *>(base) + offset;
        char *srcAddress = reinterpret_cast<char *>( reinterpret_cast<uintptr_t>(address) & ~15 );
        uintptr_t amountToClear = 16 - (reinterpret_cast<uintptr_t>(address) & 15);
        char *resultPtr = reinterpret_cast<char *>(&result);
        uintptr_t i;
        for( i=0; i<amountToClear; i++ )
        {
            *(resultPtr++) = 0;
        }
        for(;i<16;i++)
        {
            *(resultPtr++) = *(srcAddress++);
        }
        return result;
    }


    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadUnaligned(const void *base, int offset)
    {
        VectorIntrinsic result;
        char *resultPtr = reinterpret_cast<char *>(&result);
        const char *address = reinterpret_cast<const char *>(base) + offset;
        for( int i=0; i<16; i++ )
        {
            resultPtr[i] = address[i];
        }
        return result;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarFloat[0] = (v0u.scalarFloat[0] > v1u.scalarFloat[0]) ? v0u.scalarFloat[0] : v1u.scalarFloat[0];
        r.scalarFloat[1] = (v0u.scalarFloat[1] > v1u.scalarFloat[1]) ? v0u.scalarFloat[1] : v1u.scalarFloat[1];
        r.scalarFloat[2] = (v0u.scalarFloat[2] > v1u.scalarFloat[2]) ? v0u.scalarFloat[2] : v1u.scalarFloat[2];
        r.scalarFloat[3] = (v0u.scalarFloat[3] > v1u.scalarFloat[3]) ? v0u.scalarFloat[3] : v1u.scalarFloat[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarFloat[0] = (v0u.scalarFloat[0] < v1u.scalarFloat[0]) ? v0u.scalarFloat[0] : v1u.scalarFloat[0];
        r.scalarFloat[1] = (v0u.scalarFloat[1] < v1u.scalarFloat[1]) ? v0u.scalarFloat[1] : v1u.scalarFloat[1];
        r.scalarFloat[2] = (v0u.scalarFloat[2] < v1u.scalarFloat[2]) ? v0u.scalarFloat[2] : v1u.scalarFloat[2];
        r.scalarFloat[3] = (v0u.scalarFloat[3] < v1u.scalarFloat[3]) ? v0u.scalarFloat[3] : v1u.scalarFloat[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMult(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarFloat[0] = v0u.scalarFloat[0] * v1u.scalarFloat[0];
        r.scalarFloat[1] = v0u.scalarFloat[1] * v1u.scalarFloat[1];
        r.scalarFloat[2] = v0u.scalarFloat[2] * v1u.scalarFloat[2];
        r.scalarFloat[3] = v0u.scalarFloat[3] * v1u.scalarFloat[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMultAdd(VectorIntrinsicInParam mult0, VectorIntrinsicInParam mult1, VectorIntrinsicInParam add)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, mult0U, mult1U, addU;
        mult0U.vector = mult0;
        mult1U.vector = mult1;
        addU.vector = add;
        r.scalarFloat[0] = mult0U.scalarFloat[0] * mult1U.scalarFloat[0] + addU.scalarFloat[0];
        r.scalarFloat[1] = mult0U.scalarFloat[1] * mult1U.scalarFloat[1] + addU.scalarFloat[1];
        r.scalarFloat[2] = mult0U.scalarFloat[2] * mult1U.scalarFloat[2] + addU.scalarFloat[2];
        r.scalarFloat[3] = mult0U.scalarFloat[3] * mult1U.scalarFloat[3] + addU.scalarFloat[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMultSum3(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        float dp3 = v0u.scalarFloat[0] * v1u.scalarFloat[0] +
                    v0u.scalarFloat[1] * v1u.scalarFloat[1] +
                    v0u.scalarFloat[2] * v1u.scalarFloat[2];
        r.scalarFloat[0] = dp3;
        r.scalarFloat[1] = dp3;
        r.scalarFloat[2] = dp3;
        r.scalarFloat[3] = dp3;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMultSum4(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        float dp4 = v0u.scalarFloat[0] * v1u.scalarFloat[0] +
                    v0u.scalarFloat[1] * v1u.scalarFloat[1] +
                    v0u.scalarFloat[2] * v1u.scalarFloat[2] +
                    v0u.scalarFloat[3] * v1u.scalarFloat[3];
        r.scalarFloat[0] = dp4;
        r.scalarFloat[1] = dp4;
        r.scalarFloat[2] = dp4;
        r.scalarFloat[3] = dp4;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNegMultSub(VectorIntrinsicInParam mult0, VectorIntrinsicInParam mult1, VectorIntrinsicInParam sub)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u, subU;
        v0u.vector = mult0;
        v1u.vector = mult1;
        subU.vector = sub;
        r.scalarFloat[0] =  subU.scalarFloat[0] - v0u.scalarFloat[0] * v1u.scalarFloat[0];
        r.scalarFloat[1] =  subU.scalarFloat[1] - v0u.scalarFloat[1] * v1u.scalarFloat[1];
        r.scalarFloat[2] =  subU.scalarFloat[2] - v0u.scalarFloat[2] * v1u.scalarFloat[2];
        r.scalarFloat[3] =  subU.scalarFloat[3] - v0u.scalarFloat[3] * v1u.scalarFloat[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNor(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = ~(v0u.scalarUInt[0] | v1u.scalarUInt[0]);
        r.scalarUInt[1] = ~(v0u.scalarUInt[1] | v1u.scalarUInt[1]);
        r.scalarUInt[2] = ~(v0u.scalarUInt[2] | v1u.scalarUInt[2]);
        r.scalarUInt[3] = ~(v0u.scalarUInt[3] | v1u.scalarUInt[3]);
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNot(VectorIntrinsicInParam v0)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        r.scalarUInt[0] = ~v0u.scalarUInt[0];
        r.scalarUInt[1] = ~v0u.scalarUInt[1];
        r.scalarUInt[2] = ~v0u.scalarUInt[2];
        r.scalarUInt[3] = ~v0u.scalarUInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecOr(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = v0u.scalarUInt[0] | v1u.scalarUInt[0];
        r.scalarUInt[1] = v0u.scalarUInt[1] | v1u.scalarUInt[1];
        r.scalarUInt[2] = v0u.scalarUInt[2] | v1u.scalarUInt[2];
        r.scalarUInt[3] = v0u.scalarUInt[3] | v1u.scalarUInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecRecipEst(VectorIntrinsicInParam v0)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        r.scalarFloat[0] = 1.0f / v0u.scalarFloat[0];
        r.scalarFloat[1] = 1.0f / v0u.scalarFloat[1];
        r.scalarFloat[2] = 1.0f / v0u.scalarFloat[2];
        r.scalarFloat[3] = 1.0f / v0u.scalarFloat[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecRecipSqrtEst(VectorIntrinsicInParam v0)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        r.scalarFloat[0] = 1.0f / sqrtf(v0u.scalarFloat[0]);
        r.scalarFloat[1] = 1.0f / sqrtf(v0u.scalarFloat[1]);
        r.scalarFloat[2] = 1.0f / sqrtf(v0u.scalarFloat[2]);
        r.scalarFloat[3] = 1.0f / sqrtf(v0u.scalarFloat[3]);
        return r.vector;
    }

    template<int RotateAmount> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftImm(VectorIntrinsicInParam v0)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        r.scalarFloat[0] = v0u.scalarFloat[(RotateAmount+0)&3];
        r.scalarFloat[1] = v0u.scalarFloat[(RotateAmount+1)&3];
        r.scalarFloat[2] = v0u.scalarFloat[(RotateAmount+2)&3];
        r.scalarFloat[3] = v0u.scalarFloat[(RotateAmount+3)&3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSelect(VectorIntrinsicInParam if0Result, VectorIntrinsicInParam if1Result, VectorIntrinsicInParam selectionMask)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, if0u, if1u, maskU;
        if0u.vector = if0Result;
        if1u.vector = if1Result;
        maskU.vector = selectionMask;
        r.scalarUInt[0] = ( if0u.scalarUInt[0] & (~maskU.scalarUInt[0]) ) | ( if1u.scalarUInt[0] & maskU.scalarUInt[0] );
        r.scalarUInt[1] = ( if0u.scalarUInt[1] & (~maskU.scalarUInt[1]) ) | ( if1u.scalarUInt[1] & maskU.scalarUInt[1] );
        r.scalarUInt[2] = ( if0u.scalarUInt[2] & (~maskU.scalarUInt[2]) ) | ( if1u.scalarUInt[2] & maskU.scalarUInt[2] );
        r.scalarUInt[3] = ( if0u.scalarUInt[3] & (~maskU.scalarUInt[3]) ) | ( if1u.scalarUInt[3] & maskU.scalarUInt[3] );
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = v0u.scalarUInt[0];
        r.scalarUInt[1] = v1u.scalarUInt[0];
        r.scalarUInt[2] = v0u.scalarUInt[1];
        r.scalarUInt[3] = v1u.scalarUInt[1];
        return r.vector;
    }

    VPL_API template<int PermConstant> RW_MATH_FORCE_INLINE VectorIntrinsic VecPermuteSingleImm(VectorIntrinsicInParam v0)
    {
        (void)::rw::math::detail::Assert_ValidSinglePermConstant<PermConstant>();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        r.scalarUInt[0] = v0u.scalarUInt[(PermConstant>>12)&0xf];
        r.scalarUInt[1] = v0u.scalarUInt[(PermConstant>> 8)&0xf];
        r.scalarUInt[2] = v0u.scalarUInt[(PermConstant>> 4)&0xf];
        r.scalarUInt[3] = v0u.scalarUInt[(PermConstant>> 0)&0xf];
        return r.vector;
    }

    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Word(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ElementIndex, 0, 3 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        uint32_t word = v0u.scalarUInt[ElementIndex];
        r.scalarUInt[ 0] = word;
        r.scalarUInt[ 1] = word;
        r.scalarUInt[ 2] = word;
        r.scalarUInt[ 3] = word;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarFloat[0] = v0u.scalarFloat[0] - v1u.scalarFloat[0];
        r.scalarFloat[1] = v0u.scalarFloat[1] - v1u.scalarFloat[1];
        r.scalarFloat[2] = v0u.scalarFloat[2] - v1u.scalarFloat[2];
        r.scalarFloat[3] = v0u.scalarFloat[3] - v1u.scalarFloat[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API void VecStoreLeftAligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        char *address = reinterpret_cast<char *>(base) + offset;
        const char *vecPtr = reinterpret_cast<const char *>(&v0);
        // Loop until a vector boundary in memory is hit.
        do
        {
            *(address++) = *(vecPtr++);
        } while( reinterpret_cast<uintptr_t>(address) & 15 );
    }

    RW_MATH_FORCE_INLINE VPL_API void VecStoreRightAligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        char *address = reinterpret_cast<char *>(base) + offset;
        const char *vecPtr = reinterpret_cast<const char *>(&v0) + 15;
        
        uintptr_t copyCount = reinterpret_cast<uintptr_t>(address) & 15;
        while(copyCount--)
        {
            *(--address) = *(vecPtr--);
        }
    }

    RW_MATH_FORCE_INLINE VPL_API void VecStoreUnaligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        char *address = reinterpret_cast<char *>(base) + offset;
        const char *vecPtr = reinterpret_cast<const char *>(&v0);
        for(int i=0;i<16;i++)
        {
            address[i] = vecPtr[i];
        }
    }

    RW_MATH_FORCE_INLINE VPL_API void VecStoreWordElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        char *address = reinterpret_cast<char *>(base) + offset;
        const char *srcAddress = reinterpret_cast<const char *>(&v0) + (reinterpret_cast<uintptr_t>(address)&15);
        // char pointers are used to write memory to avoid aliasing issues.
        for(int i=0;i<4;i++)
        {
            address[i] = srcAddress[i];
        }
    }

    RW_MATH_FORCE_INLINE VPL_API void VecStoreHalfElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        char *address = reinterpret_cast<char *>(base) + offset;
        const char *srcAddress = reinterpret_cast<const char *>(&v0) + (reinterpret_cast<uintptr_t>(address)&15);
        // char pointers are used to write memory to avoid aliasing issues.
        for(int i=0;i<2;i++)
        {
            address[i] = srcAddress[i];
        }
    }

    RW_MATH_FORCE_INLINE VPL_API void VecStoreByteElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        char *address = reinterpret_cast<char *>(base) + offset;
        const char *srcAddress = reinterpret_cast<const char *>(&v0) + (reinterpret_cast<uintptr_t>(address)&15);
        *address = *srcAddress;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecXor(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = v0u.scalarUInt[0] ^ v1u.scalarUInt[0];
        r.scalarUInt[1] = v0u.scalarUInt[1] ^ v1u.scalarUInt[1];
        r.scalarUInt[2] = v0u.scalarUInt[2] ^ v1u.scalarUInt[2];
        r.scalarUInt[3] = v0u.scalarUInt[3] ^ v1u.scalarUInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecPermute(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1, VectorIntrinsicInParam permuteConstant)
    {
        union DoubleVectorUnion
        {
            uint8_t        scalarUChar[32];
            VectorIntrinsic     vector[ 2];
        };

        using namespace rw::math::vpu::detail;
        DoubleVectorUnion input;
        VectorIntrinsicUnion r, permuteConstantU;
        input.vector[0] = v0;
        input.vector[1] = v1;
        permuteConstantU.vector = permuteConstant;
        r.scalarUChar[ 0] = input.scalarUChar[permuteConstantU.scalarUChar[ 0]];
        r.scalarUChar[ 1] = input.scalarUChar[permuteConstantU.scalarUChar[ 1]];
        r.scalarUChar[ 2] = input.scalarUChar[permuteConstantU.scalarUChar[ 2]];
        r.scalarUChar[ 3] = input.scalarUChar[permuteConstantU.scalarUChar[ 3]];
        r.scalarUChar[ 4] = input.scalarUChar[permuteConstantU.scalarUChar[ 4]];
        r.scalarUChar[ 5] = input.scalarUChar[permuteConstantU.scalarUChar[ 5]];
        r.scalarUChar[ 6] = input.scalarUChar[permuteConstantU.scalarUChar[ 6]];
        r.scalarUChar[ 7] = input.scalarUChar[permuteConstantU.scalarUChar[ 7]];
        r.scalarUChar[ 8] = input.scalarUChar[permuteConstantU.scalarUChar[ 8]];
        r.scalarUChar[ 9] = input.scalarUChar[permuteConstantU.scalarUChar[ 9]];
        r.scalarUChar[10] = input.scalarUChar[permuteConstantU.scalarUChar[10]];
        r.scalarUChar[11] = input.scalarUChar[permuteConstantU.scalarUChar[11]];
        r.scalarUChar[12] = input.scalarUChar[permuteConstantU.scalarUChar[12]];
        r.scalarUChar[13] = input.scalarUChar[permuteConstantU.scalarUChar[13]];
        r.scalarUChar[14] = input.scalarUChar[permuteConstantU.scalarUChar[14]];
        r.scalarUChar[15] = input.scalarUChar[permuteConstantU.scalarUChar[15]];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[ 0] = v0u.scalarUChar[0];
        r.scalarUChar[ 1] = v1u.scalarUChar[0];
        r.scalarUChar[ 2] = v0u.scalarUChar[1];
        r.scalarUChar[ 3] = v1u.scalarUChar[1];
        r.scalarUChar[ 4] = v0u.scalarUChar[2];
        r.scalarUChar[ 5] = v1u.scalarUChar[2];
        r.scalarUChar[ 6] = v0u.scalarUChar[3];
        r.scalarUChar[ 7] = v1u.scalarUChar[3];
        r.scalarUChar[ 8] = v0u.scalarUChar[4];
        r.scalarUChar[ 9] = v1u.scalarUChar[4];
        r.scalarUChar[10] = v0u.scalarUChar[5];
        r.scalarUChar[11] = v1u.scalarUChar[5];
        r.scalarUChar[12] = v0u.scalarUChar[6];
        r.scalarUChar[13] = v1u.scalarUChar[6];
        r.scalarUChar[14] = v0u.scalarUChar[7];
        r.scalarUChar[15] = v1u.scalarUChar[7];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = v0u.scalarUShort[0];
        r.scalarUShort[1] = v1u.scalarUShort[0];
        r.scalarUShort[2] = v0u.scalarUShort[1];
        r.scalarUShort[3] = v1u.scalarUShort[1];
        r.scalarUShort[4] = v0u.scalarUShort[2];
        r.scalarUShort[5] = v1u.scalarUShort[2];
        r.scalarUShort[6] = v0u.scalarUShort[3];
        r.scalarUShort[7] = v1u.scalarUShort[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[ 0] = v0u.scalarUChar[ 8];
        r.scalarUChar[ 1] = v1u.scalarUChar[ 8];
        r.scalarUChar[ 2] = v0u.scalarUChar[ 9];
        r.scalarUChar[ 3] = v1u.scalarUChar[ 9];
        r.scalarUChar[ 4] = v0u.scalarUChar[10];
        r.scalarUChar[ 5] = v1u.scalarUChar[10];
        r.scalarUChar[ 6] = v0u.scalarUChar[11];
        r.scalarUChar[ 7] = v1u.scalarUChar[11];
        r.scalarUChar[ 8] = v0u.scalarUChar[12];
        r.scalarUChar[ 9] = v1u.scalarUChar[12];
        r.scalarUChar[10] = v0u.scalarUChar[13];
        r.scalarUChar[11] = v1u.scalarUChar[13];
        r.scalarUChar[12] = v0u.scalarUChar[14];
        r.scalarUChar[13] = v1u.scalarUChar[14];
        r.scalarUChar[14] = v0u.scalarUChar[15];
        r.scalarUChar[15] = v1u.scalarUChar[15];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = v0u.scalarUShort[4];
        r.scalarUShort[1] = v1u.scalarUShort[4];
        r.scalarUShort[2] = v0u.scalarUShort[5];
        r.scalarUShort[3] = v1u.scalarUShort[5];
        r.scalarUShort[4] = v0u.scalarUShort[6];
        r.scalarUShort[5] = v1u.scalarUShort[6];
        r.scalarUShort[6] = v0u.scalarUShort[7];
        r.scalarUShort[7] = v1u.scalarUShort[7];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = v0u.scalarUInt[2];
        r.scalarUInt[1] = v1u.scalarUInt[2];
        r.scalarUInt[2] = v0u.scalarUInt[3];
        r.scalarUInt[3] = v1u.scalarUInt[3];
        return r.vector;
    }

    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Byte(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ElementIndex, 0, 15 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        uint8_t byte = v0u.scalarUChar[ElementIndex];
        r.scalarUChar[ 0] = byte;
        r.scalarUChar[ 1] = byte;
        r.scalarUChar[ 2] = byte;
        r.scalarUChar[ 3] = byte;
        r.scalarUChar[ 4] = byte;
        r.scalarUChar[ 5] = byte;
        r.scalarUChar[ 6] = byte;
        r.scalarUChar[ 7] = byte;
        r.scalarUChar[ 8] = byte;
        r.scalarUChar[ 9] = byte;
        r.scalarUChar[10] = byte;
        r.scalarUChar[11] = byte;
        r.scalarUChar[12] = byte;
        r.scalarUChar[13] = byte;
        r.scalarUChar[14] = byte;
        r.scalarUChar[15] = byte;
        return r.vector;
    }

    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Half(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ElementIndex, 0, 7 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        uint16_t half = v0u.scalarUShort[ElementIndex];
        r.scalarUShort[0] = half;
        r.scalarUShort[1] = half;
        r.scalarUShort[2] = half;
        r.scalarUShort[3] = half;
        r.scalarUShort[4] = half;
        r.scalarUShort[5] = half;
        r.scalarUShort[6] = half;
        r.scalarUShort[7] = half;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLog2Est(VectorIntrinsicInParam v0)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        r.scalarFloat[0] = logf(v0u.scalarFloat[0]) * 1.4426950408889634073599246810019f;
        r.scalarFloat[1] = logf(v0u.scalarFloat[1]) * 1.4426950408889634073599246810019f;
        r.scalarFloat[2] = logf(v0u.scalarFloat[2]) * 1.4426950408889634073599246810019f;
        r.scalarFloat[3] = logf(v0u.scalarFloat[3]) * 1.4426950408889634073599246810019f;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecFloor(VectorIntrinsicInParam v0)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        r.scalarFloat[0] = floorf(v0u.scalarFloat[0]);
        r.scalarFloat[1] = floorf(v0u.scalarFloat[1]);
        r.scalarFloat[2] = floorf(v0u.scalarFloat[2]);
        r.scalarFloat[3] = floorf(v0u.scalarFloat[3]);
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<16; i++ )
        {
            r.scalarUChar[i] = (uint8_t)(v0u.scalarUChar[i] + v1u.scalarUChar[i]);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<16; i++ )
        {
            r.scalarUChar[i] = (uint8_t)(v0u.scalarUChar[i] - v1u.scalarUChar[i]);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<16; i++ )
        {
            r.scalarUChar[i] = (uint8_t)(v0u.scalarUChar[i] + v1u.scalarUChar[i]);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<16; i++ )
        {
            r.scalarUChar[i] = (uint8_t)(v0u.scalarUChar[i] - v1u.scalarUChar[i]);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<16; i++ )
        {
            int addResult = ((int8_t)v0u.scalarUChar[i] + (int8_t)v1u.scalarUChar[i]);
            addResult = addResult > 127 ? 127: addResult;
            addResult = addResult < -128 ? -128: addResult;
            r.scalarUChar[i] = (uint8_t)addResult;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<16; i++ )
        {
            int subResult = ((int8_t)v0u.scalarUChar[i] - (int8_t)v1u.scalarUChar[i]);
            subResult = subResult > 127 ? 127: subResult;
            subResult = subResult < -128 ? -128: subResult;
            r.scalarUChar[i] = (uint8_t)subResult;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<16; i++ )
        {
            int addResult = (v0u.scalarUChar[i] + v1u.scalarUChar[i]);
            r.scalarUChar[i] = (uint8_t)(addResult > 255 ? 255: addResult);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<16; i++ )
        {
            int subResult = (v0u.scalarUChar[i] - v1u.scalarUChar[i]);
            r.scalarUChar[i] = (uint8_t)(subResult < 0 ? 0: subResult);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<8; i++ )
        {
            r.scalarUShort[i] = (uint16_t)(v0u.scalarUShort[i] + v1u.scalarUShort[i]);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<8; i++ )
        {
            r.scalarUShort[i] = (uint16_t)(v0u.scalarUShort[i] - v1u.scalarUShort[i]);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<8; i++ )
        {
            r.scalarUShort[i] = (uint16_t)(v0u.scalarUShort[i] + v1u.scalarUShort[i]);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<8; i++ )
        {
            r.scalarUShort[i] = (uint16_t)(v0u.scalarUShort[i] - v1u.scalarUShort[i]);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<8; i++ )
        {
            int addResult = ((int16_t)v0u.scalarUShort[i] + (int16_t)v1u.scalarUShort[i]);
            addResult = addResult > 32767 ? 32767: addResult;
            addResult = addResult < -32768 ? -32768: addResult;
            r.scalarUShort[i] = (uint16_t)addResult;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<8; i++ )
        {
            int subResult = ((int16_t)v0u.scalarUShort[i] - (int16_t)v1u.scalarUShort[i]);
            subResult = subResult > 32767 ? 32767: subResult;
            subResult = subResult < -32768 ? -32768: subResult;
            r.scalarUShort[i] = (uint16_t)subResult;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<8; i++ )
        {
            int addResult = (v0u.scalarUShort[i] + v1u.scalarUShort[i]);
            r.scalarUShort[i] = (uint16_t)(addResult > 65535 ? 65535: addResult);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<8; i++ )
        {
            int subResult = (v0u.scalarUShort[i] - v1u.scalarUShort[i]);
            r.scalarUShort[i] = (uint16_t)(subResult < 0 ? 0: subResult);
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            r.scalarUInt[i] = v0u.scalarUInt[i] + v1u.scalarUInt[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            r.scalarUInt[i] = v0u.scalarUInt[i] - v1u.scalarUInt[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            r.scalarUInt[i] = v0u.scalarUInt[i] + v1u.scalarUInt[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            r.scalarUInt[i] = v0u.scalarUInt[i] - v1u.scalarUInt[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            int minInt = (int)2147483648U;       // This constant should actually represent -2147483647 when converted to a signed 32bit integer
            int64_t addResult = ((int64_t)(int)v0u.scalarUInt[i] + (int64_t)(int)v1u.scalarUInt[i]);
            addResult = addResult > 2147483647 ? 2147483647: addResult;
            addResult = addResult < minInt ? minInt: addResult;
            r.scalarUInt[i] = (unsigned int)(int)addResult;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            int minInt = (int)2147483648U;       // This constant should actually represent -2147483647 when converted to a signed 32bit integer
            int64_t subResult = ((int64_t)(int)v0u.scalarUInt[i] - (int64_t)(int)v1u.scalarUInt[i]);
            subResult = subResult > 2147483647 ? 2147483647: subResult;
            subResult = subResult < minInt ? minInt: subResult;
            r.scalarUInt[i] = (unsigned int)(int)subResult;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            unsigned int addResult = (v0u.scalarUInt[i] + v1u.scalarUInt[i]);
            r.scalarUInt[i] = addResult < v0u.scalarUInt[i] ? 0xffffffffU: addResult;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            unsigned int subResult = (v0u.scalarUInt[i] - v1u.scalarUInt[i]);
            r.scalarUInt[i] = subResult > v0u.scalarUInt[i] ? 0: subResult;
        }
        return r.vector;
    }

    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToSignedFixedPoint(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        rw::math::detail::Flt32UInt32Union scaleUnion;
        scaleUnion.i = (ScaleExponent + 127) << 23;
        const float scale = scaleUnion.f;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            const float scaledFloat = v0u.scalarFloat[i] * scale;
            if( scaledFloat < -2147483648.0f )
            {
                r.scalarUInt[i] = 0x80000000U;
            }
            else if( scaledFloat > 2147483647.0f )
            {
                r.scalarUInt[i] = 0x7fffffffU;
            }
            else
            {
                r.scalarUInt[i] = (unsigned int)(int)scaledFloat;
            }
        }
        return r.vector;
    }

    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertSignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        rw::math::detail::Flt32UInt32Union scaleUnion;
        scaleUnion.i = (127-ScaleExponent) << 23;
        const float scale = scaleUnion.f;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            r.scalarFloat[i] = ((float)v0u.scalarInt[i]) * scale;
        }
        return r.vector;
    }

    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToUnsignedFixedPoint(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        rw::math::detail::Flt32UInt32Union scaleUnion;
        scaleUnion.i = (ScaleExponent + 127) << 23;
        const float scale = scaleUnion.f;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            const float scaledFloat = v0u.scalarFloat[i] * scale;
            if( scaledFloat < 0.0f )
            {
                r.scalarUInt[i] = 0;
            }
            else if( scaledFloat > 4294967295.0f )
            {
                r.scalarUInt[i] = 4294967295U;
            }
            else
            {
                r.scalarUInt[i] = (unsigned int)scaledFloat;
            }
        }
        return r.vector;
    }

    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertUnsignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u;
        v0u.vector = v0;
        rw::math::detail::Flt32UInt32Union scaleUnion;
        scaleUnion.i = (127-ScaleExponent) << 23;
        const float scale = scaleUnion.f;
        r.vector = v0;  // r is initialized to prevent compiler warnings on MSVC
        for( int i=0; i<4; i++ )
        {
            r.scalarFloat[i] = ((float)v0u.scalarUInt[i]) * scale;
        }
        return r.vector;
    }

    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Char()
    {
        (void)detail::Assert_5BitSignedValue<Value>();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarChar[0] = (char)Value; // Make sure an element of r is initialized to prevent compiler warnings on MSVC
        for( int i=1; i<16; i++ )
        {
            r.scalarChar[i] = (char)Value;
        }
        return r.vector;
    }

    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Int()
    {
        (void)detail::Assert_5BitSignedValue<Value>();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarInt[0] = Value;
        r.scalarInt[1] = Value;
        r.scalarInt[2] = Value;
        r.scalarInt[3] = Value;
        return r.vector;
    }

    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoadImm_Int()
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarInt[0] = Value;
        r.scalarInt[1] = Value;
        r.scalarInt[2] = Value;
        r.scalarInt[3] = Value;
        return r.vector;
    }

    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Short()
    {
        (void)detail::Assert_5BitSignedValue<Value>();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r;
        r.scalarShort[0] = (int16_t)Value; // Make sure an element of r is initialized to prevent compiler warnings on MSVC
        for( int i=1; i<8; i++ )
        {
            r.scalarShort[i] = (int16_t)Value;
        }
        return r.vector;
    }

    VPL_API template<int Mask, int RotateLeft> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftWithMaskedInsertionImm(VectorIntrinsicInParam original, VectorIntrinsicInParam vecToInsert)
    {
        (void)detail::Assert_ValueInRange< Mask, 0, 15 >();
        (void)detail::Assert_ValueInRange< RotateLeft, 0, 3 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion originalUnion;
        VectorIntrinsicUnion vecToInsertUnion;
        VectorIntrinsicUnion r;
        originalUnion.vector = original;
        vecToInsertUnion.vector = vecToInsert;
        r.scalarUInt[0] = (Mask & 8) ? vecToInsertUnion.scalarUInt[(0+RotateLeft)&3] : originalUnion.scalarUInt[0];
        r.scalarUInt[1] = (Mask & 4) ? vecToInsertUnion.scalarUInt[(1+RotateLeft)&3] : originalUnion.scalarUInt[1];
        r.scalarUInt[2] = (Mask & 2) ? vecToInsertUnion.scalarUInt[(2+RotateLeft)&3] : originalUnion.scalarUInt[2];
        r.scalarUInt[3] = (Mask & 1) ? vecToInsertUnion.scalarUInt[(3+RotateLeft)&3] : originalUnion.scalarUInt[3];
        return r.vector;
    }

    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm_UInt(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< Shift, 0, 31 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion v0u, r;
        v0u.vector = v0;
        r.scalarUInt[0] = v0u.scalarUInt[0] << Shift;
        r.scalarUInt[1] = v0u.scalarUInt[1] << Shift;
        r.scalarUInt[2] = v0u.scalarUInt[2] << Shift;
        r.scalarUInt[3] = v0u.scalarUInt[3] << Shift;
        return r.vector;
    }

    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_UInt(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< Shift, 0, 31 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion v0u, r;
        v0u.vector = v0;
        r.scalarUInt[0] = v0u.scalarUInt[0] >> Shift;
        r.scalarUInt[1] = v0u.scalarUInt[1] >> Shift;
        r.scalarUInt[2] = v0u.scalarUInt[2] >> Shift;
        r.scalarUInt[3] = v0u.scalarUInt[3] >> Shift;
        return r.vector;
    }

    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_Int(VectorIntrinsicInParam v0)
    {
        (void)detail::Assert_ValueInRange< Shift, 0, 31 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion v0u, r;
        v0u.vector = v0;
        // Note: Technically these shift operations are not portable because
        // the C standard does dictate whether or not sign extension must
        // occur.  But in practice this should be fine for all current platforms.
        r.scalarInt[0] = v0u.scalarInt[0] >> Shift;
        r.scalarInt[1] = v0u.scalarInt[1] >> Shift;
        r.scalarInt[2] = v0u.scalarInt[2] >> Shift;
        r.scalarInt[3] = v0u.scalarInt[3] >> Shift;
        return r.vector;
    }

    VPL_API template<int ShiftAmount> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm(VectorIntrinsicInParam v0)
    {
#if defined _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 6326)  //  warning C6326: Potential comparison of a constant with another constant.
#endif
        (void)detail::Assert_ValueInRange< ShiftAmount, 0, 127 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion v0u, r;
        v0u.vector = v0;
        const int leftBitShift = ShiftAmount & 7;
        const int rightBitShift = (leftBitShift == 0) ? 0 : 8 - leftBitShift;
        int dest = 0;
        for( int i=ShiftAmount/8;i<15;i++)
        {
            r.scalarUChar[dest++] = (uint8_t)(
                    (v0u.scalarUChar[i] << leftBitShift) | 
                    ((leftBitShift == 0) ? 0 : (v0u.scalarUChar[i+1] >> rightBitShift))
                    );
        }
        r.scalarUChar[dest++] = (uint8_t)(v0u.scalarUChar[15] << leftBitShift);
        for( ;dest<16;dest++)
        {
            // Fill right most bytes with zero
            r.scalarUChar[dest] = 0;
        }
        return r.vector;
#if defined _MSC_VER
    #pragma warning(pop)
#endif
    }

    VPL_API template<int ShiftAmount> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm(VectorIntrinsicInParam v0)
    {
#if defined _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 6326)  //  warning C6326: Potential comparison of a constant with another constant.
#endif
        (void)detail::Assert_ValueInRange< ShiftAmount, 0, 127 >();
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion v0u, r;
        v0u.vector = v0;
        const int rightBitShift = ShiftAmount & 7;
        const int leftBitShift = (rightBitShift == 0) ? 0 : 8 - rightBitShift;
        int dest = 0;
        for( ;dest<ShiftAmount/8;dest++)
        {
            // Fill left most bytes with zero
            r.scalarUChar[dest] = 0;
        }
        r.scalarUChar[dest++] = (uint8_t)(v0u.scalarUChar[0] >> rightBitShift);
        for( int i=1;i<16-ShiftAmount/8;i++)
        {
            r.scalarUChar[dest++] = (uint8_t)(
                    (v0u.scalarUChar[i] >> rightBitShift) | 
                    ((rightBitShift == 0) ? 0 : (v0u.scalarUChar[i-1] << leftBitShift))
                    );
        }
        return r.vector;
#if defined _MSC_VER
    #pragma warning(pop)
#endif
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, shiftAmountU;
        r.vector = v0; // initialize r to avoid warnings about r being possibly uninitialized
        v0u.vector = v0;
        shiftAmountU.vector = shiftAmount;
        RW_MATH_ASSERT_MSG( shiftAmountU.scalarUInt[0] > 127, "The shiftAmount must be between 0 and 127 inclusive." );
        unsigned int byteShift = (shiftAmountU.scalarUInt[0] & 0xff) / 8;
        for(unsigned int i=0;i<16-byteShift;i++)
        {
            r.scalarUChar[i] = v0u.scalarUChar[i+byteShift];
        }
        unsigned int zeroStartIndex = (byteShift > 16) ? 0 : 16-byteShift;
        for(unsigned int i=zeroStartIndex;i<16;i++)
        {
            r.scalarUChar[i] = 0;
        }
        return r.vector;
    }

	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, shiftAmountU;
        r.vector = v0; // initialize r to avoid warnings about r being possibly uninitialized
        v0u.vector = v0;
        shiftAmountU.vector = shiftAmount;
        unsigned int bitShift = shiftAmountU.scalarUInt[3] & 0x00000007;
        for(unsigned int i=0;i<15;i++)
        {
			uint8_t si0 = (uint8_t)(v0u.scalarUChar[i] << bitShift);
			uint8_t si1 = (uint8_t)(v0u.scalarUChar[i+1] >> (8 - bitShift));
			r.scalarUChar[i] = (uint8_t)(si0 | si1);
        }
		r.scalarUChar[15] = (uint8_t)(r.scalarUChar[15] << bitShift);
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, shiftAmountU;
        r.vector = v0; // initialize r to avoid warnings about r being possibly uninitialized
        v0u.vector = v0;
        shiftAmountU.vector = shiftAmount;
        RW_MATH_ASSERT_MSG( shiftAmountU.scalarUInt[0] > 127, "The shiftAmount must be between 0 and 127 inclusive." );
        unsigned int byteShift = (shiftAmountU.scalarUInt[0] & 0xff) / 8;
        for(unsigned int i=0;i<byteShift;i++)
        {
            r.scalarUChar[i] = 0;
        }
        for(unsigned int i=byteShift;i<16;i++)
        {
            r.scalarUChar[i] = v0u.scalarUChar[i-byteShift];
        }
        return r.vector;
    }
	
	RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, shiftAmountU;
        r.vector = v0; // initialize r to avoid warnings about r being possibly uninitialized
        v0u.vector = v0;
        shiftAmountU.vector = shiftAmount;
        unsigned int bitShift = shiftAmountU.scalarUInt[3] & 0x00000007;
        for(unsigned int i=15;i>0;i--)
        {
			uint8_t si0 = (uint8_t)(v0u.scalarUChar[i] >> bitShift);
			uint8_t si1 = (uint8_t)(v0u.scalarUChar[i-1] << (8 - bitShift));
			r.scalarUChar[i] = (uint8_t)(si0 | si1);
        }
		r.scalarUChar[0] = (uint8_t)(r.scalarUChar[0] >> bitShift);
        return r.vector;
    }


    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarUInt[0] == v1u.scalarUInt[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarUInt[1] == v1u.scalarUInt[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarUInt[2] == v1u.scalarUInt[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarUInt[3] == v1u.scalarUInt[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarUInt[0] > v1u.scalarUInt[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarUInt[1] > v1u.scalarUInt[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarUInt[2] > v1u.scalarUInt[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarUInt[3] > v1u.scalarUInt[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarUInt[0] < v1u.scalarUInt[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarUInt[1] < v1u.scalarUInt[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarUInt[2] < v1u.scalarUInt[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarUInt[3] < v1u.scalarUInt[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarInt[0] == v1u.scalarInt[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarInt[1] == v1u.scalarInt[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarInt[2] == v1u.scalarInt[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarInt[3] == v1u.scalarInt[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarInt[0] > v1u.scalarInt[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarInt[1] > v1u.scalarInt[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarInt[2] > v1u.scalarInt[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarInt[3] > v1u.scalarInt[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarInt[0] < v1u.scalarInt[0]) ? 0xffffffffU : 0;
        r.scalarUInt[1] = (v0u.scalarInt[1] < v1u.scalarInt[1]) ? 0xffffffffU : 0;
        r.scalarUInt[2] = (v0u.scalarInt[2] < v1u.scalarInt[2]) ? 0xffffffffU : 0;
        r.scalarUInt[3] = (v0u.scalarInt[3] < v1u.scalarInt[3]) ? 0xffffffffU : 0;
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = (v0u.scalarUShort[0] == v1u.scalarUShort[0]) ?  (unsigned short)0xffffU: (unsigned short)0;
        for(int i=1;i<8;i++)
        {
            r.scalarUShort[i] = (v0u.scalarUShort[i] == v1u.scalarUShort[i]) ? (unsigned short)0xffffU: (unsigned short)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = (v0u.scalarUShort[0] > v1u.scalarUShort[0]) ? (unsigned short)0xffffU: (unsigned short)0;
        for(int i=1;i<8;i++)
        {
            r.scalarUShort[i] = (v0u.scalarUShort[i] > v1u.scalarUShort[i]) ? (unsigned short)0xffffU: (unsigned short)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = (v0u.scalarUShort[0] < v1u.scalarUShort[0]) ? (unsigned short)0xffffU: (unsigned short)0;
        for(int i=1;i<8;i++)
        {
            r.scalarUShort[i] = (v0u.scalarUShort[i] < v1u.scalarUShort[i]) ? (unsigned short)0xffffU: (unsigned short)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = (v0u.scalarShort[0] == v1u.scalarShort[0]) ? (unsigned short)0xffffU: (unsigned short)0;
        for(int i=1;i<8;i++)
        {
            r.scalarUShort[i] = (v0u.scalarShort[i] == v1u.scalarShort[i]) ? (unsigned short)0xffffU: (unsigned short)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = (v0u.scalarShort[0] > v1u.scalarShort[0]) ? (unsigned short)0xffffU: (unsigned short)0;
        for(int i=1;i<8;i++)
        {
            r.scalarUShort[i] = (v0u.scalarShort[i] > v1u.scalarShort[i]) ? (unsigned short)0xffffU: (unsigned short)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = (v0u.scalarShort[0] < v1u.scalarShort[0]) ? (unsigned short)0xffffU: (unsigned short)0;
        for(int i=1;i<8;i++)
        {
            r.scalarUShort[i] = (v0u.scalarShort[i] < v1u.scalarShort[i]) ? (unsigned short)0xffffU: (unsigned short)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[0] = (v0u.scalarUChar[0] == v1u.scalarUChar[0]) ? (unsigned char)0xffU: (unsigned char)0;
        for(int i=1;i<16;i++)
        {
            r.scalarUChar[i] = (v0u.scalarUChar[i] == v1u.scalarUChar[i]) ? (unsigned char)0xffU: (unsigned char)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[0] = (v0u.scalarUChar[0] > v1u.scalarUChar[0]) ? (unsigned char)0xffU: (unsigned char)0;
        for(int i=1;i<16;i++)
        {
            r.scalarUChar[i] = (v0u.scalarUChar[i] > v1u.scalarUChar[i]) ? (unsigned char)0xffU: (unsigned char)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[0] = (v0u.scalarUChar[0] < v1u.scalarUChar[0]) ? (unsigned char)0xffU: (unsigned char)0;
        for(int i=1;i<16;i++)
        {
            r.scalarUChar[i] = (v0u.scalarUChar[i] < v1u.scalarUChar[i]) ? (unsigned char)0xffU: (unsigned char)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[0] = (v0u.scalarChar[0] == v1u.scalarChar[0]) ? (unsigned char)0xffU: (unsigned char)0;
        for(int i=1;i<16;i++)
        {
            r.scalarUChar[i] = (v0u.scalarChar[i] == v1u.scalarChar[i]) ? (unsigned char)0xffU: (unsigned char)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[0] = (v0u.scalarChar[0] > v1u.scalarChar[0]) ? (unsigned char)0xffU: (unsigned char)0;
        for(int i=1;i<16;i++)
        {
            r.scalarUChar[i] = (v0u.scalarChar[i] > v1u.scalarChar[i]) ? (unsigned char)0xffU: (unsigned char)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[0] = (v0u.scalarChar[0] < v1u.scalarChar[0]) ? (unsigned char)0xffU: (unsigned char)0;
        for(int i=1;i<16;i++)
        {
            r.scalarUChar[i] = (v0u.scalarChar[i] < v1u.scalarChar[i]) ? (unsigned char)0xffU: (unsigned char)0;
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarUInt[0] > v1u.scalarUInt[0]) ? v0u.scalarUInt[0] : v1u.scalarUInt[0];
        r.scalarUInt[1] = (v0u.scalarUInt[1] > v1u.scalarUInt[1]) ? v0u.scalarUInt[1] : v1u.scalarUInt[1];
        r.scalarUInt[2] = (v0u.scalarUInt[2] > v1u.scalarUInt[2]) ? v0u.scalarUInt[2] : v1u.scalarUInt[2];
        r.scalarUInt[3] = (v0u.scalarUInt[3] > v1u.scalarUInt[3]) ? v0u.scalarUInt[3] : v1u.scalarUInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarInt[0] = (v0u.scalarInt[0] > v1u.scalarInt[0]) ? v0u.scalarInt[0] : v1u.scalarInt[0];
        r.scalarInt[1] = (v0u.scalarInt[1] > v1u.scalarInt[1]) ? v0u.scalarInt[1] : v1u.scalarInt[1];
        r.scalarInt[2] = (v0u.scalarInt[2] > v1u.scalarInt[2]) ? v0u.scalarInt[2] : v1u.scalarInt[2];
        r.scalarInt[3] = (v0u.scalarInt[3] > v1u.scalarInt[3]) ? v0u.scalarInt[3] : v1u.scalarInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUInt[0] = (v0u.scalarUInt[0] < v1u.scalarUInt[0]) ? v0u.scalarUInt[0] : v1u.scalarUInt[0];
        r.scalarUInt[1] = (v0u.scalarUInt[1] < v1u.scalarUInt[1]) ? v0u.scalarUInt[1] : v1u.scalarUInt[1];
        r.scalarUInt[2] = (v0u.scalarUInt[2] < v1u.scalarUInt[2]) ? v0u.scalarUInt[2] : v1u.scalarUInt[2];
        r.scalarUInt[3] = (v0u.scalarUInt[3] < v1u.scalarUInt[3]) ? v0u.scalarUInt[3] : v1u.scalarUInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarInt[0] = (v0u.scalarInt[0] < v1u.scalarInt[0]) ? v0u.scalarInt[0] : v1u.scalarInt[0];
        r.scalarInt[1] = (v0u.scalarInt[1] < v1u.scalarInt[1]) ? v0u.scalarInt[1] : v1u.scalarInt[1];
        r.scalarInt[2] = (v0u.scalarInt[2] < v1u.scalarInt[2]) ? v0u.scalarInt[2] : v1u.scalarInt[2];
        r.scalarInt[3] = (v0u.scalarInt[3] < v1u.scalarInt[3]) ? v0u.scalarInt[3] : v1u.scalarInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = (v0u.scalarUShort[0] > v1u.scalarUShort[0]) ? v0u.scalarUShort[0] : v1u.scalarUShort[0];
        for(int i=1;i<8;i++)
        {
            r.scalarUShort[i] = (v0u.scalarUShort[i] > v1u.scalarUShort[i]) ? v0u.scalarUShort[i] : v1u.scalarUShort[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarShort[0] = (v0u.scalarShort[0] > v1u.scalarShort[0]) ? v0u.scalarShort[0] : v1u.scalarShort[0];
        for(int i=1;i<8;i++)
        {
            r.scalarShort[i] = (v0u.scalarShort[i] > v1u.scalarShort[i]) ? v0u.scalarShort[i] : v1u.scalarShort[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUShort[0] = (v0u.scalarUShort[0] < v1u.scalarUShort[0]) ? v0u.scalarUShort[0] : v1u.scalarUShort[0];
        for(int i=1;i<8;i++)
        {
            r.scalarUShort[i] = (v0u.scalarUShort[i] < v1u.scalarUShort[i]) ? v0u.scalarUShort[i] : v1u.scalarUShort[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarShort[0] = (v0u.scalarShort[0] < v1u.scalarShort[0]) ? v0u.scalarShort[0] : v1u.scalarShort[0];
        for(int i=1;i<8;i++)
        {
            r.scalarShort[i] = (v0u.scalarShort[i] < v1u.scalarShort[i]) ? v0u.scalarShort[i] : v1u.scalarShort[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[0] = (v0u.scalarUChar[0] > v1u.scalarUChar[0]) ? v0u.scalarUChar[0] : v1u.scalarUChar[0];
        for(int i=1;i<16;i++)
        {
            r.scalarUChar[i] = (v0u.scalarUChar[i] > v1u.scalarUChar[i]) ? v0u.scalarUChar[i] : v1u.scalarUChar[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarChar[0] = (v0u.scalarChar[0] > v1u.scalarChar[0]) ? v0u.scalarChar[0] : v1u.scalarChar[0];
        for(int i=1;i<16;i++)
        {
            r.scalarChar[i] = (v0u.scalarChar[i] > v1u.scalarChar[i]) ? v0u.scalarChar[i] : v1u.scalarChar[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarUChar[0] = (v0u.scalarUChar[0] < v1u.scalarUChar[0]) ? v0u.scalarUChar[0] : v1u.scalarUChar[0];
        for(int i=1;i<16;i++)
        {
            r.scalarUChar[i] = (v0u.scalarUChar[i] < v1u.scalarUChar[i]) ? v0u.scalarUChar[i] : v1u.scalarUChar[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion r, v0u, v1u;
        v0u.vector = v0;
        v1u.vector = v1;
        r.scalarChar[0] = (v0u.scalarChar[0] < v1u.scalarChar[0]) ? v0u.scalarChar[0] : v1u.scalarChar[0];
        for(int i=1;i<16;i++)
        {
            r.scalarChar[i] = (v0u.scalarChar[i] < v1u.scalarChar[i]) ? v0u.scalarChar[i] : v1u.scalarChar[i];
        }
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion v0u, shiftCountU, r;
        v0u.vector = v0;
        shiftCountU.vector = shiftCount;
        r.scalarUInt[0] = v0u.scalarUInt[0] << shiftCountU.scalarUInt[0];
        r.scalarUInt[1] = v0u.scalarUInt[1] << shiftCountU.scalarUInt[1];
        r.scalarUInt[2] = v0u.scalarUInt[2] << shiftCountU.scalarUInt[2];
        r.scalarUInt[3] = v0u.scalarUInt[3] << shiftCountU.scalarUInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion v0u, shiftCountU, r;
        v0u.vector = v0;
        shiftCountU.vector = shiftCount;
        r.scalarUInt[0] = v0u.scalarUInt[0] >> shiftCountU.scalarUInt[0];
        r.scalarUInt[1] = v0u.scalarUInt[1] >> shiftCountU.scalarUInt[1];
        r.scalarUInt[2] = v0u.scalarUInt[2] >> shiftCountU.scalarUInt[2];
        r.scalarUInt[3] = v0u.scalarUInt[3] >> shiftCountU.scalarUInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion v0u, shiftCountU, r;
        v0u.vector = v0;
        shiftCountU.vector = shiftCount;
        r.scalarInt[0] = v0u.scalarInt[0] << shiftCountU.scalarInt[0];
        r.scalarInt[1] = v0u.scalarInt[1] << shiftCountU.scalarInt[1];
        r.scalarInt[2] = v0u.scalarInt[2] << shiftCountU.scalarInt[2];
        r.scalarInt[3] = v0u.scalarInt[3] << shiftCountU.scalarInt[3];
        return r.vector;
    }

    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        using namespace rw::math::vpu::detail;
        VectorIntrinsicUnion v0u, shiftCountU, r;
        v0u.vector = v0;
        shiftCountU.vector = shiftCount;
        // Note: Technically these shift operations are not portable because
        // the C standard does dictate whether or not sign extension must
        // occur.  But in practice this should be fine for all current platforms.
        r.scalarInt[0] = v0u.scalarInt[0] >> shiftCountU.scalarInt[0];
        r.scalarInt[1] = v0u.scalarInt[1] >> shiftCountU.scalarInt[1];
        r.scalarInt[2] = v0u.scalarInt[2] >> shiftCountU.scalarInt[2];
        r.scalarInt[3] = v0u.scalarInt[3] >> shiftCountU.scalarInt[3];
        return r.vector;
    }


} // namespace simulated_vecops
} // namespace detail
} // namespace vpl
} // namespace math
} // namespace rw

#endif
