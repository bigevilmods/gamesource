// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic memory allocation functions.

/*************************************************************************************************************
 Includes
 */
/* If you need to include a header for malloc & free then add it into rwdrvmemory.h */
#include "rw/core/base/ostypes.h"
#include "rw/core/string.h"
#include "rw/core/debug/debug.h"

#include "rw/core/memory.h"

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

#ifndef RwMallocGeneric

#if (defined(EA_DEBUG))

void *
RwMallocGeneric(size_t size,
                const RwChar *RWUNUSED(filename), int32_t RWUNUSED(line))
#else /* (defined(EA_DEBUG)) */
void *
RwMallocGeneric(size_t size)
#endif /* (defined(EA_DEBUG)) */
{
    void * mem;
    rwFUNCTION();

    /* Over allocate to allow for storage of base address */
    mem = malloc(size);

    if (!mem)
    {
        rwRETURN((void *)0);
    }

    rwRETURN(mem);
}

#endif /* RwMallocGeneric */

#ifndef RwReallocGeneric

#if (defined(EA_DEBUG))

void *
RwReallocGeneric(void *mem, size_t newSize,
                 const RwChar *RWUNUSED(filename), int32_t RWUNUSED(line))
#else /* (defined(EA_DEBUG)) */
void *
RwReallocGeneric(void *mem, size_t newSize)
#endif /* (defined(EA_DEBUG)) */
{
    void * newMem;

    rwFUNCTION();

    newMem = realloc(mem, newSize);

    rwRETURN(newMem);
}

#endif /* RwReallocGeneric */

#ifndef RwFreeGeneric

void
RwFreeGeneric(void *mem)
{
    rwFUNCTION();

    free(mem);

    rwRETURNVOID();
}
#endif /* RwFreeGeneric */
