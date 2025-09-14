// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Xbox2-specific Default memory allocation.

#ifndef PUBLIC_RW_DRVMEMORY_H

#if defined(RWCROSS)

#include "../../../win/rw/core/drvmemory.h"

#else

#define PUBLIC_RW_DRVMEMORY_H

#include <stdlib.h>

#include <malloc.h>

/*************************************************************************************************************
 Defines
 */

#if !defined(EA_DEBUG)
#define RwMallocGeneric  malloc
#define RwReallocGeneric realloc
#define RwFreeGeneric    free
#endif

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Internal Functions Prototypes
 */

/*************************************************************************************************************
 External Functions Prototypes
 */
#if (defined(EA_DEBUG))
extern void *   RwMallocMemDebug(size_t size, const RwChar *filename, int32_t line);
extern void *   RwReallocMemDebug(void *mem, size_t newSize, const RwChar *filename, int32_t line);
extern void     RwFreeMemDebug(void *mem);
#endif /* (defined(EA_DEBUG)) */

#endif // defined(RWCROSS)
#endif // PUBLIC_RW_DRVMEMORY_H
