// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic aligned memory allocation functions

#ifndef PUBLIC_RW_MEMORYALIGNED_H
#define PUBLIC_RW_MEMORYALIGNED_H

/*************************************************************************************************************
 Includes
 */
#include "rw/core/drvmemoryaligned.h"   // forward-include host-specific memory functions

/*************************************************************************************************************
 Defines
 */

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

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
#ifndef RwMallocAlignedGeneric
extern RWAPI void *RwMallocAlignedGeneric(size_t size, size_t alignment,
                                          const RwChar *filename, int32_t line);
#endif /* RwMallocAlignedGeneric */
#ifndef RwReallocAlignedGeneric
extern RWAPI void *RwReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment,
                                           const RwChar *filename, int32_t line);
#endif /* RwReallocAlignedGeneric */
#else /* (defined(EA_DEBUG)) */
#ifndef RwMallocAlignedGeneric
extern RWAPI void *RwMallocAlignedGeneric(size_t size, size_t alignment);
#endif /* RwMallocAlignedGeneric */
#ifndef RwReallocAlignedGeneric
extern RWAPI void *RwReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment);
#endif /* RwReallocAlignedGeneric */
#endif /* (defined(EA_DEBUG)) */

#ifndef RwFreeAlignedGeneric
extern RWAPI void  RwFreeAlignedGeneric(void *mem);
#endif /* RwFreeAlignedGeneric */

#endif // PUBLIC_RW_MEMORYALIGNED_H
