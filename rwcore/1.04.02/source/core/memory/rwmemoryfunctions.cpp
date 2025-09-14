// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic memory allocation functions.

/*************************************************************************************************************
 Includes
 */
#include "rwsharedheaders.hpp"

#include "rw/core/memory/memoryfunctions.h"
#include "rw/core/memory/memoryinitializers.h"

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

#if defined(EA_DEBUG)
static void *   RwMallocMemStub(size_t size, const RwChar *filename, int32_t line)
{
    rw::host::MemoryFunctionsInitialize();
    return rwMemoryFunctions.rwmalloc(size, filename, line);
}

static void *   RwReallocMemStub(void *mem, size_t newSize, const RwChar *filename, int32_t line)
{
    rw::host::MemoryFunctionsInitialize();
    return rwMemoryFunctions.rwrealloc(mem, newSize, filename, line);
}

static void     RwFreeMemStub(void *mem)
{
    rw::host::MemoryFunctionsInitialize();
    rwMemoryFunctions.rwfree(mem);
}
#else // defined(EA_DEBUG)
static void *   RwMallocMemStub(size_t size)
{
    rw::host::MemoryFunctionsInitialize();
    return rwMemoryFunctions.rwmalloc(size);
}

static void *   RwReallocMemStub(void *mem, size_t newSize)
{
    rw::host::MemoryFunctionsInitialize();
    return rwMemoryFunctions.rwrealloc(mem, newSize);
}

static void     RwFreeMemStub(void *mem)
{
    rw::host::MemoryFunctionsInitialize();
    rwMemoryFunctions.rwfree(mem);
}
#endif // defined(EA_DEBUG)

// In bss so defaults to all NULLs at startup
RwMemoryFunctions rwMemoryFunctions = {RwMallocMemStub, RwReallocMemStub, RwFreeMemStub};

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

void
RwMemSetMalloc(RwMallocFunc mallocFunc)
{
    rwFUNCTION();

    RwMemSetMallocMacro(mallocFunc);

    rwRETURNVOID();
}

void
RwMemSetRealloc(RwReallocFunc reallocFunc)
{
    rwFUNCTION();

    RwMemSetReallocMacro(reallocFunc);

    rwRETURNVOID();
}

void
RwMemSetFree(RwFreeFunc freeFunc)
{
    rwFUNCTION();

    RwMemSetFreeMacro(freeFunc);

    rwRETURNVOID();
}

RwMallocFunc
RwMemGetMalloc(void)
{
    rwFUNCTION();

    rwRETURN(RwMemGetMallocMacro());
}

RwReallocFunc
RwMemGetRealloc(void)
{
    rwFUNCTION();

    rwRETURN(RwMemGetReallocMacro());
}

RwFreeFunc
RwMemGetFree(void)
{
    rwFUNCTION();

    rwRETURN(RwMemGetFreeMacro());
}
