// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Windows-specific aligned memory allocation.

#ifndef PUBLIC_RW_DRVMEMORYALIGNED_H
#define PUBLIC_RW_DRVMEMORYALIGNED_H

#include <stdlib.h>

/*************************************************************************************************************
 Defines
 */

#define RwMallocAlignedGeneric  RwWinMallocAlignedGeneric
#define RwReallocAlignedGeneric RwWinReallocAlignedGeneric
#define RwFreeAlignedGeneric    RwWinFreeAlignedGeneric

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

#if (defined(EA_DEBUG))
extern RWAPI void *RwMallocAlignedMemDebug(size_t size, size_t alignment,
                                           const RwChar *filename, int32_t line);
extern RWAPI void *RwReallocAlignedMemDebug(void *mem, size_t newSize, size_t alignment,
                                            const RwChar *filename, int32_t line);
extern RWAPI void  RwFreeAlignedMemDebug(void *mem);
#endif /* (defined(EA_DEBUG)) */

#endif // PUBLIC_RW_DRVMEMORYALIGNED_H
