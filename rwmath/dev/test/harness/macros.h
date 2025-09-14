// (c) Electronic Arts. All Rights Reserved.

#if defined(EA_COMPILER_CLANG) && defined(EA_PROCESSOR_X86_64)
    // These macros are different than the generic ones to workaround an LLVM codegen issue.
    // This issue should be resolved in future releases of LLVM.

    #define MakeVectorIntrinsic( name, valueX, valueY, valueZ, valueW ) \
        ::rw::math::vpu::VectorIntrinsic name = _mm_set_ps(valueW, valueZ, valueY, valueX);

    #define MakeVectorIntrinsicUnsignedInt( name, valueX, valueY, valueZ, valueW ) \
        ::rw::math::vpu::VectorIntrinsic name = _mm_castsi128_ps(_mm_set_epi32(valueW, valueZ, valueY, valueX));

#else
    #define MakeVectorIntrinsic( name, valueX, valueY, valueZ, valueW ) \
        ::rw::math::vpu::VectorIntrinsic name; \
        ::rw::math::vpu::detail::VectorIntrinsicUnion name ## _union; \
        name ## _union.scalarFloat[0] = valueX; \
        name ## _union.scalarFloat[1] = valueY; \
        name ## _union.scalarFloat[2] = valueZ; \
        name ## _union.scalarFloat[3] = valueW; \
        name = name ## _union.vector;

    #define MakeVectorIntrinsicUnsignedInt( name, valueX, valueY, valueZ, valueW ) \
        ::rw::math::vpu::VectorIntrinsic name; \
        ::rw::math::vpu::detail::VectorIntrinsicUnion name ## _union; \
        name ## _union.scalarUInt[0] = valueX; \
        name ## _union.scalarUInt[1] = valueY; \
        name ## _union.scalarUInt[2] = valueZ; \
        name ## _union.scalarUInt[3] = valueW; \
        name = name ## _union.vector;
#endif

// This macro creates a vector intrinsic for that contains 'value' in the 'axis' element of the vector (0 elsewhere).
// Then it creates a VecFloatRef named 'name' for the specified 'axis'
#define MakeVecRef( name, axis, value ) \
    ::rw::math::vpu::detail::VectorIntrinsicUnion name ## _union; \
    name ## _union.scalarFloat[0] = ((0 == VectorAxis ## axis) ? value : 0.0f); \
    name ## _union.scalarFloat[1] = ((1 == VectorAxis ## axis) ? value : 0.0f); \
    name ## _union.scalarFloat[2] = ((2 == VectorAxis ## axis) ? value : 0.0f); \
    name ## _union.scalarFloat[3] = ((3 == VectorAxis ## axis) ? value : 0.0f); \
    ::rw::math::vpu::VecFloatRef ## axis name( name ## _union.vector );

// This macro creates a vector intrinsic for that contains 'value' in the 'axis' element of the vector (0 elsewhere).
// Then it creates a VecFloatRef named 'name' for the specified 'axis'
#define MakeVecRefIndex( name, axis, value ) \
    ::rw::math::vpu::detail::VectorIntrinsicUnion name ## _union; \
    name ## _union.scalarFloat[0] = ((0 == axis) ? value : 0.0f); \
    name ## _union.scalarFloat[1] = ((1 == axis) ? value : 0.0f); \
    name ## _union.scalarFloat[2] = ((2 == axis) ? value : 0.0f); \
    name ## _union.scalarFloat[3] = ((3 == axis) ? value : 0.0f); \
    ::rw::math::vpu::VecFloatRefIndex name( name ## _union.vector, axis );

