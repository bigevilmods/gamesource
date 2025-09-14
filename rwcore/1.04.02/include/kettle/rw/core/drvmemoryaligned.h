// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Capilano-specific aligned memory allocation.

#ifndef PUBLIC_RW_DRVMEMORYALIGNED_H
#define PUBLIC_RW_DRVMEMORYALIGNED_H

#include <stdlib.h>
/*************************************************************************************************************
 Defines
 */

#define RwMallocAlignedMemDebug  RwMallocAlignedGeneric
#define RwReallocAlignedMemDebug RwReallocAlignedGeneric
#define RwFreeAlignedMemDebug    RwFreeAlignedGeneric

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
extern RWAPI void *RwWinMallocAlignedGeneric(size_t size, size_t alignment,
                                             const RwChar *filename, int32_t line);
extern RWAPI void *RwWinReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment,
                                              const RwChar *filename, int32_t line);
#else /* (defined(EA_DEBUG)) */
extern RWAPI void *RwWinMallocAlignedGeneric(size_t size, size_t alignment);
extern RWAPI void *RwWinReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment);
#endif /* (defined(EA_DEBUG)) */
extern RWAPI void  RwWinFreeAlignedGeneric(void *mem);


#define RwMallocAlignedMemDebug  RwMallocAlignedGeneric
#define RwReallocAlignedMemDebug RwReallocAlignedGeneric
#define RwFreeAlignedMemDebug    RwFreeAlignedGeneric


#endif // PUBLIC_RW_DRVMEMORYALIGNED_H
