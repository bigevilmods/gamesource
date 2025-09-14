// (c) Electronic Arts. All Rights Reserved.

#include "rw/math/detail/base.h"

#if defined _MSC_VER
    #pragma once
#endif

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
    #include "rw/math/vpu/detail/cross/vector_intrinsic_operation_macros.h"
#elif defined RWCROSS 
    #include "rw/math/vpu/detail/cross/vector_intrinsic_operation_macros.h"
#elif defined RWMATH_USE_NEON
    #include "rw/math/vpu/detail/neon/vector_intrinsic_operation_macros.h"
#elif defined EA_PROCESSOR_ARM 
    #include "rw/math/vpu/detail/cross/vector_intrinsic_operation_macros.h"
#elif defined EA_PLATFORM_XENON
    #include "rw/math/vpu/detail/xbox2/vector_intrinsic_operation_macros.h"
#elif defined RW_MATH_TARGET_SUPPORTS_SSE
    #include "rw/math/vpu/detail/x86/vector_intrinsic_operation_macros.h"
#elif defined EA_PLATFORM_POSIX
    #include "rw/math/vpu/detail/cross/vector_intrinsic_operation_macros.h"
#elif defined EA_PLATFORM_REVOLUTION
    #include "rw/math/vpu/detail/wii/vector_intrinsic_operation_macros.h"
#elif defined EA_PLATFORM_PS3
    #include "rw/math/vpu/detail/ps3/ppu/vector_intrinsic_operation_macros.h"
#elif defined EA_PLATFORM_PS3_SPU
    #include "rw/math/vpu/detail/ps3/spu/vector_intrinsic_operation_macros.h"
#elif defined EA_PLATFORM_AIRPLAY
    #include "rw/math/vpu/detail/cross/vector_intrinsic_operation_macros.h"
#else
    #error undefined platform
#endif
