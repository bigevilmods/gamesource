// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VEC_OPERATION_INLINE_H
#define RW_MATH_VEC_OPERATION_INLINE_H

#include "rw/math/detail/ps3/spu/spu_macros.h"

namespace rw
{
namespace math
{
namespace vpl
{

namespace detail
{
    // This templated structure allows for multiple implementations of vector
    // shift depending on whether or not a shift by bytes and/or a shift by
    // bits is required.
    template <bool ByteShiftRequired, bool BitShiftRequired, int ShiftAmount>
    struct VecShiftLeftImmHelper
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0);
    };

    template<int ShiftAmount>
    struct VecShiftLeftImmHelper<true, true, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            vector signed char shiftedByBytes = __builtin_si_shlqbyi( (vector signed char)v0, ShiftAmount/8 );
            return (VectorIntrinsic)__builtin_si_shlqbii( shiftedByBytes, ShiftAmount );
        }
    };

    template<int ShiftAmount>
    struct VecShiftLeftImmHelper<true, false, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return (VectorIntrinsic)__builtin_si_shlqbyi( (vector signed char)v0, ShiftAmount/8 );
        }
    };

    template<int ShiftAmount>
    struct VecShiftLeftImmHelper<false, true, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return (VectorIntrinsic)__builtin_si_shlqbii( (vector signed char)v0, ShiftAmount );
        }
    };

    template<int ShiftAmount>
    struct VecShiftLeftImmHelper<false, false, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return v0;
        }
    };

    // This templated structure allows for multiple implementations of vector
    // shift depending on whether or not a shift by bytes and/or a shift by
    // bits is required.
    template <bool ByteShiftRequired, bool BitShiftRequired, int ShiftAmount>
    struct VecShiftRightImmHelper
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0);
    };

    template<int ShiftAmount>
    struct VecShiftRightImmHelper<true, true, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            vector signed char shiftedByBytes = __builtin_si_rotqmbyi( (vector signed char)v0, -ShiftAmount/8 );
            return (VectorIntrinsic)__builtin_si_rotqmbii( shiftedByBytes, -(ShiftAmount&7) );
        }
    };

    template<int ShiftAmount>
    struct VecShiftRightImmHelper<true, false, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return (VectorIntrinsic)__builtin_si_rotqmbyi( (vector signed char)v0, -ShiftAmount/8 );
        }
    };

    template<int ShiftAmount>
    struct VecShiftRightImmHelper<false, true, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return (VectorIntrinsic)__builtin_si_rotqmbii( (vector signed char)v0, -(ShiftAmount&7) );
        }
    };

    template<int ShiftAmount>
    struct VecShiftRightImmHelper<false, false, ShiftAmount>
    {
        static RW_MATH_FORCE_INLINE VectorIntrinsic Shift(VectorIntrinsicInParam v0)
        {
            return v0;
        }
    };

} // namespace detail

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadLeftAligned(const void *base, int offset)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(base) + offset;
        unsigned int shift = address & 15;
        return (VectorIntrinsic)spu_slqwbyte( *reinterpret_cast<vector unsigned char*>(address), shift );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadRightAligned(const void *base, int offset)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(base) + offset;
        vector signed char shift = __builtin_si_ori( (vector signed char)spu_promote( address, 0 ), 16 );
        return (VectorIntrinsic)__builtin_si_rotqmby( *reinterpret_cast<vector signed char*>(address), shift );
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadUnaligned(const void *base, int offset)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(base) + offset;
        unsigned int shiftLeft = address & 15;
        vector signed char shiftRight = __builtin_si_ori( (vector signed char)spu_promote( address, 0 ), -16 );
        return (VectorIntrinsic)__builtin_si_or( 
                    spu_slqwbyte( *reinterpret_cast<vector signed char*>(address), shiftLeft ),
                    __builtin_si_rotqmby( *reinterpret_cast<vector signed char*>(address+16), shiftRight )
                );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMergeH( v0, v1 );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecPerm( v0, v1, rw::math::vpu::detail::gMergeHighBytePermuteConstant);
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecPerm( v0, v1, rw::math::vpu::detail::gMergeHighHalfPermuteConstant);
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecMergeL( v0, v1 );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecPerm( v0, v1, rw::math::vpu::detail::gMergeLowBytePermuteConstant);
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RWMATH_VPU_DETAIL_VecPerm( v0, v1, rw::math::vpu::detail::gMergeLowHalfPermuteConstant);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_nor / nor
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNor(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return RW_MATH_VPU_Not_R(v0, v1); 
    }

    // VPL_PLATFORM_DOC On the SPU, this instruction also supports indexes outside the range 0-31.  
    //  - If the most significant 2 bits of the index are '01', then the corresponding byte in the output will be 0.
    //  - If the most significant 3 bits of the index are '110', then the corresponding byte in the output will be 0xff.
    //  - If the most significant 3 bits of the index are '111', then the corresponding byte in the output will be 0x80.
    //  For maximum portability, code should not rely on this extra behavior, because it is specific to the SPU.
    //
    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_shuffle / shufb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecPermute(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1, VectorIntrinsicInParam permuteConstant)
    {
        return RWMATH_VPU_DETAIL_VecPerm(v0, v1, permuteConstant);
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int PermConstant> RW_MATH_FORCE_INLINE VectorIntrinsic VecPermuteSingleImm(VectorIntrinsicInParam v0)
    {
        rw::math::detail::Assert_ValidSinglePermConstant<PermConstant>();
        VectorIntrinsic result;
        RW_MATH_VPU_PermuteSingle_I(result, v0, PermConstant);
        return result;
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Byte(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ElementIndex, 0, 15 >();
        // The permute control value is created using an immediate load
        // instruction to avoid requiring additional memory for the permution
        // constant.
        return RWMATH_VPU_DETAIL_VecPerm( v0, v0, __builtin_si_ilh((ElementIndex << 8) + ElementIndex) );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Half(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ElementIndex, 0, 7 >();
        // The permute control value is created using an immediate load
        // instruction to avoid requiring additional memory for the permution
        // constant.
        return RWMATH_VPU_DETAIL_VecPerm( v0, v0, __builtin_si_ilh( ((ElementIndex*sizeof(short))<<8) + ((ElementIndex*sizeof(short)) + 1) ) );
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API void VecStoreLeftAligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(base) + offset;
        VectorIntrinsic *dest = reinterpret_cast<VectorIntrinsic*>(address);
        vector signed char shift = (vector signed char)spu_promote( 32-(address&15), 0 );
        // Bit mask has all bits set to 1
        vector signed char bitMask = __builtin_si_ilh( -1 );
        vector signed char inputShiftedRight = (vector signed char)__builtin_si_rotqmby( (vector signed char)v0, shift );
        vector signed char maskShiftedRight = (vector signed char)__builtin_si_rotqmby( bitMask, shift );
        *dest = RW_MATH_VPU_Select_R( (vector signed char)*dest, inputShiftedRight, maskShiftedRight ); 
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API void VecStoreRightAligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(base) + offset;
        VectorIntrinsic *dest = reinterpret_cast<VectorIntrinsic*>(address);
        vector signed char shift = (vector signed char)spu_promote( 32-(address&15), 0 );
        // Bit mask has all bits set to 1
        vector signed char bitMask = __builtin_si_ilh( -1 );
        vector signed char inputRotateRight = (vector signed char)__builtin_si_rotqby( (vector signed char)v0, shift );
        vector signed char maskShiftedRight = (vector signed char)__builtin_si_rotqmby( bitMask, shift );
        *dest = RW_MATH_VPU_Select_R( inputRotateRight, (vector signed char)*dest, maskShiftedRight ); 
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API void VecStoreUnaligned(VectorIntrinsicInParam v0, void *base, int offset)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(base) + offset;
        VectorIntrinsic *destLeft = reinterpret_cast<VectorIntrinsic*>(address);
        VectorIntrinsic *destRight = reinterpret_cast<VectorIntrinsic*>(address+16);
        const VectorIntrinsic leftValue = *destLeft;
        const VectorIntrinsic rightValue = *destRight;
        vector signed char shift = (vector signed char)spu_promote( 32-(address&15), 0 );
        // Bit mask has all bits set to 1
        vector signed char bitMask = __builtin_si_ilh( -1 );
        vector signed char inputShiftedRight = (vector signed char)__builtin_si_rotqmby( (vector signed char)v0, shift );
        vector signed char maskShiftedRight = (vector signed char)__builtin_si_rotqmby( bitMask, shift );
        *destLeft = RW_MATH_VPU_Select_R( (vector signed char)leftValue, inputShiftedRight, maskShiftedRight ); 

        vector signed char inputRotateRight = (vector signed char)__builtin_si_rotqby( (vector signed char)v0, shift );
        *destRight = RW_MATH_VPU_Select_R( inputRotateRight, (vector signed char)rightValue, maskShiftedRight ); 
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API void VecStoreWordElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(base) + offset;
        VectorIntrinsic *dest = reinterpret_cast<VectorIntrinsic*>(address);
        vector signed char highWordMask = __builtin_si_fsmbi(0xf000);       // Create a bit mask for the high word
        vector signed char shift = (vector signed char)spu_promote( -address, 0 );
        vector signed char wordMask = __builtin_si_rotqby( highWordMask, shift ); // Shift the high word right so that the correct word is masked.
        *dest = RW_MATH_VPU_Select_R( *dest, v0, wordMask );
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API void VecStoreHalfElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(base) + offset;
        VectorIntrinsic *dest = reinterpret_cast<VectorIntrinsic*>(address);
        vector signed char highHalfMask = __builtin_si_fsmbi(0xc000);       // Create a bit mask for the high half word
        vector signed char shift = (vector signed char)spu_promote( -address, 0 );
        vector signed char halfMask = __builtin_si_rotqby( highHalfMask, shift ); // Shift the high half word right so that the correct half word is masked.
        *dest = RW_MATH_VPU_Select_R( *dest, v0, halfMask );
    }

    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API void VecStoreByteElement(VectorIntrinsicInParam v0, void *base, int offset)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(base) + offset;
        VectorIntrinsic *dest = reinterpret_cast<VectorIntrinsic*>(address);
        vector signed char highByteMask = __builtin_si_fsmbi(0x8000);       // Create a bit mask for the high byte
        vector signed char shift = (vector signed char)spu_promote( -address, 0 );
        vector signed char byteMask = __builtin_si_rotqby( highByteMask, shift ); // Shift the high byte right so that the correct byte is masked.
        *dest = RW_MATH_VPU_Select_R( *dest, v0, byteMask );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_add / ah
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_spu_add_3( (vector signed short)v0, (vector signed short)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_sub / sfh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_spu_sub_0( (vector unsigned short)v0, (vector unsigned short)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_add / ah
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_spu_add_3( (vector signed short)v0, (vector signed short)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_sub / sfh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_spu_sub_1( (vector signed short)v0, (vector signed short)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_add / a
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_spu_add_0( (vector unsigned int)v0, (vector unsigned int)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_sub / sf
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_spu_sub_2( (vector unsigned int)v0, (vector unsigned int)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_add / a
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_spu_add_1( (vector signed int)v0, (vector signed int)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_sub / sf
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_spu_sub_3( (vector signed int)v0, (vector signed int)v1 );
    }

    // VPL_PLATFORM_DOC This function takes several instructions on the SPU, because it must detect
    // if the result underflows or overflows a 32bit integer.  If you are using this function and know
    // that results can only underflow or only overflow, you could write faster code that checks for one
    // of these conditions.
    //
    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        // This optimized SPU code is "equivalent" to this scalar code that does saturated add for a single pair of values.
        //
        // int add_saturate(int a, int b)
        // {
        //  int sa = a < 0;
        //  int sb = b < 0;
        //  unsigned int addResult = (unsigned int)a + (unsigned int)b;
        //  if( sa == sb )
        //  {
        //      // Signs are the same, so over/under flow is possible.
        //      if( sa == 0 )
        //      {
        //          // Both signs are positive
        //          if( addResult > 0x7fffffffU )
        //          {
        //              return 0x7fffffffU;
        //          }
        //          else
        //          {
        //              return addResult;
        //          }
        //      }
        //      else
        //      {
        //          // Both signs are positive
        //          if( addResult < 0x80000000U )
        //          {
        //              return 0x80000000U;
        //          }
        //          else
        //          {
        //              return addResult;
        //          }
        //      }
        //  }
        //  return (int)addResult;
        // }
        const VectorIntrinsic minInt = (VectorIntrinsic)__builtin_si_ilhu(0x8000);                        // Minimum value of a signed int
        const VectorIntrinsic maxInt = (VectorIntrinsic)__builtin_si_ai((vector signed char)minInt, -1);  // Maximum value of a signed int

        VectorIntrinsic addResult = (VectorIntrinsic)__builtin_spu_add_0( (vector unsigned int)v0, (vector unsigned int)v1 );
        VectorIntrinsic v0Positive = (VectorIntrinsic)__builtin_si_cgti( (vector signed char)v0, -1 );      // Mask to indicate if v0 is positive
        VectorIntrinsic v1Positive = (VectorIntrinsic)__builtin_si_cgti( (vector signed char)v1, -1 );      // Mask to indicate if v1 is positive
        VectorIntrinsic signsEqual = (VectorIntrinsic)__builtin_spu_eqv_3( (vector unsigned int)v0Positive, (vector unsigned int)v1Positive );
        VectorIntrinsic addResultPositive = (VectorIntrinsic)__builtin_si_cgti( (vector signed char)addResult, -1 );      // Mask to indicate if addResult is positive
        VectorIntrinsic negativeCandidateResult = RW_MATH_VPU_Select_R( addResult, minInt, addResultPositive );
        VectorIntrinsic positiveCandidateResult = RW_MATH_VPU_Select_R( maxInt, addResult, addResultPositive );
        VectorIntrinsic candidateResult = RW_MATH_VPU_Select_R( negativeCandidateResult, positiveCandidateResult, v0Positive );
        VectorIntrinsic result = RW_MATH_VPU_Select_R( addResult, candidateResult, signsEqual );
        return result;
    }

    // VPL_PLATFORM_DOC This function takes several instructions on the SPU, because it must detect
    // if the result underflows or overflows a 32bit integer.  If you are using this function and know
    // that results can only underflow or only overflow, you could write faster code that checks for one
    // of these conditions.
    //
    // VPL_PLATFORM_RATING_3
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        // One instruction could be saved by implementing code similar to VecAddSat_Int.
        return VecAddSat_Int( v0, (VectorIntrinsic)__builtin_spu_sub_9(0, (vector signed int)v1) );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic addResult = (VectorIntrinsic)__builtin_spu_add_0( (vector unsigned int)v0, (vector unsigned int)v1 );
        VectorIntrinsic carryResult = (VectorIntrinsic)__builtin_spu_genc_1( (vector unsigned int)v0, (vector unsigned int)v1 );
        VectorIntrinsic one = (VectorIntrinsic)__builtin_si_ila(1);
        VectorIntrinsic overflowMask = (VectorIntrinsic)__builtin_spu_cmpeq_4( (vector unsigned int)one, (vector unsigned int)carryResult );
        VectorIntrinsic result = RW_MATH_VPU_Or_R( overflowMask, addResult );
        return result;
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic subResult = (VectorIntrinsic)__builtin_spu_sub_2( (vector unsigned int)v0, (vector unsigned int)v1 );
        VectorIntrinsic underflowMask = (VectorIntrinsic)__builtin_si_clgt( (vector signed char)v1, (vector signed char)v0 );
        VectorIntrinsic result = RW_MATH_VPU_And_Rc( subResult, underflowMask );
        return result;
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_convts / cflts
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToSignedFixedPoint(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return (VectorIntrinsic)__builtin_spu_convts( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_convtf / csflt
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertSignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return (VectorIntrinsic)__builtin_spu_convtf_1( (vector signed int)v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_convtu / cfltu
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToUnsignedFixedPoint(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return (VectorIntrinsic)__builtin_spu_convtu( v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_convtf / cuflt
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertUnsignedFixedPointToFloat(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< ScaleExponent, 0, 31 >();
        return (VectorIntrinsic)__builtin_spu_convtf_0( (vector unsigned int)v0, ScaleExponent );
    }

    // VPL_PLATFORM_RATING_1
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Char()
    {
        
        detail::Assert_5BitSignedValue<Value>();
        return (VectorIntrinsic)__builtin_si_ilh( (Value * 256) + (Value & 0xff) );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT il
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Int()
    {
        detail::Assert_5BitSignedValue<Value>();
        return (VectorIntrinsic)__builtin_si_il(Value);
    }

    // VPL_PLATFORM_RATING_1    
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoadImm_Int()
    {
        return RWMATH_VPU_DETAIL_VecSplat(spu_promote(Value, 0), 0);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT ilh
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Short()
    {
        detail::Assert_5BitSignedValue<Value>();
        return (VectorIntrinsic)__builtin_si_ilh( Value );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Mask, int RotateLeft> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftWithMaskedInsertionImm(VectorIntrinsicInParam original, VectorIntrinsicInParam vecToInsert)
    {
        detail::Assert_ValueInRange< Mask, 0, 15 >();
        detail::Assert_ValueInRange< RotateLeft, 0, 3 >();
        VectorIntrinsic permConstant = (VectorIntrinsic)RWMATH_VPU_DETAIL_GetVectorPermuteConstantI(
                (::rw::math::vpu::detail::VectorPermSource) ( ( (Mask>>3)&1 ) * ((RotateLeft+0)&3) + ( 1-((Mask>>3)&1)  ) * 4 ),
                (::rw::math::vpu::detail::VectorPermSource) ( ( (Mask>>2)&1 ) * ((RotateLeft+1)&3) + ( 1-((Mask>>2)&1)  ) * 5 ),
                (::rw::math::vpu::detail::VectorPermSource) ( ( (Mask>>1)&1 ) * ((RotateLeft+2)&3) + ( 1-((Mask>>1)&1)  ) * 6 ),
                (::rw::math::vpu::detail::VectorPermSource) ( ( (Mask>>0)&1 ) * ((RotateLeft+3)&3) + ( 1-((Mask>>0)&1)  ) * 7 )
            );
        return RWMATH_VPU_DETAIL_VecPerm(vecToInsert, original, permConstant);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_sl / shli
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm_UInt(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 31 >();
        return (VectorIntrinsic)__builtin_si_shli( (vector signed char)v0, Shift);
    }

    // VPL_PLATFORM_RATING_1
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_UInt(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 31 >();
        return (VectorIntrinsic)__builtin_si_rotmi( (vector signed char)v0, -Shift);
    }

    // VPL_PLATFORM_RATING_1
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_Int(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 31 >();
        return (VectorIntrinsic)__builtin_si_rotmai( (vector signed char)v0, -Shift);
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 127 >();
        return detail::VecShiftLeftImmHelper< (Shift >= 8), ((Shift&7)!=0), Shift>::Shift(v0);
    }

    // VPL_PLATFORM_RATING_2
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm(VectorIntrinsicInParam v0)
    {
        detail::Assert_ValueInRange< Shift, 0, 127 >();
        return detail::VecShiftRightImmHelper< Shift >= 8, (Shift&7)!=0, Shift>::Shift(v0);
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_slqwbytebc / shlqbybi
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return (VectorIntrinsic)__builtin_si_shlqbybi( (vector signed char)v0, (vector signed char)shiftAmount );
    }

	// VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_slqw / shlqbi
	VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        return (VectorIntrinsic)__builtin_si_shlqbi( (vector signed char)v0, (vector signed char)shiftAmount );
    }

    // VPL_PLATFORM_RATING_2
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
        // The Xor/Add pair do a manual two's complement of the shift amount.
        // The entire shift amount is encoded in the lower 8 bits (5 of which
        // are the byte count).  Early versions of the SPU assembly language
        // reference incorrectly said this was a 7bit field.
        return (VectorIntrinsic)__builtin_si_rotqmbybi( (vector signed char)v0, __builtin_si_ai( __builtin_si_xori((vector signed char)shiftAmount, 0xff), 8) );
    }

	// VPL_PLATFORM_RATING_2
    // VPL_PLATFORM_EQUIVALENT spu_rlmaskqw / rotqmbi
	VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount)
    {
		// The Xor/Add pair do a manual two's complement of the shift amount.
        return (VectorIntrinsic)__builtin_si_rotqmbi( (vector signed char)v0, __builtin_si_ai( __builtin_si_xori((vector signed char)shiftAmount, 0xff), 1) );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_sl / shl
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return (VectorIntrinsic)__builtin_si_shl( (vector signed char)v0, (vector signed char)shiftCount );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return (VectorIntrinsic)__builtin_si_rotm( (vector signed char)v0, RWMATH_VPU_DETAIL_SubtractIntVectorFromImmediate( shiftCount, 0 ) );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_sl / shl
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return (VectorIntrinsic)__builtin_si_shl( (vector signed char)v0, (vector signed char)shiftCount );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount)
    {
        return (VectorIntrinsic)__builtin_si_rotma( (vector signed char)v0, RWMATH_VPU_DETAIL_SubtractIntVectorFromImmediate( shiftCount, 0 ) );
    }


    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpeq / ceq
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_ceq( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpgt / clgt
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_clgt( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_clgt( (vector signed char)v1, (vector signed char)v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpeq / ceq
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_ceq( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpgt / cgt
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_cgt( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_cgt( (vector signed char)v1, (vector signed char)v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpeq / ceqh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_ceqh( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpgt / clgth
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_clgth( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_clgth( (vector signed char)v1, (vector signed char)v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpeq / ceqh
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_ceqh( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpgt / cgth
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_cgth( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_cgth( (vector signed char)v1, (vector signed char)v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpeq / ceqb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_ceqb( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpgt / clgtb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_clgtb( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_clgtb( (vector signed char)v1, (vector signed char)v0 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpeq / ceqb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_ceqb( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    // VPL_PLATFORM_EQUIVALENT spu_cmpgt / cgtb
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_cgtb( (vector signed char)v0, (vector signed char)v1 );
    }

    // VPL_PLATFORM_RATING_1
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        return (VectorIntrinsic)__builtin_si_cgtb( (vector signed char)v1, (vector signed char)v0 );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_clgt( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v1, v0, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_clgth( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v1, v0, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_clgtb( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v1, v0, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_cgt( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v1, v0, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_cgth( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v1, v0, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_cgtb( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v1, v0, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_clgt( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v0, v1, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_clgth( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v0, v1, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_clgtb( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v0, v1, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_cgt( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v0, v1, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_cgth( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v0, v1, greaterThanMask );
    }

    // VPL_PLATFORM_RATING_2
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1)
    {
        VectorIntrinsic greaterThanMask = (VectorIntrinsic)__builtin_si_cgtb( (vector signed char)v0, (vector signed char)v1 );
        return RW_MATH_VPU_Select_R( v0, v1, greaterThanMask );
    }


} // namespace vpl
} // namespace math
} // namespace rw

#ifdef RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#include "rw/math/vpl/detail/simulated/vec_operation_inline.h"

namespace rw
{
namespace math
{
namespace vpl
{
    using rw::math::vpl::detail::simulated_vecops::VecFloor;
    using rw::math::vpl::detail::simulated_vecops::VecLog2Est;
    using rw::math::vpl::detail::simulated_vecops::VecAdd_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecAdd_Char;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_Char;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecAddSat_Short;
    using rw::math::vpl::detail::simulated_vecops::VecSub_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecSub_Char;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_UChar;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_Char;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_UShort;
    using rw::math::vpl::detail::simulated_vecops::VecSubSat_Short;

} // namespace vpl
} // namespace math
} // namespace rw

#endif // RW_MATH_ENABLE_SIMULATED_VPL_OPERATIONS

#endif // RW_MATH_VEC_OPERATION_INLINE_H
