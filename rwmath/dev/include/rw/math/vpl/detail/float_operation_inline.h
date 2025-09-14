// (c) Electronic Arts. All Rights Reserved.

#if defined _MSC_VER
    #pragma once
#endif

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION || defined RWCROSS || defined EA_PLATFORM_POSIX || defined EA_PROCESSOR_ARM || defined EA_PLATFORM_APPLE || defined EA_PLATFORM_AIRPLAY || defined RW_MATH_TARGET_SUPPORTS_SSE
    #include "rw/math/vpl/detail/cross/float_operation_inline.h"
#elif defined EA_PLATFORM_REVOLUTION
    #include "rw/math/vpl/detail/wii/float_operation_inline.h"
#elif defined EA_PLATFORM_PS3
    #include "rw/math/vpl/detail/ps3/ppu/float_operation_inline.h"
#elif defined EA_PLATFORM_PS3_SPU
    #include "rw/math/vpl/detail/ps3/spu/float_operation_inline.h"
#elif defined EA_PLATFORM_WINDOWS
    #include "rw/math/vpl/detail/cross/float_operation_inline.h"
#elif defined EA_PLATFORM_XENON
    #include "rw/math/vpl/detail/xbox2/float_operation_inline.h"
#else
    #error Unsupported platform
#endif

