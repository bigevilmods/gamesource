// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Capilano-specific Default memory allocation.

#ifndef PUBLIC_RW_DRVMEMORY_H
#define PUBLIC_RW_DRVMEMORY_H

#include <stdlib.h>
#include <malloc.h>

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Internal Functions Prototypes
 */

/*************************************************************************************************************
 External Functions Prototypes
 */
#if defined(EA_DEBUG)
extern RWAPI void *   RwMallocMemDebug(size_t size, const RwChar *filename, int32_t line);
extern RWAPI void *   RwReallocMemDebug(void *mem, size_t newSize, const RwChar *filename, int32_t line);
extern RWAPI void     RwFreeMemDebug(void *mem);
#endif /* (defined(EA_DEBUG)) */

#endif // PUBLIC_RW_DRVMEMORY_H
