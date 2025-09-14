// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_H
#define RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#if defined RW_MATH_USE_REFERENCE_IMPLEMENTATION
    #include "rw/math/fpu/detail/cross/float_intrinsic_operation.h"
#elif defined RWCROSS 
	#include "rw/math/fpu/detail/cross/float_intrinsic_operation.h"
#elif defined EA_PROCESSOR_ARM
    #include "rw/math/fpu/detail/cross/float_intrinsic_operation.h"
#elif defined EA_PLATFORM_PS3
    #include "rw/math/fpu/detail/ps3/ppu/float_intrinsic_operation.h"
#elif defined EA_PLATFORM_PS3_SPU
    #include "rw/math/fpu/detail/ps3/spu/float_intrinsic_operation.h"
#elif defined EA_PLATFORM_XENON
    #include "rw/math/fpu/detail/xbox2/float_intrinsic_operation.h"
#elif defined EA_PLATFORM_WINDOWS || defined RW_MATH_TARGET_SUPPORTS_SSE
    #include "rw/math/fpu/detail/cross/float_intrinsic_operation.h"
#elif defined EA_PLATFORM_POSIX
    #include "rw/math/fpu/detail/cross/float_intrinsic_operation.h"
#elif defined EA_PLATFORM_APPLE
    #include "rw/math/fpu/detail/cross/float_intrinsic_operation.h" 
#elif defined EA_PLATFORM_REVOLUTION
    #include "rw/math/fpu/detail/wii/float_intrinsic_operation.h"
#elif defined EA_PLATFORM_AIRPLAY
    #include "rw/math/fpu/detail/cross/float_intrinsic_operation.h"
#else
    #error Unsupported platform
#endif


#endif // RW_MATH_FPU_FLOAT_INTRINSIC_OPERATION_H
