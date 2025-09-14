// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"

#if defined _MSC_VER
    #pragma once
#endif

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
    #include "rw/math/vpu/detail/cross/scalar_operation_platform_inline.h"
#elif defined RWCROSS 
    #include "rw/math/vpu/detail/cross/scalar_operation_platform_inline.h"
#elif defined EA_PLATFORM_PSP2 
    #include "rw/math/vpu/detail/psp2/scalar_operation_platform_inline.h"
#elif defined EA_PROCESSOR_ARM 
    #include "rw/math/vpu/detail/cross/scalar_operation_platform_inline.h"
#elif defined RW_MATH_TARGET_SUPPORTS_SSE
    #include "rw/math/vpu/detail/x86/scalar_operation_platform_inline.h"
#elif defined EA_PLATFORM_POSIX
    #include "rw/math/vpu/detail/cross/scalar_operation_platform_inline.h"
#elif defined EA_PLATFORM_AIRPLAY
    #include "rw/math/vpu/detail/cross/scalar_operation_platform_inline.h"
#elif defined EA_PLATFORM_REVOLUTION
    #include "rw/math/vpu/detail/cross/scalar_operation_platform_inline.h"
#elif defined EA_PLATFORM_PS3
    #include "rw/math/vpu/detail/ps3/ppu/scalar_operation_platform_inline.h"
#elif defined EA_PLATFORM_PS3_SPU
    #include "rw/math/vpu/detail/ps3/spu/scalar_operation_platform_inline.h"
#elif defined EA_PLATFORM_XENON
    #include "rw/math/vpu/detail/xbox2/scalar_operation_platform_inline.h"
#else
    #error Unsupported platform
#endif


