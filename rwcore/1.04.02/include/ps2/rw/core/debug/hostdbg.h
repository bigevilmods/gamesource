// (c) Electronic Arts. All Rights Reserved.

/// \file
/// PS2 host debugging macros

#ifndef RW_HOSTDBG_H

#if defined(RWCROSS)

#include "../../../../win/rw/core/debug/hostdbg.h"

#else

#define RW_HOSTDBG_H


#ifdef __MWERKS__
#define rwBREAK() __asm__ volatile ("breakc 0")
#else
#define rwBREAK() __asm__ volatile ("break 0")
#endif

#endif // defined(RWCROSS)
#endif /* !RW_HOSTDBG_H */
