// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic aligned memory allocation functions.

/*************************************************************************************************************
 Includes
 */
/* If you need to include a header for malloc & free then add it into rwdrvmemoryaligned.h */
#include "rw/core/base/ostypes.h"
#include "rw/core/string.h"
#include "rw/core/debug/debug.h"

#include "rw/core/memoryaligned.h"

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

#ifndef RwMallocAlignedGeneric

#if (defined(EA_DEBUG))

void *
RwMallocAlignedGeneric(size_t size, size_t alignment,
                       const RwChar *RWUNUSED(filename), int32_t RWUNUSED(line))
#else /* (defined(EA_DEBUG)) */

void *
RwMallocAlignedGeneric(size_t size, size_t alignment)
#endif /* (defined(EA_DEBUG)) */
{
    void * alignedMem;
    void * mem;
    rwFUNCTION();

    if (0 == alignment)
    {
        alignment = 1;
    }

    /* Over allocate to allow for storage of base address */
    mem = malloc(size + sizeof(void *) + sizeof(size_t) + (alignment - 1));
    if (!mem)
    {
        rwRETURN((void *)0);
    }

    alignedMem = RwMemAlign((uint8_t *)mem + sizeof(void *) + sizeof(size_t), alignment);

    /* Store base address */
    *((void **)alignedMem - 1) = mem;

    /* Store size */
    *((size_t *)((void **)alignedMem - 1) - 1) = size;
    rwRETURN(alignedMem);
}

#endif /* RwMallocAlignedGeneric */


#ifndef RwReallocAlignedGeneric

#if (defined(EA_DEBUG))
void *
RwReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment,
                        const RwChar *RWUNUSED(filename), int32_t RWUNUSED(line))
#else /* (defined(EA_DEBUG)) */
void *
RwReallocAlignedGeneric(void *mem, size_t newSize, size_t alignment)
#endif /* (defined(EA_DEBUG)) */
{
    void * newMem;

    rwFUNCTION();

    if (mem)
    {
        if (0 == newSize)
        {
            RwFreeAlignedGeneric(mem);
            rwRETURN((void *)0);
        }
        else
        {
#if defined(EA_DEBUG)
            newMem = RwMallocAlignedGeneric(newSize, alignment, NULL, 0);
#else // defined(EA_DEBUG)
            newMem = RwMallocAlignedGeneric(newSize, alignment);
#endif // defined(EA_DEBUG)
            if (newMem)
            {
                size_t oldSize;

                oldSize = *((size_t *)((void **)mem - 1) - 1);

                if (newSize > oldSize)
                {
                    RwMemcpy(newMem, mem, oldSize);
                }
                else
                {
                    RwMemcpy(newMem, mem, newSize);
                }

                RwFreeAlignedGeneric(mem);
                rwRETURN(newMem);
            }
            else
            {
                rwRETURN((void *)0);
            }
        }
    }
    else
    {
#if defined(EA_DEBUG)
        newMem = RwMallocAlignedGeneric(newSize, alignment, NULL, 0);
#else // defined(EA_DEBUG)
        newMem = RwMallocAlignedGeneric(newSize, alignment);
#endif // defined(EA_DEBUG)
        if (newMem)
        {
            rwRETURN(newMem);
        }
        else
        {
            rwRETURN((void *)0);
        }
    }
}

#endif /* RwReallocAlignedGeneric */

#ifndef RwFreeAlignedGeneric

void
RwFreeAlignedGeneric(void *mem)
{
    rwFUNCTION();

    if (mem)
    {
        void * base;

        base = *((void **)mem - 1);

        free(base);
    }

    rwRETURNVOID();
}
#endif /* RwFreeAlignedGeneric */
