// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic aligned memory allocation functions.

/*************************************************************************************************************
 Includes
 */

#pragma warning(disable: 4548)  // disable warning coming from SDK headers

#if defined(EA_DEBUG)
#include <crtdbg.h>
#endif /* defined(EA_DEBUG) */
#include <malloc.h>

#include "rw/core/base/ostypes.h"
#include "rw/core/debug/debug.h"
#include "rw/core/memoryaligned.h"


#include "rw/core/string.h"
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
 Static Variables
 */

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function Prototypes
 */

/*************************************************************************************************************
 Static Functions
 */

/*************************************************************************************************************
 Internal Functions
 */

/*************************************************************************************************************
 External Functions
 */

#if (defined(EA_DEBUG))

void *
RwWinMallocAlignedGeneric(size_t size, size_t alignment,
                          const RwChar *RWUNUSED(filename), int32_t RWUNUSED(line))
#else /* (defined(EA_DEBUG)) */
void *
RwWinMallocAlignedGeneric(size_t size, size_t alignment)
#endif /* (defined(EA_DEBUG)) */
{
    rwFUNCTION();
    rwRETURN(_aligned_malloc(size, alignment));
}

#if (defined(EA_DEBUG))

void *
RwWinReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment,
                           const RwChar *RWUNUSED(filename), int32_t RWUNUSED(line))
#else /* (defined(EA_DEBUG)) */
void *
RwWinReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment)
#endif /* (defined(EA_DEBUG)) */
{
    rwFUNCTION();

    rwRETURN(_aligned_realloc(mem, newSize, alignment));
}

void
RwWinFreeAlignedGeneric(void *mem)
{
    rwFUNCTION();

    _aligned_free(mem);

    rwRETURNVOID();
}

#if (defined(EA_DEBUG))
void *
RwMallocAlignedMemDebug(size_t size, size_t alignment, const RwChar *filename, int32_t line)
{
    rwFUNCTION();

    rwRETURN(_aligned_malloc_dbg(size, alignment, filename, line));
}

void *
RwReallocAlignedMemDebug(void *mem, size_t newSize,  size_t alignment,
                         const RwChar *filename, int32_t line)
{
    rwFUNCTION();

    rwRETURN(_aligned_realloc_dbg(mem, newSize, alignment, filename, line));
}

void
RwFreeAlignedMemDebug(void *mem)
{
    rwFUNCTION();

    _aligned_free_dbg(mem);

    rwRETURNVOID();
}

#endif /* (defined(EA_DEBUG)) */
