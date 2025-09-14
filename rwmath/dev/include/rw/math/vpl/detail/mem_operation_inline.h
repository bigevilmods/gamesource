// (c) Electronic Arts. All Rights Reserved.

#if defined _MSC_VER
    #pragma once
#endif

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION || defined RWCROSS || defined EA_PROCESSOR_ARM
    #include "rw/math/vpl/detail/cross/mem_operation_inline.h"
#elif defined EA_PLATFORM_REVOLUTION
    #include "rw/math/vpl/detail/wii/mem_operation_inline.h"
#elif defined EA_PLATFORM_PS3
    #include "rw/math/vpl/detail/ps3/ppu/mem_operation_inline.h"
#elif defined EA_PLATFORM_PS3_SPU
    #include "rw/math/vpl/detail/cross/mem_operation_inline.h"
#elif defined EA_PLATFORM_XENON
    #include "rw/math/vpl/detail/xbox2/mem_operation_inline.h"
#elif defined RW_MATH_TARGET_SUPPORTS_SSE
    #include "rw/math/vpl/detail/x86/mem_operation_inline.h"
#elif defined EA_PLATFORM_WINDOWS
    #include "rw/math/vpl/detail/cross/mem_operation_inline.h"
#elif defined EA_PLATFORM_POSIX
    #include "rw/math/vpl/detail/cross/mem_operation_inline.h"
#elif defined EA_PLATFORM_APPLE
    #include "rw/math/vpl/detail/cross/mem_operation_inline.h"
#elif defined EA_PLATFORM_AIRPLAY
    #include "rw/math/vpl/detail/cross/mem_operation_inline.h"
#else
    #error Unsupported platform
#endif

