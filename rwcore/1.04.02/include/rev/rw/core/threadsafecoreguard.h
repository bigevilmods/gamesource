// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Wii definition of RWTHREADSAFECORE

#if defined(RWCROSS)

#include "../../../win/rw/core/threadsafecoreguard.h"

#else

#ifdef RWTHREADSAFECORE
    #if !(RWTHREADSAFECORE == 0)
    #error 'RWTHREADSAFECORE = 1' is not compatible with the compiled libraries.
    #endif
#endif
#undef RWTHREADSAFECORE
#define RWTHREADSAFECORE 0

#endif
