// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Memory interface.

#ifndef PUBLIC_RW_MEMORY_H
#define PUBLIC_RW_MEMORY_H

/*************************************************************************************************************
 Includes
 */
#include "rw/core/drvmemory.h"   // forward-include host-specific memory functions

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
#ifndef RwMallocGeneric
extern RWAPI void *RwMallocGeneric(size_t size,
                                   const RwChar *filename, int32_t line);
#endif /* RwMallocGeneric */
#ifndef RwReallocGeneric
extern RWAPI void *RwReallocGeneric(void *mem, size_t newSize,
                                    const RwChar *filename, int32_t line);
#endif /* RwReallocGeneric */
#else /* (defined(EA_DEBUG)) */
#ifndef RwMallocGeneric
extern RWAPI void *RwMallocGeneric(size_t size);
#endif /* RwMallocGeneric */
#ifndef RwReallocGeneric
extern RWAPI void *RwReallocGeneric(void *mem, size_t newSize);
#endif /* RwReallocGeneric */
#endif /* (defined(EA_DEBUG)) */

#ifndef RwFreeGeneric
extern RWAPI void  RwFreeGeneric(void *mem);
#endif /* RwFreeGeneric */

#endif // PUBLIC_RW_MEMORY_H
