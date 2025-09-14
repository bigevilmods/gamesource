// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Xbox2-specific aligned memory allocation.

#ifndef PUBLIC_RW_DRVMEMORYALIGNED_H

#if defined(RWCROSS)

#include "../../../win/rw/core/drvmemoryaligned.h"

#else

#define PUBLIC_RW_DRVMEMORYALIGNED_H

/*************************************************************************************************************
 Defines
 */

#if !defined(EA_DEBUG)

#define RwMallocAlignedGeneric  _aligned_malloc
#define RwReallocAlignedGeneric _aligned_realloc
#define RwFreeAlignedGeneric    _aligned_free

#else

#define RwMallocAlignedGeneric  RwWinMallocAlignedGeneric
#define RwReallocAlignedGeneric RwWinReallocAlignedGeneric
#define RwFreeAlignedGeneric    RwWinFreeAlignedGeneric

#endif /* (!defined(RWMEMDEBUG) && !defined(EA_DEBUG)) */

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
extern void *RwWinMallocAlignedGeneric(size_t size, size_t alignment,
                                       const RwChar *filename, int32_t line);
extern void *RwWinReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment,
                                        const RwChar *filename, int32_t line);
#else /* (defined(EA_DEBUG)) */
extern void *RwWinMallocAlignedGeneric(size_t size, size_t alignment);
extern void *RwWinReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment);
#endif /* (defined(EA_DEBUG)) */
extern void  RwWinFreeAlignedGeneric(void *mem);

#if (defined(EA_DEBUG))
extern void *RwMallocAlignedMemDebug(size_t size, size_t alignment,
                                     const RwChar *filename, int32_t line);
extern void *RwReallocAlignedMemDebug(void *mem, size_t newSize, size_t alignment,
                                      const RwChar *filename, int32_t line);
extern void  RwFreeAlignedMemDebug(void *mem);
#endif /* (defined(EA_DEBUG)) */

#endif // defined(RWCROSS)
#endif // PUBLIC_RW_DRVMEMORYALIGNED_H
