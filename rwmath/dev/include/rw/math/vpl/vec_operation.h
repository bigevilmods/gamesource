// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_VEC_OPERATION_H
#define RW_MATH_VEC_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "rw/math/vpl/detail/vplbase.h"

namespace rw
{
namespace math
{
namespace vpl
{

    /*! \brief Loads a vector of data from an <em>aligned</em> memory address indexed by base + offset.
     *
     *  This function assumes that the effective address being loaded is
     *  vector aligned.  If the address is not vector aligned the result of the
     *  load will be platform dependent and could generate an exception.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoad(const void *base, int offset);

    /*! \brief Stores a vector to a <em>aligned</em> memory address indexed by base + offset.
     *
     *  This function assumes that the effective address being stored to is
     *  vector aligned.  If the address is not vector aligned the result of the
     *  store will be platform dependent and could generate an exception.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API void VecStore(VectorIntrinsicInParam v0, void *base, int offset);

// Define API's that are conditional on the platform
#if VPL_SUPPORTED_PLATFORM( VPL_SSE | VPL_PPU | VPL_SPU | VPL_XENON | VPL_SSE2 | VPL_NEON)
    /*! \brief Loads a, potentially unaligned, vector of data indexed by base + offset into a vector.
     *
     *  Most vector hardware is only able to directly load a vector of data
     *  that is aligned on a 16 byte boundary.  This function allows unaligned data to be
     *  efficiently loaded into a vector register.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadUnaligned(const void *base, int offset);

    /*! \brief Stores a vector to a potentially unaligned memory address indexed by base + offset.
     *
     *  Most vector hardware is only able to directly store a vector of data
     *  that is aligned on a 16 byte boundary.  This function allows unaligned
     *  data to be efficiently stored from a vector register.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API void VecStoreUnaligned(VectorIntrinsicInParam v0, void *base, int offset);

    /*! \brief Interleaves the high word elements of two vectors.
     *
     * This image shows how the elements of \c v0 are interleaved with elements
     * from \c v1 to form the result \c R.
     *
     * \image html VecMergeHigh_Word.png
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Interleaves the low word elements of two vectors.
     *
     * This image shows how the elements of \c v0 are interleaved with elements
     * from \c v1 to form the result \c R.
     *
     * \image html VecMergeLow_Word.png
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Word(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs a bit-wise 'Nor' of two vectors
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNor(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Permutes the word elements of a single vector and returns the result.
     *
     *  This function allows you to create a new vector by selecting any
     *  combination of word elements from the input vector.  This operation
     *  is similar to the swizzle operations supported in languages like HLSL.
     *
     *  The permution indices are described using a 16bit integer.  Each nibble
     *  (or hex digit) of the integer describes an index into the input vector.
     *  (The most significant nibble is the index for X, and the least significant
     *  is the index for W).
     *
     *  Here is an example that will hopefully clarify the encoding:
     *  \code
     *      rw::math::vpu::Vector4 v(2.0f, 4.0f, 6.0f, 8.0f);
     *      VectorIntrinsic result = VecPermuteSingleImm<0x3120>(v.GetVector());    // Swizzle is WYZX
     *      // Result should now be equal to Vector4(8.0f, 4.0f, 6.0f, 2.0f)
     *  \endcode
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int PermConstant> RW_MATH_FORCE_INLINE VectorIntrinsic VecPermuteSingleImm(VectorIntrinsicInParam v0);
    
#endif

#if VPL_SUPPORTED_PLATFORM( VPL_SSE | VPL_PPU | VPL_SPU | VPL_XENON | VPL_SSE2 | VPL_NEON)
    /*! \brief Compute the absolute value of a vector
     *
     *  Computes the element-wise absolute value of a floating point vector.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAbs(VectorIntrinsicInParam v);

    /*! \brief Add two vectors
     *  
     *  Performs element-wise floating point addition between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns "true" iff \em all elements of \c v are true.
     *  
     *  This function may be slow on the 360 and the PS3 PPU because it must
     *  update the condition register.  Branch free code should generally be preferred
     *  over use of this function.  However, there are cases where it may be useful
     *  to "early out" of a loop or expensive test when multiple elements pass/fail a
     *  condition.
     *
     *  Note: This function treats a word equal to \c 0xffffffff as true and \c 0x00000000 as false.
     *  Other word values will give undefined results.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API bool VecAllTrue_Word(VectorIntrinsicInParam v);

    /*! \brief Returns "true" iff \em all elements of \c v are false.
     *  
     *  This function may be slow on the 360 and the PS3 PPU because it must
     *  update the condition register.  Branch free code should generally be preferred
     *  over use of this function.  However, there are cases where it may be useful
     *  to "early out" of a loop or expensive test when multiple elements pass/fail a
     *  condition.
     *
     *  Note: This function treats a word equal to \c 0xffffffff as true and \c 0x00000000 as false.
     *  Other word values will give undefined results.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API bool VecAllFalse_Word(VectorIntrinsicInParam v);

    /*! \brief Performs a bit-wise 'And' of two vectors
     *  
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAnd(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs a bit-wise 'AndComplement' of two vectors
     *  
     *  This function complements each bit in v1.  Then it AND's together v0
     *  and v1's complement.  In C notation it is equivalent to (v0 & (~v1)).
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAndComplement(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns "true" iff \em any elements of \c v are true.
     *  
     *  This function may be slow on the 360 and the PS3 PPU because it must
     *  update the condition register.  Branch free code should generally be preferred
     *  over use of this function.  However, there are cases where it may be useful
     *  to "early out" of a loop or expensive test when multiple elements pass/fail a
     *  condition.
     *
     *  Note: This function treats a word equal to \c 0xffffffff as true and \c 0x00000000 as false.
     *  Other word values will give undefined results.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API bool VecAnyTrue_Word(VectorIntrinsicInParam v);

    /*! \brief Returns "true" iff \em any elements of \c v are false.
     *  
     *  This function may be slow on the 360 and the PS3 PPU because it must
     *  update the condition register.  Branch free code should generally be preferred
     *  over use of this function.  However, there are cases where it may be useful
     *  to "early out" of a loop or expensive test when multiple elements pass/fail a
     *  condition.
     *
     *  Note: This function treats a word equal to \c 0xffffffff as true and \c 0x00000000 as false.
     *  Other word values will give undefined results.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API bool VecAnyFalse_Word(VectorIntrinsicInParam v);

    /*! \brief Performs an element-wise floating point equality comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If the elements are equal under \em floating-point
     *  comparison, the corresponding element of the result has all its bits
     *  set to 1.  Otherwise, the corresponding elements of the result are set
     *  to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise floating point greater than or equal to comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is greater or equal to an element
     *  of v1 under \em floating-point comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise floating point greater than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is greater than an element
     *  of v1 under \em floating-point comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise floating point less than or equal to comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is less or equal to an element
     *  of v1 under \em floating-point comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessEqual(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise floating point less than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is less than an element
     *  of v1 under \em floating-point comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector consisting entirely of the value 1 (in floating point).
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetOne();

    /*! \brief Returns a vector consisting entirely of the value 0.5 (in floating point).
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetHalf();

    /*! \brief Returns a vector consisting entirely of the value 2 (in floating point).
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetTwo();

    /*! \brief Returns a vector consisting entirely of the value 0 (in floating point).
     *
     * Note: A floating point zero has a bit pattern of all 0's.  So, this
     * function also returns a 0 vector when it is interpretted as a integer
     * vector.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetZero();

    /*! \brief Returns a vector consisting entirely of the value -1 (in floating point).
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegOne();

    /*! \brief Returns a vector consisting entirely of the value -0.5 (in floating point).
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegHalf();

    /*! \brief Returns a vector consisting entirely of the value -2 (in floating point).
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegTwo();

    /*! \brief Returns a vector consisting entirely of the value -0 (in floating point).
     *
     *  Floating point numbers allow for both 0 and -0 to be represented.  The
     *  only difference between the floating point representation of these two
     *  values, is the sign bit.  A negative zero is encoded as 0x80000000 if
     *  viewed as an integer.  This value is useful, because XOR'ing a vector by
     *  this value will negate a vector.  AndComplement'ing a vector with this value
     *  will return the vector's absolute value.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecGetNegZero();

    /*! \brief Returns a vector that contains the element-wise maximum of two floating point vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the maximum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise minimum of two floating point vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the minimum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise product of two floating point vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMult(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the product of two vectors added to a third vector.
     *
     *  For each element the result is equal to (mult0 * mult1) + add.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMultAdd(VectorIntrinsicInParam mult0, VectorIntrinsicInParam mult1, VectorIntrinsicInParam add);

    /*! \brief Computes the dot product (of 3 elements) between two vectors and replicates the value to all elements of the result vector.
     *
     *  This function is equivalent to \c vmsum3fp on Xenon.  It computes the dot
     *  product of the x, y, z components of two floating point input vectors.
     *  This result is then returned in all elements of the result vector.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMultSum3(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Computes the dot product (of 4 elements) between two vectors and replicates the value to all elements of the result vector.
     *
     *  This function is equivalent to \c vmsum4fp on Xenon.  It computes the dot
     *  product of the x, y, z, w components of two floating point input vectors.
     *  This result is then returned in all elements of the result vector.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMultSum4(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Computes -((mult0 * mult1) - sub) for each floating point element of the result vectors.
     *
     *  This function is similar to the \ref VecMultAdd instruction.  Except,
     *  it negates the product.  Another way of looking at this instruction is
     *  that it computes -(mult0*mult1) + v2.
     *
     *  This function is identical to the VMX instruction vnmsubfp.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNegMultSub(VectorIntrinsicInParam mult0, VectorIntrinsicInParam mult1, VectorIntrinsicInParam sub);

    /*! \brief Performs a bit-wise 'Or' of two vectors
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecOr(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Calculates an estimate of the reciprocal of each element in the input vector.
     *
     * Note: the precision of this "estimate" may vary greatly between
     * platforms.  On the PC, a full precision calculation is performed (i.e.
     * this is not an estimate on the PC).  Xenon and PS3 calculate the reciprocal
     * using the \c vrefp instruction.  The SPU uses the estimate and
     * floating point interpolate instruction to find the reciprocal.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecRecipEst(VectorIntrinsicInParam v0);

    /*! \brief Calculates an estimate of the reciprocal of the squareroot each element in the input vector.
     *
     * Note: the precision of this "estimate" may vary greatly between
     * platforms.  On the PC, a full precision calculation is performed (i.e.
     * this is not an estimate on the PC).  Xenon and PS3 calculate the reciprocal squareroot
     * using the \c vrsqrtefp instruction.  The SPU uses the estimate and
     * floating point interpolate instruction to find the reciprocal squareroot.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecRecipSqrtEst(VectorIntrinsicInParam v0);

    /*! \brief Rotates the entire vector left by 'RotateAmount' words.
     *
     * The entire contents of the vector are rotated left by \c RotateAmount words.  \c RotateAmount must be a value between 0 and 3 inclusive.
     *
     * VPL_AUTODOC
     */
    VPL_API template<int RotateAmount> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftImm(VectorIntrinsicInParam v0);

    /*! \brief Performs a bitwise select operation between two input vectors using the bit pattern in the third input vector.
     *
     *  This operation is used to select between two vectors according to a bit pattern.
     *  For each bit in v2, the corresponding result bit will be:
     *  - the corresponding bit in \c if0Result if the bit in \c selectionMask is 0
     *  - the corresponding bit in \c if1Result if the bit in \c selectionMask is 1
     *
     *  It is equivalent to the following C expression evaluated per bit:
     *  \code
     *      result = selectionMask ? if1Result : if0Result;
     *  \endcode
     *
     *  Note: The order of parameters for this function is not the same as that
     *  used by the math libraries high level C++ API.  The C++ API choosing an
     *  ordering that matches the C ternary operator.  However, the low level VPL
     *  implementation uses the same ordering as the VMX \c vsel instruction.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSelect(VectorIntrinsicInParam if0Result, VectorIntrinsicInParam if1Result, VectorIntrinsicInParam selectionMask);

    /*! \brief Splats the specified element of the input vector across the result vector.
     *
     * This function takes the element indexed by the template parameter and
     * splats it (i.e. replicates the value) across all elements of the output
     * vector.  The template parameter must be between 0 and 3 inclusive.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Word(VectorIntrinsicInParam v0);

    /*! \brief Subtract two vectors
     *
     *  Performs element-wise floating point subtraction between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs a bit-wise 'Xor' of two vectors
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecXor(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs a bit-wise 'Not' of the vector
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecNot(VectorIntrinsicInParam v0);

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_VMX | VPL_SPU | VPL_NEON )

    /*! \brief Permute two vectors
     * 
     *  Permute's two vectors according to a shuffle pattern.  This function is
     *  equivalent to the VMX \c vperm instruction.  Here is some pseudocode for the
     *  operation:
     * 
     *  \code
     *  VectorIntrinsic result;
     *  char sourceArray[32] = concatenate(v0, v1);
     *  for( int i=0; i<16; i++)
     *  {
     *      result.SetByte(i, sourceArray[ permuteConstant.GetByte(i) ];
     *  }
     *  return result; 
     *  \endcode
     *  
     *  Each byte in \c permuteConstant gives an index from which to read a byte of input.
     *  Indexes 0-15 refer to bytes in \c v0, and indexes 16-31 refer to bytes
     *  in \c v1.  For portability all indexes should be in the range [0,31].
     * 
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecPermute(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1, VectorIntrinsicInParam permuteConstant);

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_VMX | VPL_SPU )
        /*! \brief Loads the vector indexed by base + offset and left aligns the data within the vector (see full description for details).
     * 
     *  This function is equivalent to the non-standard VMX instruction \c lvlx.
     *  It first computes the effective address ea = base + offset.  Then it loads the vector of
     *  memory at (ea & ~15).  Then it shifts the vector that is loaded left by
     *  ea & 15 bytes (filling with zeroes on the right).
     * 
     *  This function may be used in combination with \ref VecLoadRightAligned
     *  to load an entire of data that is misaligned.  It is also useful for
     *  loading a misaligned scalar element into the left most bytes of a vector.
     *  It can also load multiple data elements (e.g. 2 floats) as long as the data
     *  does not span a vector boundary.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadLeftAligned(const void *base, int offset);

    /*! \brief Loads the vector indexed by base + offset and right aligns the vector (see full description for details).
     * 
     *  This function is equivalent to the non-standard VMX instruction \c lvrx.
     *  It first computes the effective address ea = base + offset.  Then it loads the vector of
     *  memory at (ea & ~15).  Then it shifts the vector that is loaded right by
     *  (16 - (ea & 15)) bytes (filling with zeroes on the left).
     * 
     *  This function may be used in combination with \ref VecLoadLeftAligned
     *  to load an entire of data that is misaligned.  However, it you may
     *  want to use \ref VecLoadUnaligned to accomplish this operation.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLoadRightAligned(const void *base, int offset);

    /*! \brief Store's the left most portion of the vector that falls within the vector of memory addressed.
     *
     *  This instruction first computes the effective address ea = base +
     *  offset.  Then it stores (16-(ea%16)) of the left most bytes of the
     *  vector to memory beginning at address ea.
     *
     *  For the PPU and 360, this function maps to the \c stvlx instruction.
     *  Please consult the Xbox 360 development documentation for details and \e
     *  restrictions for this instruction.
     *
     *  This diagram demonstrates how \ref VecStoreLeftAligned and \ref VecStoreRightAligned may be used to
     *  store a vector to a memory address that is not vector aligned.
     *
     *  \image html vecstorealigned_instructions.png
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API void VecStoreLeftAligned(VectorIntrinsicInParam v0, void *base, int offset);

    /*! \brief Store's the right most portion of the vector that falls within the vector of memory addressed.
     *
     *  This instruction first computes the effective address ea = base +
     *  offset.  Then it stores (ea%16) of the right most bytes of the
     *  vector to memory beginning at address (ea & ~15).
     *
     *  For the PPU and 360, this function maps to the \c stvrx instruction.
     *  Please consult the Xbox 360 development documentation for details and \e
     *  restrictions for this instruction.
     *
     *  This diagram demonstrates how \ref VecStoreLeftAligned and \ref VecStoreRightAligned may be used to
     *  store a vector to a memory address that is not vector aligned.
     *
     *  \image html vecstorealigned_instructions.png
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API void VecStoreRightAligned(VectorIntrinsicInParam v0, void *base, int offset);

    /*! \brief Store's a word element from a vector to the address specified.
     *  
     *  This instruction first computes the effective address ea = base +
     *  offset.  Then it stores the word sized element located position (ea%16)
     *  to ea.
     *
     *  \e Tip: If the exact alignment of the pointer is unknown, you may want to
     *  consider splatting (using \ref VecSplat_Word) the element you wish to
     *  store across the entire vector.  Then, assuming the address is \e
     *  element \e aligned, you may store the splatted vector to the address to
     *  store the element you originally wanted to store.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API void VecStoreWordElement(VectorIntrinsicInParam v0, void *base, int offset);

    /*! \brief Store's a byte element from a vector to the address specified.
     *  
     *  This instruction first computes the effective address ea = base +
     *  offset.  Then it stores the byte sized element located position (ea%16)
     *  to ea.
     *
     *  \e Tip: If the exact alignment of the pointer is unknown, you may want to
     *  consider splatting (using \ref VecSplat_Byte) the element you wish to
     *  store across the entire vector.  Then, assuming the address is \e
     *  element \e aligned, you may store the splatted vector to the address to
     *  store the element you originally wanted to store.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API void VecStoreByteElement(VectorIntrinsicInParam v0, void *base, int offset);

    /*! \brief Store's a half word element from a vector to the address specified.
     *  
     *  This instruction first computes the effective address ea = base +
     *  offset.  Then it stores the half word sized element located position (ea%16)
     *  to ea.
     *
     *  \e Tip: If the exact alignment of the pointer is unknown, you may want to
     *  consider splatting (using \ref VecSplat_Half) the element you wish to
     *  store across the entire vector.  Then, assuming the address is \e
     *  element \e aligned, you may store the splatted vector to the address to
     *  store the element you originally wanted to store.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API void VecStoreHalfElement(VectorIntrinsicInParam v0, void *base, int offset);

    /*! \brief Shifts an entire 128bit vector left.
     *
     *  Shifts an entire 128bit vector left by \c Shift bits.
     *
     *  \c Shift must be in the range [0,127].
     *
     *  \e Note: Shifting an entire vector often takes multiple instructions.
     *  Shifting an entire vector generally requires a byte shift followed by a
     *  bit shift of 0-7 bits.  If possible, please consider shifting elements
     *  within the vector instead of shifting the entire vector.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm(VectorIntrinsicInParam v0);

    /*! \brief Shifts an entire 128bit vector left.
     *
     *  Shifts an entire 128bit vector right by \c Shift bits.
     *
     *  \c Shift must be in the range [0,127].
     *
     *  \e Note: Shifting an entire vector often takes multiple instructions.
     *  Shifting an entire vector generally requires a byte shift followed by a
     *  bit shift of 0-7 bits.  If possible, please consider shifting elements
     *  within the vector instead of shifting the entire vector.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm(VectorIntrinsicInParam v0);

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_VMX | VPL_SPU | VPL_SSE2 | VPL_NEON)
    /*! \brief Converts a vector consisting of floating point data, to fixed point 32bit signed integer data.
     *
     *  This function converts each floating point element in \c v0 to a signed
     *  integer.  Before doing the float to int conversion, the function scales
     *  the floating point value by pow(2.0f, ScaleExponent).  The scaled float
     *  values are clamped to the range [-2147483648, 2147483647] before the
     *  conversion.
     *
     *  Note: \c ScaleExponent must be in the range [0, 31].  Platform specific
     *  intrinsics may be used if a larger range for \c ScaleExponent is
     *  required.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToSignedFixedPoint(VectorIntrinsicInParam v0);

    /*! \brief Converts a vector consisting of fixed point 32bit signed integer data, to floating point data.
     *
     *  This function converts each signed integer element in \c v0 to a
     *  floating point value.  After doing the int to float conversion, the
     *  function scales the floating point value by pow(2.0f, -ScaleExponent).
     *
     *  Note: \c ScaleExponent must be in the range [0, 31].  Platform specific
     *  intrinsics may be used if a larger range for \c ScaleExponent is
     *  required.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertSignedFixedPointToFloat(VectorIntrinsicInParam v0);

    /*! \brief Converts a vector consisting of floating point data, to fixed point 32bit unsigned integer data.
     *
     *  This function converts each floating point element in \c v0 to a unsigned
     *  integer.  Before doing the float to int conversion, the function scales
     *  the floating point value by pow(2.0f, ScaleExponent).  The scaled float
     *  values are clamped to the range [0, 4294967295] before the conversion.
     *
     *  Note: \c ScaleExponent must be in the range [0, 31].  Platform specific
     *  intrinsics may be used if a larger range for \c ScaleExponent is
     *  required.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertFloatToUnsignedFixedPoint(VectorIntrinsicInParam v0);

    /*! \brief Converts a vector consisting of fixed point 32bit unsigned integer data, to floating point data.
     *
     *  This function converts each unsigned integer element in \c v0 to a
     *  floating point value.  After doing the int to float conversion, the
     *  function scales the floating point value by pow(2.0f, -ScaleExponent).
     *
     *  Note: \c ScaleExponent must be in the range [0, 31].  Platform specific
     *  intrinsics may be used if a larger range for \c ScaleExponent is
     *  required.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int ScaleExponent> RW_MATH_FORCE_INLINE VectorIntrinsic VecConvertUnsignedFixedPointToFloat(VectorIntrinsicInParam v0);

    /*! \brief Loads a 5bit immediate signed character into each element of the vector.
     *
     *  The 5bit immediate value specified is sign extended and then loaded
     *  into each element of the result vector.
     *  A 5bit signed value covers the range [-16, 15].
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Char();

    /*! \brief Loads a 5bit immediate signed integer into each element of the vector.
     *
     *  The 5bit immediate value specified is sign extended and then loaded
     *  into each element of the result vector.
     *  A 5bit signed value covers the range [-16, 15].
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Int();

    /*! \brief Loads a 32bit immediate signed integer into each element of the vector.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoadImm_Int();

    /*! \brief Loads a 5bit immediate signed short into each element of the vector.
     *
     *  The 5bit immediate value specified is sign extended and then loaded
     *  into each element of the result vector.
     *  A 5bit signed value covers the range [-16, 15].
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Value> RW_MATH_FORCE_INLINE VectorIntrinsic VecLoad5BitImm_Short();

    /*! \brief Interleaves the high byte elements of two vectors.
     *
     * This image shows how the elements of \c v0 are interleaved with elements
     * from \c v1 to form the result \c R.
     *
     * \image html VecMergeHigh_Byte.png
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Interleaves the high half elements of two vectors.
     *
     * This image shows how the elements of \c v0 are interleaved with elements
     * from \c v1 to form the result \c R.
     *
     * \image html VecMergeHigh_Half.png
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeHigh_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Interleaves the low byte elements of two vectors.
     *
     * This image shows how the elements of \c v0 are interleaved with elements
     * from \c v1 to form the result \c R.
     *
     * \image html VecMergeLow_Byte.png
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Byte(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Interleaves the low half elements of two vectors.
     *
     * This image shows how the elements of \c v0 are interleaved with elements
     * from \c v1 to form the result \c R.
     *
     * \image html VecMergeLow_Half.png
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMergeLow_Half(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Splats the specified element of the input vector across the result vector.
     *
     * This function takes the element indexed by the template parameter and
     * splats it (i.e. replicates the value) across all elements of the output
     * vector.  The template parameter must be between 0 and 15 inclusive.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Byte(VectorIntrinsicInParam v0);

    /*! \brief Splats the specified element of the input vector across the result vector.
     *
     * This function takes the element indexed by the template parameter and
     * splats it (i.e. replicates the value) across all elements of the output
     * vector.  The template parameter must be between 0 and 7 inclusive.
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int ElementIndex> RW_MATH_FORCE_INLINE VectorIntrinsic VecSplat_Half(VectorIntrinsicInParam v0);

    /*! \brief Add two vectors
     *  
     *  Performs element-wise signed short addition between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors
     *  
     *  Performs element-wise signed short subtraction between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors
     *  
     *  Performs element-wise unsigned short addition between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors
     *  
     *  Performs element-wise unsigned short subtraction between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors
     *  
     *  Performs element-wise signed integer addition between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors
     *  
     *  Performs element-wise signed integer subtraction between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors
     *  
     *  Performs element-wise unsigned integer addition between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors
     *  
     *  Performs element-wise unsigned integer subtraction between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors (with saturation)
     *  
     *  Performs element-wise unsigned integer addition between the vectors and saturates the result.  Saturation clamps the result to the range [-2147483648, 2147483647].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors (with saturation)
     *  
     *  Performs element-wise unsigned integer subtraction between the vectors and saturates the result.  Saturation clamps the result to the range [-2147483648, 2147483647].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors (with saturation)
     *  
     *  Performs element-wise signed integer addition between the vectors and saturates the result.  Saturation clamps the result to the range [0, 4294967296].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors (with saturation)
     *  
     *  Performs element-wise signed integer subtraction between the vectors and saturates the result.  Saturation clamps the result to the range [0, 4294967296].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_VMX | VPL_SPU | VPL_SSE2 )
    /*! \brief Returns a vector formed by combining elements of \c source and rotated elements of \c vecToInsert according to a mask.
     *  
     *  The function returns a vector result that is formed by combining
     *  \c original and the result of rotating \c vecToInsert.  Which elements
     *  are taken from \c original and which are taken from the rotate vector
     *  are determined by \c Mask.
     *
     *  Explanation of \c Mask:
     *
     *  \c Mask is a 4 bit value indicating which elements of \c original should be
     *  replaced in the final result vector.
     *
     *  \table
     *    \tr
     *      \th Element to Overwrite \endth
     *      \th Mask Value \endth
     *    \endtr
     *    \tr
     *      \td X \endtd
     *      \td 8 \endtd
     *    \endtr
     *    \tr
     *      \td Y \endtd
     *      \td 4 \endtd
     *    \endtr
     *    \tr
     *      \td Z \endtd
     *      \td 2 \endtd
     *    \endtr
     *    \tr
     *      \td W \endtd
     *      \td 1 \endtd
     *    \endtr
     *  \endtable
     *
     *  Example:
     *
     * \code
     * VectorIntrinsic original = { 1.0f, 2.0f, 3.0f, 4.0f };
     * VectorIntrinsic vecToInsert = { 10.0f, 20.0f, 30.0f, 40.0f };
     *
     * // Update the Y and Z elements of 'original' to be the Z and W values (respectively)
     * // of 'vecToInsert' and store in 'result'.
     * // i.e. result.x = original.x
     * //      result.y = vecToInsert.z
     * //      result.z = vecToInsert.w
     * //      result.w = original.w
     * // The required mask is (4 OR 2) since the Y and Z elements of the original vector are being modified.
     * // The rotate value is 1 so that after rotation the Z and W values of 'vecToInsert' are moved to elements
     * // X and Z.
     * //
     * // After the operation, result should be (1.0f, 30.0f, 40.0f, 4.0f)
     * //
     * VectorIntrinsic result = VecRotateLeftWithMaskedInsertionImm< (4|2), 1 >( original, vecToInsert );
     * \endcode
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Mask, int RotateLeft> RW_MATH_FORCE_INLINE VectorIntrinsic VecRotateLeftWithMaskedInsertionImm(VectorIntrinsicInParam original, VectorIntrinsicInParam vecToInsert);

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_VMX | VPL_SPU | VPL_SSE2 | VPL_NEON)
    /*! \brief Shifts an entire 128bit vector left by \e bytes using a 'bit count' taken from a vector register.
     *
     *  Shifts an entire 128bit vector left by 'n' bytes.  'n' is calculated by
     *  dividing a word element of \c shiftAmount by 8. 
     *
     *  \c shiftAmount must be between 0 and 127 inclusive.
     *
     *  Note: To keep this function portable between platforms each word element of \c shiftAmount should equal
     *  the number of bits to shift.  If this precondition is not met the function will fail on some platforms.
     *
     *  VPL_AUTODOC
     */
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount);

    /*! \brief Shifts an entire 128bit vector right by \e bytes using a 'bit count' taken from a vector register.
     *
     *  Shifts an entire 128bit vector right by 'n' bytes.  'n' is calculated by
     *  dividing a word element of \c shiftAmount by 8.  The shift is a logical
     *  (i.e. it does \e not perform sign extension).
     *
     *  \c shiftAmount must be between 0 and 127 inclusive.
     *
     *  Note: To keep this function portable between platforms each word element of \c shiftAmount should equal
     *  the number of bits to shift.  If this precondition is not met the function will fail on some platforms.
     *
     *  VPL_AUTODOC
     */
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightByBytesUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount);

    /*! \brief Shifts each unsigned 32bit integer element of the vector left.
     *
     *  Shifts each unsigned integer element of the vector left by \c Shift bits.
     *
     *  \c Shift must be in the range [0,31].
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftImm_UInt(VectorIntrinsicInParam v0);

    /*! \brief Shifts each unsigned 32bit integer element of the vector right.
     *
     *  Shifts each unsigned integer element of the vector right by \c Shift bits.
     *  Since the elements are treated as \e unsigned values, zeros are always
     *  shifted in on the left.
     *
     *  \c Shift must be in the range [0,31].
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_UInt(VectorIntrinsicInParam v0);

    /*! \brief Shifts each signed 32bit integer element of the vector right.
     *
     *  Shifts each signed integer element of the vector right by \c Shift bits.
     *  Since the elements are treated as \e signed values, the left most
     *  bit of each element is shifted in on the left.
     *
     *  \c Shift must be in the range [0,31].
     *
     *  VPL_AUTODOC
     */
    VPL_API template<int Shift> RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightImm_Int(VectorIntrinsicInParam v0);

    /*! \brief Performs an element-wise 32bit unsigned integer equality comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If the elements are equal under \e 32bit \e unsigned 
     *  comparison, the corresponding element of the result has all its bits
     *  set to 1.  Otherwise, the corresponding elements of the result are set
     *  to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise 32bit unsigned integer greater than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is greater than an element
     *  of v1 under \e 32bit \e unsigned comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise 32bit signed integer equality comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If the elements are equal under \e 32bit \e signed 
     *  comparison, the corresponding element of the result has all its bits
     *  set to 1.  Otherwise, the corresponding elements of the result are set
     *  to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise 32bit unsigned integer less than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is less than an element
     *  of v1 under \e 32bit \e unsigned comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise 32bit signed integer greater than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is greater than an element
     *  of v1 under \e 32bit \e signed comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise 32bit signed integer less than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is less than an element
     *  of v1 under \e 32bit \e signed comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise unsigned short equality comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If the elements are equal under \e unsigned \e short
     *  comparison, the corresponding element of the result has all its bits
     *  set to 1.  Otherwise, the corresponding elements of the result are set
     *  to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise unsigned short greater than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is greater than an element
     *  of v1 under \e unsigned  \e short comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise unsigned short less than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is less than an element
     *  of v1 under \e unsigned  \e short comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise signed short equality comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If the elements are equal under \e signed  \e short 
     *  comparison, the corresponding element of the result has all its bits
     *  set to 1.  Otherwise, the corresponding elements of the result are set
     *  to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise signed short greater than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is greater than an element
     *  of v1 under \e signed  \e short comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise signed short less than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is less than an element
     *  of v1 under \e signed  \e short comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise unsigned char equality comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If the elements are equal under \e unsigned \e char
     *  comparison, the corresponding element of the result has all its bits
     *  set to 1.  Otherwise, the corresponding elements of the result are set
     *  to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise unsigned char greater than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is greater than an element
     *  of v1 under \e unsigned  \e char comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise unsigned char less than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is less than an element
     *  of v1 under \e unsigned  \e char comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise signed char equality comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If the elements are equal under \e signed  \e char 
     *  comparison, the corresponding element of the result has all its bits
     *  set to 1.  Otherwise, the corresponding elements of the result are set
     *  to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompEqual_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise signed char greater than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is greater than an element
     *  of v1 under \e signed  \e char comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompGreaterThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Performs an element-wise signed char less than comparison of two vectors.
     *  
     *  This function compares each element in v0, with its corresponding
     *  element in v1.  If an element of v0 is less than an element
     *  of v1 under \e signed  \e char comparison, the corresponding element of
     *  the result has all its bits set to 1.  Otherwise, the corresponding
     *  elements of the result are set to all zeros.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecCompLessThan_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise maximum of two 32bit unsigned integer vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the maximum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise maximum of two unsigned short vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the maximum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise maximum of two unsigned char vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the maximum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise maximum of two 32bit signed integer vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the maximum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise maximum of two signed short vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the maximum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise maximum of two signed char vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the maximum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMax_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise minimum of two 32bit unsigned integer vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the minimum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise minimum of two unsigned short vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the minimum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise minimum of two unsigned char vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the minimum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise minimum of two 32bit signed integer vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the minimum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise minimum of two signed short vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the minimum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Returns a vector that contains the element-wise minimum of two signed char vectors.
     *
     *  The result vector will do a comparison between corresponding elements
     *  of the two input vectors and return the minimum value in the
     *  corresponding element of the result vector.  
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecMin_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Shifts each unsigned 32bit integer element of the vector left using the corresponding count in \c shiftCount.
     *
     *  Shifts each unsigned integer element of the vector left independently
     *  by the count specified in \c shiftCount.
     *
     *  For portability, the elements of \c shiftCount should be in the range [0, 31].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount);

    /*! \brief Shifts each unsigned 32bit integer element of the vector right using the corresponding count in \c shiftCount.
     *
     *  Shifts each unsigned integer element of the vector right independently
     *  by the count specified in \c shiftCount.  The shift operation performed
     *  shifts in zero bits on the left.
     *
     *  For portability, the elements of \c shiftCount should be in the range [0, 31].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_UInt(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount);

    /*! \brief Shifts each signed 32bit integer element of the vector left using the corresponding count in \c shiftCount.
     *
     *  Shifts each signed integer element of the vector left independently
     *  by the count specified in \c shiftCount.
     *
     *  For portability, the elements of \c shiftCount should be in the range [0, 31].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftLeftUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount);

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_VMX | VPL_SPU | VPL_SSE2)

    /*! \brief Shifts each signed 32bit integer element of the vector right using the corresponding count in \c shiftCount.
     *
     *  Shifts each signed integer element of the vector right independently
     *  by the count specified in \c shiftCount.  The shift operation performed
     *  preserves the sign of the original value.
     *
     *  For portability, the elements of \c shiftCount should be in the range [0, 31].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecShiftRightUsingVector_Int(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftCount);

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_PS3 | VPL_XENON )

	 /*! \brief Shifts an entire 128bit vector left by \e bits using a 'bit count' taken from a vector register.
     *
	 * The number of bits to be shifted must be between 0 and 7 inclusive.
     *
     *  Note: To keep this function portable between platforms, the contents of the low-order three bits of each byte element of \c shiftAmount should equal
     *  the number of bits to shift.  If this precondition is not met the function will fail on some platforms.
     *
     *  VPL_AUTODOC
     */
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftLeftByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount);

    /*! \brief Shifts an entire 128bit vector right by \e bits using a 'bit count' taken from a vector register.
     *
	 * The number of bits to be shifted must be between 0 and 7 inclusive.
     *
     *  Note: To keep this function portable between platforms, the contents of the low-order three bits of each byte element of \c shiftAmount should equal
     *  the number of bits to shift.  If this precondition is not met the function will fail on some platforms.
     *
     *  VPL_AUTODOC
     */
    VPL_API RW_MATH_FORCE_INLINE VectorIntrinsic VecShiftRightByBitsUsingVectorBitCount(VectorIntrinsicInParam v0, VectorIntrinsicInParam shiftAmount);

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_VMX )
    
    /*! \brief Estimate log (base 2) for each floating point element of a vector.
     * 
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecLog2Est(VectorIntrinsicInParam v0);

#endif

#if VPL_SUPPORTED_PLATFORM( VPL_VMX | VPL_SSE2 | VPL_NEON)

    /*! \brief Floors each element of the vector
     * 
     *  The result contains the result of flooring (i.e. round towards negative infinity) each floating point element of the vector.
     * 
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecFloor(VectorIntrinsicInParam v0);

    /*! \brief Add two vectors
     *  
     *  Performs element-wise signed character addition between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors
     *  
     *  Performs element-wise signed character subtraction between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors
     *  
     *  Performs element-wise unsigned character addition between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAdd_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors
     *  
     *  Performs element-wise unsigned character subtraction between the vectors.
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSub_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors (with saturation)
     *  
     *  Performs element-wise unsigned character addition between the vectors and saturates the result.  Saturation clamps the result to the range [-128, 127].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors (with saturation)
     *  
     *  Performs element-wise unsigned character subtraction between the vectors and saturates the result.  Saturation clamps the result to the range [-128, 127].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Char(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors (with saturation)
     *  
     *  Performs element-wise signed character addition between the vectors and saturates the result.  Saturation clamps the result to the range [0, 255].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors (with saturation)
     *  
     *  Performs element-wise signed character subtraction between the vectors and saturates the result.  Saturation clamps the result to the range [0, 255].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UChar(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors (with saturation)
     *  
     *  Performs element-wise unsigned short addition between the vectors and saturates the result.  Saturation clamps the result to the range [-32768, 32767].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors (with saturation)
     *  
     *  Performs element-wise unsigned short subtraction between the vectors and saturates the result.  Saturation clamps the result to the range [-32768, 32767].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_Short(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Add two vectors (with saturation)
     *  
     *  Performs element-wise signed short addition between the vectors and saturates the result.  Saturation clamps the result to the range [0, 65535].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecAddSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

    /*! \brief Subtract two vectors (with saturation)
     *  
     *  Performs element-wise signed short subtraction between the vectors and saturates the result.  Saturation clamps the result to the range [0, 65535].
     *
     *  VPL_AUTODOC
     */
    RW_MATH_FORCE_INLINE VPL_API VectorIntrinsic VecSubSat_UShort(VectorIntrinsicInParam v0, VectorIntrinsicInParam v1);

#endif

} // namespace vpl
} // namespace math
} // namespace rw

#include "rw/math/detail/static_asserts.h"
#include "rw/math/vpl/detail/vec_operation_static_asserts.h"
#include "rw/math/vpl/detail/vec_operation_inline.h"

#endif // RW_MATH_VEC_OPERATION_H

