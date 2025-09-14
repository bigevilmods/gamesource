// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Capilano definition of RWTHREADSAFECORE

#ifdef RWTHREADSAFECORE
    #if (RWTHREADSAFECORE == 0)
    #error 'RWTHREADSAFECORE = 0' is not compatible with the compiled libraries.
    #endif
#endif
#undef RWTHREADSAFECORE
#define RWTHREADSAFECORE 1
