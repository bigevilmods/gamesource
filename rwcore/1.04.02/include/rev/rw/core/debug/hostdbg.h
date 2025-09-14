// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Revolution host debugging macros

#ifndef RW_HOSTDBG_H

#if defined(RWCROSS)

#include "../../../../win/rw/core/debug/hostdbg.h"

#else

#define RW_HOSTDBG_H

#define rwBREAK() OSHalt("Software Break")

#endif // defined(RWCROSS)
#endif /* !RW_HOSTDBG_H */
