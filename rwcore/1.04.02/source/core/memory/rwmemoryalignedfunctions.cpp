// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic aligned memory allocation functions.

/*************************************************************************************************************
 Includes
 */
/* If you need to include a header for malloc & free then add it into rwdrvmemoryaligned.h */
#include "rwsharedheaders.hpp"
#include "rw/core/memory/memoryalignedfunctions.h"
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
static void *   RwMallocMemAlignedStub(size_t size, size_t alignment, const RwChar *filename, int32_t line)
{
    rw::host::MemoryAlignedFunctionsInitialize();
    return rwMemoryAlignedFunctions.rwMallocAligned(size, alignment, filename, line);
}

static void *   RwReallocMemAlignedStub(void *mem, size_t newSize, size_t alignment, const RwChar *filename, int32_t line)
{
    rw::host::MemoryAlignedFunctionsInitialize();
    return rwMemoryAlignedFunctions.rwReallocAligned(mem, newSize, alignment, filename, line);
}

static void     RwFreeMemAlignedStub(void *mem)
{
    rw::host::MemoryAlignedFunctionsInitialize();
    rwMemoryAlignedFunctions.rwFreeAligned(mem);
}
#else // defined(EA_DEBUG)
static void *   RwMallocMemAlignedStub(size_t size, size_t alignment)
{
    rw::host::MemoryAlignedFunctionsInitialize();
    return rwMemoryAlignedFunctions.rwMallocAligned(size, alignment);
}

static void *   RwReallocMemAlignedStub(void *mem, size_t newSize, size_t alignment)
{
    rw::host::MemoryAlignedFunctionsInitialize();
    return rwMemoryAlignedFunctions.rwReallocAligned(mem, newSize, alignment);
}

static void     RwFreeMemAlignedStub(void *mem)
{
    rw::host::MemoryAlignedFunctionsInitialize();
    rwMemoryAlignedFunctions.rwFreeAligned(mem);
}
#endif // defined(EA_DEBUG)


// In bss so defaults to all NULLs at startup
RwMemoryAlignedFunctions rwMemoryAlignedFunctions = {RwMallocMemAlignedStub, RwReallocMemAlignedStub, RwFreeMemAlignedStub};

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
RwMemSetMallocAligned(RwMallocAlignedFunc mallocAlignedFunc)
{
    rwFUNCTION();

    RwMemSetMallocAlignedMacro(mallocAlignedFunc);

    rwRETURNVOID();
}

void
RwMemSetReallocAligned(RwReallocAlignedFunc reallocAlignedFunc)
{
    rwFUNCTION();

    RwMemSetReallocAlignedMacro(reallocAlignedFunc);

    rwRETURNVOID();
}

void
RwMemSetFreeAligned(RwFreeAlignedFunc freeAlignedFunc)
{
    rwFUNCTION();

    RwMemSetFreeAlignedMacro(freeAlignedFunc);

    rwRETURNVOID();
}

RwMallocAlignedFunc
RwMemGetMallocAligned(void)
{
    rwFUNCTION();

    rwRETURN(RwMemGetMallocAlignedMacro());
}

RwReallocAlignedFunc
RwMemGetReallocAligned(void)
{
    rwFUNCTION();

    rwRETURN(RwMemGetReallocAlignedMacro());
}

RwFreeAlignedFunc
RwMemGetFreeAligned(void)
{
    rwFUNCTION();

    rwRETURN(RwMemGetFreeAlignedMacro());
}
