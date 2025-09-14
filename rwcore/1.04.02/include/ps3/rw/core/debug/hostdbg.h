// (c) Electronic Arts. All Rights Reserved.

/// \file
/// PS3 PPU host debugging macros

#ifndef RW_HOSTDBG_H

#if defined(RWCROSS)

#include "../../../../win/rw/core/debug/hostdbg.h"

#else

#define RW_HOSTDBG_H
#if defined(__SPU__)
#include "rw/core/debug/hostdbg.h"
#else

#ifdef __SNC__
#define rwBREAK() __builtin_snpause()
#else
#define rwBREAK() __asm__ volatile ("tw 31, 1, 1")
#endif

#ifdef EA_DEBUG
#ifdef RWVERBOSE
# define rwVERBOSE(I) do { printf I; fflush(stdout); } while(0)
#endif
#endif

#endif // defined(__SPU__)
#endif // defined(RWCROSS)
#endif /* RW_HOSTDBG_H */
